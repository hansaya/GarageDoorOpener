#ifndef garage_door_h
#define garage_door_h

#include <Arduino.h>
#include <Ticker.h>

class GarageDoor
{
public:
    GarageDoor (String identification, uint16_t relayPin, uint16_t statusPin);
    void begin ();
    void loop ();
    void open ();
    void close ();
    bool opened ();
private:
    void mqttAnnounce ();
    void publishStatus ();
    void subscribeToCmd ();
    void triggerRelay ();

    Ticker m_lightTicker;

    String m_id;
    uint16_t m_relayPin;
    uint16_t m_statusPin;
    char m_topicMQTTHeader[50];
    bool m_publishConfig;
    bool m_doorOpen;
};

#endif