#ifndef mqtt_h
#define mqtt_h

#define CALL_BACK_LIMIT 5
#define TOPIC_CALLBACK_SIGNATURE std::function<void(String)>
#define MQTT_QOS 1

#include <functional>
#include <PubSubClient.h>
#ifdef ESP32
#include <wifi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <Arduino.h>

class Mqtt
{
public:
    Mqtt();
    // Execute anything that belong in setup ()
    void begin();
    // logic that needs to run with loop
    void loop();
    // Return true if connectecd to MQTT server
    const bool connected();
    // Publish a message to MQTT server.
    void publishToMQTT(const char *p_topic, const char *p_payload, bool retained = true);
    // Get the unique id.
    const char *getUniqueId() const { return m_uniqueId; }
    // Set a call back for subscribed topics.
    void subscribe(String topic, TOPIC_CALLBACK_SIGNATURE callback);
    // Get availability header.
    const char *getAvailabilityTopic() const { return m_availHeader; }

private:
    // Subscribe to a topic
    void subscribe(const char *topic);
    // Call back function for MQTT messages.
    void mqttCalllBack(char *topic, byte *payload, unsigned int length);
    // Publish availability message
    void publishBirthMessage();
    // Connect to mqtt server.
    void connect();

    WiFiClient m_espClient;
    PubSubClient m_client;
    char m_uniqueId[5];
    int m_subTopicCnt;
    String m_topics[CALL_BACK_LIMIT];
    TOPIC_CALLBACK_SIGNATURE m_callBacks[CALL_BACK_LIMIT];
    char m_topicMQTTHeader[50];
    char m_availHeader[50];
    bool m_hassioAlive;
    bool m_error;
};

extern Mqtt g_mqtt;

#endif