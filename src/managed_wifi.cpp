#include <ESPmDNS.h>
#include "managed_wifi.h"
#include "config.h"
#include "led.h"

bool ManagedWiFi::m_gotTheConfig = true;

ManagedWiFi::ManagedWiFi ()
  : m_hostName("NA"),
  m_macString ("NA")
{
  // Set the host name
  WiFi.macAddress(m_mac);
  m_mac[6] = '\0';
  char hostname [24];
  snprintf(hostname, 24, "%s_%02X%02X", CLIENT, m_mac[4], m_mac[5]);
  m_hostName = hostname;
  char macAddress[24] = {'\0'};
  snprintf(macAddress, 24, "%02X%02X%02X%02X%02X%02X", m_mac[0], m_mac[1], m_mac[2], m_mac[3], m_mac[4], m_mac[5]);
  m_macString = macAddress;
}

void ManagedWiFi::begin ()
{
  DEBUG_PRINT("Mac address: ");
  DEBUG_PRINT(m_macString);
  DEBUG_PRINT(" Host: ");
  DEBUG_PRINTLN(m_hostName);

  // Connect to access point
  WiFi.onEvent(ManagedWiFi::event);
  manageWiFi ();

  MDNS.begin(m_hostName.c_str());
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(m_hostName.c_str());
  WiFi.mode(WIFI_STA);
}

// Gets called when WiFiManager enters configuration mode
void ManagedWiFi::configModeCallback (WiFiManager *myWiFiManager) {
  DEBUG_PRINTLN("Entered config mode");
  DEBUG_PRINTLN(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  DEBUG_PRINTLN(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  g_led.BlinkLed ();
}

// Call back for saving the config.
void ManagedWiFi::saveConfigCallback () {
  DEBUG_PRINTLN("Should save config");
  m_gotTheConfig = true;
}

// Manages wifi portal
void ManagedWiFi::manageWiFi (bool reset_config)
{
  // The extra parameters to be configured (can be either global or just in the setup)
  WiFiManagerParameter customMqttServer("server", "MQTT server", g_config.getConfig()["mqtt_server"], 40);
  WiFiManagerParameter customMqttPort("port", "MQTT port", g_config.getConfig()["mqtt_port"], 6);

  WiFiManager wifiManager;
  wifiManager.setConnectTimeout(60);
  wifiManager.setAPCallback(ManagedWiFi::configModeCallback);
  wifiManager.setConfigPortalTimeout(60);
  wifiManager.setMinimumSignalQuality(10);
  wifiManager.addParameter(&customMqttServer);
  wifiManager.addParameter(&customMqttPort);
  wifiManager.setSaveConfigCallback(&saveConfigCallback);
  
  if (reset_config)
    wifiManager.startConfigPortal(CLIENT);
  else
    wifiManager.autoConnect(CLIENT); // This will hold the connection till it get a connection

  if (m_gotTheConfig)
  {
    g_config.getConfig()["mqtt_server"] = customMqttServer.getValue();
    g_config.getConfig()["mqtt_port"] = customMqttPort.getValue();

    DEBUG_PRINT("mqtt server: ");
    DEBUG_PRINTLN(g_config.getConfig()["mqtt_server"].as<const char*>());
    g_config.saveConfig ();
  }

  g_led.StopBlinkLed();
}

// Wifi status check
void ManagedWiFi::event(WiFiEvent_t event) {
    DEBUG_PRINT_WITH_FMT("[WiFi-event] event: %d\n", event);
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
        // g_mqttTicker.attach(2, connectToMqtt);
        // connectToMqtt ();
        break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
        DEBUG_PRINTLN("WiFi lost connection");
        // g_mqttTicker.detach();
        break;
      default:
        break;
    }
}

String ManagedWiFi::getHostName ()
{
  return m_hostName;
}

String ManagedWiFi::getMacStr ()
{
  return m_macString;
}

byte* ManagedWiFi::getMac ()
{
  return m_mac;
}

bool ManagedWiFi::wifiConnected ()
{
  return WiFi.isConnected ();
}

ManagedWiFi g_managedWiFi;