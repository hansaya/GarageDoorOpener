#include "log.h"

void Log::begin ()
{
   // Run the telnet server for debugging.
#if defined(DEBUG_TELNET)
   m_telnetServer.begin();
   m_telnetServer.setNoDelay(true);
#else
   Serial.updateBaudRate(115200);
#endif
}

void Log::loop ()
{
#if defined(DEBUG_TELNET)
   // handle the Telnet connection
   handleTelnet();

   if (m_telnetClient.connected() && m_logs.size() > 0)
   {
      switch (m_logLevel.shift())
      {
      case Debug:
         m_telnetClient.print("DEBUG: ");
         break;
      case Warn:
         m_telnetClient.print("WARN: ");
         break;
      case Error:
         m_telnetClient.print("ERROR: ");
         break;
      default:
         m_telnetClient.print("UNKNOWN: ");
      }
      m_telnetClient.println(m_logs.shift());
   }
#else
   if (Serial && m_logs.size() > 0)
   {
      switch (m_logLevel.shift())
      {
      case Debug:
         Serial.print("DEBUG: ");
         break;
      case Warn:
         Serial.print("WARN: ");
         break;
      case Error:
         Serial.print("ERROR: ");
         break;
      default:
         Serial.print("UNKNOWN: ");
      }
      Serial.println(m_logs.shift());
   }
#endif
}

void Log::write(Log::Level level, String desc)
{
   if (LOG_LEVEL <= level)
   {
      m_logs.push(desc);
      m_logLevel.push(level);
   }
}

#if defined(DEBUG_TELNET)
void Log::handleTelnet()
{
   if (m_telnetServer.hasClient())
   {
      if (!m_telnetClient || !m_telnetClient.connected())
      {
         if (m_telnetClient)
         {
            m_telnetClient.stop();
         }
         m_telnetClient = m_telnetServer.available();
      }
      else
      {
         m_telnetServer.available().stop();
      }
   }
}
#endif

Log g_log;