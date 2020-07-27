#ifndef led_h
#define led_h

#include <Arduino.h>
#include <Ticker.h>
#include <FastLED.h>

class Led
{
public:
    void begin ();
    void loop ();
    void TheLight (bool blinkTwice = false);
    void DoubleFastBlink ();
    void BlinkLed ();
    void StopBlinkLed ();
    void SetPixColor (CRGB color);
    void ShowPixColor ();

private:
    Ticker m_lightTicker;
    CRGB m_leds[1];
};

extern Led g_led;

#endif