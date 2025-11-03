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

const int trigPin1 = 12;
const int echoPin1 = 14;
const int trigPin2 = 33;
const int echoPin2 = 32;

// ---------------- Ultrasonic thresholds
constexpr float OPEN_ON_CM  = 19.0f;
constexpr float OPEN_OFF_CM = 21.0f;
constexpr uint32_t FRESH_TIMEOUT_MS = 600;

#define SOUND_SPEED 0.034
unsigned long timeout = 30000; // 30ms max wait
unsigned long interval = 100;  // measurement interval in ms

// Distance variables
float distanceCm1 = NAN, distanceCm2 = NAN;

// ---------------- App state
SystemMode g_mode = MODE_AUTO;
volatile BridgeCmd g_cmd_manual = CMD_IDLE;
volatile BridgeCmd g_cmd_auto   = CMD_IDLE;
volatile bool g_emergency = false;
volatile MarineStatus g_marine_status = MARINE_CLEAR; //Initial UI status 


unsigned long previousMillis = 0; const unsigned long BLINK_INTERVAL_MS = 500; int ledState = LOW;
float g_distance_cm = -1.0f;   // nearest distance

// ---------------- Ultrasonic state machine
enum US_State { US_IDLE, US_TRIGGER, US_WAIT_ECHO };
static US_State us_state = US_IDLE;
static int us_index = 0;
static uint32_t us_timer = 0;
static unsigned long us_pulseStart[2] = {0, 0};
static bool us_seenHigh[2] = {false, false};

const int trigPins[2] = { trigPin1, trigPin2 };
const int echoPins[2] = { echoPin1, echoPin2 };

// Forward declarations
void setupWiFi();
void setupWebServer();
void setupMotorFunction();
void motorFunctionLoop();
void setupNightLights();
void nightLightsLoop();
void setupWarningLights();
void warningLightsLoop();
void setupAltitudeLight();
void altitudeLightLoop();
void handleWebServerClients();

// ---------------- Helper: microseconds to cm
static inline float us_to_cm(unsigned long us) {
  return (us == 0UL) ? NAN : (us * SOUND_SPEED) / 2.0f;
}

// ---------------- Init ultrasonic sensors
void initUltrasonic() {
  for (int i = 0; i < 2; ++i) {
    pinMode(trigPins[i], OUTPUT);
    digitalWrite(trigPins[i], LOW);
    pinMode(echoPins[i], INPUT_PULLDOWN);  // stable idle
  }
  distanceCm1 = distanceCm2 = NAN;
}

// ---------------- Ultrasonic update
void updateUltrasonic() {
  switch (us_state) {
    case US_IDLE:
      us_index = 0;
      us_state = US_TRIGGER;
      break;

    case US_TRIGGER:
      // reset edge-tracking
      us_pulseStart[us_index] = 0;
      us_seenHigh[us_index] = false;

      digitalWrite(trigPins[us_index], LOW);
      delayMicroseconds(2);
      digitalWrite(trigPins[us_index], HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPins[us_index], LOW);

      us_timer = micros();
      us_state = US_WAIT_ECHO;
      break;

    case US_WAIT_ECHO: {
      int e = echoPins[us_index];

      // Rising edge
      if (!us_seenHigh[us_index] && digitalRead(e) == HIGH) {
        us_pulseStart[us_index] = micros();
        us_seenHigh[us_index] = true;
      }

      // Falling edge
      if (us_seenHigh[us_index] && digitalRead(e) == LOW) {
        unsigned long pulse = micros() - us_pulseStart[us_index];
        float cm = us_to_cm(pulse);

        if (us_index == 0) distanceCm1 = cm;
        else               distanceCm2 = cm;

        us_index++;
        us_state = (us_index >= 2) ? US_IDLE : US_TRIGGER;
        return;
      }

      // Timeout
      if (micros() - us_timer > timeout) {
        if (us_index == 0) distanceCm1 = NAN;
        else               distanceCm2 = NAN;

        us_index++;
        us_state = (us_index >= 2) ? US_IDLE : US_TRIGGER;
        return;
      }
    } break;
  }
}

// ---------------- Auto controller
static void autoController(float distance_cm) {
  static BridgeCmd last = CMD_STOP;
  static uint32_t lastValidMs = 0;

  if (!isnan(distance_cm) && distance_cm > 0.0f)
    lastValidMs = millis();

  if (millis() - lastValidMs > FRESH_TIMEOUT_MS) {
    g_cmd_auto = CMD_STOP;
    last = CMD_STOP;
    return;
  }

  BridgeCmd next = last;
  if (distance_cm <= OPEN_ON_CM)       next = CMD_OPEN;
  else if (distance_cm >= OPEN_OFF_CM) next = CMD_STOP;
  else                                 next = CMD_STOP;

  g_cmd_auto = next;
  last = next;
}

// ---------------- Main setup / loop
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
  handleWebServerClients();
  updateUltrasonic();

  if (us_state == US_IDLE && g_mode == MODE_AUTO) {
    float nearest = NAN;
    float arr[2] = { distanceCm1, distanceCm2 };

    for (int i = 0; i < 2; i++) {
      if (!isnan(arr[i]) && arr[i] > 0) {
        if (isnan(nearest) || arr[i] < nearest)
          nearest = arr[i];
      }
    }

    g_distance_cm = nearest;
    autoController(g_distance_cm);

    Serial.printf("[US] D1=%.1f cm, D2=%.1f cm -> nearest=%.1f cm\n",
                  distanceCm1, distanceCm2, g_distance_cm);
  }

  motorFunctionLoop();
  nightLightsLoop();
  warningLightsLoop();
  altitudeLightLoop();
}
