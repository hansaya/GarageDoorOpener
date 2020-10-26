#ifndef ccs811_h
#define ccs811_h

#include "user_config.h"
#include <Adafruit_CCS811.h>
#include <Ticker.h>

class CCS811
{
public:
    CCS811() : m_notFound(true), m_error(false), m_collectData(true), m_enable(false){};
    // Execute anything that belong in setup ().
    void begin();
    // logic that needs to run with loop.
    void loop();
    // Start collecting data.
    void startCollectingData();
    
private:
    // Publish sensor data to mqtt server
    void publish();
    // Announce home assistant autodiscovery config.
    void mqttAnnounce(String name, String unit, String id) const;
    // Reset the sensor.
    void reset();
    // Put the sensor into sleep.
    void sleep() const;
    // Wake up the sensor.
    void wake() const;

    bool m_notFound;
    char m_topicMQTTHeader[50];
    Adafruit_CCS811 m_ccs;
    bool m_error;
    bool m_collectData;
    bool m_enable;
    Ticker m_pollingTicker;
    Ticker m_initTicker;
};

extern CCS811 g_ccs811;

#endif