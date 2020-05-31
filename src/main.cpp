#include <SmartThingsESP8266WiFi.h>
#include <Everything.h> //Master Brain of ST_Anything library that ties everything together and performs ST Shield communications
#include <S_TimedRelay.h> //Implements a Sensor to control a digital output pin with timing capabilities

#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define BUILTIN_LED 16

#define PIN_TIMEDRELAY_1 14 //SmartThings Capability "Relay Switch"
#define PIN_TIMEDRELAY_2 12 //SmartThings Capability "Relay Switch"
#define PIN_TIMEDRELAY_3 13 //SmartThings Capability "Relay Switch"

const unsigned int serverPort = 8090; // port to run the http server on

// Smartthings / Hubitat Hub TCP/IP Address
IPAddress hubIp(172,168,1,25);    // smartthings/hubitat hub ip //  <---You must edit this line!

// SmartThings / Hubitat Hub TCP/IP Address: UNCOMMENT line that corresponds to your hub, COMMENT the other
const unsigned int hubPort = 39500;   // smartthings hub port

// Manages wifi portal
void ManageWifi (bool reset_config = false)
{
  WiFiManager wifiManager;
  wifiManager.setConnectTimeout(60);

  wifiManager.setConfigPortalTimeout(60);
  wifiManager.setMinimumSignalQuality(10);
  
  if (reset_config)
    wifiManager.startConfigPortal("Garage Door Opener");
  else
    wifiManager.autoConnect("St_anything"); // This will hold the connection till it get a connection
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  byte mac[7];
  WiFi.macAddress(mac);
  char hostname [20];
  snprintf(hostname, 20, "GarageDoor_%02X%02X", mac[4], mac[5]);
  MDNS.begin(hostname);
  WiFi.hostname(hostname);
  wifi_station_set_hostname(hostname);

  // Connect to access point
  ManageWifi ();
  
  //Special sensors/executors (uses portions of both polling and executor classes)
  static st::S_TimedRelay sensor1(F("relaySwitch1"), PIN_TIMEDRELAY_1, LOW, false, 400, 0, 1);
//  static st::S_TimedRelay sensor2(F("relaySwitch2"), PIN_TIMEDRELAY_2, LOW, false, 400, 0, 1);
  static st::S_TimedRelay sensor3(F("relaySwitch3"), PIN_TIMEDRELAY_3, LOW, false, 400, 0, 1);

  st::Everything::debug=true;
  st::Executor::debug=true;
  st::Device::debug=true;

  st::Everything::SmartThing = new st::SmartThingsESP8266WiFi(serverPort, hubIp, hubPort, st::receiveSmartString);
  st::Everything::init();
  
  st::Everything::addSensor(&sensor1);
//  st::Everything::addSensor(&sensor2);
  st::Everything::addSensor(&sensor3); 
        
  st::Everything::initDevices();

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  
  ArduinoOTA.begin();  
}

void loop()
{
  ArduinoOTA.handle();
  st::Everything::run();
}