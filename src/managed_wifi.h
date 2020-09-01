#ifndef managed_wifi_h
#define managed_wifi_h

#include <Arduino.h>
#include "user_config.h"
#include "WiFiManager.h"

class ManagedWiFi
{
public:
    ManagedWiFi();
    // Execute anything that belong in setup ()
    void begin();
    // Reconnect logic if connection drops
    void loop();
    // Manage wifi portal
    void manageWiFi(const bool reset_config = false);
    // Get the hostname
    String getHostName() const;
    // Get the mac address as a string.
    String getMacStr() const;
    // Get the mac address in bytes.
    byte *getMac();
    // Return true if connected to a device.
    bool connected() const;

private:
    // Config mode callback
    static void configModeCallback(WiFiManager *myWiFiManager);
    // Save Config callback
    static void saveConfigCallback();

    String m_hostName;
    String m_macString;
    byte m_mac[7];
    static bool m_gotTheConfig;
};

extern ManagedWiFi g_managedWiFi;

#endif