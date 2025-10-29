const int trigPin1 = 22;
const int echoPin1 = 23;
const int trigPin2 = 32;
const int echoPin2 = 21;
const int trigPin3 = 33;
const int echoPin3 = 19;
const int trigPin4 = 26;
const int echoPin4 = 18;

#define SOUND_SPEED 0.034
unsigned long timeout = 30000; // 30ms max wait
unsigned long interval = 100;  // measurement interval in ms

// Distance variables
float distanceCm1, distanceCm2, distanceCm3, distanceCm4;

// Timing variables
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);

  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);
  pinMode(trigPin4, OUTPUT);
  pinMode(echoPin4, INPUT);
}

// Function to measure distance without pulseIn
unsigned long measureDistance(int trigPin, int echoPin) {
  // Trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Wait for echo HIGH
  unsigned long startMicros = micros();
  while (digitalRead(echoPin) == LOW) {
    if (micros() - startMicros > timeout) return 0; // timeout
  }

  // Measure echo HIGH duration
  unsigned long pulseStart = micros();
  while (digitalRead(echoPin) == HIGH) {
    if (micros() - pulseStart > timeout) return 0; // timeout
  }
  return micros() - pulseStart;
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Measure distances
    distanceCm1 = measureDistance(trigPin1, echoPin1) * SOUND_SPEED / 2.0;
    distanceCm2 = measureDistance(trigPin2, echoPin2) * SOUND_SPEED / 2.0;
    distanceCm3 = measureDistance(trigPin3, echoPin3) * SOUND_SPEED / 2.0;
    distanceCm4 = measureDistance(trigPin4, echoPin4) * SOUND_SPEED / 2.0;

    // Print distances
    Serial.print("D1: "); Serial.print(distanceCm1); Serial.print(" cm, ");
    Serial.print("D2: "); Serial.print(distanceCm2); Serial.print(" cm, ");
    Serial.print("D3: "); Serial.print(distanceCm3); Serial.print(" cm, ");
    Serial.print("D4: "); Serial.println(distanceCm4); Serial.print(" cm");
  }
}