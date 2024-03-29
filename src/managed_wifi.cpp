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
#include "log.h"

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
  g_log.write(Log::Debug, "WIFI: Mac address: " + m_macString + " Host: " + m_hostName);

  // Set the hostname
  WiFi.mode(WIFI_STA);
#ifdef ESP32
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(m_hostName.c_str());
  // Set the wifi power level to a low
  esp_wifi_set_max_tx_power(20);
#else
  WiFi.hostname(m_hostName.c_str());
#endif

  // Connect to access point
  manageWiFi();

  // Get the wifi power level.
  int8_t power;
  esp_wifi_get_max_tx_power(&power);
  g_log.write(Log::Debug, "WIFI: Power level: " + String(power) + " RSSI: " + String(WiFi.RSSI()));

  // Submit the hostname to DNS
  MDNS.begin(m_hostName.c_str());
}

// Gets called when WiFiManager enters configuration mode
void ManagedWiFi::configModeCallback(WiFiManager *myWiFiManager)
{
  //if you used auto generated SSID, print it
  g_log.write(Log::Debug, "WIFI: Entered config mode, IP:" + WiFi.softAPIP().toString() + " AP:" + myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  g_led.blinkLed();
}

// Call back for saving the config.
void ManagedWiFi::saveConfigCallback()
{
  g_log.write(Log::Debug, "WIFI: Should save config");
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
  WiFiManagerParameter mqttPrefix("ha_prefix", "HA MQTT Prefix", g_config.getConfig()["ha_prefix"], 20);

  WiFiManager wifiManager;
  wifiManager.setConnectTimeout(60);
  wifiManager.setAPCallback(ManagedWiFi::configModeCallback);
  wifiManager.setConfigPortalTimeout(60);
  wifiManager.setTimeout(120);
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.addParameter(&customMqttServer);
  wifiManager.addParameter(&customMqttPort);
  wifiManager.addParameter(&customMqttUser);
  wifiManager.addParameter(&customMqttPass);
  wifiManager.addParameter(&mqttPrefix);
  wifiManager.setSaveConfigCallback(ManagedWiFi::saveConfigCallback);

  if (reset_config)
  {
    if (!wifiManager.startConfigPortal(CLIENT))
      ESP.restart();
  }
  else
  {
    if (!wifiManager.autoConnect(CLIENT)) // This will hold the connection till it get a connection
      ESP.restart();
  }

  if (m_gotTheConfig)
  {
    g_config.getConfig()["mqtt_server"] = customMqttServer.getValue();
    g_config.getConfig()["mqtt_port"] = customMqttPort.getValue();
    g_config.getConfig()["mqtt_user"] = customMqttUser.getValue();
    g_config.getConfig()["mqtt_pass"] = customMqttPass.getValue();
    g_config.getConfig()["ha_prefix"] = mqttPrefix.getValue();

    g_log.write(Log::Debug, "WIFI: mqtt server: " + String(g_config.getConfig()["mqtt_server"].as<const char *>()));
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
  return (WiFi.status() == WL_CONNECTED);
}

void ManagedWiFi::reset()
{
  manageWiFi(true);
}

ManagedWiFi g_managedWiFi;
