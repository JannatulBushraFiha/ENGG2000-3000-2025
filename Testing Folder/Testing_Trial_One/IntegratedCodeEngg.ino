#include <Arduino.h>
#include <WiFi.h> 
#include <WebServer.h>
<<<<<<< HEAD:Testing Folder/newcode/IntegratedCodeEngg.ino
#include "state.h"


SystemMode g_mode = MODE_MANUAL;  // start manual
BridgeCmd  g_cmd  = CMD_IDLE;
=======
#include <PulseInput.h>
>>>>>>> d9106992d81f40d8e40a5c69f2c40ebbf2f8bcac:Testing Folder/IntegratedCodeEngg.ino

//Some forward declarations so the files compiled even not in alphabetical order 
void setupWiFi(); 
void setupWebServer(); 
void setupMotorFunction(); 
void motorFunctionLoop(); 
//forward declare handleWebServerClients() so not out of scope. 
void handleWebServerClients(); 

void initUltrasonic();
bool readUltrasonicOnce(float& out_cm);


#define trigPin 22
#define echoPin1 23
#define echoPin2 4 //needs changing
#define echoPin3 5 //needs changing
#define echoPin4 6 //needs changing
#define RED_WARNING_LIGHT 25 // LED pin
#define motor_driver_in1 15 //motor spin "high"
#define motor_driver_in2 2 //motor spin "low"

//fixed syntax error for compliation. 
unsigned long previousMillis = 0;
const unsigned long interval = 500;
int LED_STATE = LOW;

float g_distance_cm = -1.0f;   // single source of truth

void setup() {
<<<<<<< HEAD:Testing Folder/newcode/IntegratedCodeEngg.ino
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

=======
  //--- Ultrasonic Sensor variable setup ---//
  const int echoReadDelay = 10;
  unsigned long microSeconds;
  unsigned long timeSincePing;
  int USstage = 1;
  float distance_1, distance_2, distance_3, distance_4;
  
  volatile uint16_t duration_1;
  volatile uint16_t duration_2;
  volatile uint16_t duration_3;
  volatile uint16_t duration_4;

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(echoPin2, INPUT);
  pinMode(echoPin3, INPUT);
  pinMode(echoPin4, INPUT);

  attachPulseInput(echoPin1, duration_1);
  attachPulseInput(echoPin2, duration_2);
  attachPulseInput(echoPin3, duration_3);
  attachPulseInput(echoPin4, duration_4);
  //--- Ultrasonic Sensor Setup End ---//
  
>>>>>>> d9106992d81f40d8e40a5c69f2c40ebbf2f8bcac:Testing Folder/IntegratedCodeEngg.ino
  pinMode(RED_WARNING_LIGHT, OUTPUT);
  pinMode(motor_driver_in1, OUTPUT);
  pinMode(motor_driver_in2, OUTPUT);
  //Serial.begin(9600);
  //set the monitor to 115200 instead - ESP32's default system speed 
  Serial.begin(115200);
  
  //Call Wifi&Server functions 
  initUltrasonic();
  setupWiFi(); 
  setupWebServer();
  setupMotorFunction();
}

void loop() {
  
  //add this otherwise the website wouldn't load. 
  motorFunctionLoop();
<<<<<<< HEAD:Testing Folder/newcode/IntegratedCodeEngg.ino
  handleWebServerClients();

  static uint32_t lastPing = 0;
  if (millis() - lastPing >= 60) {
    lastPing = millis();
    float cm;
    if (readUltrasonicOnce(cm)) {
      g_distance_cm = cm;       // only update on success
    }
  }

  Serial.print("Distance: ");
  Serial.println(g_distance_cm, 2);
  // Trigger the ultrasonic pulse
  /*digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);/*/

  // Measure echo time
 
  

=======

  //--- Ultrasonic Ping, Detection and Processing END ---//
  microSeconds = micros();
  
  if (USstage == 1) {
    digitalWrite(trigPin,HIGH);
    timeSincePing = microSeconds;
    USstage++;
  };
  
  if (USstage == 2 && (microSeconds - timeSincePing) >= echoReadDelay) {
    digitalWrite(trigPin, LOW);
    USstage++;
  };
  
  if (USstage == 3 && (microSeconds - timeSincePing) >= 200000) {
    USstage = 1;
  }

  distance_1 = (duration_1*.0343)/2;
  distance_2 = (duration_2*.0343)/2;
  distance_3 = (duration_3*.0343)/2;
  distance_4 = (duration_4*.0343)/2;

  Serial.print("Distance: ");
  Serial.print(distance_1);
  Serial.print("  ");
  Serial.print(distance_2);
  Serial.print("  ");
  Serial.println(distance_3);
  Serial.print("  ");
  Serial.print(distance_4);
  //--- Ultrasonic Ping, Detection and Processing END ---//
  
>>>>>>> d9106992d81f40d8e40a5c69f2c40ebbf2f8bcac:Testing Folder/IntegratedCodeEngg.ino
  // Blink LED if object < 20 cm
  /*if (distance < 20) {
    digitalWrite(motor_driver_in1, HIGH);
    digitalWrite(motor_driver_in2, LOW);

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // toggle LED state
      LED_STATE = (LED_STATE == LOW) ? HIGH : LOW;
      digitalWrite(RED_WARNING_LIGHT, LED_STATE);
    }
  } else {
    //Otherwise keep LED OFF
    digitalWrite(motor_driver_in1, LOW);
    digitalWrite(motor_driver_in2, LOW);
    digitalWrite(RED_WARNING_LIGHT, LOW);
    LED_STATE = LOW;
  }/*/

<<<<<<< HEAD:Testing Folder/newcode/IntegratedCodeEngg.ino
  //delay(50);//small delay for stability
=======
  //delay(50); //small delay for stability
>>>>>>> d9106992d81f40d8e40a5c69f2c40ebbf2f8bcac:Testing Folder/IntegratedCodeEngg.ino
}
    
