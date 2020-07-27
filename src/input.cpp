#include "input.h"
#include <Ticker.h>
#include "user_config.h"

// Button intput interupt function.
void IRAM_ATTR ButtonInterrupt()
{
    g_input.readButton ();
}

Input::Input ():
    m_sortButtonPress ({50, false}),
    m_longButtonPress ({800, false}),
    m_resetButtonPress ({6000, false}),
    m_bootButton {BUTTON, false, {&m_sortButtonPress, &m_longButtonPress, &m_resetButtonPress}}
{
}

void Input::begin ()
{
    pinMode(m_bootButton.pin, INPUT_PULLUP);
    pinMode(STATUS_LED, OUTPUT);
    attachInterrupt(m_bootButton.pin, ButtonInterrupt, CHANGE);

    pinMode(ISO_IN_PIN, INPUT);

}

// Button intput interupt function.
void IRAM_ATTR Input::readButton()
{
  static volatile unsigned long buttonCurrentHigh;
  static volatile unsigned long buttonCurrentLow;
  //if this is true the button was just pressed down
  if (digitalRead (m_bootButton.pin) == LOW)
  {
    //note the time the button was pressed
    buttonCurrentHigh = millis();
    m_bootButton.pressed = true;
    return;
  }
  //if button is not high then button got released. The millis function will increase while a button is hold down the loop function will be cycled (no change, so no interrupt is active)
  if (digitalRead (m_bootButton.pin) == HIGH && m_bootButton.pressed == true)
  {
    buttonCurrentLow = millis();
    for (int i=m_bootButton.eventLength; i>0; i--)
    {
     if ((buttonCurrentLow - buttonCurrentHigh) > m_bootButton.events[i-1]->minDuration)
     {
        m_bootButton.events[i-1]->pressed=true;
        break;
     }
    }
  }
}

Input g_input;