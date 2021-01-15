#ifndef led_h
#define led_h

#include <Arduino.h>
#include <Ticker.h>
#include <FastLED.h>

/**
 * Class Led handles led outputs.
 */
class Led
{
public:
    /**
     * Execute anything that belong in setup ().
     */
    void begin();
    /**
     * Double blink the led once.
     */
    void doubleFastBlink();
    /**
     * Blink the led continuesly.
     */
    void blinkLed();
    /**
     * Stop the led blinking.
     */
    void stopBlinkLed();
    /**
     * Set the neo pixel color.
     */
    void setPixColor(CRGB color);
    /**
     * Light up the neo pixel.
     */
    void showPixColor() const;

private:
    /**
     * Change the led status.
     */
    void IRAM_ATTR changeLedState(uint16_t blinkCount = 0);

    Ticker m_lightTicker;
    CRGB m_leds[1];
};

extern Led g_led;

#endif