#ifndef ota_h
#define ota_h

#include "user_config.h"

class Ota
{
public:
    Ota () : m_firmwareUpdating(false) {};
    void begin ();
    void loop ();
    bool busy () { return m_firmwareUpdating; }
    bool m_firmwareUpdating;
};

extern Ota g_ota;

#endif