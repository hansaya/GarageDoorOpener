#include <SPIFFS.h>
#include "config.h"
#include "ArduinoJson.h"
#include "debug.h"

Config::Config ():
  m_jsonConfig (256)
{
  m_jsonConfig["mqtt_server"] = MQTT_BROKER;
  m_jsonConfig["mqtt_port"] = MQTT_PORT;
  m_jsonConfig["mqtt_user"] = MQTT_USERNAME;
  m_jsonConfig["mqtt_pass"] = MQTT_PASSWORD;
}

void Config::begin ()
{
  readConfig ();
}

// Save the config.
void Config::saveConfig ()
{
  m_shouldSaveConfig = true;
  writeToMemory ();
}

// Saving the config to SPIFF
void Config::writeToMemory () {
  DEBUG_PRINT("saving config :");

  File configFile = SPIFFS.open("/config.json", "w+");
  if (!configFile) {
    DEBUG_PRINTLN("failed to open config file for writing");
  }

  serializeJson(m_jsonConfig, Serial);
  DEBUG_PRINTLN ();
  serializeJson(m_jsonConfig, configFile);
  configFile.close();
  m_shouldSaveConfig = false;
}

// Reading the config from SPIFF
void Config::readConfig ()
{
  // pass in true to format the SPIFFS
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      File configFile = SPIFFS.open("/config.json", "r+");
      if (configFile) {
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(size+1);
        DeserializationError error = deserializeJson(json, buf.get());
        if (!error) {
          DEBUG_PRINT("parsed json config: ");
          serializeJson(json, Serial);
          DEBUG_PRINTLN ();
          m_jsonConfig = json;
        } else {
          DEBUG_PRINTLN("failed to load json config");
          writeToMemory ();
        }
      }
    }
  } 
  else {
    DEBUG_PRINTLN ("SPIFFS Mount failed. Formatting SPIFFS..");
    SPIFFS.begin(true);
    ESP.restart();
  }
}

// Get config
DynamicJsonDocument& Config::getConfig ()
{
  return m_jsonConfig;
}

Config g_config;