#include <WiFi.h>
//const char* apSSID = "BridgeESP32"; 
//const char* apPassword = "bridge1234"; 
//WARNING: Do not declare as apSSID when using STA mode!
//static const char* apSSID = "JannatulBushra's A15"; 
//static const char* apPassword = "yq2m8q7ixmf53i5"; 
//static const char* STA_SSID = "JannatulBushra's A15"; 
//static const char* STA_PASSWORD = "yq2m8q7ixmf53i5"; 
static const char* STA_SSID = "iPhone"; 
static const char* STA_PASSWORD = "12345678$"; 




void setupWiFi() {
    //Try with STA instead of AP 
    //Start WiFi in Access Point mode
    //Remove for testing STA  
    //WiFi.softAP(apSSID, apPassword); 
    //Get IP address assigned to the ESP32 
    //Remove for testing STA 
    //IPAddress IP = WiFi.softAPIP();

    WiFi.mode(WIFI_STA);
    WiFi.begin(STA_SSID, STA_PASSWORD);
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    
    Serial.println("ESP32 connected to Wi-Fi");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
}

    //Serial.println("ESP32 Access Point Started!");
    //Serial.println(apSSID);
    //Serial.print("Password: ");
    //Serial.println(apPassword);
    //Serial.print("AP IP address: ");
    //Serial.println(IP);

