#ifndef log_h
#define log_h

#include <Arduino.h>
#include <CircularBuffer.h>
#ifdef ESP32
#include <wifi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include "user_config.h"

#define DEBUG_TELNET_PORT 23
#define LOG_SIZE 10

class Log
{
public:
   enum Level
   {
      Debug,
      Warn,
      Error
   };

   Log() :
#ifdef DEBUG_TELNET
           m_telnetServer(DEBUG_TELNET_PORT),
           m_telnetClient(),
#endif
           m_logs(),
           m_logLevel()
   {}
   // Execute anything that belong in setup ().
   void begin();
   // Output logs to a stream when available.
   void loop();
   // Log a message
   void write(Log::Level level, String desc);

   // Macros for debugging
// #ifdef DEBUG_TELNET
  
//   extern WiFiServer telnetServer;
//   extern WiFiClient telnetClient;
//   #define DEBUG_PRINT(x) telnetClient.print(x)
//   #define DEBUG_PRINT_WITH_BASE(x, base) telnetClient.print(x, base)
//   #define DEBUG_PRINT_WITH_FMT(x, fmt) telnetClient.printf(x, fmt)
//   #define DEBUG_PRINTLN(x) telnetClient.println(x)
//   #define DEBUG_PRINTLN_WITH_FMT(x, fmt) telnetClient.println(x, fmt)
//   #define DEBUG_STREAM telnetClient
// #elif defined(DEBUG_SERIAL)
//   #define DEBUG_PRINT(x) Serial.print(x)
//   #define DEBUG_PRINT_WITH_BASE(x, base) Serial.print(x, base)
//   #define DEBUG_PRINT_WITH_FMT(x, fmt) Serial.printf(x, fmt)
//   #define DEBUG_PRINTLN(x) Serial.println(x)
//   #define DEBUG_PRINTLN_WITH_FMT(x, fmt) Serial.println(x, fmt)
//   #define DEBUG_STREAM Serial
// #else
//   #define DEBUG_PRINT(x)
//   #define DEBUG_PRINT_WITH_FMT(x, fmt)
//   #define DEBUG_PRINTLN(x)
//   #define DEBUG_PRINTLN_WITH_FMT(x, fmt)
//   #define DEBUG_STREAM Serial
// #endif

private:
#ifdef DEBUG_TELNET
   WiFiServer m_telnetServer;
   WiFiClient m_telnetClient;
   // Handle telnet debuging
   void handleTelnet();
#endif

   CircularBuffer<String, LOG_SIZE> m_logs;
   CircularBuffer<Level, LOG_SIZE> m_logLevel;
};

extern Log g_log;

#endif