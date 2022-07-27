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
#define LOG_SIZE 10 // Max buffer size in lines for log history.

/**
 * Class Log gives the capability to output log message. This is designed to buffer the logs until someone reads it. Capable of outputing to telnet and serial.
 */
class Log
{
public:
   /**
    * Enum Level identifies the log level.
    */
   enum Level
   {
      Debug,
      Warn,
      Error
   };

   /**
    * Constructor.
    */
   Log()
#ifdef DEBUG_TELNET
       : m_telnetServer(DEBUG_TELNET_PORT),
         m_telnetClient(),
         m_logs(),
         m_logLevel()
#endif
   {
   }
   /**
    * Execute anything that belong in setup ().
    */
   void begin();
   /**
    * Output logs to a stream when available.
    */
   void loop();
   /**
    * Log a message.
    */
   void write(Log::Level level, String desc, bool flush = false);

private:
#ifdef DEBUG_TELNET
   WiFiServer m_telnetServer;
   WiFiClient m_telnetClient;
   /**
    * Handle telnet debuging.
    */
   void handleTelnet();
#endif

   /**
    * Print to the output.
    */
   template <class OUT>
   void print(OUT &output, Level level, String text);

#ifdef DEBUG_TELNET
   CircularBuffer<String, LOG_SIZE> m_logs;
   CircularBuffer<Level, LOG_SIZE> m_logLevel;
#endif
};

extern Log g_log;

#endif