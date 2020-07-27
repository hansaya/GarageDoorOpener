#include "led.h"
#include <Ticker.h>
#include "user_config.h"

void IRAM_ATTR LedCalllBack(bool blinkTwice)
{
    g_led.TheLight (blinkTwice);
}

void Led::begin ()
{
    pinMode(STATUS_LED, OUTPUT);

    // setup fast led
    FastLED.addLeds<NEOPIXEL, NEO_PIXEL_PIN>(m_leds, 1);
    FastLED.setBrightness(1);
    FastLED.clear();
    // g_leds[0] = CRGB::Green;
    FastLED.show();
}

// Blink the light
void Led::TheLight (bool blinkTwice)
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
void Led::DoubleFastBlink ()
{
    digitalWrite(STATUS_LED, LOW);
    m_lightTicker.attach(0.2, LedCalllBack, true);
}

void Led::BlinkLed ()
{
    m_lightTicker.attach(0.2, LedCalllBack, false);
}

void Led::StopBlinkLed ()
{
    digitalWrite(STATUS_LED, LOW);
    m_lightTicker.detach();
}

void Led::SetPixColor (CRGB color)
{
    m_leds[0] = color; 
}

void Led::ShowPixColor ()
{
    FastLED.show();
}

Led g_led;