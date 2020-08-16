#include "ccs811.h"
#include "debug.h"
#include "mqtt.h"
#include "managed_wifi.h"
#include <Arduino.h>
#include <ArduinoJson.h>

void CCS811::begin()
{
    snprintf(m_topicMQTTHeader, 50, "%s/sensor/%s", MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX, g_managedWiFi.getHostName().c_str());

    // Setup cc811 sensor
    pinMode(CCS811_WAKE, OUTPUT);
    digitalWrite(CCS811_WAKE, LOW);
    if (!ccs.begin(0x5A))
        DEBUG_PRINTLN("Failed to start CCS811 sensor! Please check your wiring.");
    else
        ccs.setTempOffset(14.0);
    digitalWrite(CCS811_WAKE, HIGH);
}

void CCS811::loop()
{
    unsigned long currentMillis = millis(); // Time now
    // Publish auto discovery home assistant config. This is only needed for very first initialization.
    if (!m_publishConfig && g_mqtt.connected())
    {
        co2MqttAnnounce();
        vocMqttAnnounce();
        tempMqttAnnounce();
        m_publishConfig = true;
    }

    // Publish data every 30 secs
    if (g_mqtt.connected())
    {
        static unsigned long sensorDataPublishPeriod;
        if (currentMillis - sensorDataPublishPeriod >= 30000)
        {
            sensorDataPublishPeriod = currentMillis;
            publish();
        }
    }
}

void CCS811::publish()
{
    digitalWrite(CCS811_WAKE, LOW);
    if (ccs.available() && !ccs.readData())
    {
        char statusTopic[80];
        snprintf(statusTopic, 80, "%s/state", m_topicMQTTHeader);

        StaticJsonDocument<100> json;
        json["co2"] = ccs.geteCO2();
        json["tvoc"] = ccs.getTVOC();
        json["temp"] = ccs.calculateTemperature();

        char outgoingJsonBuffer[100];
        serializeJson(json, outgoingJsonBuffer);
        g_mqtt.publishToMQTT(statusTopic, outgoingJsonBuffer);
    }
    else
    {
        DEBUG_PRINTLN("Cannot publish, not connected to MQTT server.");
    }
    digitalWrite(CCS811_WAKE, HIGH);
}

void CCS811::co2MqttAnnounce() const
{
    char statusDiscoverTopic[80];
    snprintf(statusDiscoverTopic, 80, "%s/co2/config", m_topicMQTTHeader);

    char id[80];
    snprintf(id, 80, "co2%s", g_mqtt.getUniqueId());

    StaticJsonDocument<500> root;
    root["~"] = m_topicMQTTHeader;
    root["uniq_id"] = id;
    root["name"] = "co2";
    root["avty_t"] = g_mqtt.getAvailabilityTopic();
    root["stat_t"] = "~/state";
    root["unit_of_meas"] = "ppm";
    root["val_tpl"] = "{{value_json.co2}}";
    root["device"]["ids"] = g_mqtt.getUniqueId();
    root["device"]["name"] = g_managedWiFi.getHostName().c_str();
    root["device"]["mf"] = "DIY";
    root["device"]["mdl"] = "DIY";
    root["device"]["sw"] = "1.1";
    char outgoingJsonBuffer[500];
    serializeJson(root, outgoingJsonBuffer);
    g_mqtt.publishToMQTT(statusDiscoverTopic, outgoingJsonBuffer);
}

void CCS811::vocMqttAnnounce() const
{
    char statusDiscoverTopic[80];
    snprintf(statusDiscoverTopic, 80, "%s/voc/config", m_topicMQTTHeader);

    char id[80];
    snprintf(id, 80, "voc%s", g_mqtt.getUniqueId());

    StaticJsonDocument<500> root;
    root["~"] = m_topicMQTTHeader;
    root["uniq_id"] = id;
    root["name"] = "voc";
    root["avty_t"] = g_mqtt.getAvailabilityTopic();
    root["stat_t"] = "~/state";
    root["unit_of_meas"] = "ppb";
    root["val_tpl"] = "{{value_json.tvoc}}";
    root["device"]["ids"] = g_mqtt.getUniqueId();
    root["device"]["name"] = g_managedWiFi.getHostName().c_str();
    root["device"]["mf"] = "DIY";
    root["device"]["mdl"] = "DIY";
    root["device"]["sw"] = "1.1";
    char outgoingJsonBuffer[500];
    serializeJson(root, outgoingJsonBuffer);
    g_mqtt.publishToMQTT(statusDiscoverTopic, outgoingJsonBuffer);
}

void CCS811::tempMqttAnnounce() const
{
    char statusDiscoverTopic[80];
    snprintf(statusDiscoverTopic, 80, "%s/temp/config", m_topicMQTTHeader);

    char id[80];
    snprintf(id, 80, "temp%s", g_mqtt.getUniqueId());

    StaticJsonDocument<500> root;
    root["~"] = m_topicMQTTHeader;
    root["uniq_id"] = id;
    root["name"] = "Temperature";
    root["avty_t"] = g_mqtt.getAvailabilityTopic();
    root["stat_t"] = "~/state";
    root["unit_of_meas"] = "°C";
    root["val_tpl"] = "{{value_json.temp}}";
    root["device"]["ids"] = g_mqtt.getUniqueId();
    root["device"]["name"] = g_managedWiFi.getHostName().c_str();
    root["device"]["mf"] = "DIY";
    root["device"]["mdl"] = "DIY";
    root["device"]["sw"] = "1.1";
    char outgoingJsonBuffer[500];
    serializeJson(root, outgoingJsonBuffer);
    g_mqtt.publishToMQTT(statusDiscoverTopic, outgoingJsonBuffer);
}

CCS811 g_ccs811;