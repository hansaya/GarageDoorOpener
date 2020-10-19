#include "mqtt.h"
#include "user_config.h"
#include "managed_wifi.h"
#include "config.h"
#include "log.h"
#include "led.h"

Mqtt::Mqtt()
    : m_espClient(),
      m_client(m_espClient),
      m_hassioAlive(false),
      m_error(false)
{
}

void Mqtt::begin()
{
  snprintf(m_uniqueId, 20, "%02X%02X", g_managedWiFi.getMac()[4], g_managedWiFi.getMac()[5]);
  snprintf(m_topicMQTTHeader, 50, "%s/cover/%s", MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX, g_managedWiFi.getHostName().c_str());
  snprintf(m_availHeader, 50, "%s/%s/avail", MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX, g_managedWiFi.getHostName().c_str());

  // Set the random seed for client id.
  randomSeed(analogRead(0));

  // Set the server settings
  m_client.setServer(g_config.getConfig()["mqtt_server"].as<const char *>(), atoi(g_config.getConfig()["mqtt_port"].as<const char *>()));

  // Call back for MQTT subs
  m_client.setCallback([this](char *topic, byte *payload, unsigned int length) {
    this->mqttCalllBack(topic, payload, length);
  });

  // Set the buffer size for json payload
  m_client.setBufferSize(812);

  if (g_managedWiFi.connected())
    connect();

  // Keep track of home assistant
  subscribe("homeassistant/status", [this](String payload) {
    g_log.write(Log::Debug, payload);
    if (payload == "online")
    {
      // If home assistant changed the status, prepare to connect again.
      if(!this->m_hassioAlive)
        m_error = true;
      this->m_hassioAlive = true;
    }
    else if (payload == "offline")
    {
      this->m_hassioAlive = false;
    }
  });

  // If self will message goes offline, reconnect to the MQTT server.
  subscribe(m_availHeader, [this](String payload) {
    g_log.write(Log::Debug, payload);
    if (payload == "offline")
      m_error = true;
  });

  // Ignore any errors at start
  m_error = false;
}

void Mqtt::loop()
{
  unsigned long currentMillis = millis(); // Time now
  // Connect to MQTT server
  if ((m_error || !m_client.connected()) && g_managedWiFi.connected())
  {
    static unsigned long mqttConnectWaitPeriod;
    if (currentMillis - mqttConnectWaitPeriod >= 30000)
    {
      mqttConnectWaitPeriod = currentMillis;
      connect();
      m_error = false;
    }
  }

  // // Publish availability every 10 minutes
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
  return m_client.connected() && m_hassioAlive;
}

// Function that publishes birthMessage
void Mqtt::publishBirthMessage()
{
  // Publish the birthMessage
  g_log.write(Log::Debug, "Publishing birth message \"");
  publishToMQTT(m_availHeader, "online", true);
}

// Callback when MQTT message is received; calls triggerDoorAction(), passing topic and payload as parameters
void Mqtt::mqttCalllBack(char *topic, byte *payload, unsigned int length)
{
  g_log.write(Log::Debug, "Message arrived [" + String (topic) + "] ");
  for (unsigned int i = 0; i < length; i++)
  {
    g_log.write(Log::Debug, String ((char)payload[i]));
  }

  String topicToProcess = topic;
  payload[length] = '\0';
  String payloadToProcess = (char *)payload;

  // Call the call backs if the topic matches.
  for (int i = 0; i < m_subTopicCnt; i++)
  {
    if (m_topics[i] == topicToProcess && m_callBacks[i])
      m_callBacks[i](payloadToProcess);
  }
}

void Mqtt::connect()
{
  g_log.write(Log::Debug, "Connecting to MQTT with client id " + String(m_uniqueId) + "...");
  m_client.disconnect();

  // Attempt to connect
  if (m_client.connect(m_uniqueId, g_config.getConfig()["mqtt_user"], g_config.getConfig()["mqtt_pass"], m_availHeader, MQTT_QOS, true, "offline"))
  {
    // Subscribe to the topics.
    for (int i = 0; i < m_subTopicCnt; i++)
      subscribe(m_topics[i].c_str());
    publishBirthMessage();
  }
  else
  {
    g_log.write(Log::Error, "Failed to connect to MQTT Server! " + String(m_client.state()) + ",Trying again in 30 seconds");
  }
}

// Publish the MQTT payload.
void Mqtt::publishToMQTT(const char *p_topic, const char *p_payload, bool retained)
{
  if (m_client.publish(p_topic, p_payload, retained))
  {
    String message = "MQTT message published successfully, topic: " + String(p_topic) + ", payload: " + String(p_payload);
    g_log.write(Log::Debug, message);
    g_led.doubleFastBlink();
  }
  else
  {
    String message = "MQTT message not published, either connection lost, or message too large. Topic: " + String(p_topic) + " , payload: " + String(p_payload);
    g_log.write(Log::Error, message);
  }
}

void Mqtt::subscribe(String topic, TOPIC_CALLBACK_SIGNATURE callback)
{
  if (m_client.connected())
    subscribe(topic.c_str());
  m_topics[m_subTopicCnt] = topic;
  m_callBacks[m_subTopicCnt] = callback;
  m_subTopicCnt++;
}

void Mqtt::subscribe(const char *topic)
{
  g_log.write(Log::Debug, "Subscribing to " + String(topic) + "...");
  m_client.subscribe(topic, MQTT_QOS);
}

Mqtt g_mqtt;