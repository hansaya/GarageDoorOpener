#ifndef garage_door_h
#define garage_door_h

#include <Arduino.h>

class GarageDoor
{
public:
    GarageDoor (String identification, uint16_t relayPin, uint16_t statusPin);
    void begin ();
    void open ();
    bool opened ();
private:
    void mqttAnnounce ();
    void publishStatus ();


    String m_id;
    uint16_t m_relayPin;
    uint16_t m_statusPin;
    char m_topicMQTTHeader[50];
};

#endif