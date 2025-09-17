<<<<<<< HEAD
//#include wifi.h


#define motor_driver_in1 23 //motor spin "high"
#define motor_driver_in2 22 //motor spin "low"
#define motor_driver_pwm 9 //motor speed control
=======
#define motor_driver_in1 26 //motor spin "high"
#define motor_driver_in2 25 //motor spin "low"
>>>>>>> dfc02922e311f2c3747b444d67c439ec910c7f95

// LED Pins
#define RED_WARNING_LIGHT 4 // turns red when there is marine vehicles coming in
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
  pinMode(RED_WARNING_LIGHT, OUTPUT);
  pinMode(RED_ALTITUDE_LIGHT, OUTPUT);
                                                           
}

void loop() {
  // put your main code here, to run repeatedly:
<<<<<<< HEAD
    bridge_open();
    digitalWrite(RED_WARNING_LIGHT, HIGH);
    delay(1000);
    digitalWrite(RED_WARNING_LIGHT, LOW);
    delay(1000);




=======
  blink_altitude_light();
  handleWebServerClients();
>>>>>>> dfc02922e311f2c3747b444d67c439ec910c7f95
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
<<<<<<< HEAD
    //warning_light();
    //bridge_up = true;
=======
    warning_light();
    bridge_up = true;
    return true;
>>>>>>> dfc02922e311f2c3747b444d67c439ec910c7f95
}

//for given time that the bridge will take to reach ground
//return true if operation succeed. 
bool bridge_close(){
    digitalWrite(motor_driver_in1, LOW);
    digitalWrite(motor_driver_in2, HIGH);
    bridge_up = false;\
<<<<<<< HEAD
   // warning_light();
=======
    warning_light();
    return true; 
>>>>>>> dfc02922e311f2c3747b444d67c439ec910c7f95
}

//it is run when the timer ends, or for emeregency stop
bool stop(){
    digitalWrite(motor_driver_in1, HIGH);
    digitalWrite(motor_driver_in2, HIGH);
    bridge_halt= true;
<<<<<<< HEAD
    //warning_light();
=======
    warning_light();
    return true;
>>>>>>> dfc02922e311f2c3747b444d67c439ec910c7f95
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

