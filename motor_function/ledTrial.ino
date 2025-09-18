// LED Pins
#define RED_WARNING_LIGHT 25 // turns red when there is marine vehicles coming in
#define RED_ALTITUDE_LIGHT 26 // changed from 11 because 11 is forbidden by the ESP32 so it crashes
#define NIGHT_LIGHTS 27 // changed from 12 so ESP32 will allow it
unsigned long previousMillis = 0;
const long interval = 500;
int LED_STATE = LOW;

void setupLedTrial() {
    pinMode (RED_WARNING_LIGHT, OUTPUT);
    pinMode (RED_ALTITUDE_LIGHT, OUTPUT);
}

void ledTrialLoop () {
    unsigned long current_Millis = millis();

     if (current_Millis - previousMillis >= interval) {
        previousMillis = current_Millis;

        if (digitalRead(RED_WARNING_LIGHT)==LOW) {
            LED_STATE = HIGH;
        }
        else {
            LED_STATE = LOW;
        }
     }
    digitalWrite(RED_WARNING_LIGHT, LED_STATE);
}
