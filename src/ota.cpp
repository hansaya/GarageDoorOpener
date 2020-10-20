#include "ota.h"
#include <ArduinoOTA.h>
#include "managed_wifi.h"
#include "led.h"
#include "log.h"

void Ota::begin()
{
    // Config OTA updates
    ArduinoOTA.onStart([]() {
        g_ota.m_firmwareUpdating = true;
        g_led.blinkLed();
        g_log.write(Log::Debug, "OTA Start", true);
    });
    ArduinoOTA.onEnd([]() {
        g_ota.m_firmwareUpdating = false;
        g_led.stopBlinkLed();
        g_log.write(Log::Debug, "OTA End", true);
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        // DEBUG_PRINT_WITH_FMT("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        g_ota.m_firmwareUpdating = false;
        if (error == OTA_AUTH_ERROR)
            g_log.write(Log::Error, "OTA ERROR! Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
            g_log.write(Log::Error, "OTA ERROR! Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
            g_log.write(Log::Error, "OTA ERROR! Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
            g_log.write(Log::Error, "OTA ERROR! Receive Failed");
        else if (error == OTA_END_ERROR)
            g_log.write(Log::Error, "OTA ERROR! End Failed");
    });
    ArduinoOTA.setHostname(g_managedWiFi.getHostName().c_str());
    ArduinoOTA.begin();
}

void Ota::loop()
{
    ArduinoOTA.handle();
}

Ota g_ota;