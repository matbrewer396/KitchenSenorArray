#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "config.h";
#include <SimpleTimer.h> //https://github.com/jfturcot/SimpleTimer
#include <ArduinoJson.h>
#include <ArduinoOTA.h> //https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

//
//


// Pin Settings
//----------------------------------------------------------------------------------------------------------------------

#define PIN_DishWasherLDR                       2
#define PIN_DishWasherDoor                      5
#define PIN_WasherDoor                          14
#define PIN_WasherDoorLDR                       12
#define PIN_PIR                                 13
#define PIN_Backdoor                            0
#define PIN_window                              9

#define PIN_DHT                                 4


// Old Status Vars
//----------------------------------------------------------------------------------------------------------------------
int DishWasherDoor;
int DishWasherLDR;
int WasherDoor;
int WasherLDR;
int BackDoor;
int window;
int PIR;
bool startUp = true;


//Load congf
//----------------------------------------------------------------------------------------------------------------------
// Wifi: SSID and password
const char* WIFI_SSID = _WIFI_SSID;
const char* WIFI_PASSWORD = _WIFI_PASSWORD;


//// MQTT: ID, server IP, port, username and password
const PROGMEM char* MQTT_CLIENT_ID = _DEVICENAME;
const PROGMEM char* MQTT_SERVER_IP = _MQTT_SERVER_IP;
const PROGMEM uint16_t MQTT_SERVER_PORT = _MQTT_SERVER_PORT;
const PROGMEM char* MQTT_USER = _MQTT_USER;
const PROGMEM char* MQTT_PASSWORD = _MQTT_PASSWORD;

// MQTT Topic
const PROGMEM char* MQTT_CHECKIN_TOPIC = _MQTT_CHECKIN_TOPIC;



// Dependances Setup
//----------------------------------------------------------------------------------------------------------------------
SimpleTimer timer;
DHT dht;
WiFiClient wifiClient;
PubSubClient client(wifiClient);



void setup_wifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() 
{
  // Loop until we're reconnected
  int retries = 0;
  while (!client.connected()) {
    if(retries < 15)
    {
        Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
        Serial.println("WIFI_PASSWORD");
        // Once connected, publish an announcement...
        if(startUp == true) {
          client.publish(MQTT_CHECKIN_TOPIC,"Rebooted");
          startUp = false;
        }
        else if(startUp == false) {
          client.publish(MQTT_CHECKIN_TOPIC,"Reconnected"); 
        }
      } 
      else {
        Serial.print("ERROR: MQTT failed, rc=");
        Serial.print(client.state());
        Serial.println("DEBUG: try again in 5 seconds");
        retries++;
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }

    
    if(retries > 14) {
      ESP.restart();
    }
  }
}

void checkIn()
{
  client.publish(MQTT_CHECKIN_TOPIC,"OK");
}
void forceStatusPost()
{
  DishWasherDoor = -1;
  DishWasherLDR = -1;
  WasherDoor =-1;
  WasherLDR =-1;
  BackDoor =-1;
  PIR =-1;
  window =-1;
}

void readRealTimeSenors() {
  DishWasherDoor = postBoolStatus(DishWasherDoor,digitalRead(PIN_DishWasherDoor),_MQTT_DISHWASHERDOOR_TOPIC);
  DishWasherLDR = postBoolStatus(DishWasherLDR,digitalRead(PIN_DishWasherLDR),_MQTT_DISHWASHERLDR_TOPIC);
  WasherDoor = postBoolStatus(WasherDoor,digitalRead(PIN_WasherDoor),_MQTT_WASHERDOOR_TOPIC);
  WasherLDR = postBoolStatus(WasherLDR,digitalRead(PIN_WasherDoorLDR),_MQTT_WASHERLDR_TOPIC);
  BackDoor = postBoolStatus(BackDoor,digitalRead(PIN_Backdoor),_MQTT_BACKDOOR_TOPIC);
  PIR = postBoolStatus(PIR,digitalRead(PIN_PIR),_MQTT_PIR_TOPIC);
  window = postBoolStatus(window,digitalRead(PIN_window),_MQTT_WINDOW_TOPIC);
}

int postBoolStatus(int oldstatus, int newStatus, char* topic)
{ 
  if( (newStatus != oldstatus || oldstatus == -1) && newStatus == 0)
  {
    client.publish(topic,"true", true);    
    return newStatus;
  }
  else if((newStatus != oldstatus || oldstatus == -1) && newStatus == 1)
  {
    client.publish(topic,"false", true);
    return newStatus;
  }else {
    return oldstatus;
  };
  return newStatus;
  
}

void postSensorValues ()
{
  float humidity =  0;
  humidity = dht.getHumidity();                                 // Get humidity value
  float temperature = dht.getTemperature();                     // Get temperature value
  
  if (dht.getStatusString() == "OK") {

    StaticJsonDocument<200> doc;
    //JsonObject& doc = doc.createObject();
    doc["temperature"] = (String)temperature;
    doc["humidity"] = (String)humidity;
    doc["lightlevel"] = (String)getLightLevel();
    //root.prettyPrintTo(Serial);
    
    char data[200];
    //doc.printTo(data, measureJson(doc) + 1);
    serializeJson(doc, data);
    client.publish(_MQTT_SENSOR_TOPIC, data, true);
    yield();
  } else {
    client.publish(_MQTT_SENSOR_TOPIC, dht.getStatusString(), true);
  }
  
}

// Get light level
float getLightLevel() {
  int sensorValue = analogRead(A0);                               // read the input on analog pin 0
  float lightLevel = (sensorValue * (3.0 / 1023.0))/3.000 *100;   // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V) - Then to percentages
  return lightLevel;
}

void setup() {
   // init the serial
  Serial.begin(115200);
  Serial.print("Setup");
  

  ArduinoOTA.setHostname(_DEVICENAME);
  ArduinoOTA.begin(); 
//
//
  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("INFO: Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(_WIFI_SSID);
  WiFi.begin(_WIFI_SSID, _WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("INFO: WiFi connected");
  Serial.print("INFO: IP address: ");
  Serial.println(WiFi.localIP());
//
//  // init the MQTT connection
//
//  //client.setCallback(callback);
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  
  // Setup pinds
  //----------------------------------------------------------------------------------------------------------------------
  pinMode(PIN_DishWasherLDR, INPUT_PULLUP);
  pinMode(PIN_DishWasherDoor, INPUT_PULLUP);
  pinMode(PIN_WasherDoor, INPUT_PULLUP);
  pinMode(PIN_WasherDoorLDR, INPUT_PULLUP);
  pinMode(PIN_PIR, INPUT_PULLUP);
  pinMode(PIN_Backdoor, INPUT_PULLUP);
  pinMode(PIN_window, INPUT_PULLUP);
  
  
  dht.setup(PIN_DHT);
  
  forceStatusPost();

  // Timer
  //----------------------------------------------------------------------------------------------------------------------
  long CHECKIN_INTERVAL = _CHECKIN_INTERVAL;
  // Confirm satus every two minutes
  timer.setInterval(CHECKIN_INTERVAL, checkIn);
  timer.setInterval(100, readRealTimeSenors);
  timer.setInterval(10000, postSensorValues);


 
}

void loop() {  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();
  timer.run();
}
