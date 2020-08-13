#ifndef user_config_h
#define user_config_h

// #define DEBUG_SERIAL
#define DEBUG_TELNET

// -- Project -------------------------------------------
#define CLIENT "GarageDoorOpener" // Client ID for the ESP

// MQTT Parameters
#define MQTT_BROKER "example.com"
#define MQTT_PORT "1883" // [MQTT] MQTT Port on the Server
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX "homeassistant"

// Pins for peripherals
#define STATUS_LED 25
#define NEO_PIXEL_PIN 21 // v1 board
//#define NEO_PIXEL_PIN 23 // v2 board
#define BUTTON 0
#define ISO_IN_PIN 16

//CCS811 pins
#define SCL_CUSTOM_PIN 23 // v1 board
//#define SCL_CUSTOM_PIN 21 // V2 board
#define SDA_CUSTOM_PIN 22
#define CCS811_WAKE 13
#define CCS811_RST 14
#define CCS811_RST 15

// Relay Parameters
#define ACTIVE_HIGH_RELAY true

// door sensors
#define ACTIVE_HIGH_DOOR_SENSOR false

// Door 1 Parameters
#define DOOR1_ALIAS "Left Garage Door"
#define DOOR1_RELAY_PIN 2
#define DOOR1_STATUS_PIN 17

// Door 2 Parameters
// #define DOOR2_ALIAS "Door 2"
// #define DOOR2_RELAY_PIN 4
// #define DOOR2_STATUS_PIN 18

// Door 3 Parameters
#define DOOR3_ALIAS "Right Garage Door"
#define DOOR3_RELAY_PIN 12
#define DOOR3_STATUS_PIN 19

#endif