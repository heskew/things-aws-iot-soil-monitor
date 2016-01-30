// The MIT License (MIT)

// Copyright (c) 2016 Nathan Heskew

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <AmazonIOTClient.h>
#include <Esp8266AWSImplementations.h>
#include <AWSFoundationalTypes.h>
#include "keys.h"

uint8_t sensorEnable = D8;
uint8_t sensor = A0;
uint8_t sensorSwitch = D1;

const int sleepTimeS = 30;

void printWiFiData();
void printCurrentNetwork();
bool switchIsDisabled();
void publish(const char *topic, uint16_t data);
void checkSoilMoisture();

void setup() {

  Serial.begin(9600);

  // Pin configuration
  pinMode(sensorEnable, OUTPUT);
  pinMode(sensorSwitch, INPUT);

  Serial.println("Started!");

  checkSoilMoisture();
  ESP.deepSleep(sleepTimeS * 1000000);
}

void loop() {
}

void printWiFiData() {
  
  // IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
}

void printCurrentNetwork() {
  
  // SSID
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // signal strength:
  Serial.print("signal strength (RSSI): ");
  Serial.println(WiFi.RSSI());
}

bool sensorIsDisabled() {

  return digitalRead(sensorSwitch) == LOW;
}

void publish(const char *topic, uint8_t data) {
  
  AmazonIOTClient iotClient;
  ActionError actionError;

  Esp8266HttpClient httpClient;
  Esp8266DateTimeProvider dateTimeProvider;

  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(wifiSsid);
  Serial.println("...");

  WiFi.begin(wifiSsid, wifiPwd);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(50);
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  
  printCurrentNetwork();
  printWiFiData();

  delay(50);

  Serial.println("Initializing IoT client...");

  iotClient.setAWSRegion(awsIotRegion);
  iotClient.setAWSEndpoint(awsIotEndpoint);
  iotClient.setAWSDomain(awsIotDomain);
  iotClient.setAWSPath("/things/soil-sensor-one/shadow");
  iotClient.setAWSKeyID(awsKeyID);
  iotClient.setAWSSecretKey(awsSecKey);
  iotClient.setHttpClient(&httpClient);
  iotClient.setDateTimeProvider(&dateTimeProvider);

  delay(50);

  Serial.println("Updating thing shadow...");
  
  MinimalString shadow = ("{\"state\":{\"reported\":{\"moisture\":" + String(data, DEC) + "}}}").c_str();
  char* result = iotClient.update_shadow(shadow, actionError);

  Serial.print("result: ");
  Serial.println(result);
}

void checkSoilMoisture() {

  Serial.println("Checking soil moisture level");

  if (sensorIsDisabled()) {
    Serial.println("Soil check is disabled..");
    return;
  }

  digitalWrite(sensorEnable, HIGH);
  delay(100);

  int sensorValue = analogRead(sensor);
  digitalWrite(sensorEnable, LOW);

  Serial.print("Sensor value: ");
  Serial.println(sensorValue);

  uint8_t moistureLevel = sensorValue / 1023.0f * 100;
 
  Serial.print("Moisture level: ");
  Serial.println(moistureLevel);
 
  publish("soil/moisture", moistureLevel);
}

