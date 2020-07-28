/*   
 * GarHAge
 * a Home-Automation-friendly ESP8266-based MQTT Garage Door Controller
 * Licensed under the MIT License, Copyright (c) 2017 marthoc
*/

#include <Arduino.h>
#include "ota.h"
#include "config.h"
#include "managed_wifi.h"
#include "input.h"
#include "led.h"
#include "Adafruit_CCS811.h"
#include "garage_door.h"
#include "mqtt.h"

const int relayActiveTime = 500;
int door1_lastStatusValue = 2;
int door2_lastStatusValue = 2;
unsigned long door1_lastSwitchTime = 0;
unsigned long door2_lastSwitchTime = 0;
int debounceTime = 2000;

const boolean activeHighRelay = ACTIVE_HIGH_RELAY;

Adafruit_CCS811 ccs;
GarageDoor leftDoor("LeftGarageDoor", DOOR1_OPEN_PIN, DOOR1_STATUS_PIN);
GarageDoor rightDoor("RightGarageDoor", DOOR2_OPEN_PIN, DOOR2_STATUS_PIN);

// Function that toggles the relevant relay-connected output pin
void toggleRelay(int pin) {
  if (activeHighRelay) {
    digitalWrite(pin, HIGH);
    delay(relayActiveTime);
    digitalWrite(pin, LOW);
  }
  else {
    digitalWrite(pin, LOW);
    delay(relayActiveTime);
    digitalWrite(pin, HIGH);
  }
}

// Handle input
void handleButtonInput ()
{
  if (g_input.GetButton().events[0]->pressed)
  {
    g_input.GetButton().events[0]->pressed = false;
    Serial.print("S");
  }
  if (g_input.GetButton().events[1]->pressed)
  {
    g_input.GetButton().events[1]->pressed = false;
    Serial.print("L");
  }
  if (g_input.GetButton().events[2]->pressed)
  {
    g_input.GetButton().events[2]->pressed = false;
    Serial.print("R");
    g_managedWiFi.manageWiFi(true);
  }
}

void setup() {
  g_input.begin ();
  g_led.begin ();


  Serial.begin(115200);

  DEBUG_PRINTLN("Starting GarHAge...");

  // Connect to access point
  g_managedWiFi.begin ();

#if defined(DEBUG_TELNET)
  telnetServer.begin();
  telnetServer.setNoDelay(true);
#endif

  // Wire.begin(22,23);
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