#include "garage_door.h"
#include "managed_wifi.h"
#include "mqtt.h"
#include <ArduinoJson.h>
#include "debug.h"

GarageDoor::GarageDoor(String identification, uint16_t relayPin, uint16_t statusPin, String name)
    : m_name(name),
      m_id(identification),
      m_relayPin(relayPin),
      m_statusPin(statusPin),
      m_publishConfig(false),
      m_doorOpen(false),
      m_sortInputTrigger({50, false, false}),
      m_sensor(statusPin, ACTIVE_HIGH_DOOR_SENSOR, &m_sortInputTrigger),
      m_event(false)
{
}

void GarageDoor::loop()
{
    // Publish auto discovery home assistant config. This is only needed for very first initialization.
    if (!m_publishConfig && g_mqtt.connected())
    {
        mqttAnnounce();
        m_publishConfig = true;
    }
    // Keep track of the door sensor.
    if (m_sortInputTrigger.pressed == m_doorOpen)
    {
        m_doorOpen = !m_sortInputTrigger.pressed;
        m_event = true;
    }
    // Publish any changes to mqtt server.
    if (m_event && g_mqtt.connected())
    {
        m_event = false;
        publishStatus();
    }
}

void GarageDoor::begin()
{
    snprintf(m_topicMQTTHeader, 50, "%s/cover/%s", MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX, g_managedWiFi.getHostName().c_str());
    pinMode(m_relayPin, OUTPUT);

    m_sensor.begin();

    // Setup a call back function to handle commands.
    char cmdTopic[80];
    snprintf(cmdTopic, 80, "%s/%s/cmd", m_topicMQTTHeader, m_id.c_str());
    g_mqtt.subscribe(cmdTopic, [this](String payload) {
        DEBUG_PRINTLN(payload);
        if (payload == "OPEN")
        {
            this->open();
        }
        else if (payload == "CLOSE")
        {
            this->close();
        }
        else if (payload == "STOP")
        {
            this->publishStatus();
        }
    });
}

void GarageDoor::open()
{
    // Sensor only triggered when door is closed so we can use it to trigger if the door is down.
    if (!m_doorOpen)
        triggerRelay();
}

void GarageDoor::close()
{
    // Sensor only triggered when door is closed so we can use it not trigger if the door is down.
    if (m_doorOpen)
        triggerRelay();
}

bool GarageDoor::opened() const
{
    return m_doorOpen;
}

void GarageDoor::triggerRelay()
{
    // Turn on the relay and let the ticker turn it off after 400ms
    digitalWrite(m_relayPin, ACTIVE_HIGH_RELAY);
    m_ticker.once_ms<uint16_t>(
        400, [](uint16_t pin) {
            DEBUG_PRINTLN("Relay triggered for 400ms");
            digitalWrite(pin, !ACTIVE_HIGH_RELAY);
        },
        m_relayPin);
}

void GarageDoor::mqttAnnounce() const
{
    char statusDiscoverTopic[80];
    snprintf(statusDiscoverTopic, 80, "%s/%s/config", m_topicMQTTHeader, m_id.c_str());

    StaticJsonDocument<500> root;
    root["~"] = m_topicMQTTHeader;
    root["uniq_id"] = m_id;
    root["name"] = m_name;
    root["avty_t"] = g_mqtt.getAvailabilityTopic();
    root["stat_t"] = "~/" + m_id + "/state";
    root["cmd_t"] = "~/" + m_id + "/cmd";
    root["val_tpl"] = "{{value_json.status}}";
    root["opt"] = "false";
    root["device"]["ids"] = g_mqtt.getUniqueId();
    root["device"]["name"] = g_managedWiFi.getHostName();
    root["device"]["mf"] = "DIY";
    root["device"]["mdl"] = "DIY";
    root["device"]["sw"] = "1.1";
    char outgoingJsonBuffer[500];
    serializeJson(root, outgoingJsonBuffer);
    g_mqtt.publishToMQTT(statusDiscoverTopic, outgoingJsonBuffer);
}

void GarageDoor::publishStatus() const
{
    if (g_mqtt.connected())
    {
        char statusTopic[80];
        snprintf(statusTopic, 80, "%s/%s/state", m_topicMQTTHeader, m_id.c_str());

        StaticJsonDocument<100> json;
        if (m_doorOpen)
            json["status"] = "open";
        else
            json["status"] = "closed";

        char outgoingJsonBuffer[100];
        serializeJson(json, outgoingJsonBuffer);
        g_mqtt.publishToMQTT(statusTopic, outgoingJsonBuffer);
    }
    else
    {
        DEBUG_PRINTLN("Cannot publish, not connected to MQTT server.");
    }
}