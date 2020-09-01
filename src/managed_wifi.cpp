#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#endif
#include "managed_wifi.h"
#include "config.h"
#include "led.h"
#include "debug.h"

bool ManagedWiFi::m_gotTheConfig = false;

ManagedWiFi::ManagedWiFi()
    : m_hostName("NA"),
      m_macString("NA")
{
  // Set the host name
  WiFi.macAddress(m_mac);
  m_mac[6] = '\0';
  char hostname[24];
  snprintf(hostname, 24, "%s_%02X%02X", CLIENT, m_mac[4], m_mac[5]);
  m_hostName = hostname;
  char macAddress[24] = {'\0'};
  snprintf(macAddress, 24, "%02X%02X%02X%02X%02X%02X", m_mac[0], m_mac[1], m_mac[2], m_mac[3], m_mac[4], m_mac[5]);
  m_macString = macAddress;
}

void ManagedWiFi::begin()
{
  DEBUG_PRINT("Mac address: ");
  DEBUG_PRINT(m_macString);
  DEBUG_PRINT(" Host: ");
  DEBUG_PRINTLN(m_hostName);

  // Connect to access point
  manageWiFi();

  // Submit the hostname to DNS
  MDNS.begin(m_hostName.c_str());
#ifdef ESP32
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(m_hostName.c_str());
#else
  WiFi.hostname(m_hostName);
#endif
  WiFi.mode(WIFI_STA);
}

// Gets called when WiFiManager enters configuration mode
void ManagedWiFi::configModeCallback(WiFiManager *myWiFiManager)
{
  DEBUG_PRINTLN("Entered config mode");
  DEBUG_PRINTLN(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  DEBUG_PRINTLN(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  g_led.blinkLed();
}

// Call back for saving the config.
void ManagedWiFi::saveConfigCallback()
{
  DEBUG_PRINTLN("Should save config");
  m_gotTheConfig = true;
}

// Manages wifi portal
void ManagedWiFi::manageWiFi(const bool reset_config)
{
  // The extra parameters to be configured (can be either global or just in the setup)
  WiFiManagerParameter customMqttServer("server", "MQTT server", g_config.getConfig()["mqtt_server"], 40);
  WiFiManagerParameter customMqttPort("port", "MQTT port", g_config.getConfig()["mqtt_port"], 6);
  WiFiManagerParameter customMqttUser("user", "MQTT user", g_config.getConfig()["mqtt_user"], 20);
  WiFiManagerParameter customMqttPass("pass", "MQTT pass", g_config.getConfig()["mqtt_pass"], 20);

  WiFiManager wifiManager;
  wifiManager.setConnectTimeout(60);
  wifiManager.setAPCallback(ManagedWiFi::configModeCallback);
  wifiManager.setConfigPortalTimeout(60);
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.addParameter(&customMqttServer);
  wifiManager.addParameter(&customMqttPort);
  wifiManager.addParameter(&customMqttUser);
  wifiManager.addParameter(&customMqttPass);
  wifiManager.setSaveConfigCallback(ManagedWiFi::saveConfigCallback);

  if (reset_config)
    wifiManager.startConfigPortal(CLIENT);
  else
    wifiManager.autoConnect(CLIENT); // This will hold the connection till it get a connection

  if (m_gotTheConfig)
  {
    g_config.getConfig()["mqtt_server"] = customMqttServer.getValue();
    g_config.getConfig()["mqtt_port"] = customMqttPort.getValue();
    g_config.getConfig()["mqtt_user"] = customMqttUser.getValue();
    g_config.getConfig()["mqtt_pass"] = customMqttPass.getValue();

    DEBUG_PRINT("mqtt server: ");
    DEBUG_PRINTLN(g_config.getConfig()["mqtt_server"].as<const char *>());
    g_config.saveConfig();
    ESP.restart();
  }

  g_led.stopBlinkLed();
}

void ManagedWiFi::loop()
{
  if (!connected())
  {
    manageWiFi(true);
  }
}

String ManagedWiFi::getHostName() const
{
  return m_hostName;
}

String ManagedWiFi::getMacStr() const
{
  return m_macString;
}

byte *ManagedWiFi::getMac()
{
  return m_mac;
}

bool ManagedWiFi::connected() const
{
  return WiFi.isConnected();
}

ManagedWiFi g_managedWiFi;