#include <Arduino.h>
#include <WiFi.h> 
#include <WebServer.h>
#include "state.h"


SystemMode g_mode = MODE_MANUAL;  // start manual
BridgeCmd  g_cmd  = CMD_IDLE;

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
#define echoPin 23
#define RED_WARNING_LIGHT 25 // LED pin
#define motor_driver_in1 15 //motor spin "high"
#define motor_driver_in2 2 //motor spin "low"

//fixed syntax error for compliation. 
unsigned long previousMillis = 0;
const unsigned long interval = 500;
int LED_STATE = LOW;

float g_distance_cm = -1.0f;   // single source of truth

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

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

  //delay(50);//small delay for stability
}
    