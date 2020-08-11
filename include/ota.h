#ifndef ota_h
#define ota_h

#include "user_config.h"

class Ota
{
public:
    Ota () : m_firmwareUpdating(false) {};
    // Execute anything that belong in setup ().
    void begin ();
    // logic that needs to run with loop.
    void loop ();
    // Return true if OTA is busy.
    bool busy () { return m_firmwareUpdating; }
private:
    bool m_firmwareUpdating;
};

extern Ota g_ota;

#endif