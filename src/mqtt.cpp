#include "mqtt.h"
#include "user_config.h"
#include "managed_wifi.h"
#include "config.h"
#include "debug.h"
#include "led.h"

Mqtt::Mqtt()
    : m_espClient(),
      m_client(m_espClient)
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
}

void Mqtt::loop()
{
  unsigned long currentMillis = millis(); // Time now
  // Connect to MQTT server
  if (!m_client.connected() && g_managedWiFi.connected())
  {
    static unsigned long mqttConnectWaitPeriod;
    if (currentMillis - mqttConnectWaitPeriod >= 30000)
    {
      mqttConnectWaitPeriod = currentMillis;
      connect();
    }
  }

  // Publish availability every 10 minutes
  if (m_client.connected())
  {
    static unsigned long aliveMessageResendPeriod;
    if (currentMillis - aliveMessageResendPeriod >= 60 * 10 * 1000)
    {
      aliveMessageResendPeriod = currentMillis;
      publishBirthMessage();
    }
  }
  m_client.loop();
}

const bool Mqtt::connected()
{
  return m_client.connected();
}

// Function that publishes birthMessage
void Mqtt::publishBirthMessage()
{
  // Publish the birthMessage
  DEBUG_PRINT("Publishing birth message \"");
  publishToMQTT(m_availHeader, "online");
}

// Callback when MQTT message is received; calls triggerDoorAction(), passing topic and payload as parameters
void Mqtt::mqttCalllBack(char *topic, byte *payload, unsigned int length)
{
  DEBUG_PRINT("Message arrived [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    DEBUG_PRINT((char)payload[i]);
  }
  DEBUG_PRINTLN();

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
  DEBUG_PRINT("Connecting to MQTT with client id ");
  String clientId = CLIENT;
  clientId += String(random(0xffff), HEX);
  DEBUG_PRINT(clientId);
  DEBUG_PRINTLN("...");

  // Attempt to connect
  if (m_client.connect(clientId.c_str(), g_config.getConfig()["mqtt_user"], g_config.getConfig()["mqtt_pass"], m_availHeader, 0, true, "offline"))
  {
    // Subscribe to the topics.
    for (int i = 0; i < m_subTopicCnt; i++)
      subscribe(m_topics[i].c_str());
    publishBirthMessage();
  }
  else
  {
    DEBUG_PRINT("Failed to connect to MQTT! ");
    DEBUG_PRINT(m_client.state());
    DEBUG_PRINTLN(" Trying again in 30 seconds");
  }
}

// Publish the MQTT payload.
void Mqtt::publishToMQTT(const char *p_topic, const char *p_payload)
{
  if (m_client.publish(p_topic, p_payload, true))
  {
    DEBUG_PRINT(F("INFO: MQTT message published successfully, topic: "));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F(", payload: "));
    DEBUG_PRINTLN(p_payload);
    g_led.doubleFastBlink();
  }
  else
  {
    DEBUG_PRINTLN(F("ERROR: MQTT message not published, either connection lost, or message too large. Topic: "));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F(" , payload: "));
    DEBUG_PRINTLN(p_payload);
  }
}

void Mqtt::subscribe(String topic, TOPIC_CALLBACK_SIGNATURE callback)
{
  if (connected())
    subscribe(topic.c_str());
  m_topics[m_subTopicCnt] = topic;
  m_callBacks[m_subTopicCnt] = callback;
  m_subTopicCnt++;
}

void Mqtt::subscribe(const char *topic)
{
  DEBUG_PRINT("Subscribing to ");
  DEBUG_PRINT(topic);
  DEBUG_PRINTLN("...");
  m_client.subscribe(topic);
}

Mqtt g_mqtt;