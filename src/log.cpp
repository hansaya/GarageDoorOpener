#include "log.h"

void Log::begin()
{
   // Run the telnet server for debugging.
#if defined(DEBUG_TELNET)
   m_telnetServer.begin();
   m_telnetServer.setNoDelay(true);
#endif
}

template <class OUT>
void Log::print(OUT &output, Level level, String text)
{
   switch (level)
   {
   case Debug:
      output.print("DEBUG: ");
      break;
   case Warn:
      output.print("WARN: ");
      break;
   case Error:
      output.print("ERROR: ");
      break;
   default:
      output.print("UNKNOWN: ");
   }
   output.println(text);
}

void Log::loop()
{
#if defined(DEBUG_TELNET)
   // handle the Telnet connection
   handleTelnet();

   if (m_telnetClient.connected() && m_logs.size() > 0)
   {
      print(m_telnetClient, m_logLevel.shift(), m_logs.shift());
   }
#else
   // print(Serial, m_logLevel.shift(), m_logs.shift());
#endif
}

void Log::write(Log::Level level, String desc, bool flush)
{
   if (flush)
   {
#if defined(DEBUG_TELNET)
      m_telnetClient.print("FLUSH: ");
      m_telnetClient.println(desc);
#else
      Serial.print("FLUSH: ");
      Serial.println(desc);
#endif
      return;
   }

   if (LOG_LEVEL <= level)
   {
      // Always print to Serial output
      print(Serial, level, desc);
#if defined(DEBUG_TELNET)
      m_logs.push(desc);
      m_logLevel.push(level);
#endif
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