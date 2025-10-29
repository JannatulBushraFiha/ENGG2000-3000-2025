// LED Pins
unsigned long previousMillis = 0;
const long interval = 500;
int LED_STATE = LOW;

void setupWarningLights() {
  pinMode (RED_WARNING_LED, OUTPUT);   
}
void setupAltitudeLight() {
  pinMode (ALTITUDE_LIGHTS, OUTPUT);
}

void warningLightsLoop () {
    unsigned long current_Millis = millis();

     if (current_Millis - previousMillis >= interval) {
        previousMillis = current_Millis;

        if (digitalRead(RED_WARNING_LED)==LOW) {
            LED_STATE = HIGH;
        }
        else {
            LED_STATE = LOW;
        }
     }
    digitalWrite(RED_WARNING_LED, LED_STATE);
}

void altitudeLightLoop() {
  digitalWrite(ALTITUDE_LIGHTS,HIGH);
}