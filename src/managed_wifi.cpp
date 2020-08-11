#include <ESPmDNS.h>
#include "managed_wifi.h"
#include "config.h"
#include "led.h"
#include "debug.h"

bool ManagedWiFi::m_gotTheConfig = false;

ManagedWiFi::ManagedWiFi ()
  : m_hostName("NA"),
  m_macString ("NA"),
  m_connected (false)
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

  //Set a call back to handle wifi events.
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
  {
    this->eventCallback (event);
  });

  // Connect to access point
  manageWiFi ();

  // Submit the hostname to DNS
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
  g_led.blinkLed ();
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
    DEBUG_PRINTLN(g_config.getConfig()["mqtt_server"].as<const char*>());
    g_config.saveConfig ();
  }

  g_led.stopBlinkLed();
}

// Wifi status check
void ManagedWiFi::eventCallback(WiFiEvent_t event) {
    DEBUG_PRINT_WITH_FMT("[WiFi-event] event: %d\n", event);
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
        m_connected = true;
        g_led.setPixColor (CRGB::Blue);
        g_led.showPixColor();
        break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
        DEBUG_PRINTLN("WiFi lost connection");
        m_connected = false;
        g_led.setPixColor (CRGB::Red);
        g_led.showPixColor();
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

bool ManagedWiFi::connected ()
{
  return m_connected;
}

ManagedWiFi g_managedWiFi;