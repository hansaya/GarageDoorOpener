#include "garage_door.h"
#include "managed_wifi.h"
#include "mqtt.h"
#include <ArduinoJson.h>
#include "log.h"
#include "config.h"

unsigned long GarageDoor::m_lastCall = 0;

GarageDoor::GarageDoor(String identification, uint16_t relayPin, uint16_t statusPin, String name)
    : m_name(name),
      m_id(identification),
      m_relayPin(relayPin),
      m_statusPin(statusPin),
      m_doorOpen(false),
      m_sortInputTrigger({50, false, false}),
      m_sensor(statusPin, ACTIVE_HIGH_DOOR_SENSOR, &m_sortInputTrigger),
      m_event(false)
{
}

void GarageDoor::loop()
{
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
    snprintf(m_topicMQTTHeader, 50, "%s/cover/%s", g_config.getConfig()["ha_prefix"].as<const char *>(), g_managedWiFi.getHostName().c_str());
    pinMode(m_relayPin, OUTPUT);

    m_sensor.begin();

    // Setup a call back function to handle commands.
    char cmdTopic[80];
    snprintf(cmdTopic, 80, "%s/%s/cmd", m_topicMQTTHeader, m_id.c_str());
    g_mqtt.subscribe(cmdTopic, [this](String payload) {
        if (payload == "OPEN")
            open();
        else if (payload == "CLOSE")
            close();
        else if (payload == "STOP")
            publishStatus();
    });

    // Publish auto discovery home assistant config. This is only needed for very first initialization.
    g_mqtt.publishConfig([this]() {
        mqttAnnounce();
    });
}

void GarageDoor::open()
{
    // Sensor only triggered when door is closed so we can use it to trigger if the door is down.
    if (!m_doorOpen)
        buttonPress();
}

void GarageDoor::close()
{
    // Sensor only triggered when door is closed so we can use it not trigger if the door is down.
    if (m_doorOpen)
        buttonPress();
}

bool GarageDoor::opened() const
{
    return m_doorOpen;
}

void GarageDoor::buttonPress()
{
    // Make sure there is a gap between each button press
    if (millis() - m_lastCall > DELAY_BETWEEN_CALLS)
    {
        triggerRelay();
    }
    else
    {
        g_log.write(Log::Debug, "GD: Call put on to a queue.");
        // Wait to press the button again.
        m_queueTicker.once_ms<GarageDoor *>(
            DELAY_BETWEEN_CALLS, [](GarageDoor *door) {
                door->triggerRelay();
            },
            this);
    }
}

void GarageDoor::triggerRelay()
{
    m_lastCall = millis();
    // Turn on the relay and let the ticker turn it off after 400ms
    digitalWrite(m_relayPin, ACTIVE_HIGH_RELAY);
    m_ticker.once_ms<uint16_t>(
        RELAY_LATCH_TIME, [](uint16_t pin) {
            g_log.write(Log::Debug, "GD: Relay triggered for 400ms");
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
    root["qos"] = MQTT_QOS;
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