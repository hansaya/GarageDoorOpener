#ifndef ota_h
#define ota_h

#include "user_config.h"

/**
 * Class Ota handles over the air update initialization and management.
 */
class Ota
{
public:
    /**
     * Constructor.
     */
    Ota() : m_firmwareUpdating(false){};
    /**
     * Execute anything that belong in setup().
     */
    void begin();
    /**
     * logic that needs to run with in loop().
     */
    void loop();
    /**
     * Return true if OTA is busy.
     */
    const bool busy() const { return m_firmwareUpdating; }

private:
    bool m_firmwareUpdating;
};

extern Ota g_ota;

#endif