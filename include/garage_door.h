#ifndef garage_door_h
#define garage_door_h

#include <Arduino.h>
#include <Ticker.h>

// Class GarageDoor handles all things required to open the garage door.
class GarageDoor
{
public:
    GarageDoor (String identification, uint16_t relayPin, uint16_t statusPin);

    // Execute anything that belong in setup ()
    void begin ();
    // logic that needs to run with loop
    void loop ();
    // Open the garage door
    void open ();
    // Close the garage door
    void close ();
    // Indication as to garage door is open
    bool opened ();
private:
    // Announce the cover config to home assistant.
    void mqttAnnounce ();
    // Publish door status
    void publishStatus ();
    // Subscribe to command topic
    void subscribeToCmd ();
    // trigger the relay to open the garage door.
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