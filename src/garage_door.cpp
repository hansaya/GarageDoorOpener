#include "garage_door.h"
#include "managed_wifi.h"
#include "mqtt.h"
#include <ArduinoJson.h>

GarageDoor::GarageDoor (String identification, uint16_t relayPin, uint16_t statusPin) 
    : m_id (identification),
    m_relayPin (relayPin),
    m_statusPin (statusPin),
    m_publishConfig (false),
    m_doorOpen (false)
{
}

void GarageDoor::loop ()
{
    if (!m_publishConfig && g_mqtt.connected ())
    {
        mqttAnnounce ();
        subscribeToCmd ();
        m_publishConfig = true;
    }
    if (digitalRead (m_statusPin) == HIGH)
    {
        m_doorOpen = true;
    }
}

void GarageDoor::begin ()
{
    pinMode(m_relayPin, OUTPUT);
    pinMode(m_statusPin, INPUT_PULLUP);

    snprintf(m_topicMQTTHeader, 50, "%s/cover/%s", MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX, g_managedWiFi.getHostName ().c_str ());

    // Setup a call back function to handle commands.
    char cmdTopic[80];
    snprintf(cmdTopic, 80, "%s/%s/cmd", m_topicMQTTHeader, m_id.c_str ());
    g_mqtt.addCallBack (cmdTopic, [this](String payload) {
        DEBUG_PRINTLN(payload);
        if (payload == "OPEN")
        {
            this->open ();
        }
        else if(payload == "CLOSE")
        {
            this->close ();
        }
        else if(payload == "STOP")
        {
            this->publishStatus ();
        }
    });
}

void GarageDoor::open ()
{
    DEBUG_PRINT(m_id);
    DEBUG_PRINTLN("opening Lamda");

    // Sensor only triggered when door is closed so we can use it to trigger if the door is down.
    if (!m_doorOpen)
        triggerRelay ();
}

void GarageDoor::close ()
{
    DEBUG_PRINT(m_id);
    DEBUG_PRINTLN("closing Lamda");

    // Sensor only triggered when door is closed so we can use it not trigger if the door is down.
    if (m_doorOpen)
        triggerRelay ();
}

bool GarageDoor::opened ()
{
    return m_doorOpen;
}

void GarageDoor::triggerRelay ()
{
    digitalWrite (m_relayPin, HIGH);
    m_lightTicker.once_ms<uint16_t>(400, [](uint16_t pin) {
        DEBUG_PRINTLN("Relay triggered for 400ms");
        digitalWrite (pin, LOW);
    }, m_relayPin);
}

void GarageDoor::mqttAnnounce ()
{
    char statusDiscoverTopic[80];
    snprintf(statusDiscoverTopic, 80, "%s/%s/config", m_topicMQTTHeader, m_id.c_str ());

    StaticJsonDocument<500> root;
    root["~"] = m_topicMQTTHeader;
    // root["dev_cla"] = "garage_door";
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

void GarageDoor::subscribeToCmd ()
{
    char cmdTopic[80];
    snprintf(cmdTopic, 80, "%s/%s/cmd", m_topicMQTTHeader, m_id.c_str ());
    g_mqtt.subscribe(cmdTopic);
}

void GarageDoor::publishStatus ()
{
    char statusTopic[80];
    snprintf(statusTopic, 80, "%s/%s/state", m_topicMQTTHeader, m_id.c_str ());

    StaticJsonDocument<100> json;
    if (opened ())
        json["status"] = "open";
    else
        json["status"] = "closed";

    char outgoingJsonBuffer[100];
    serializeJson(json, outgoingJsonBuffer);
    g_mqtt.publishToMQTT(statusTopic, outgoingJsonBuffer);
}