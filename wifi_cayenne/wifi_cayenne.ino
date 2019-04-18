#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP8266.h>
#include "DHT.h"

#include <NTPClient.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID "iPhone de Ludwig"
#define STAPSK  "z09gh90k2gxrw"
#endif

#define TEMPERATURE_CHANNEL 1
#define LUMINOSITY_CHANNEL 2

#define ANALOGIC_SENSOR A0 //water level and soil moisture
#define DHT_SENSOR      D1
#define PUMP            D2
#define LIGHT           D3
#define FAN             D4
#define ANALOG_SWITCH   D5

#define DHTTYPE DHT11

const char* ssid          = STASSID;
const char* password      = STAPSK;
const char* MQTT_USERNAME = "76d792b0-6109-11e9-bdb5-dfd20f02ea3f";
const char* MQTT_PASSWORD = "3ae900a6b871295011df29232f488e7ba4be2ae0";
const char* CLIENT_ID     = "de76f850-611f-11e9-b189-ab9cb6660d7e";

int MIN_TEMP = 0;
int MAX_TEMP = 0;
int MIN_MOIST = 0;
int MAX_MOIST = 0;
int MIN_WATER_LEVEL = 0;
int MAX_WATER_LEVEL = 0;

float currentTemp = 0.0;

DHT dht(DHT_SENSOR, DHTTYPE);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setup() {
  
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  
  dht.begin();
  timeClient.begin();
  
  Cayenne.begin(MQTT_USERNAME, MQTT_PASSWORD, CLIENT_ID, STASSID, STAPSK);

}

void loop() {  

  timeClient.forceUpdate();
  Serial.println(timeClient.getFormattedDate());
  //Cayenne.loop();

}

// Default function for sending sensor data at intervals to Cayenne.
// You can also use functions for specific channels, e.g CAYENNE_OUT(1) for sending channel 1 data.
CAYENNE_OUT_DEFAULT()
{
  Cayenne.virtualWrite(LUMINOSITY_CHANNEL,currentTemp,"temp","c");
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
  CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}
