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

#include <ArduinoJson.h>

#include <Adafruit_NeoPixel.h>

#define PIN 15
 
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(32, PIN, NEO_GRB + NEO_KHZ800);

void setColor(uint8_t r, uint8_t g, uint8_t b, bool immediate = false);
uint8_t getSoilMoistureValue();

void setup() {

  Serial.begin(115200);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  setColor(1, 1, 1, true);

  Serial.println("Started!");
}

void loop() {

  uint8_t value = getSoilMoistureValue();
  if (value <= 10) {
    // too little water
    setColor(1, 0, 0);
  }
  else if (value > 10 && value <= 30) {
    // should probably water
    setColor(1, 1, 0);
  }
  else if (value > 30 && value <= 60) {
    // just enough water
    setColor(0, 1, 0);
  }
  else {
    // too much water
    setColor(0, 0, 1);
  }

  delay(5000);
}

void setColor(uint8_t r, uint8_t g, uint8_t b, bool immediate) {
    
  for (uint8_t i = 0; i < 32; ++i) {
    strip.setPixelColor(i, r, g, b);
    if (immediate == false && (i+1)%8 == 0) {
      strip.show();
      delay(50);
    }
  }

  if (immediate == true) {
    strip.show();
  }
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
  Serial.println( WiFi.RSSI());
}

uint8_t getSoilMoistureValue() {
  
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
  
  Serial.println();
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

  Serial.println("Getting soil moisture value...");
  char* shadow = iotClient.get_shadow(actionError);

  int i = 0, l = strlen(shadow);
  char* body;
  
  while (i++ < l) {
    
    if (i > 0 && shadow[i] == '\n' && shadow[i-1] == '\n') {

      body = new char[l-1-i]();
      
      int j = 0, il = l + 1 - i;
      while (j < il) {
        body[j++] = shadow[i+j];
      }
      
      break;
    }
  }

  delete[] shadow;
  
  if (strlen(body) < 1) {
    Serial.println("There is no body");
    return 0;
  }

  StaticJsonBuffer<500> jsonBuffer;
  
  JsonObject& root = jsonBuffer.parseObject(body);
  delete[] body;
  
  Serial.println();
  Serial.print("Shadow JSON: ");
  root.printTo(Serial);
  Serial.println();
  
  uint8_t level = root["state"]["reported"]["moisture"];
  Serial.print("Moisture level: ");
  Serial.println(level);

  //Serial.println(ESP.getFreeHeap());
  Serial.println("Done!");

  return level;
}

