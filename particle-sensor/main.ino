#include <application.h>

int sensorEnable = D7;
int sensor = A0;

int switchOut = D0;
int switchIn = D1;

Timer soilMoistureTimer(5000, checkSoilMoisture);

void setup() {

Serial.println("Starting soil moister sensor...");

  // Pin configuration
  pinMode(sensorEnable, OUTPUT);
  pinMode(switchOut, OUTPUT);
  pinMode(switchIn, INPUT_PULLDOWN);

  Serial.println("Enabling switch");
  digitalWrite(switchOut, HIGH);

  Serial.println("Starting the soil moisture timer...");
  soilMoistureTimer.start();

  Serial.println("Started!");
}

void loop() {

}

void checkSoilMoisture() {

  Serial.println("Checking soil moisture level");

  if (switchDisable()) {
    Serial.println("Soil check is disabled..");
    return;
  }

  digitalWrite(sensorEnable, HIGH);

  delay(100);

  int sensorValue = analogRead(sensor);

  Serial.print("Sensor value: ");
  Serial.println(sensorValue);

  digitalWrite(sensorEnable, LOW);
}

bool switchDisable() {

  return digitalRead(switchIn) == HIGH;
}