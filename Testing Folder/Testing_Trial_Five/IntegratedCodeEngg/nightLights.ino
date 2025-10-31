
void setupNightLights() {
  pinMode(NIGHT_LIGHTS, OUTPUT);
  pinMode(LDR_PIN, INPUT);
}

void nightLightsLoop() {
  static unsigned long nightLightCheck = 0;
  if (millis() - nightLightCheck >= 200) {
    nightLightCheck = millis();

    if (digitalRead(LDR_PIN) == LOW) {
      digitalWrite(NIGHT_LIGHTS, HIGH);  // Turn ON when dark
    } else {
      digitalWrite(NIGHT_LIGHTS, LOW);   // Turn OFF when bright
    }
  }
}
