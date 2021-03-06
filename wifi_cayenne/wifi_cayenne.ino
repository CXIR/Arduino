#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP8266.h>
#include "DHT.h"

#include <NTPClient.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID "iPhone de Ludwig"
#define STAPSK  "z09gh90k2gxrw"
#endif

// CAYENNE CHANNELS DEFINITION
#define TEMPERATURE_CHANNEL   1
#define HUMIDITY_CHANNEL      2
#define WATER_WINGS_CHANNEL   3 //0 is full
#define SOIL_MOISTURE_CHANNEL 4

// SENSORS PIN DEFINITION
#define SOIL_MOISTURE   A0
#define DHT_SENSOR      D5
#define WATER_WINGS     D6

// RELAY PINS DEFINITION
#define FAN             D1
#define LIGHT           D2
#define PUMP            D8

// TEMPERATURE AND HUMIDITY SENSOR DEFINITION
#define DHTTYPE DHT11

// NETWORK IDENTIFIERS
const char* ssid          = STASSID;
const char* password      = STAPSK;

// CAYENNE IDENTIFIERS
const char* MQTT_USERNAME = "76d792b0-6109-11e9-bdb5-dfd20f02ea3f";
const char* MQTT_PASSWORD = "3ae900a6b871295011df29232f488e7ba4be2ae0";
const char* CLIENT_ID     = "7f083040-62b0-11e9-933e-cf08617625ed";

// THRESHOLDS DEFINITION
int MIN_TEMPERATURE     = 10;
int MAX_TEMPERATURE     = 25;
int MIN_SOIL_MOISTURE   = 50;
int MAX_SOIL_MOISTURE   = 80;
int MIN_SUMMER_HUMIDITY = 20;
int MIN_WINTER_HUMIDITY = 20;
int MAX_SUMMER_HUMIDITY = 60;
int MAX_WINTER_HUMIDITY = 80;

int START_DAY = 8;
int END_DAY   = 20;

// STEP VALUES
float currentTemperature  = 0.0;
int   currentHumidity     = 0;
int   currentWaterWings   = 0;
int   currentSoilMoisture = 0;

int   currentHour        = 0;
int   currentMonth       = 0;
int   currentMinute      = 0;

bool isSummer    = false;
bool isAutomatic = false;

DHT dht(DHT_SENSOR, DHTTYPE);

WiFiUDP   ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;
String dayStamp;
String timeStamp;

void setup() {

  Serial.begin(115200);

  pinMode(WATER_WINGS, INPUT);
  pinMode(DHT_SENSOR, INPUT);
  pinMode(FAN,   OUTPUT);
  pinMode(LIGHT, OUTPUT);
  pinMode(PUMP,  OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  dht.begin();
  timeClient.begin();
  timeClient.setTimeOffset(7200);
  
  Cayenne.begin(MQTT_USERNAME, 
                MQTT_PASSWORD, 
                CLIENT_ID, 
                STASSID, 
                STAPSK);
}

void loop() {

  currentHumidity     = dht.readHumidity();
  currentTemperature  = dht.readTemperature();
  currentWaterWings   = digitalRead(WATER_WINGS);
  currentSoilMoisture = analogRead(SOIL_MOISTURE);

  if( isAutomatic) {
    
      fanControl();
      lightControl();
      pumpControl();
  }

  fanON();
  lightON();
  pumpON();

  Test();

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  formattedDate = timeClient.getFormattedDate();
  prepareDate();

  Cayenne.loop();
}

void Test () {
  
  Serial.print("Humidité : ");
  Serial.println(currentHumidity);

  Serial.print("Température : ");
  Serial.println(currentTemperature);

  Serial.print("Flotteur : ");
  Serial.println(currentWaterWings);

  Serial.print("Humidité de la Terre : ");
  Serial.println(currentSoilMoisture);

  Serial.print("Date : ");
  Serial.println(formattedDate);

  Serial.print("Minute : ");
  Serial.println(getMinute());

  Serial.print("Month : ");
  Serial.println(getMonth());

}

void fanControl () {

  if (isSummer && 
      currentHumidity > MAX_SUMMER_HUMIDITY) {
        
    fanON();
  }
  else if (!isSummer && 
           currentHumidity > MAX_WINTER_HUMIDITY) {
            
   fanON();
  }
  else if (currentTemperature > MAX_TEMPERATURE) {
    
    fanON();
  }
  else fanOFF();
} 

void pumpControl () {

  if(digitalRead(PUMP)) {
    if(getMinute() != currentMinute) {
      pumpOFF();
    }
  }
  else if (MIN_SOIL_MOISTURE < currentSoilMoisture < MAX_SOIL_MOISTURE ) {
    if (!analogRead(WATER_WINGS)) {
      
      currentMinute = getMinute();
      pumpON();
    }
  }
  else if (currentSoilMoisture > MAX_SOIL_MOISTURE) {
    pumpOFF();
  }
}

void lightControl () {

  if (START_DAY < currentHour < END_DAY) {
    
    if (!digitalRead(LIGHT)) {
      lightON();
    }
  }
  else if (digitalRead(LIGHT)) lightOFF();
}

void prepareDate () {

  int splitT = formattedDate.indexOf("T");
  dayStamp   = formattedDate.substring(0, splitT);

  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
}

int getDay () {
  return dayStamp.substring(8, 10).toInt();
}

int getMonth () {
  return dayStamp.substring(5, 7).toInt();
}

int getYear () {
  return dayStamp.substring(0, 4).toInt();
}

int getHour () {
  return timeStamp.substring(0, 2).toInt();
}

int getMinute () {
  return timeStamp.substring(3, 5).toInt();
}

bool checkSeason () {
  if(6 < getMonth() < 9) {
    isSummer = true;
  }
}

void pumpON () {
  if(!digitalRead(PUMP))  digitalWrite(PUMP, HIGH);
}

void pumpOFF () {
  
  if(digitalRead(PUMP)) digitalWrite(PUMP, LOW);
}

void lightON () {
  
  if(!digitalRead(LIGHT)) digitalWrite(LIGHT, HIGH);
}

void lightOFF () {
   if(digitalRead(LIGHT)) digitalWrite(LIGHT, LOW);
}

void fanON () {
  if(!digitalRead(FAN)) digitalWrite(FAN, HIGH);
}

void fanOFF () {
  if(digitalRead(FAN)) digitalWrite(FAN, LOW);
}
 
// Default function for sending sensor data at intervals to Cayenne.
// You can also use functions for specific channels, e.g CAYENNE_OUT(1) for sending channel 1 data.
CAYENNE_OUT_DEFAULT()
{
  Cayenne.virtualWrite(TEMPERATURE_CHANNEL,   currentTemperature,  "temp",       "c");
  Cayenne.virtualWrite(HUMIDITY_CHANNEL,      currentHumidity,     "rel_hum",    "p");
  Cayenne.virtualWrite(WATER_WINGS_CHANNEL,   currentWaterWings,   "prox",       "d");
  Cayenne.virtualWrite(SOIL_MOISTURE_CHANNEL, currentSoilMoisture, "soil_moist", "p");
}
 
// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}
