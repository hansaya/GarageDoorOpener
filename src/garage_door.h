#ifndef garage_door_h
#define garage_door_h

#include <Arduino.h>
#include <Ticker.h>
#include "button.h"

// Class GarageDoor handles all things required to open the garage door.
class GarageDoor
{
public:
    GarageDoor(String identification, uint16_t relayPin, uint16_t statusPin, String name);
    // Execute anything that belong in setup ()
    void begin();
    // logic that needs to run with loop
    void loop();
    // Open the garage door
    void open();
    // Close the garage door
    void close();
    // Indication as to garage door is open
    bool opened() const;

private:
    // Announce the cover config to home assistant.
    void mqttAnnounce() const;
    // Publish door status
    void publishStatus() const;
    // Subscribe to command topic
    void triggerRelay();

    String m_name;
    Ticker m_ticker;
    String m_id;
    uint16_t m_relayPin;
    uint16_t m_statusPin;
    char m_topicMQTTHeader[50];
    bool m_publishConfig;
    bool m_doorOpen;
    ButtonEvents m_sortInputTrigger;
    Button<1> m_sensor;
    bool m_event;
};

#endif