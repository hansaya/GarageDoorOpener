#ifndef managed_wifi_h
#define managed_wifi_h

#include <Arduino.h>
#include <Ticker.h>
#include "user_config.h"
#include "WiFiManager.h"

class ManagedWiFi
{
public:
    ManagedWiFi ();
    void begin ();
    void manageWiFi (bool reset_config = false);
    static void event (WiFiEvent_t event);
    static void configModeCallback (WiFiManager *myWiFiManager);
    static void saveConfigCallback ();
    String getHostName ();
    String getMacStr ();
    byte* getMac ();
    bool wifiConnected ();
private:
    static bool m_gotTheConfig;
    String m_hostName;
    String m_macString;
    byte m_mac[7];
};

extern ManagedWiFi g_managedWiFi;

#endif