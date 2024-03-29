#include "mqtt.h"
#include "user_config.h"
#include "managed_wifi.h"
#include "config.h"
#include "log.h"
#include "led.h"

Mqtt::Mqtt()
    : m_espClient(),
      m_client(m_espClient),
      m_hassioAlive(true), // Expect Hassio to be alive at start up to avoid unecessary recovery steps.
      m_error(false),
      m_connected(false)
{
}

void Mqtt::begin()
{
  snprintf(m_uniqueId, 20, "%02X%02X", g_managedWiFi.getMac()[4], g_managedWiFi.getMac()[5]);
  snprintf(m_topicMQTTHeader, 50, "%s/cover/%s", g_config.getConfig()["ha_prefix"].as<const char *>(), g_managedWiFi.getHostName().c_str());
  snprintf(m_availHeader, 50, "%s/%s/avail", g_config.getConfig()["ha_prefix"].as<const char *>(), g_managedWiFi.getHostName().c_str());

  // Set the random seed for client id.
  randomSeed(analogRead(35));

  // Set the server settings
  m_client.setServer(g_config.getConfig()["mqtt_server"].as<const char *>(), atoi(g_config.getConfig()["mqtt_port"].as<const char *>()));

  // Call back for MQTT subs
  m_client.setCallback([this](char *topic, byte *payload, unsigned int length)
                       { this->mqttCalllBack(topic, payload, length); });

  // Set the buffer size for json payload
  m_client.setBufferSize(812);

  // Keep track of home assistant
  subscribe("homeassistant/status", [this](String payload) {
    if (payload == "online")
    {
      // If home assistant changed the status, prepare to connect again.
      if(!m_hassioAlive)
      {
        m_error = true;
        g_log.write(Log::Warn, "MQTT: Home Assistant Came online! Reconnecting to MQTT..");
      }
      m_hassioAlive = true;
    }
    else if (payload == "offline")
    {
      m_hassioAlive = false;
    } 
  });

  // If self "will" message goes offline, reconnect to the MQTT server.
  subscribe(m_availHeader, [this](String payload) {
    if (payload == "online")
    {
      m_connected = true;
    }
    else if (m_connected && payload == "offline")
    {
      m_connected = false;
      m_error = true;
      g_log.write(Log::Warn, "MQTT: Self \"will\" Message went offline.");
    } 
  });
}

void Mqtt::loop()
{
  unsigned long currentMillis = millis(); // Time now
  // Connect to MQTT server
  if ((m_error || !m_client.connected()) && g_managedWiFi.connected())
  {
    static unsigned long mqttConnectWaitPeriod = 25000;
    if (currentMillis - mqttConnectWaitPeriod >= 30000)
    {
      mqttConnectWaitPeriod = currentMillis;
      connect();
      m_error = false;
      m_configEvent = false;
    }
  }

  // If there is a change in config, resubscribe/publish config
  if (m_configEvent && m_client.connected() && !m_error)
  {
    m_configEvent = false;
    subscribe();
    publishConfig();
  }

  // Publish availability every 10 minutes
  // if (m_client.connected())
  // {
  //   static unsigned long aliveMessageResendPeriod;
  //   if (currentMillis - aliveMessageResendPeriod >= 60 * 10 * 1000)
  //   {
  //     aliveMessageResendPeriod = currentMillis;
  //     publishBirthMessage();
  //   }
  // }
  m_client.loop();
}

const bool Mqtt::connected()
{
  return m_connected && m_client.connected();
}

// Function that publishes birthMessage
void Mqtt::publishBirthMessage()
{
  // Publish the birthMessage
  g_log.write(Log::Debug, "MQTT: Publishing birth message");
  publishToMQTT(m_availHeader, "online", true);
}

// Callback when MQTT message is received; calls triggerDoorAction(), passing topic and payload as parameters
void Mqtt::mqttCalllBack(char *topic, byte *payload, unsigned int length)
{

  String topicToProcess = topic;
  payload[length] = '\0';
  String payloadToProcess = (char *)payload;
  g_log.write(Log::Debug, "MQTT: Message arrived [" + String(topic) + "]:" + payloadToProcess);

  // Call the call backs if the topic matches.
  for (int i = 0; i < m_subTopicCnt; i++)
  {
    if (m_subTopics[i] == topicToProcess && m_subCallBacks[i])
      m_subCallBacks[i](payloadToProcess);
  }
}

void Mqtt::connect()
{
  g_log.write(Log::Debug, "MQTT: Connecting to MQTT with client id " + String(m_uniqueId) + "...");
  m_client.disconnect();
  m_connected = false;

  // Attempt to connect
  if (m_client.connect(m_uniqueId, g_config.getConfig()["mqtt_user"], g_config.getConfig()["mqtt_pass"], m_availHeader, MQTT_QOS, true, "offline"))
  {
    subscribe();
    publishConfig();
    publishBirthMessage();
  }
  else
  {
    g_log.write(Log::Error, "MQTT: Failed to connect to MQTT Server! " + String(m_client.state()) + ",Trying again in 30 seconds");
  }
}

// Publish the MQTT payload.
void Mqtt::publishToMQTT(const char *topic, const char *payload, bool retained)
{
  if (m_client.publish(topic, payload, retained))
  {
    String message = "MQTT: Message published successfully, topic: " + String(topic) + ", payload: " + String(payload);
    g_log.write(Log::Debug, message);
    g_led.doubleFastBlink();
  }
  else
  {
    String message = "MQTT: MQTT message not published, either connection lost, or message too large. Topic: " + String(topic) + " , payload: " + String(payload);
    g_log.write(Log::Error, message);
  }
}

void Mqtt::subscribe(String topic, TOPIC_CALLBACK_SIGNATURE callback)
{
  if (m_client.connected())
    subscribe(topic.c_str());
  m_subTopics[m_subTopicCnt] = topic;
  m_subCallBacks[m_subTopicCnt] = callback;
  m_subTopicCnt++;
  m_configEvent = true;
}

void Mqtt::subscribe(const char *topic)
{
  g_log.write(Log::Debug, "MQTT: Subscribing to " + String(topic) + "...");
  m_client.subscribe(topic, MQTT_QOS);
}

void Mqtt::subscribe()
{
  // Subscribe to the topics.
  for (int i = 0; i < m_subTopicCnt; i++)
    subscribe(m_subTopics[i].c_str());
}

void Mqtt::publishConfig(CONFIG_CALLBACK_SIGNATURE callback)
{
  m_configCallBacks[m_configCnt] = callback;
  m_configCnt++;
  m_configEvent = true;
}

void Mqtt::publishConfig() const
{
  // Publish configs.
  for (int i = 0; i < m_configCnt; i++)
    m_configCallBacks[i]();
}

Mqtt g_mqtt;