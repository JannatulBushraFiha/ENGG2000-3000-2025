// LED Pins
#define RED_WARNING_LIGHT 10 // turns red when there is marine vehicles coming in
#define RED_ALTITUDE_LIGHT 11
#define NIGHT_LIGHTS 12

void setup() {
    pinMode (RED_WARNING_LIGHT, OUTPUT);
    pinMode (RED_ALTITUDE_LIGHT, OUTPUT);
}

void loop () {
    digitalWrite(RED_WARNING_LIGHT, HIGH);
    delay(500);
    digitalWrite(RED_WARNING_LIGHT, LOW);
    delay(500);
}