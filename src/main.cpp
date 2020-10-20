/*   
 * A home assistant friendly esp32-based MQTT Garage Door Controller
 * Licensed under the MIT License, Copyright (c) 2017 hansaya
*/

#include <Arduino.h>
#include "ota.h"
#include "config.h"
#include "managed_wifi.h"
#include "button.h"
#include "led.h"
#include "garage_door.h"
#include "mqtt.h"
#include "ccs811.h"
#include "log.h"

// Garage door config.
GarageDoor leftDoor("LeftGarageDoor", DOOR1_RELAY_PIN, DOOR1_STATUS_PIN, DOOR1_ALIAS);
GarageDoor rightDoor("RightGarageDoor", DOOR3_RELAY_PIN, DOOR3_STATUS_PIN, DOOR3_ALIAS);

// Setup a button with events.
ButtonEvents sortButtonPress({50, false, true});
ButtonEvents longButtonPress({800, false, true});
ButtonEvents resetButtonPress({6000, false, true});
Button<3> gpio0Button(BUTTON, LOW, &sortButtonPress, &longButtonPress, &resetButtonPress);

// Handle button inputs
void handleButtonInput()
{
  if (sortButtonPress.pressed)
  {
    sortButtonPress.pressed = false;
    g_log.write(Log::Debug, "S");
  }
  if (longButtonPress.pressed)
  {
    longButtonPress.pressed = false;
    g_log.write(Log::Debug, "L");
  }
  if (resetButtonPress.pressed)
  {
    resetButtonPress.pressed = false;
    g_log.write(Log::Debug, "R");
    g_managedWiFi.manageWiFi(true);
  }
}

void setup()
{
  Serial.begin(115200);

  // Read the flash for the previous config
  g_config.begin();
  // Function button setup.
  gpio0Button.begin();
  // Led Setup
  g_led.begin();

  g_log.write(Log::Debug, "Starting GarHAge...");

  // Connect to access point
  g_managedWiFi.begin();

  // Start the logger.
  g_log.begin();

  // Start the services.
  g_ota.begin();
  g_mqtt.begin();
  leftDoor.begin();
  rightDoor.begin();
  g_ccs811.begin();

  // Shows a green color to indicate startup
  g_led.setPixColor(CRGB::Green);
  g_led.showPixColor();
}

void loop()
{
  g_ota.loop();
  g_managedWiFi.loop();
  g_log.loop();

  // routine functions.
  if (!g_ota.busy())
  {
    g_mqtt.loop();
    leftDoor.loop();
    rightDoor.loop();
    g_ccs811.loop();

    if (digitalRead(ISO_IN_PIN) == HIGH)
    {
      g_led.setPixColor(CRGB::Purple);
      g_led.showPixColor();
    }

    handleButtonInput();
  }
}