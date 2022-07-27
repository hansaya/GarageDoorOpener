#ifndef mqtt_h
#define mqtt_h

#define CALL_BACK_LIMIT 5
#define TOPIC_CALLBACK_SIGNATURE std::function<void(String)>
#define CONFIG_CALLBACK_SIGNATURE std::function<void()>
#define MQTT_QOS 1

#include <functional>
#include <PubSubClient.h>
#ifdef ESP32
#include <wifi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <Arduino.h>

/**
 * Class Mqtt handles MQTT communication. This is designed around Home Assistant integration.
 */
class Mqtt
{
public:
    /**
     * Constructor.
     */
    Mqtt();
    /**
     * Execute anything that belong in setup ().
     */
    void begin();
    /**
     * logic that needs to run with in loop ().
     */
    void loop();
    /**
     * Return true if connectecd to MQTT server.
     */
    const bool connected();
    /**
     * Publish a message to MQTT server.
     * @param[in] topic MQTT topic name.
     * @param[in] payload MQTT playload.
     * @param[in] retained Message retain flag.
     */
    void publishToMQTT(const char *topic, const char *payload, bool retained = true);
    /**
     * Get the unique id.
     * @return String containing system unique id.
     */
    const char *getUniqueId() const { return m_uniqueId; }
    /**
     * Set a call back for subscribed topics.
     * @param[in] topic MQTT Topic name.
     * @param[in] callback Call back function pointer.
     */
    void subscribe(String topic, TOPIC_CALLBACK_SIGNATURE callback);
    /**
     * Get availability header.
     * @return MQTT avalability topic name.
     */
    const char *getAvailabilityTopic() const { return m_availHeader; }
    /**
     * Publish Config.
     * @param[in] callback Call back function.
     */
    void publishConfig(CONFIG_CALLBACK_SIGNATURE callback);

private:
    /**
     * Subscribe to a topic.
     * @param[in] topic MQTT topic.
     */
    void subscribe(const char *topic);

    /**
     * Subscribe to a topic using the internal record.
     */
    void subscribe();

    /**
     * Call back function for MQTT messages.
     * @param[in] topic MQTT topic.
     * @param[in] payload MQTT playload.
     * @param[in] length MQTT playload size.
     */
    void mqttCalllBack(char *topic, byte *payload, unsigned int length);
    /**
     * Publish availability message.
     */
    void publishBirthMessage();
    /**
     * Connect to mqtt server.
     */
    void connect();
    /**
     * Publish Config using the internal records.
     */
    void publishConfig() const;

    WiFiClient m_espClient;
    PubSubClient m_client;
    char m_uniqueId[5];
    int m_subTopicCnt;
    String m_subTopics[CALL_BACK_LIMIT];
    TOPIC_CALLBACK_SIGNATURE m_subCallBacks[CALL_BACK_LIMIT];
    int m_configCnt;
    CONFIG_CALLBACK_SIGNATURE m_configCallBacks[CALL_BACK_LIMIT];
    char m_topicMQTTHeader[50];
    char m_availHeader[50];
    bool m_hassioAlive;
    bool m_error;
    bool m_connected;
    bool m_configEvent;
};

extern Mqtt g_mqtt;

#endif