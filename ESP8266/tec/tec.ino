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

#include <SoftwareSerial.h>
SoftwareSerial swSer(14, 12, false, 256);
String sensorstring = "";
boolean sensor_string_complete = false;

void setup() {
  Serial.begin(9600);
  swSer.begin(9600);

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
  sensorstring.reserve(30);
}

void loop() {

  if (swSer.available() > 0) {                     //if we see that the Atlas Scientific product has sent a character
    char inchar = (char)swSer.read();              //get the char we just received
    sensorstring += inchar;                           //add the char to the var called sensorstring
    if (inchar == '\r') {                             //if the incoming character is a <CR>
      sensor_string_complete = true;                  //set the flag
    }
  }

  if (sensor_string_complete == true) {               //if a string from the Atlas Scientific product has been received in its entirety
    if (isdigit(sensorstring[0]) == false) {          //if the first character in the string is a digit
      Serial.println(sensorstring);                   //send that string to the PC's serial monitor
    }
    else                                              //if the first character in the string is NOT a digit
    {
      post_EC_data();                                //then call this function
    }
    sensorstring = "";                                //clear the string
    sensor_string_complete = false;                   //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
  }

}


void post_EC_data(void) {                            //this function will pars the string
  char sensorstring_array[30];                        //we make a char array
  char *EC;
  sensorstring.toCharArray(sensorstring_array, 30);   //convert the string to a char array
  EC = strtok(sensorstring_array, ",");               //let's pars the array at each comma

  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);
  Serial.println("DONE");
  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(temperature);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& postData = jsonBuffer.createObject();
  postData["t"] = temperature;
  postData["ec"] = EC;
  JsonObject& timestamp = postData.createNestedObject("time");
  timestamp[".sv"] = "timestamp";

  // upload to firebase
  Firebase.push("data", postData);
  if (Firebase.failed()) {
    Serial.print("setting /data failed:");
    Serial.println(Firebase.error());
    return;
  }
}
