#define motor_driver_in1 5 //motor spin "high"
#define motor_driver_in2 6 //motor spin "low"
#define motor_driver_pwm 9 //motor speed control

// LED Pins
#define RED_WARNING_LIGHT 10 // turns red when there is marine vehicles coming in
#define RED_ALTITUDE_LIGHT 11
#define NIGHT_LIGHTS 12


bool bridge_up = false;
bool bridge_halt= true;


//pins for sensor here

void setup() {
  // put your setup code here, to run once:

  //Pins
  pinMode(motor_driver_in1, OUTPUT);
  pinMode(motor_driver_in2, OUTPUT);
  pinMode(motor_driver_pwm, OUTPUT);
  pinMode (RED_WARNING_LIGHT, OUTPUT);
  pinMode (RED_ALTITUDE_LIGHT, OUTPUT);
                                                           
}

void loop() {
  // put your main code here, to run repeatedly:
blink_altitude_light();
}

void automatedSensor(){
  //read for sensor input

}


//Manual control from the UI

//for given time that the bridge will take to reach max height
void bridge_open(){
    digitalWrite(motor_driver_in1, HIGH);
    digitalWrite(motor_driver_in2, LOW);
    warning_light();
    bridge_up = true;
}

//for given time that the bridge will take to reach ground
void bridge_close(){
    digitalWrite(motor_driver_in1, LOW);
    digitalWrite(motor_driver_in2, HIGH);
    bridge_up = false;\
    warning_light();
}

//it is run when the timer ends, or for emeregency stop
void stop(){
    digitalWrite(motor_driver_in1, HIGH);
    digitalWrite(motor_driver_in2, HIGH);
    bridge_halt= true;
    warning_light();
}

void warning_light() {
  if (bridge_up || bridge_halt) {
    if (incoming marine vehicles sensed) { 
      digitalWrite(RED_WARNING_LIGHT, HIGH);
    }
    else {
      digitalWrite(RED_WARNING_LIGHT, LOW);
      }
  }
}

void blink_altitude_light() {
  digitalWrite(RED_ALTITUDE_LIGHT, HIGH);
  delay(500);
  digitalWrite(RED_ALTITUDE_LIGHT, LOW);
  delay(500);
}

void night_light_state() {

}

