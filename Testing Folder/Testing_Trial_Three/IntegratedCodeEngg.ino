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

//void initUltrasonic();
//bool readUltrasonicOnce(float& out_cm);

#define nightLightPin 17
#define altitudeLightPin 5

#define trigPin 22
#define echoPin1 23
#define echoPin2 21
#define echoPin3 19
#define echoPin4 18

volatile unsigned long startTimes[4];
volatile unsigned long durations[4];
volatile bool pulseDone[4];
float distance_1, distance_2, distance_3, distance_4;
int USstage = 1;
unsigned long timeSincePing;
unsigned long microSeconds;
const unsigned long echoReadDelay = 10;

void IRAM_ATTR handleEchoChange(int index, int pin) {
  if (digitalRead(pin) == HIGH) {
    startTimes[index] = micros();
  } else {
    durations[index] = micros() - startTimes[index];
    pulseDone[index] = true;
  }
}

void IRAM_ATTR echo1Change() { handleEchoChange(0, echoPin1); }
void IRAM_ATTR echo2Change() { handleEchoChange(1, echoPin2); }
void IRAM_ATTR echo3Change() { handleEchoChange(2, echoPin3); }
void IRAM_ATTR echo4Change() { handleEchoChange(3, echoPin4); }



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

  //--- Ultrasonic Sensor variable setup ---//
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(echoPin2, INPUT);
  pinMode(echoPin3, INPUT);
  pinMode(echoPin4, INPUT);

  attachInterrupt(digitalPinToInterrupt(echoPin1), echo1Change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(echoPin2), echo2Change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(echoPin3), echo3Change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(echoPin4), echo4Change, CHANGE);
  //--- Ultrasonic Sensor Setup End ---//
  
  pinMode(RED_WARNING_LIGHT, OUTPUT);
  pinMode(motor_driver_in1, OUTPUT);
  pinMode(motor_driver_in2, OUTPUT);
  //Serial.begin(9600);
  //set the monitor to 115200 instead - ESP32's default system speed 
  Serial.begin(115200);
  Serial.println("Begin");
  //Call Wifi&Server functions 
  //initUltrasonic();
  setupWiFi(); 
  setupWebServer();
  setupMotorFunction();
}

void loop() {
  //add this otherwise the website wouldn't load. 
  motorFunctionLoop();
  handleWebServerClients();

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

  noInterrupts();
  unsigned long d1 = durations[0];
  unsigned long d2 = durations[1];
  unsigned long d3 = durations[2];
  unsigned long d4 = durations[3];
  interrupts();

  distance_1 = (d1 * 0.0343) / 2;
  distance_2 = (d2 * 0.0343) / 2;
  distance_3 = (d3 * 0.0343) / 2;
  distance_4 = (d4 * 0.0343) / 2;


  Serial.print("Distance: ");
  Serial.print(distance_1);
  Serial.print("  ");
  Serial.print(distance_2);
  Serial.print("  ");
  Serial.println(distance_3);
  Serial.print("  ");
  Serial.print(distance_4);
  //--- Ultrasonic Ping, Detection and Processing END ---//
  
  // Blink LED if object < 20 cm
  if (distance_1 < 20) {
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
  }

  //delay(50);//small delay for stability
}
    
