#include "ccs811.h"
#include "log.h"
#include "mqtt.h"
#include "managed_wifi.h"
#include "led.h"
#include "config.h"
#include <Arduino.h>
#include <ArduinoJson.h>

#define SENSOR_NAME "ccs811"
#define SENSOR_POLE_TIME 100

void CCS811::begin()
{
    snprintf(m_topicMQTTHeader, 50, "%s/sensor/%s", g_config.getConfig()["ha_prefix"].as<const char *>(), g_managedWiFi.getHostName().c_str());

    // Setup cc811 sensor
    pinMode(CCS811_WAKE, OUTPUT);
    pinMode(CCS811_RST, OUTPUT);
    digitalWrite(CCS811_WAKE, LOW);
    digitalWrite(CCS811_RST, HIGH);

    if (!m_ccs.begin(CCS811_ADD))
    {
        g_log.write(Log::Error, "CCS811: Failed to start CCS811 sensor! Reseting the sensor..");
        reset();
        if (!m_ccs.begin(CCS811_ADD))
        {
            g_log.write(Log::Error, "CCS811: Failed to start CCS811 sensor! Please check your wiring.");
            m_enable = false;
            return;
        }
    }

    m_ccs.setTempOffset(14.0);
    m_ccs.setDriveMode(CCS811_DRIVE_MODE_10SEC);
    m_ccs.disableInterrupt();

    // Let sensor wake up to have accurate resutls.
    m_initTicker.once<CCS811 *>(
        60, [](CCS811 *ccs811) {
            ccs811->m_enable = true;
        },
        this);

    // Publish auto discovery home assistant config. This is only needed for very first initialization.
    g_mqtt.publishConfig([this]() {
        mqttAnnounce("Garage Opener CO2", "ppm", "co2");
        mqttAnnounce("Garage Opener Temperature", "°C", "temp");
        mqttAnnounce("Garage Opener VOC", "ppb", "tvoc");
    });
}

void CCS811::loop()
{
    // If there is a critical error skip doing anything.
    if (!m_enable)
        return;

    unsigned long currentMillis = millis(); // Time now

    // Publish data every 60 secs
    static unsigned long sensorDataPublishPeriod;
    if (currentMillis - sensorDataPublishPeriod >= 60 * 1000)
    {
        sensorDataPublishPeriod = currentMillis;
        // If there is non-critical error, reset the sensor
        if (m_error)
        {
            if (m_ccs.checkError())
                g_log.write(Log::Error, "CCS811: CCS811 internal error! Reseting the sensor..");
            else
                g_log.write(Log::Error, "CCS811: CCS811 not working properly! Reseting the sensor..");

            g_led.setPixColor(CRGB::Red);
            g_led.showPixColor();
            reset();
            return;
        }
        startCollectingData();
    }

    // Start collecting the sensor data.
    if (m_collectData)
    {
        m_collectData = false;
        publish();
        sleep();
    }
}

void CCS811::startCollectingData()
{
    // Wake the sensor before requesting data/
    wake();

    // Check every 100ms for availability of the data.
    m_pollingTicker.attach_ms<CCS811 *>(
        SENSOR_POLE_TIME, [](CCS811 *ccs811) {
            static short count;
            // Error out after 15 seconds.
            if (count++ > (15 * 1000) / SENSOR_POLE_TIME)
            {
                count = 0;
                ccs811->m_error = true;
                ccs811->m_pollingTicker.detach();
            }

            if (ccs811->m_ccs.available())
            {
                count = 0;
                ccs811->m_collectData = true;
                ccs811->m_pollingTicker.detach();
                return;
            }
        },
        this);
}

void CCS811::publish()
{
    if (!m_ccs.checkError() && !m_ccs.readData())
    {
        char statusTopic[80];
        snprintf(statusTopic, 80, "%s/" SENSOR_NAME "/state", m_topicMQTTHeader);

        StaticJsonDocument<100> json;
        json["co2"] = m_ccs.geteCO2();
        json["tvoc"] = m_ccs.getTVOC();
        json["temp"] = m_ccs.calculateTemperature();

        char outgoingJsonBuffer[100];
        serializeJson(json, outgoingJsonBuffer);
        g_mqtt.publishToMQTT(statusTopic, outgoingJsonBuffer);
    }
    else
        m_error = true;
}

void CCS811::mqttAnnounce(const String &name, const String &unit, const String &id) const
{
    // Generate a unique id for hassio to keep track.
    char uniqId[80];
    snprintf(uniqId, 80, SENSOR_NAME "%s%s", id.c_str(), g_mqtt.getUniqueId());

    // Topic to push sensor config. Path needs to be unique.
    char statusDiscoverTopic[80];
    snprintf(statusDiscoverTopic, 80, "%s/%s/config", m_topicMQTTHeader, uniqId);

    char val[80];
    snprintf(val, 80, "{{value_json.%s}}", id.c_str());

    StaticJsonDocument<500> root;
    root["~"] = m_topicMQTTHeader;
    root["uniq_id"] = uniqId;
    root["name"] = name;
    root["avty_t"] = g_mqtt.getAvailabilityTopic();
    root["stat_t"] = "~/" SENSOR_NAME "/state";
    root["unit_of_meas"] = unit;
    root["val_tpl"] = val;
    root["qos"] = MQTT_QOS;
    root["device"]["ids"] = g_mqtt.getUniqueId();
    root["device"]["name"] = g_managedWiFi.getHostName();
    root["device"]["mf"] = "DIY";
    root["device"]["mdl"] = "DIY";
    root["device"]["sw"] = "1.0";
    char outgoingJsonBuffer[500];
    serializeJson(root, outgoingJsonBuffer);
    g_mqtt.publishToMQTT(statusDiscoverTopic, outgoingJsonBuffer);
}

void CCS811::reset ()
{
    digitalWrite(CCS811_RST, LOW);
    // Enable the sensor after a second.
    m_initTicker.once<CCS811 *>(
        1, [](CCS811 *sensor) {
            digitalWrite(CCS811_RST, HIGH);
            sensor->m_ccs.SWReset();
            sensor->m_ccs.begin(CCS811_ADD);
            sensor->m_error = false;
        },
        this);
}

void CCS811::wake() const
{
    digitalWrite(CCS811_WAKE, LOW);
}

void CCS811::sleep() const
{
    digitalWrite(CCS811_WAKE, HIGH);
}

CCS811 g_ccs811;