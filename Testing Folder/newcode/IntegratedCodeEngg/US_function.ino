<<<<<<< HEAD:Testing Folder/newcode/IntegratedCodeEngg/US_function.ino
// Ultrasonic helpers (US_function.ino)

// Pins come from main via #define trigPin / echoPin

// One-time init (call from setup)
void initUltrasonic() {
  pinMode(trigPin, OUTPUT);
  // try INPUT_PULLDOWN if echo line floats on your board; otherwise INPUT is fine
  pinMode(echoPin, INPUT_PULLDOWN);
  digitalWrite(trigPin, LOW);   // idle low per datasheet
  delay(50);
}

// Do one reading; returns true if valid
bool readUltrasonicOnce(float& out_cm) {
  // Trigger 10 µs pulse
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo with timeout (~40 ms ≈ ~7 m)
  unsigned long dur = pulseIn(echoPin, HIGH, 40000UL);
  if (dur == 0) return false;   // timeout = no echo
  out_cm = (dur * 0.0343f) / 2.0f;
  return true;
}
=======
// Ultrasonic helpers (US_function.ino)

// Pins come from main via #define trigPin / echoPin

// One-time init (call from setup)
//void initUltrasonic() {
//  pinMode(trigPin, OUTPUT);
//  // try INPUT_PULLDOWN if echo line floats on your board; otherwise INPUT is fine
//  pinMode(echoPin, INPUT_PULLDOWN);
//  digitalWrite(trigPin, LOW);   // idle low per datasheet
//  delay(50);
//}

// Do one reading; returns true if valid
//bool readUltrasonicOnce(float& out_cm) {
//  // Trigger 10 µs pulse
//  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
//  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
//  digitalWrite(trigPin, LOW);
//
//  // Read echo with timeout (~40 ms ≈ ~7 m)
//  unsigned long dur = pulseIn(echoPin, HIGH, 40000UL);
//  if (dur == 0) return false;   // timeout = no echo
//  out_cm = (dur * 0.0343f) / 2.0f;
//  return true;
//}
>>>>>>> ff52ac656e9b083ff6d8e8eee037e96512d51569:Testing Folder/Testing_Trial_Three/US_function.ino
