#ifndef configuration_h
#define configuration_h

#define CONFIG_SIZE_LIMIT 200

#include <Arduino.h>
#include <Ticker.h>
#include "ArduinoJson.h"

class Config
{
public:
    Config();
    // Execute anything that belong in setup ()
    void begin();
    // Save the config in flash.
    void saveConfig();
    // Read the config file from from the flash.
    void readConfig();
    // Get the json config file.
    DynamicJsonDocument &getConfig();

private:
    // Write the config file to the memory.
    void writeToMemory();

    DynamicJsonDocument m_jsonConfig;
    bool m_shouldSaveConfig;
};

extern Config g_config;

#endif