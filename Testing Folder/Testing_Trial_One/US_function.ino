//const int trigPin = 2;
//const int echoPin1 = 8;
//const int echoPin2 = 9;
//const int echoPin3 = 10;
//const int echoPin4 = 30;

const int echoReadDelay = 10;

unsigned long microSeconds;

unsigned long timeSincePing;

int USstage = 1;

float distance_1, distance_2, distance_3, distance_4;

volatile uint16_t duration_1;
volatile uint16_t duration_2;
volatile uint16_t duration_3;
volatile uint16_t duration_4;

#include <PulseInput.h>
void setupUS() {

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(echoPin2, INPUT);
  pinMode(echoPin3, INPUT);
  pinMode(echoPin4, INPUT);

  
  attachPulseInput(echoPin1, duration_1);
  attachPulseInput(echoPin2, duration_2);
  attachPulseInput(echoPin3, duration_3);
  attachPulseInput(echoPin4, duration_4);
}

void loopUS() {
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
}
