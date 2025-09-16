#include <WiFi.h>

const char* apSSID = "BridgeESP32"; 
const char* apPassword = "bridge1234"; 

void setupWiFi() {
    //Talking over USB 
    Serial.begin(115200);
    //1 second  
    delay(1000); 

    //Start WiFi in Access Point mode 
    WiFi.softAP(apSSID, apPassword); 

    //Get IP address assigned to the ESP32 
    IPAddress IP = WiFi.softAPIP();
    Serial.println("ESP32 Access Point Started!");
    Serial.print("SSID: ");
    Serial.println(apSSID);
    Serial.print("Password: ");
    Serial.println(apPassword);
    Serial.print("AP IP address: ");
    Serial.println(IP);


}