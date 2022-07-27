#ifdef ESP32
#include <SPIFFS.h>
#else
#include <FS.h>
#endif
#include "config.h"
#include "ArduinoJson.h"
#include "log.h"
#include "user_config.h"

Config::Config() : m_jsonConfig(JSON_OBJECT_SIZE(5) + CONFIG_SIZE_LIMIT)
{
  m_jsonConfig["mqtt_server"] = MQTT_BROKER;
  m_jsonConfig["mqtt_port"] = MQTT_PORT;
  m_jsonConfig["mqtt_user"] = MQTT_USERNAME;
  m_jsonConfig["mqtt_pass"] = MQTT_PASSWORD;
  m_jsonConfig["ha_prefix"] = MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX;
}

void Config::begin()
{
  readConfig();
}

// Save the config.
void Config::saveConfig()
{
  m_shouldSaveConfig = true;
  writeToMemory(m_jsonConfig);
}

// Saving the config to SPIFF
void Config::writeToMemory(DynamicJsonDocument &json)
{
  g_log.write(Log::Debug, "CONFIG: Saving config :");

  File configFile = SPIFFS.open("/config.json", "w+");
  if (!configFile)
    g_log.write(Log::Error, "CONFIG: failed to open config file for writing");

  // serializeJson(m_jsonConfig, DEBUG_STREAM);
  serializeJson(json, configFile);
  configFile.close();
  m_shouldSaveConfig = false;
}

// Reading the config from SPIFF
void Config::readConfig()
{
  bool error = false;
  auto defaultConfig = m_jsonConfig;

  // pass in true to format the SPIFFS
  if (SPIFFS.begin())
  {
    if (SPIFFS.exists("/config.json"))
    {
      File configFile = SPIFFS.open("/config.json", "r+");
      if (configFile)
      {
        size_t size = configFile.size();
        if (size > CONFIG_SIZE_LIMIT)
        {
          g_log.write(Log::Error, "CONFIG: Config file bigger than allocated memory!");
          return;
        }
        // Read the config file. Make sure it matches with expectations. Otherwise wipe it.
        m_jsonConfig.clear();
        DeserializationError errorMsg = deserializeJson(m_jsonConfig, configFile);
        if (!errorMsg && defaultConfig.size() == m_jsonConfig.size())
        {
          char outgoingJsonBuffer[500];
          serializeJson(m_jsonConfig, outgoingJsonBuffer);
          g_log.write(Log::Debug, "CONFIG: Parsed json config: " + String(outgoingJsonBuffer));
        }
        else
        {
          g_log.write(Log::Debug, "CONFIG: Failed to load json config: " + String(errorMsg.c_str()));
          error = true;
        }
      }
      else
      {
        g_log.write(Log::Debug, "CONFIG: Failed to open config.json. Creating new one..");
        error = true;
      }
      configFile.close();
    }
    else
    {
      g_log.write(Log::Debug, "CONFIG: config.json missing. Creating new one..");
      error = true;
    }
  }
  else
  {
    g_log.write(Log::Warn, "CONFIG: SPIFFS Mount failed. Formatting SPIFFS..");
    error = true;
  }

  // Format and rewrite the default config file
  if (error)
  {
    SPIFFS.format();
    writeToMemory(defaultConfig);
    ESP.restart();
  }
}

// Get config
DynamicJsonDocument &Config::getConfig()
{
  return m_jsonConfig;
}

Config g_config;