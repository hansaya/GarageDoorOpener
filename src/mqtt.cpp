#include "mqtt.h"
#include "user_config.h"
#include "managed_wifi.h"

const char* mqtt_broker = MQTT_BROKER;
const char* mqtt_clientId = CLIENT;
const char* mqtt_username = MQTT_USERNAME;
const char* mqtt_password = MQTT_PASSWORD;

const char* door1_alias = DOOR1_ALIAS;
const char* mqtt_door1_action_topic = MQTT_DOOR1_ACTION_TOPIC;
const char* mqtt_door1_status_topic = MQTT_DOOR1_STATUS_TOPIC;
const int door1_openPin = DOOR1_OPEN_PIN;
const int door1_closePin = DOOR1_CLOSE_PIN;
const int door1_statusPin = DOOR1_STATUS_PIN;
const char* door1_statusSwitchLogic = DOOR1_STATUS_SWITCH_LOGIC;

const boolean door2_enabled = DOOR2_ENABLED;
const char* door2_alias = DOOR2_ALIAS;
const char* mqtt_door2_action_topic = MQTT_DOOR2_ACTION_TOPIC;
const char* mqtt_door2_status_topic = MQTT_DOOR2_STATUS_TOPIC;
const int door2_openPin = DOOR2_OPEN_PIN;
const int door2_closePin = DOOR2_CLOSE_PIN;
const int door2_statusPin = DOOR2_STATUS_PIN;
const char* door2_statusSwitchLogic = DOOR2_STATUS_SWITCH_LOGIC;

String availabilityBase = CLIENT;
String availabilitySuffix = "/availability";
String availabilityTopicStr = availabilityBase + availabilitySuffix;
const char* availabilityTopic = availabilityTopicStr.c_str();
const char* birthMessage = "online";
const char* lwtMessage = "offline";

void onMqttMessage(char* topic, byte* payload, unsigned int length)
{
    g_mqtt.mqttCalllBack (topic, payload, length);
}

void Mqtt::begin ()
{
    m_client.setServer(mqtt_broker, 1883);
    m_client.setCallback(onMqttMessage);
    snprintf(m_uniqueId, 20, "%02X%02X", g_managedWiFi.getMac()[4], g_managedWiFi.getMac()[5]);
}

// Function that publishes birthMessage
void Mqtt::publishBirthMessage() {
  // Publish the birthMessage
  DEBUG_PRINT("Publishing birth message \"");
  DEBUG_PRINT(birthMessage);
  DEBUG_PRINT("\" to ");
  DEBUG_PRINT(availabilityTopic);
  DEBUG_PRINTLN("...");
  m_client.publish(availabilityTopic, birthMessage, true);
}

// // Function called by callback() when a message is received 
// // Passes the message topic as the "requestedDoor" parameter and the message payload as the "requestedAction" parameter
// void Mqtt::triggerDoorAction(String requestedDoor, String requestedAction) {
//   if (requestedDoor == mqtt_door1_action_topic && requestedAction == "OPEN") {
//     DEBUG_PRINT("Triggering ");
//     DEBUG_PRINT(door1_alias);
//     DEBUG_PRINTLN(" OPEN relay!");
//     m_relay1.event = true;
//     m_relay1.cmdOn = true;
//     // toggleRelay(door1_openPin);
//   }
//   else if (requestedDoor == mqtt_door1_action_topic && requestedAction == "CLOSE") {
//     DEBUG_PRINT("Triggering ");
//     DEBUG_PRINT(door1_alias);
//     DEBUG_PRINTLN(" CLOSE relay!");
//     m_relay2.event = true;
//     m_relay1.cmdOn = false;
//     // toggleRelay(door1_closePin);
//   }
//   else if (requestedDoor == mqtt_door1_action_topic && requestedAction == "STATE") {
//     DEBUG_PRINT("Publishing on-demand status update for ");
//     DEBUG_PRINT(door1_alias);
//     DEBUG_PRINTLN("!");
//     publishBirthMessage();
//     publish_door1_status();
//   }
//   else if (requestedDoor == mqtt_door2_action_topic && requestedAction == "OPEN") {
//     DEBUG_PRINT("Triggering ");
//     DEBUG_PRINT(door2_alias);
//     DEBUG_PRINTLN(" OPEN relay!");
//     // toggleRelay(door2_openPin);
//   }
//   else if (requestedDoor == mqtt_door2_action_topic && requestedAction == "CLOSE") {
//     DEBUG_PRINT("Triggering ");
//     DEBUG_PRINT(door2_alias);
//     DEBUG_PRINTLN(" CLOSE relay!");
//     // toggleRelay(door2_closePin);
//   }
//   else if (requestedDoor == mqtt_door2_action_topic && requestedAction == "STATE") {
//     DEBUG_PRINT("Publishing on-demand status update for ");
//     DEBUG_PRINT(door2_alias);
//     DEBUG_PRINTLN("!");
//     publishBirthMessage();
//     publish_door2_status();
//   }  
//   else { DEBUG_PRINTLN("Unrecognized action payload... taking no action!");
//   }
// }

// Callback when MQTT message is received; calls triggerDoorAction(), passing topic and payload as parameters
void Mqtt::mqttCalllBack(char* topic, byte* payload, unsigned int length) {
    DEBUG_PRINT("Message arrived [");
    DEBUG_PRINT(topic);
    DEBUG_PRINT("] ");

    for (int i = 0; i < length; i++) {
        DEBUG_PRINT((char)payload[i]);
    }

    DEBUG_PRINTLN();

    String topicToProcess = topic;
    payload[length] = '\0';
    String payloadToProcess = (char*)payload;

    // Call the call backs if the topic matches.
    for (int i=0; i<m_subTopicCnt; i++)
    {
        if (m_topics[i] == topicToProcess && m_callBacks[i])
            m_callBacks[i](payloadToProcess);
    }
}

// Function that runs in loop() to connect/reconnect to the MQTT broker, and publish the current door statuses on connect
void Mqtt::reconnect() {
  // Loop until we're reconnected
  while (!m_client.connected()) {
    DEBUG_PRINT("Attempting MQTT connection...");
    // Attempt to connect
    if (m_client.connect(mqtt_clientId, mqtt_username, mqtt_password, availabilityTopic, 0, true, lwtMessage)) {
      DEBUG_PRINTLN("Connected!");

      // Publish the birth message on connect/reconnect
      publishBirthMessage();

    //   // Subscribe to the action topics to listen for action messages
    //   DEBUG_PRINT("Subscribing to ");
    //   DEBUG_PRINT(mqtt_door1_action_topic);
    //   DEBUG_PRINTLN("...");
    //   m_client.subscribe(mqtt_door1_action_topic);
      
    //   if (door2_enabled) {
    //     DEBUG_PRINT("Subscribing to ");
    //     DEBUG_PRINT(mqtt_door2_action_topic);
    //     DEBUG_PRINTLN("...");
    //     m_client.subscribe(mqtt_door2_action_topic);
    //   }

      // Publish the current door status on connect/reconnect to ensure status is synced with whatever happened while disconnected
    //   publish_door1_status();
    //   publish_door2_status();
    } 
    else {
      DEBUG_PRINT("failed, rc=");
      DEBUG_PRINT(m_client.state());
      DEBUG_PRINTLN(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// // Functions that check door status and publish an update when called
// void Mqtt::publish_door1_status() {
//   if (digitalRead(door1_statusPin) == LOW) {
//     if (door1_statusSwitchLogic == "NO") {
//       DEBUG_PRINT(door1_alias);
//       DEBUG_PRINT(" closed! Publishing to ");
//       DEBUG_PRINT(mqtt_door1_status_topic);
//       DEBUG_PRINTLN("...");
//       m_client.publish(mqtt_door1_status_topic, "closed", true);
//     }
//     else if (door1_statusSwitchLogic == "NC") {
//       DEBUG_PRINT(door1_alias);
//       DEBUG_PRINT(" open! Publishing to ");
//       DEBUG_PRINT(mqtt_door1_status_topic);
//       DEBUG_PRINTLN("...");
//       m_client.publish(mqtt_door1_status_topic, "open", true);      
//     }
//     else {
//       DEBUG_PRINTLN("Error! Specify only either NO or NC for DOOR1_STATUS_SWITCH_LOGIC in config.h! Not publishing...");
//     }
//   }
//   else {
//     if (door1_statusSwitchLogic == "NO") {
//       DEBUG_PRINT(door1_alias);
//       DEBUG_PRINT(" open! Publishing to ");
//       DEBUG_PRINT(mqtt_door1_status_topic);
//       DEBUG_PRINTLN("...");
//       m_client.publish(mqtt_door1_status_topic, "open", true);
//     }
//     else if (door1_statusSwitchLogic == "NC") {
//       DEBUG_PRINT(door1_alias);
//       DEBUG_PRINT(" closed! Publishing to ");
//       DEBUG_PRINT(mqtt_door1_status_topic);
//       DEBUG_PRINTLN("...");
//       m_client.publish(mqtt_door1_status_topic, "closed", true);      
//     }
//     else {
//       DEBUG_PRINTLN("Error! Specify only either NO or NC for DOOR1_STATUS_SWITCH_LOGIC in config.h! Not publishing...");
//     }
//   }
// }

// void Mqtt::publish_door2_status() {
//   if (digitalRead(door2_statusPin) == LOW) {
//     if (door2_statusSwitchLogic == "NO") {
//       DEBUG_PRINT(door2_alias);
//       DEBUG_PRINT(" closed! Publishing to ");
//       DEBUG_PRINT(mqtt_door2_status_topic);
//       DEBUG_PRINTLN("...");
//       m_client.publish(mqtt_door2_status_topic, "closed", true);
//     }
//     else if (door2_statusSwitchLogic == "NC") {
//       DEBUG_PRINT(door2_alias);
//       DEBUG_PRINT(" open! Publishing to ");
//       DEBUG_PRINT(mqtt_door2_status_topic);
//       DEBUG_PRINTLN("...");
//       m_client.publish(mqtt_door2_status_topic, "open", true);      
//     }
//     else {
//       DEBUG_PRINTLN("Error! Specify only either NO or NC for DOOR2_STATUS_SWITCH_LOGIC in config.h! Not publishing...");
//     }
//   }
//   else {
//     if (door2_statusSwitchLogic == "NO") {
//       DEBUG_PRINT(door2_alias);
//       DEBUG_PRINT(" open! Publishing to ");
//       DEBUG_PRINT(mqtt_door2_status_topic);
//       DEBUG_PRINTLN("...");
//       m_client.publish(mqtt_door2_status_topic, "open", true);
//     }
//     else if (door2_statusSwitchLogic == "NC") {
//       DEBUG_PRINT(door2_alias);
//       DEBUG_PRINT(" closed! Publishing to ");
//       DEBUG_PRINT(mqtt_door2_status_topic);
//       DEBUG_PRINTLN("...");
//       m_client.publish(mqtt_door2_status_topic, "closed", true);      
//     }
//     else {
//       DEBUG_PRINTLN("Error! Specify only either NO or NC for DOOR2_STATUS_SWITCH_LOGIC in config.h! Not publishing...");
//     }
//   }
// }

// Publish the MQTT payload.
void Mqtt::publishToMQTT(const char* p_topic,const char* p_payload) {
  if (m_client.publish(p_topic, p_payload, true)) {
    DEBUG_PRINT(F("INFO: MQTT message published successfully, topic: "));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F(", payload: "));
    DEBUG_PRINTLN(p_payload);
  } else {
    DEBUG_PRINTLN(F("ERROR: MQTT message not published, either connection lost, or message too large. Topic: "));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F(" , payload: "));
    DEBUG_PRINTLN(p_payload);
  }
}

void Mqtt::addCallBack (String topic, TOPIC_CALLBACK_SIGNATURE callback)
{
    m_topics[m_subTopicCnt]=topic;
    m_callBacks[m_subTopicCnt]=callback;
    m_subTopicCnt++;
}

Mqtt g_mqtt;