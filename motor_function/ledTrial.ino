// LED Pins
#define RED_WARNING_LIGHT 5 // turns red when there is marine vehicles coming in
#define RED_ALTITUDE_LIGHT 11
#define NIGHT_LIGHTS 12
unsigned long previousMillis = 0;
const long interval = 500;
int LED_STATE = LOW;

void setup() {
    pinMode (RED_WARNING_LIGHT, OUTPUT);
    pinMode (RED_ALTITUDE_LIGHT, OUTPUT);
}

void loop () {
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
