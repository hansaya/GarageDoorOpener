#ifndef debug_h
#define debug_h

#include <Arduino.h>
#include "user_config.h"
#include <wifi.h>

// Macros for debugging
#ifdef DEBUG_TELNET
  #define DEBUG_TELNET_PORT 23
  extern WiFiServer telnetServer;
  extern WiFiClient telnetClient;
  #define DEBUG_PRINT(x) telnetClient.print(x)
  #define DEBUG_PRINT_WITH_FMT(x, fmt) telnetClient.printf(x, fmt)
  #define DEBUG_PRINTLN(x) telnetClient.println(x)
  #define DEBUG_PRINTLN_WITH_FMT(x, fmt) telnetClient.println(x, fmt)
  #define DEBUG_STREAM telnetClient
#elif defined(DEBUG_SERIAL)
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINT_WITH_FMT(x, fmt) Serial.printf(x, fmt)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTLN_WITH_FMT(x, fmt) Serial.println(x, fmt)
  #define DEBUG_STREAM Serial
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINT_WITH_FMT(x, fmt)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTLN_WITH_FMT(x, fmt)
#endif

// Handle telnet debuging
#if defined(DEBUG_TELNET)
inline void handleTelnet(void)
{
  if (telnetServer.hasClient())
  {
    if (!telnetClient || !telnetClient.connected())
    {
      if (telnetClient)
      {
        telnetClient.stop();
      }
      telnetClient = telnetServer.available();
    }
    else
    {
      telnetServer.available().stop();
    }
  }
}
#endif

#endif