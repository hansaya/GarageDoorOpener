#include "mqtt.h"
#include "user_config.h"
#include "managed_wifi.h"

const char* mqtt_broker = MQTT_BROKER;
const char* mqtt_clientId = CLIENT;
const char* mqtt_username = MQTT_USERNAME;
const char* mqtt_password = MQTT_PASSWORD;

const char* birthMessage = "online";
const char* lwtMessage = "offline";

void onMqttMessage(char* topic, byte* payload, unsigned int length)
{
    g_mqtt.mqttCalllBack (topic, payload, length);
}

void Mqtt::begin ()
{
    m_client.setServer(mqtt_broker, 1883);
    m_client.setCallback(onMqttMessage);
    snprintf(m_uniqueId, 20, "%02X%02X", g_managedWiFi.getMac()[4], g_managedWiFi.getMac()[5]);
    m_client.setBufferSize(812);
    snprintf(m_topicMQTTHeader, 50, "%s/cover/%s", MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX, g_managedWiFi.getHostName ().c_str ());
}

void Mqtt::loop ()
{
    if (!m_client.connected()) {
      reconnect();
    }
    m_client.loop();
}

bool Mqtt::connected ()
{
  return m_client.connected();
}

// Function that publishes birthMessage
void Mqtt::publishBirthMessage() {

  char availabilityTopic[80];
  snprintf(availabilityTopic, 80, "%s/avail", m_topicMQTTHeader);

  // Publish the birthMessage
  DEBUG_PRINT("Publishing birth message \"");
  DEBUG_PRINT(birthMessage);
  DEBUG_PRINT("\" to ");
  DEBUG_PRINT(availabilityTopic);
  DEBUG_PRINTLN("...");
  m_client.publish(availabilityTopic, birthMessage, true);
}

// Callback when MQTT message is received; calls triggerDoorAction(), passing topic and payload as parameters
void Mqtt::mqttCalllBack(char* topic, byte* payload, unsigned int length) {
    DEBUG_PRINT("Message arrived [");
    DEBUG_PRINT(topic);
    DEBUG_PRINT("] ");
    for (int i = 0; i < length; i++) {
        DEBUG_PRINT((char)payload[i]);
    }
    DEBUG_PRINTLN();

    String topicToProcess = topic;
    payload[length] = '\0';
    String payloadToProcess = (char*)payload;

    // Call the call backs if the topic matches.
    for (int i=0; i<m_subTopicCnt; i++)
    {
        if (m_topics[i] == topicToProcess && m_callBacks[i])
            m_callBacks[i](payloadToProcess);
    }
}

// Function that runs in loop() to connect/reconnect to the MQTT broker, and publish the current door statuses on connect
void Mqtt::reconnect() {
  // Loop until we're reconnected
  char availabilityTopic[80];
  snprintf(availabilityTopic, 80, "%s/avail", m_topicMQTTHeader);

  while (!m_client.connected()) {
    DEBUG_PRINT("Attempting MQTT connection...");
    // Attempt to connect
    if (m_client.connect(mqtt_clientId, mqtt_username, mqtt_password, availabilityTopic, 0, true, lwtMessage)) {
      DEBUG_PRINTLN("Connected!");
      // Publish the birth message on connect/reconnect
      publishBirthMessage();
    } 
    else {
      DEBUG_PRINT("failed, rc=");
      DEBUG_PRINT(m_client.state());
      DEBUG_PRINTLN(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void Mqtt::subscribe(char* topic)
{
  DEBUG_PRINT("Subscribing to ");
  DEBUG_PRINT(topic);
  DEBUG_PRINTLN("...");
    
  m_client.subscribe(topic);
}

// Publish the MQTT payload.
void Mqtt::publishToMQTT(const char* p_topic,const char* p_payload) {
  if (m_client.publish(p_topic, p_payload, true)) {
    DEBUG_PRINT(F("INFO: MQTT message published successfully, topic: "));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F(", payload: "));
    DEBUG_PRINTLN(p_payload);
  } else {
    DEBUG_PRINTLN(F("ERROR: MQTT message not published, either connection lost, or message too large. Topic: "));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F(" , payload: "));
    DEBUG_PRINTLN(p_payload);
  }
}

void Mqtt::addCallBack (String topic, TOPIC_CALLBACK_SIGNATURE callback)
{
    m_topics[m_subTopicCnt]=topic;
    m_callBacks[m_subTopicCnt]=callback;
    m_subTopicCnt++;
}

Mqtt g_mqtt;