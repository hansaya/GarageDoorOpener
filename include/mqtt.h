#ifndef mqtt_h
#define mqtt_h

#define CALL_BACK_LIMIT 5

#include <functional>
#define TOPIC_CALLBACK_SIGNATURE std::function<void(String)>

#include <PubSubClient.h>
#include <wifi.h>
#include <Arduino.h>

class Mqtt
{
public:
    Mqtt (): m_espClient(), m_client(m_espClient) {}
    void begin ();
    void loop ();
    void publishBirthMessage ();
    bool connected();
    void triggerDoorAction(String requestedDoor, String requestedAction);
    void mqttCalllBack(char* topic, byte* payload, unsigned int length);
    void reconnect ();
    void publish_door1_status ();
    void publish_door2_status ();
    void getGarageDoor1Command ();
    void publishToMQTT(const char* p_topic,const char* p_payload);
    char* getUniqueId () { return m_uniqueId; }
    void addCallBack (String topic, TOPIC_CALLBACK_SIGNATURE callback);
private:
    WiFiClient m_espClient;
    PubSubClient m_client;
    char m_uniqueId[5];
    int m_subTopicCnt;
    String m_topics[CALL_BACK_LIMIT];
    TOPIC_CALLBACK_SIGNATURE m_callBacks[CALL_BACK_LIMIT];
    char m_topicMQTTHeader[50];
};

extern Mqtt g_mqtt;

#endif