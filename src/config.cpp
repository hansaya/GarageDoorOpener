#ifdef ESP32
#include <SPIFFS.h>
#else
#include <FS.h>
#endif
#include "config.h"
#include "ArduinoJson.h"
#include "log.h"
#include "user_config.h"

Config::Config() : m_jsonConfig(JSON_OBJECT_SIZE(4) + CONFIG_SIZE_LIMIT)
{
  m_jsonConfig["mqtt_server"] = MQTT_BROKER;
  m_jsonConfig["mqtt_port"] = MQTT_PORT;
  m_jsonConfig["mqtt_user"] = MQTT_USERNAME;
  m_jsonConfig["mqtt_pass"] = MQTT_PASSWORD;
}

void Config::begin()
{
  readConfig();
}

// Save the config.
void Config::saveConfig()
{
  m_shouldSaveConfig = true;
  writeToMemory();
}

// Saving the config to SPIFF
void Config::writeToMemory()
{
  g_log.write(Log::Debug, "Saving config :");

  File configFile = SPIFFS.open("/config.json", "w+");
  if (!configFile)
    g_log.write(Log::Error, "failed to open config file for writing");

  // serializeJson(m_jsonConfig, DEBUG_STREAM);
  serializeJson(m_jsonConfig, configFile);
  configFile.close();
  m_shouldSaveConfig = false;
}

// Reading the config from SPIFF
void Config::readConfig()
{
  // pass in true to format the SPIFFS
  if (SPIFFS.begin())
  {
    if (SPIFFS.exists("/config.json"))
    {
      //file exists, reading and loading
      File configFile = SPIFFS.open("/config.json", "r+");
      if (configFile)
      {
        size_t size = configFile.size();
        if (size > CONFIG_SIZE_LIMIT)
        {
          g_log.write(Log::Error, "Config file bigger than allocated memory!");
          return;
        }
        // Read the config file
        m_jsonConfig.clear();
        DeserializationError error = deserializeJson(m_jsonConfig, configFile);
        if (!error)
        {
          g_log.write(Log::Debug, "Parsed json config: ");
          // serializeJson(m_jsonConfig, DEBUG_STREAM);
        }
        else
        {
          g_log.write(Log::Debug, "Failed to load json config: " + String (error.c_str()));
          SPIFFS.format();
          writeToMemory();
          ESP.restart();
        }
      }
      configFile.close();
    }
  }
  else
  {
    g_log.write(Log::Warn, "SPIFFS Mount failed. Formatting SPIFFS..");
    SPIFFS.format();
    ESP.restart();
  }
}

// Get config
DynamicJsonDocument &Config::getConfig()
{
  return m_jsonConfig;
}

Config g_config;