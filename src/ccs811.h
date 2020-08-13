#ifndef ccs811_h
#define ccs811_h

#include "user_config.h"
#include <Adafruit_CCS811.h>

class CCS811
{
public:
    CCS811() : m_notFound(true), m_publishConfig(false){};
    // Execute anything that belong in setup ().
    void begin();
    // logic that needs to run with loop.
    void loop();
    // Publish sensor data to mqtt server
    void publish();

private:
    // Announce home assistant autodiscovery config.
    void mqttAnnounce();
    void co2MqttAnnounce();
    void vocMqttAnnounce();
    void tempMqttAnnounce();

    bool m_notFound;
    char m_topicMQTTHeader[50];
    bool m_publishConfig;
    Adafruit_CCS811 ccs;
};

extern CCS811 g_ccs811;

#endif