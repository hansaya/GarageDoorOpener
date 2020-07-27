#include "garage_door.h"
#include "managed_wifi.h"
#include "mqtt.h"
#include <ArduinoJson.h>

// void topicCallback(String payload)
// {
//     g_mqtt.mqttCalllBack (topic, payload, length);
// }

GarageDoor::GarageDoor (String identification, uint16_t relayPin, uint16_t statusPin) 
    : m_id (identification),
    m_relayPin (relayPin),
    m_statusPin (statusPin),
    m_publishConfig (false)
{
}

void GarageDoor::loop ()
{
    if (!m_publishConfig && g_mqtt.connected ())
    {
        mqttAnnounce ();
        m_publishConfig = true;
    }
}


void GarageDoor::begin ()
{
    pinMode(m_relayPin, OUTPUT);
    pinMode(m_statusPin, INPUT_PULLUP);

    snprintf(m_topicMQTTHeader, 50, "%s/cover/%s", MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX, g_managedWiFi.getHostName ().c_str ());
    char cmdTopic[80];
    snprintf(cmdTopic, 80, "%s/%s/cmd", m_topicMQTTHeader, m_id.c_str ());

    g_mqtt.addCallBack (cmdTopic, [](String payload) {
        DEBUG_PRINTLN(payload);
    });
}

void GarageDoor::open ()
{
}

void GarageDoor::mqttAnnounce ()
{
    char statusDiscoverTopic[80];
    snprintf(statusDiscoverTopic, 80, "%s/%s/config", m_topicMQTTHeader, m_id.c_str ());

    StaticJsonDocument<500> root;
    root["~"] = m_topicMQTTHeader;
    root["dev_cla"] = "garage_door";
    root["uniq_id"] = m_id;
    root["name"] = m_id + " Status";
    root["avty_t"] = "~/avail";
    root["stat_t"] = "~/" + m_id + "/state";
    root["cmd_t"] = "~/" + m_id + "/cmd";
    root["val_tpl"] = "{{value_json.status}}";
    root["device"]["ids"] = g_mqtt.getUniqueId ();
    root["device"]["name"] = g_managedWiFi.getHostName ().c_str ();
    root["device"]["mf"] = "DIY";
    root["device"]["mdl"] = "DIY";
    root["device"]["sw"] = "1.1";
    char outgoingJsonBuffer[500];
    serializeJson(root, outgoingJsonBuffer);
    g_mqtt.publishToMQTT(statusDiscoverTopic, outgoingJsonBuffer);
}

void GarageDoor::publishStatus ()
{
    char statusTopic[80];
    snprintf(statusTopic, 80, "%s/%s/state", m_topicMQTTHeader, m_id);

    StaticJsonDocument<100> json;
    if (opened ())
        json["status"] = "open";
    else
        json["status"] = "close";

    char outgoingJsonBuffer[100];
    serializeJson(json, outgoingJsonBuffer);
    g_mqtt.publishToMQTT(statusTopic, outgoingJsonBuffer);
}