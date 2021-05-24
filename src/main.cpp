#include <Arduino.h>
#include <M5StickCPlus.h>
#include <OneButton.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#include "autohue.h"
#include "secret.h"

#define CONNECT_TIMEOUT_MS 30000

Preferences preferences;
AutoHue hue;

// connectToWiFi adapted from ESP32 example code. See, e.g.:
// https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClient/WiFiClient.ino
void connectToWiFi() {
    unsigned long startTime = millis();
    Serial.println("Connecting to: " + String(WIFI_SSID));

    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PWD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");

        if (millis() - startTime > CONNECT_TIMEOUT_MS) {
            Serial.println();
            Serial.println("Failed to connect.");
            return;
        }
    }

    WiFi.setAutoReconnect(true);

    Serial.println();
    Serial.println("Connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);
    preferences.begin("mr-lightremote", false); 
    M5.begin();
    M5.IMU.Init();

    connectToWiFi();

    hue.detectHueIp();
    Serial.println(hue.getIp());

    while(!hue.requestNewUser());
    Serial.println(hue.getUser());

    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(80, 15);
    M5.Lcd.println("IMU TEST");
    M5.Lcd.setCursor(30, 30);
    M5.Lcd.println("  X       Y       Z");
    M5.Lcd.setCursor(30, 70);
    M5.Lcd.println("  Pitch   Roll    Yaw");
}

float temp = 0;

void loop() {
    float pitch = 0.0F;
    float roll  = 0.0F;
    float yaw   = 0.0F;

    M5.IMU.getAhrsData(&pitch,&roll,&yaw);
    M5.IMU.getTempData(&temp);

    M5.Lcd.setCursor(30, 80);
    M5.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", pitch, roll, yaw);

    M5.Lcd.setCursor(30, 95);
    M5.Lcd.printf("Temperature : %.2f C", temp);
    delay(100);
}