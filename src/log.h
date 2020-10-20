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
#define LOG_SIZE 15

class Log
{
public:
   // Log level
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
   void write(Log::Level level, String desc, bool flush = false);

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