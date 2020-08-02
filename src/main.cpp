/*   
 * GarHAge
 * a Home-Automation-friendly ESP8266-based MQTT Garage Door Controller
 * Licensed under the MIT License, Copyright (c) 2017 marthoc
*/

#include <Arduino.h>
#include "ota.h"
#include "config.h"
#include "managed_wifi.h"
#include "button.h"
#include "led.h"
#include "Adafruit_CCS811.h"
#include "garage_door.h"
#include "mqtt.h"

Adafruit_CCS811 ccs;
GarageDoor leftDoor("LeftGarageDoor", DOOR1_RELAY_PIN, DOOR1_STATUS_PIN, "Left Garage Door");
GarageDoor rightDoor("RightGarageDoor", DOOR3_RELAY_PIN, DOOR3_STATUS_PIN, "Right Garage Door");

ButtonEvents sortButtonPress ({50, false, true});
ButtonEvents longButtonPress ({800, false, true});
ButtonEvents resetButtonPress ({6000, false, true});
Button<3> gpio0Button (BUTTON, LOW, &sortButtonPress, &longButtonPress, &resetButtonPress);

// Handle input
void handleButtonInput ()
{
  if (sortButtonPress.pressed)
  {
    sortButtonPress.pressed = false;
    Serial.print("S");
  }
  if (longButtonPress.pressed)
  {
    longButtonPress.pressed = false;
    Serial.print("L");
  }
  if (resetButtonPress.pressed)
  {
    resetButtonPress.pressed = false;
    Serial.print("R");
    g_managedWiFi.manageWiFi(true);
  }
}

void setup() {
  Serial.begin(115200);

  gpio0Button.begin ();
  g_led.begin ();

  DEBUG_PRINTLN("Starting GarHAge...");

  // Connect to access point
  g_managedWiFi.begin ();

#if defined(DEBUG_TELNET)
  telnetServer.begin();
  telnetServer.setNoDelay(true);
#endif

  // Setup cc811 sensor
  pinMode(CCS811_WAKE, OUTPUT);
  digitalWrite(CCS811_WAKE, LOW);
  if(!ccs.begin(0x5A)){
    DEBUG_PRINTLN("Failed to start CCS811 sensor! Please check your wiring.");
  }
  else
    ccs.setTempOffset (14.0);
  digitalWrite(CCS811_WAKE, HIGH);

  g_ota.begin ();
  g_mqtt.begin ();
  leftDoor.begin ();
  rightDoor.begin ();
}

void loop() {
  unsigned long currentMillis = millis (); 
  g_ota.loop ();

// handle the Telnet connection
#if defined(DEBUG_TELNET)
   handleTelnet();
#endif

  if (!g_ota.busy ())
  {
    g_mqtt.loop ();
    leftDoor.loop ();
    rightDoor.loop ();

    if (digitalRead (ISO_IN_PIN) == HIGH)
    {
      g_led.SetPixColor (CRGB::Purple);
      g_led.ShowPixColor();
    }

    static unsigned long mqttConnectWaitPeriod;
    if (currentMillis - mqttConnectWaitPeriod >= 30000)
    {
      mqttConnectWaitPeriod = currentMillis;

      // CCS811
      digitalWrite(CCS811_WAKE, LOW);
      if(ccs.available()){
        if(!ccs.readData()){
          DEBUG_PRINT("CO2: ");
          DEBUG_PRINT(ccs.geteCO2());
          DEBUG_PRINT("ppm, TVOC: ");
          DEBUG_PRINT(ccs.getTVOC());
          DEBUG_PRINT(" Temp: ");
          DEBUG_PRINTLN(ccs.calculateTemperature());
        }
        else
        {
          g_led.SetPixColor (CRGB::Red);
          g_led.ShowPixColor();
        }
      }
      else
      {
        g_led.SetPixColor (CRGB::Orange);
        g_led.ShowPixColor();
      }
      digitalWrite(CCS811_WAKE, HIGH);
    }

    handleButtonInput ();
  }

}