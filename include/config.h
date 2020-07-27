#ifndef configuration_h
#define configuration_h

#include <Arduino.h>
#include <Ticker.h>
#include "user_config.h"
#include "WiFiManager.h"
#include "ArduinoJson.h"

class Config
{
public:
    Config () {}
    void begin ();
    void saveConfig ();
    void readConfig ();
    void writeToMemory ();
    DynamicJsonDocument& getConfig ();

    // static char *g_mqttServer;
    // static char *g_mqttPort;
    // static char *g_mqttSleepPeriod;
    // static char *g_mqttSleepEn;
private:
    static DynamicJsonDocument m_jsonConfig;
    bool m_shouldSaveConfig;
};

extern Config g_config;

#endif