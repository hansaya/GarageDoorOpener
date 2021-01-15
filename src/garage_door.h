#ifndef garage_door_h
#define garage_door_h

#include <Arduino.h>
#include <Ticker.h>
#include "button.h"

#define RELAY_LATCH_TIME 400
#define DELAY_BETWEEN_CALLS RELAY_LATCH_TIME+100
/**
 * Class GarageDoor handles all things required to open the garage door.
 */
class GarageDoor
{
public:
    /**
     * Constructor
     * @param[in] identification Unique id of the door.
     * @param[in] relayPin Pin connected to the relay.
     * @param[in] statusPin Pin connected to the door sensor.
     * @param[in] name Name of the door.
     */
    GarageDoor(String identification, uint16_t relayPin, uint16_t statusPin, String name);
    /**
     * Execute anything that belong in setup ()
     */
    void begin();
    /**
     * logic that needs to run within loop()
     */
    void loop();
    /**
     * Open the garage door
     */
    void open();
    /**
     * Close the garage door
     */
    void close();
    /**
     * Indication as to garage door is open.
     * @return Door opened status. 
     */
    bool opened() const;

private:
    /**
     * Announce the cover config to home assistant.
     */
    void mqttAnnounce() const;
    /**
     * Publish door status.
     */
    void publishStatus() const;
    /**
     * Button Press.
     */
    void buttonPress();
    /**
     * Trigger relay output.
     */
    void triggerRelay();

    String m_name;
    String m_id;
    uint16_t m_relayPin;
    uint16_t m_statusPin;
    char m_topicMQTTHeader[50];
    bool m_doorOpen;
    ButtonEvents m_sortInputTrigger;
    Button<1> m_sensor;
    bool m_event;
    Ticker m_ticker;
    Ticker m_queueTicker;
    static unsigned long m_lastCall;
};

#endif