

#define motor_driver_in1 = 5; //motor spin "high"
#define motor_driver_in2 = 6; //motor spin "low"
#define motor_driver_pwm = 9; //motor speed control
#define bridge_up = false;
#define bridge_stop = true;


//pins for sensor here

void setup() {
  // put your setup code here, to run once:

  //Pins
  pinMode(motor_driver_in1, OUTPUT);
  pinMode(motor_driver_in2, OUTPUT);
  pinMode(motor_driver_pwm, OUTPUT);
                                                            ```````````````````````````````````````````````````````
}

void loop() {
  // put your main code here, to run repeatedly:

}

void automatedSensor(){
  //read for sensor input

}


//Manual control from the UI

//for given time that the bridge will take to reach max height
void bridge_open(){
    digitalWrite(motor_driver_in1, HIGH);
    digitalWrite(motor_driver_in2, LOW);
    bridge_up = true;
}

//for given time that the bridge will take to reach ground
void bridge_close(){
    digitalWrite(motor_driver_in1, LOW);
    digitalWrite(motor_driver_in2, HIGH);
}

//it is run when the timer ends, or for emeregency stop
void stop(){
    digitalWrite(motor_driver_in1, HIGH);
    digitalWrite(motor_driver_in2, HIGH);
    bridge_stop = true;
}
