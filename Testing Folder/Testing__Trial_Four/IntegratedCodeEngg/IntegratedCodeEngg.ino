#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include "state.h"

// ---------------- Pins (avoid GPIO6 on ESP32)

#define RED_WARNING_LED 25
#define ALTITUDE_LIGHTS 5
#define MOTOR_IN1       15
#define MOTOR_IN2        2
#define LDR_PIN         27
#define NIGHT_LIGHTS    17
const int trigPin1 = 22;
const int echoPin1 = 23;
const int trigPin2 = 33;
const int echoPin2 = 32;
const int trigPin3 = 33;
const int echoPin3 = 19;
const int trigPin4 = 18;
const int echoPin4 = 26;

constexpr float OPEN_ON_CM  = 19.0f;  // start opening when <= 19 cm
constexpr float OPEN_OFF_CM = 21.0f;  // stop opening when >= 21 cm (hysteresis)
constexpr uint32_t FRESH_TIMEOUT_MS = 600; // stop if no valid ping for 0.6s

#define SOUND_SPEED 0.034
unsigned long timeout = 30000; // 30ms max wait
unsigned long interval = 100;  // measurement interval in ms

static bool us_seenHigh[4] = {false,false,false,false};


// Distance variables
float distanceCm1, distanceCm2, distanceCm3, distanceCm4;

// ---------------- App state
SystemMode g_mode = MODE_AUTO;             // manual is default
volatile BridgeCmd  g_cmd_manual = CMD_IDLE; // set by web UI
volatile BridgeCmd  g_cmd_auto   = CMD_IDLE; // set by autoController()
volatile bool      g_emergency  = false;   // <-- NEW
//Global MarineStatus Definition(declared in state.h)
volatile MarineStatus g_marine_status = MARINE_CLEAR; //Initial UI status 


unsigned long previousMillis = 0;
const unsigned long BLINK_INTERVAL_MS = 500;
int ledState = LOW;

float g_distance_cm = -1.0f;   // single source of truth (e.g., front sensor)

enum US_State { US_IDLE, US_TRIGGER, US_WAIT_ECHO };
static US_State us_state = US_IDLE;
static int us_index = 0;
static uint32_t us_timer = 0;
static unsigned long us_pulseStart[4] = {0,0,0,0};

const int trigPins[4] = {trigPin1, trigPin2, trigPin3, trigPin4};
const int echoPins[4] = {echoPin1, echoPin2, echoPin3, echoPin4};

// Forward decls (your existing code)
void setupWiFi();
void setupWebServer();
void setupMotorFunction();
void motorFunctionLoop();
void setupNightLights();
void nightLightsLoop();
void setupWarningLights();
void warningLightsLoop ();
void setupAltitudeLight();
void altitudeLightLoop();
void handleWebServerClients();

static inline float us_to_cm(unsigned long us) {
  return (us == 0UL) ? NAN : (us * SOUND_SPEED) / 2.0f;
}

void initUltrasonic() {
  for (int i = 0; i < 4; ++i) {
    pinMode(trigPins[i], OUTPUT);
    digitalWrite(trigPins[i], LOW);
    pinMode(echoPins[i], INPUT_PULLDOWN);  // <-- important for stable idle
  }
  distanceCm1 = distanceCm2 = distanceCm3 = distanceCm4 = NAN;
}

void updateUltrasonic() {
  switch (us_state) {
    case US_IDLE:
      us_index = 0;
      us_state = US_TRIGGER;
      break;

    case US_TRIGGER:
      // reset edge-tracking for this channel
      us_pulseStart[us_index] = 0;
      us_seenHigh[us_index]   = false;

      digitalWrite(trigPins[us_index], LOW);  delayMicroseconds(2);
      digitalWrite(trigPins[us_index], HIGH); delayMicroseconds(10);
      digitalWrite(trigPins[us_index], LOW);

      us_timer = micros();
      us_state = US_WAIT_ECHO;
      break;

    case US_WAIT_ECHO: {
      int e = echoPins[us_index];

      // latch the FIRST rising edge only
      if (!us_seenHigh[us_index] && digitalRead(e) == HIGH) {
        us_pulseStart[us_index] = micros();
        us_seenHigh[us_index]   = true;
      }

      // after we’ve seen HIGH, wait for falling edge
      if (us_seenHigh[us_index] && digitalRead(e) == LOW) {
        unsigned long pulse = micros() - us_pulseStart[us_index];
        float cm = us_to_cm(pulse);

        if      (us_index == 0) distanceCm1 = cm;
        else if (us_index == 1) distanceCm2 = cm;
        else if (us_index == 2) distanceCm3 = cm;
        else                    distanceCm4 = cm;

        us_index++;
        us_state = (us_index >= 4) ? US_IDLE : US_TRIGGER;
        return;  // avoid also taking timeout path this cycle
      }
      // Timeout safety: no echo or too long
      else if (micros() - us_timer > timeout) {
        if      (us_index == 0) distanceCm1 = NAN;
        else if (us_index == 1) distanceCm2 = NAN;
        else if (us_index == 2) distanceCm3 = NAN;
        else                    distanceCm4 = NAN;

        us_index++;
        us_state = (us_index >= 4) ? US_IDLE : US_TRIGGER;
        return;
      }
    } break; // end US_WAIT_ECHO
  } // end switch
} // end updateUltrasonic()


// ===== AUTO controller: computes g_cmd_auto only (never touches manual) =====
static void autoController(float distance_cm) {
  static BridgeCmd last = CMD_STOP;
  static uint32_t lastValidMs = 0;

  // track freshness
  if (!isnan(distance_cm) && distance_cm > 0.0f) {
    lastValidMs = millis();
  }
  if (millis() - lastValidMs > FRESH_TIMEOUT_MS) {
    g_cmd_auto = CMD_STOP;
    last = g_cmd_auto;
    return;
  }

  BridgeCmd next = last;
  if (distance_cm <= OPEN_ON_CM)       next = CMD_OPEN;
  else if (distance_cm >= OPEN_OFF_CM) next = CMD_STOP;   // stop when not within ~20 cm
  else                                  next = CMD_STOP;  // mid-band: be safe

  g_cmd_auto = next;
  last = next;
}


//Marine Status Controller 
static const float MS_PASSING_NEAR_CM = 40.0f;     // 40cm PASSING
static const float MS_DETECT_FAR_CM   = 80.0f;    // 80cm DETECTED
static const float MS_HYST            = 5.0f;      // hysteresis margin (cm)
static const unsigned long MS_DEPARTED_HOLD_MS = 3000; // show DEPARTED for 3s

static unsigned long s_departed_since_ms = 0;

void marineController(float nearest_distance_cm) {
  MarineStatus curr = g_marine_status;

  switch (curr) {
    case MARINE_CLEAR:
      if (!isnan(nearest_distance_cm) && nearest_distance_cm > 0) {
        if (nearest_distance_cm <= MS_PASSING_NEAR_CM) {
          g_marine_status = MARINE_PASSING;
        } else if (nearest_distance_cm <= MS_DETECT_FAR_CM) {
          g_marine_status = MARINE_DETECTED;
        }
      }
      break;

    case MARINE_DETECTED:
      if (nearest_distance_cm <= MS_PASSING_NEAR_CM - MS_HYST) {
        g_marine_status = MARINE_PASSING;
      } else if (isnan(nearest_distance_cm) || nearest_distance_cm > MS_DETECT_FAR_CM + MS_HYST) {
        g_marine_status = MARINE_DEPARTED;
        s_departed_since_ms = millis();
      }
      break;

    case MARINE_PASSING:
      if (isnan(nearest_distance_cm) || nearest_distance_cm > MS_PASSING_NEAR_CM + MS_HYST) {
        if (!isnan(nearest_distance_cm) && nearest_distance_cm <= MS_DETECT_FAR_CM - MS_HYST) {
          g_marine_status = MARINE_DETECTED;
        } else {
          g_marine_status = MARINE_DEPARTED;
          s_departed_since_ms = millis();
        }
      }
      break;

    case MARINE_DEPARTED:
      if (!isnan(nearest_distance_cm) && nearest_distance_cm <= MS_DETECT_FAR_CM - MS_HYST) {
        g_marine_status = (nearest_distance_cm <= MS_PASSING_NEAR_CM)
                          ? MARINE_PASSING
                          : MARINE_DETECTED;
      } else if (millis() - s_departed_since_ms >= MS_DEPARTED_HOLD_MS) {
        g_marine_status = MARINE_CLEAR;
      }
      break;
  }
}






// --- Ultrasonic helpers


// Trigger and read all four sensors once; returns false if all time out


void blinkEmergencyLeds() {
  static unsigned long lastBlink = 0;
  static bool state = false;

  if (millis() - lastBlink >= 500) { // blink interval
    lastBlink = millis();
    state = !state;
    digitalWrite(NIGHT_LIGHTS, state ? HIGH : LOW);
    digitalWrite(RED_WARNING_LED, state ? HIGH : LOW);
    digitalWrite(ALTITUDE_LIGHTS, state ? HIGH : LOW);
  }
}


//helper function for debugging
static const char* marineStatusToString(MarineStatus s) {
  switch (s) {
      case MARINE_CLEAR:    return "CLEAR";
      case MARINE_DETECTED: return "DETECTED";
      case MARINE_PASSING:  return "PASSING";
      case MARINE_DEPARTED: return "DEPARTED";
      default:              return "?";
  }
}

void setup() {
  Serial.begin(115200);


  pinMode(RED_WARNING_LED, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  
  setupWiFi();
  setupWebServer();
  setupMotorFunction();
  initUltrasonic();
  setupNightLights();
  setupWarningLights();
  setupAltitudeLight();
}

void loop() {
  // keep web/motor responsive
  handleWebServerClients();
  updateUltrasonic();   // <<— make sure this runs every iteration

  if (us_state == US_IDLE && g_mode == MODE_AUTO) {
    // All 4 values updated --> choose nearest
    float nearest = NAN;
    float arr[4] = { distanceCm1, distanceCm2, distanceCm3, distanceCm4 };

    for (int i = 0; i < 4; i++) {
      if (!isnan(arr[i]) && arr[i] > 0) {
        if (isnan(nearest) || arr[i] < nearest)
          nearest = arr[i];
      }
    }

    g_distance_cm = nearest;
    // Continuously updating marine status for UI
    marineController(g_distance_cm); 
    // Compute the AUTO command based on latest distance
    autoController(g_distance_cm);
    Serial.printf("[US] D1=%.1f D2=%.1f D3=%.1f D4=%.1f -> nearest=%.1f\n",
              distanceCm1, distanceCm2, distanceCm3, distanceCm4, g_distance_cm, marineStatusToString(g_marine_status));

  }
  // ---------------------------------------------------------------

  // Single place that drives the H-bridge (picks manual OR auto)
  motorFunctionLoop();
  if (g_emergency) {
  blinkEmergencyLeds();  // all LEDs blink
} else {
  nightLightsLoop();
  warningLightsLoop();
  altitudeLightLoop();
}

  // small pause to reduce serial spam / CPU (non-blocking is better, but fine)
  
}


