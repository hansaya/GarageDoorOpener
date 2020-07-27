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

// Functions that run in loop() to check each loop if door status (open/closed) has changed and call publish_doorX_status() to publish any change if so
// void check_door1_status() {
//   int currentStatusValue = digitalRead(door1_statusPin);
//   if (currentStatusValue != door1_lastStatusValue) {
//     unsigned int currentTime = millis();
//     if (currentTime - door1_lastSwitchTime >= debounceTime) {
//       publish_door1_status();
//       door1_lastStatusValue = currentStatusValue;
//       door1_lastSwitchTime = currentTime;
//     }
//   }
// }

// void check_door2_status() {
//   int currentStatusValue = digitalRead(door2_statusPin);
//   if (currentStatusValue != door2_lastStatusValue) {
//     unsigned int currentTime = millis();
//     if (currentTime - door2_lastSwitchTime >= debounceTime) {
//       // publish_door2_status();
//       door2_lastStatusValue = currentStatusValue;
//       door2_lastSwitchTime = currentTime;
//     }
//   }
// }

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
  leftDoor.begin ();
  rightDoor.begin ();

  Serial.begin(115200);

  // Connect to access point
  g_managedWiFi.begin ();

  // Wire.begin(22,23);
  pinMode(CCS811_WAKE, OUTPUT);
  digitalWrite(CCS811_WAKE, LOW);
  if(!ccs.begin(0x5A)){
    DEBUG_PRINTLN("Failed to start CCS811 sensor! Please check your wiring.");
  }
  else
    ccs.setTempOffset (14.0);
  digitalWrite(CCS811_WAKE, HIGH);
  
  // Setup the output and input pins used in the sketch
  // Set the lastStatusValue variables to the state of the status pins at setup time

  // // Setup Door 1 pins
  // pinMode(door1_openPin, OUTPUT);
  // pinMode(door1_closePin, OUTPUT);
  // // Set output pins LOW with an active-high relay
  // if (activeHighRelay) {
  //   digitalWrite(door1_openPin, LOW);
  //   digitalWrite(door1_closePin, LOW);
  // }
  // // Set output pins HIGH with an active-low relay
  // else {
  //   digitalWrite(door1_openPin, HIGH);
  //   digitalWrite(door1_closePin, HIGH);
  // }
  // // Set input pin to use internal pullup resistor
  // pinMode(door1_statusPin, INPUT_PULLUP);
  // // Update variable with current door state
  // door1_lastStatusValue = digitalRead(door1_statusPin);

  // // Setup Door 2 pins
  // if (door2_enabled) {
  //   pinMode(door2_openPin, OUTPUT);
  //   pinMode(door2_closePin, OUTPUT);
  //   // Set output pins LOW with an active-high relay
  //   if (activeHighRelay) {
  //     digitalWrite(door2_openPin, LOW);
  //     digitalWrite(door2_closePin, LOW);
  //   }
  //   // Set output pins HIGH with an active-low relay
  //   else {
  //     digitalWrite(door2_openPin, HIGH);
  //     digitalWrite(door2_closePin, HIGH);
  //   }
  //   // Set input pin to use internal pullup resistor
  //   pinMode(door2_statusPin, INPUT_PULLUP);
  //   // Update variable with current door state
  //   door2_lastStatusValue = digitalRead(door2_statusPin);
  // }

  DEBUG_PRINTLN("Starting GarHAge...");

  if (activeHighRelay) {
    DEBUG_PRINTLN("Relay mode: Active-High");
  }
  else {
    DEBUG_PRINTLN("Relay mode: Active-Low");
  }

  g_ota.begin ();
}

void loop() {
  unsigned long currentMillis = millis (); 
  g_ota.loop ();
  // Connect/reconnect to the MQTT broker and listen for messages
  if (!g_ota.busy ())
  {
    // if (!client.connected()) {
    //   reconnect();
    // }
    // client.loop();
    // Check door open/closed status each loop and publish changes
    // check_door1_status();
    // check_door2_status();

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