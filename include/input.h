#ifndef input_h
#define input_h

#include <Arduino.h>
#include <Ticker.h>
#include "button.h"

class Input
{
public:
    Input ();
    void begin ();
    void loop ();

    // Read the button inputs.
    void IRAM_ATTR readButton ();

    Button<3> GetButton () { return m_bootButton; }
    
private:
    // For button inputs
    ButtonEvents m_sortButtonPress;
    ButtonEvents m_longButtonPress;
    ButtonEvents m_resetButtonPress;
    Button<3> m_bootButton;
};

extern Input g_input;

#endif