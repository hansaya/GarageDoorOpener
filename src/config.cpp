#include <SPIFFS.h>
#include "config.h"
#include "ArduinoJson.h"

// char* publisher::g_mqttServer;
// char* publisher::g_mqttPort;
// char* publisher::g_mqttSleepPeriod;
// char* publisher::g_mqttSleepEn;
DynamicJsonDocument Config::m_jsonConfig (256);

void Config::begin ()
{
  m_jsonConfig["mqtt_server"] = MQTT_BROKER;
  m_jsonConfig["mqtt_port"] = MQTT_PORT;
  readConfig ();
}

// Save the config.
void Config::saveConfig ()
{
  m_shouldSaveConfig = true;
}

// Saving the config to SPIFF
void Config::writeToMemory () {
  Serial.print("saving config :");

  File configFile = SPIFFS.open("/config.json", "w+");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  serializeJson(m_jsonConfig, Serial);
  Serial.println ();
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
          Serial.print("parsed json config: ");
          serializeJson(json, Serial);
          Serial.println ();
          m_jsonConfig = json;
        } else {
          Serial.println("failed to load json config");
          writeToMemory ();
        }
      }
    }
  } else {
    Serial.println ("SPIFFS Mount failed. Formatting SPIFFS..");
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