#define motor_driver_in1 26 //motor spin "high"
#define motor_driver_in2 25 //motor spin "low"

// LED Pins
#define RED_WARNING_LIGHT 10 // turns red when there is marine vehicles coming in
#define RED_ALTITUDE_LIGHT 11
#define NIGHT_LIGHTS 12


bool bridge_up = false;
bool bridge_halt= true;


//pins for sensor here

void setup() {
  // put your setup code here, to run once:
  //call the setup WebServer function from http server 
  setupWebServer(); 
  setupWiFi(); 

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
  handleWebServerClients();
}

void automatedSensor(){
  //read for sensor input

}


//Manual control from the UI

//for given time that the bridge will take to reach max height
//return true if operation succeed. 
bool bridge_open(){
    digitalWrite(motor_driver_in1, HIGH);
    digitalWrite(motor_driver_in2, LOW);
    warning_light();
    bridge_up = true;
    return true;
}

//for given time that the bridge will take to reach ground
//return true if operation succeed. 
bool bridge_close(){
    digitalWrite(motor_driver_in1, LOW);
    digitalWrite(motor_driver_in2, HIGH);
    bridge_up = false;\
    warning_light();
    return true; 
}

//it is run when the timer ends, or for emeregency stop
bool stop(){
    digitalWrite(motor_driver_in1, HIGH);
    digitalWrite(motor_driver_in2, HIGH);
    bridge_halt= true;
    warning_light();
    return true;
}

void warning_light() {
  if (bridge_up || bridge_halt) {
    if (/*incoming marine vehicles sensed*/ false) { 
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

