#include "ota.h"
#include <ArduinoOTA.h>
#include "managed_wifi.h"
#include "led.h"
#include "debug.h"

void Ota::begin()
{
    // Config OTA updates
    ArduinoOTA.onStart([]() {
        g_ota.m_firmwareUpdating = true;
        g_led.blinkLed();
        DEBUG_PRINTLN("Starting OTA");
    });
    ArduinoOTA.onEnd([]() {
        g_ota.m_firmwareUpdating = false;
        g_led.stopBlinkLed();
        DEBUG_PRINTLN("\nEnding OTA");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        // DEBUG_PRINT_WITH_FMT("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        g_ota.m_firmwareUpdating = false;
        DEBUG_PRINT_WITH_FMT("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
            DEBUG_PRINTLN("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
            DEBUG_PRINTLN("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
            DEBUG_PRINTLN("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
            DEBUG_PRINTLN("Receive Failed");
        else if (error == OTA_END_ERROR)
            DEBUG_PRINTLN("End Failed");
    });
    ArduinoOTA.setHostname(g_managedWiFi.getHostName().c_str());
    ArduinoOTA.begin();
}

void Ota::loop()
{
    ArduinoOTA.handle();
}

Ota g_ota;