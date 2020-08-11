#ifndef led_h
#define led_h

#include <Arduino.h>
#include <Ticker.h>
#include <FastLED.h>

class Led
{
public:
    // Execute anything that belong in setup ().
    void begin ();
    // logic that needs to run with loop.
    void loop ();
    // Change the led status.
    void changeLedState (bool blinkTwice = false);
    // Double blink the led once.
    void doubleFastBlink ();
    // Blink the led continuesly.
    void blinkLed ();
    // Stop the led blinking.
    void stopBlinkLed ();
    // Set the neo pixel color.
    void setPixColor (CRGB color);
    // Light up the neo pixel.
    void showPixColor ();

private:
    Ticker m_lightTicker;
    CRGB m_leds[1];
};

extern Led g_led;

#endif