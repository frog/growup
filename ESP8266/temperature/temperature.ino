//
// Copyright Frog Design Inc.
// 
// Authur: XY Feng
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// Firebase Database
#define FIREBASE_HOST "frog-grow.firebaseio.com"
#define FIREBASE_AUTH "Jloy4eF0qW3IpZdxQCbKAzzrR1EBSxhimARXh5Ts"

// WiFi 
#define WIFI_SSID "growup"
#define WIFI_PASSWORD ""

// Temperature Sensor
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  // connect to firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // start up temperator sensor
  sensors.begin();
}

void loop() {
  
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);
  Serial.println("DONE");
  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(temperature);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& postData = jsonBuffer.createObject();
  postData["value"] = temperature;
  JsonObject& timestamp = postData.createNestedObject("time");
  timestamp[".sv"] = "timestamp";

  // upload to firebase
  Firebase.push("temperature", postData);
  if (Firebase.failed()) {
      Serial.print("setting /temperature failed:");
      Serial.println(Firebase.error());  
      return;
  }

  delay(1000);
}
