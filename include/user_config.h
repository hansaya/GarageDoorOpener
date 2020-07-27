#ifndef user_config_h
#define user_config_h
/*   
 * GarHAge
 * a Home-Automation-friendly ESP8266-based MQTT Garage Door Controller
 * Licensed under the MIT License, Copyright (c) 2017 marthoc
 * 
 * User-configurable Parameters 
*/

#define DEBUG_SERIAL

// -- Project -------------------------------------------
#define CLIENT                 "GarageDoorTest"          // Client ID for the ESP

// MQTT Parameters

#define MQTT_BROKER "mqtt.hansperera.com"
#define MQTT_PORT               1883                // [MQTT] MQTT Port on the Server
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX  "t_homeassistant"

#define STATUS_LED 25
#define NEO_PIXEL_PIN 21 // v1 board
// #define NEO_PIXEL_PIN 23 // v2 board
#define BUTTON 0
#define ISO_IN_PIN 16

//CCS811 pins
#define SDA_CUSTOM_PIN 23 // v1 board
// #define SDA_CUSTOM_PIN 21 // V2 board
#define SDA_CUSTOM_PIN 22
#define CCS811_WAKE 13
#define CCS811_RST 14
#define CCS811_RST 15

// Relay Parameters
#define ACTIVE_HIGH_RELAY true

// Door 1 Parameters
#define DOOR1_ALIAS "Garage Door Left"
#define MQTT_DOOR1_ACTION_TOPIC "garageTest/door/1/action"
#define MQTT_DOOR1_STATUS_TOPIC "garageTest/door/1/status"
#define DOOR1_OPEN_PIN 2
#define DOOR1_CLOSE_PIN 2
#define DOOR1_STATUS_PIN 17
#define DOOR1_STATUS_SWITCH_LOGIC "NO"

// Door 2 Parameters
#define DOOR2_ENABLED true
#define DOOR2_ALIAS "Garage Door Right"
#define MQTT_DOOR2_ACTION_TOPIC "garageTest/door/2/action"
#define MQTT_DOOR2_STATUS_TOPIC "garageTest/door/2/status"
#define DOOR2_OPEN_PIN 4
#define DOOR2_CLOSE_PIN 4
#define DOOR2_STATUS_PIN 18
#define DOOR2_STATUS_SWITCH_LOGIC "NO"

// Door 3 Parameters
// #define DOOR2_ENABLED false
// #define DOOR2_ALIAS "Door 3"
// #define MQTT_DOOR2_ACTION_TOPIC "garage/door/3/action"
// #define MQTT_DOOR2_STATUS_TOPIC "garage/door/3/status"
// #define DOOR2_OPEN_PIN 12
// #define DOOR2_CLOSE_PIN 12
// #define DOOR2_STATUS_SWITCH_LOGIC "NO"

// Macros for debugging
#ifdef DEBUG_TELNET
   #define     DEBUG_TELNET_PORT 23
   WiFiServer  telnetServer(DEBUG_TELNET_PORT);
   WiFiClient  telnetClient;
   #define     DEBUG_PRINT(x)    telnetClient.print(x)
   #define     DEBUG_PRINT_WITH_FMT(x, fmt)    telnetClient.printf(x, fmt)
   #define     DEBUG_PRINTLN(x)  telnetClient.println(x)
   #define     DEBUG_PRINTLN_WITH_FMT(x, fmt)  telnetClient.println(x, fmt)
#elif defined(DEBUG_SERIAL)
   #define     DEBUG_PRINT(x)    Serial.print(x)
   #define     DEBUG_PRINT_WITH_FMT(x, fmt)    Serial.printf(x, fmt)
   #define     DEBUG_PRINTLN(x)  Serial.println(x)
   #define     DEBUG_PRINTLN_WITH_FMT(x, fmt)  Serial.println(x, fmt)
#else
   #define     DEBUG_PRINT(x)
   #define     DEBUG_PRINT_WITH_FMT(x, fmt)
   #define     DEBUG_PRINTLN(x)
   #define     DEBUG_PRINTLN_WITH_FMT(x, fmt)
#endif

#endif