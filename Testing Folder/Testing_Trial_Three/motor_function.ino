// Motor control (motor_function.ino)

extern float g_distance_cm; 
extern SystemMode g_mode;
extern BridgeCmd  g_cmd;         // comes from main
void motor_open_spin();
void motor_close_spin();
void motor_coast();



void setupMotorFunction() {
  // Removed: setupUS();   // ❌ wrong name
  // Nothing needed here for ultrasonic; it is initialized by initUltrasonic() in main
  pinMode(motor_driver_in1, OUTPUT);
  pinMode(motor_driver_in2, OUTPUT);
  pinMode(RED_WARNING_LIGHT, OUTPUT);
  motor_coast();
  digitalWrite(RED_WARNING_LIGHT, LOW);
}

inline void motor_coast() {
  digitalWrite(motor_driver_in1, LOW);
  digitalWrite(motor_driver_in2, LOW);
}
inline void motor_open_spin() {
  digitalWrite(motor_driver_in1, HIGH);
  digitalWrite(motor_driver_in2, LOW);
}
inline void motor_close_spin() {
  digitalWrite(motor_driver_in1, LOW);
  digitalWrite(motor_driver_in2, HIGH);
}

void motorFunctionLoop() {
  // AUTO logic based on g_distance_cm (optional)
  // Two thresholds to avoid chattering
  constexpr float TH_OPEN  = 18.0f;  // go OPEN below this
  constexpr float TH_CLOSE = 25.0f;  // go CLOSE above this
  static BridgeCmd last_cmd = CMD_IDLE;

  if (g_distance_cm > 0) { // ignore invalid/timeout
    if (g_distance_cm < TH_OPEN) {
      g_cmd = CMD_OPEN;
    } else if (g_distance_cm > TH_CLOSE) {
      g_cmd = CMD_IDLE;
    } else {
      // within 18..25 cm keep previous command (no chatter)
      g_cmd = last_cmd == CMD_IDLE ? CMD_CLOSE : last_cmd;
      // ^ pick a sensible default on first run; choose CLOSE or OPEN as you prefer
    }
    last_cmd = g_cmd;
  }
  

  // Execute command continuously
  switch (g_cmd) {
    case CMD_OPEN:
      motor_open_spin();
      break;
    case CMD_CLOSE:
      motor_close_spin();
      break;
    case CMD_STOP:
      motor_coast();
      digitalWrite(RED_WARNING_LIGHT, HIGH); // show STOP
      return; // early return keeps LED solid
    case CMD_IDLE:
    default:
      motor_coast();
      break;
  }

  // Optional: blink while opening
  static uint32_t ledPrev = 0;
  static int ledState = LOW;
  if (g_cmd == CMD_OPEN) {
    if (millis() - ledPrev >= 500) {
      ledPrev = millis();
      ledState = (ledState == LOW) ? HIGH : LOW;
      digitalWrite(RED_WARNING_LIGHT, ledState);
    }
  } else {
    digitalWrite(RED_WARNING_LIGHT, LOW);
  }
}
