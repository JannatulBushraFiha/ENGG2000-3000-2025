#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include "state.h"

// ---------------- Pins (avoid GPIO6 on ESP32)
#define TRIG_PIN        22
#define ECHO_PIN1       23
#define ECHO_PIN2       21
#define ECHO_PIN3       19
#define ECHO_PIN4       18
#define RED_WARNING_LED 25
#define ALTITUDE_LIGHTS 5
#define MOTOR_IN1       15
#define MOTOR_IN2        2
#define LDR_PIN         27
#define NIGHT_LIGHTS    17

// ---------------- App state
SystemMode g_mode = MODE_AUTO;             // manual is default
volatile BridgeCmd  g_cmd_manual = CMD_IDLE; // set by web UI
volatile BridgeCmd  g_cmd_auto   = CMD_IDLE; // set by autoController()
volatile bool      g_emergency  = false;   // <-- NEW


unsigned long previousMillis = 0;
const unsigned long BLINK_INTERVAL_MS = 500;
int ledState = LOW;

float g_distance_cm = -1.0f;   // single source of truth (e.g., front sensor)

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

// ===== AUTO controller: computes g_cmd_auto only (never touches manual) =====
static void autoController(float distance_cm) {
  // Hysteresis band: <18 -> OPEN, >25 -> CLOSE, otherwise hold last
  static BridgeCmd last = CMD_IDLE;
  constexpr float OPEN_ON  = 18.0f;  // start opening when <= 18 cm
  constexpr float OPEN_OFF = 22.0f;  // stop opening when >= 22 cm

  BridgeCmd next = last;

  if (distance_cm > 0) {
    if (distance_cm <= OPEN_ON)       next = CMD_OPEN;  // object is close
    else if (distance_cm >= OPEN_OFF) next = CMD_STOP;  // object moved away
    // else: in 18..22 cm band -> hold last to avoid chatter
  } else {
    // Invalid / no echo -> be conservative
    next = CMD_STOP;
  }

  g_cmd_auto = next;
  last = next;
}

// --- Ultrasonic helpers
void initUltrasonic() {
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);

  pinMode(ECHO_PIN1, INPUT);
  pinMode(ECHO_PIN2, INPUT);
  pinMode(ECHO_PIN3, INPUT);
  pinMode(ECHO_PIN4, INPUT);
}

// Trigger and read all four sensors once; returns false if all time out
static bool readUltrasonicOnce(float &d1, float &d2, float &d3, float &d4) {
  // send 10 µs trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 30ms timeout (≈5m); adjust to your max range
  const unsigned long TO = 30000UL;

  unsigned long t1 = pulseIn(ECHO_PIN1, HIGH, TO);
  unsigned long t2 = pulseIn(ECHO_PIN2, HIGH, TO);
  unsigned long t3 = pulseIn(ECHO_PIN3, HIGH, TO);
  unsigned long t4 = pulseIn(ECHO_PIN4, HIGH, TO);

  // convert to cm (0.0343 cm/µs; divide by 2 for round-trip)
  d1 = t1 ? (t1 * 0.0343f) / 2.0f : NAN;
  d2 = t2 ? (t2 * 0.0343f) / 2.0f : NAN;
  d3 = t3 ? (t3 * 0.0343f) / 2.0f : NAN;
  d4 = t4 ? (t4 * 0.0343f) / 2.0f : NAN;

  return (t1 || t2 || t3 || t4);
}

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


void setup() {
  Serial.begin(115200);

  initUltrasonic();

  pinMode(RED_WARNING_LED, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  setupWiFi();
  setupWebServer();
  setupMotorFunction();
  setupNightLights();
  setupWarningLights();
  setupAltitudeLight();
}

void loop() {
  // keep web/motor responsive
  handleWebServerClients();

  // ------------ ONLY read sensors & plan when in AUTO ------------
  if (g_mode == MODE_AUTO) {
    static uint32_t lastPingMs = 0;
    if (millis() - lastPingMs >= 60) {      // ~16 Hz ping rate
      lastPingMs = millis();
      float d1, d2, d3, d4;
      if (readUltrasonicOnce(d1, d2, d3, d4)) {
        // choose "front" as primary; use your mapping as needed
        g_distance_cm = isnan(d1) ? (isnan(d2) ? (isnan(d3) ? d4 : d3) : d2) : d1;
      }
      // Optional: print for debugging
      Serial.print("AUTO distances (cm): ");
      Serial.print(d1); Serial.print("  ");
      Serial.print(d2); Serial.print("  ");
      Serial.print(d3); Serial.print("  ");
      Serial.println(d4);
    }

    // Compute the AUTO command based on latest distance
    autoController(g_distance_cm);
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
  delay(10);
}
