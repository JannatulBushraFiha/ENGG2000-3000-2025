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
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(RED_WARNING_LIGHT, OUTPUT);
  digitalWrite(RED_WARNING_LIGHT, LOW);
}

static inline void driveOpen()  { digitalWrite(MOTOR_IN1, HIGH); digitalWrite(MOTOR_IN2, LOW);  }
static inline void driveClose() { digitalWrite(MOTOR_IN1, LOW);  digitalWrite(MOTOR_IN2, HIGH); }
static inline void driveStop()  { digitalWrite(MOTOR_IN1, LOW);  digitalWrite(MOTOR_IN2, LOW);  }

void motorFunctionLoop() {


  if (g_emergency) {
    driveStop();
    return;
  }


  // Choose the command source
  BridgeCmd cmd = (g_mode == MODE_AUTO) ? g_cmd_auto : g_cmd_manual;
  // Optional: stop once on mode change to avoid surprises
 
  // Drive outputs
  switch (cmd) {
    case CMD_OPEN:  driveOpen();  break;
    case CMD_CLOSE: driveClose(); break;
    case CMD_STOP:
    case CMD_IDLE:
    default:        driveStop();  break;
  }

  }
  // AUTO logic based on g_distance_cm (optional)
  // Two thresholds to avoid chattering

  

  // Execute command continuously
  

  // Optional: blink while opening
  
