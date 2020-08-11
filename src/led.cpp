#include "led.h"
#include <Ticker.h>
#include "user_config.h"

void Led::begin ()
{
    pinMode(STATUS_LED, OUTPUT);

    // setup fast led
    FastLED.addLeds<NEOPIXEL, NEO_PIXEL_PIN>(m_leds, 1);
    FastLED.clear();
    FastLED.setBrightness(1);
    FastLED.show();
}

// Blink the light
void Led::changeLedState (bool blinkTwice)
{
  if (blinkTwice)
  {
    static short count;
    if (count++ >= 2)
    {
        count = 0;
        m_lightTicker.detach();
        digitalWrite(STATUS_LED, LOW);
    }
  }
  digitalWrite(STATUS_LED, !(digitalRead(STATUS_LED)));
}

// Bilnk twice to indicate successful mqtt call.
void Led::doubleFastBlink ()
{
    digitalWrite(STATUS_LED, LOW);
    m_lightTicker.attach_ms<Led*>(200, [](Led* led)
    {
        led->changeLedState (true);
    }, this);
}

void Led::blinkLed ()
{
    m_lightTicker.attach_ms<Led*>(200, [](Led* led)
    {
        led->changeLedState (false);
    }, this);
}

void Led::stopBlinkLed ()
{
    digitalWrite(STATUS_LED, LOW);
    m_lightTicker.detach();
}

void Led::setPixColor (CRGB color)
{
    m_leds[0] = color; 
}

void Led::showPixColor ()
{
    FastLED.show();
}

Led g_led;