#ifndef button_h
#define button_h

#include <Arduino.h>
#include <Ticker.h>
#include <FunctionalInterrupt.h>
#include "user_config.h"

// For button events
struct ButtonEvents {
  const unsigned long minDuration; // This needs to higher than 100ms for events that are not triggering on release.
  bool pressed;
  bool triggerOnRelease; // This flag will turn on a ticker to check the input periodically.
};

template <int N>
class Button
{
public:
  template<typename... Args>
  Button (uint16_t pin, bool active, Args ... events):
    m_pin (pin),
    m_pressed (false),
    m_active (active)
  {
    // Add events recursively.
    addEvents (events...);
  }

  // Setup pin mode and interrupt
  void begin ()
  {
    if (!m_active)
      pinMode(m_pin, INPUT_PULLUP);
    else
      pinMode(m_pin, INPUT_PULLDOWN);

    attachInterrupt(m_pin, std::bind(&Button::readButton, this), CHANGE);

    // Attach a ticker to check the input for the non-human triggers. This will make sure to trigger an event for missed interrupts.
    for (int i=m_eventLength; i>0; i--)
    {
      if (!m_events[i-1]->triggerOnRelease)
      {
        m_ticker.attach_ms<Button<N>*>(100, [](Button* button) {
          // Force check the input every 100ms to avoid interrupt issues.
          button->readButton();
          for (int i=button->m_eventLength; i>0; i--)
          {
            if (!button->m_events[i-1]->triggerOnRelease)
            {
              if (button->m_pressed && (millis() - button->m_buttonCurrentHigh) > button->m_events[i-1]->minDuration)
                button->m_events[i-1]->pressed=true;
              else if(!button->m_pressed)
                button->m_events[i-1]->pressed=false;
            }
          }
        }, this);
      }
    }
  }

  // Button intput interupt function.
  void IRAM_ATTR readButton()
  {
    // Input got triggered.
    if (digitalRead (m_pin) == m_active && !m_pressed)
    {
      // Note the time the input was triggered.
      m_buttonCurrentHigh = millis();
      m_pressed = true;
      return;
    }
    // Input released.
    if (digitalRead (m_pin) == !m_active && m_pressed)
    {
      m_pressed = false;
      for (int i=m_eventLength; i>0; i--)
      {
        // If the input was held down long enough, trigger the event.
        if (m_events[i-1]->triggerOnRelease && (millis() - m_buttonCurrentHigh) > m_events[i-1]->minDuration)
        {
          m_events[i-1]->pressed=true;
          break;
        }
      }
    }
  }
    
private:
  // Add events
  template<class T, typename ...Rest>
  void addEvents (T event, Rest... rest)
  {
    m_events[m_eventLength] = event;
    m_eventLength++;
    addEvents (rest...);
  }

  // Add events
  template<typename Arg>
  void addEvents (Arg event)
  {
    m_events[m_eventLength] = event;
    m_eventLength++;
  }

  // For button inputs
  const uint16_t m_pin;
  bool m_pressed;
  ButtonEvents* m_events[N];
  int m_eventLength;
  const bool m_active;
  Ticker m_ticker;
  volatile unsigned long m_buttonCurrentHigh;
  
  
};

#endif