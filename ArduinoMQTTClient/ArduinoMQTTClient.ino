#include "MQTT.h"
#include "WiFiConnection.h"

// Tiny Demo program for secure MQTT connections

// Can be deployed to ESP32 or ESP8266
// Makes a connection and subscribes to a topic
// Add your WiFi and MQTT credentials to make it work

// Enjoy.  Rob Miles January 2021

WiFiConnection* wifiConnection;
MQTT* mqtt;

void messageDump(char* title, unsigned char* buffer, unsigned int length)
{
    Serial.print(title);
    for (int i = 0; i < length; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void MQTTMessageReceiver(unsigned char* buffer, unsigned int length)
{
    messageDump("MQTT message received: ", buffer, length);
}

void setup() {

  Serial.begin(115200);

  Serial.println(
    "\n\nDemo MQTT Client\n\n"
    "Connects to the server and says hello\n");

  wifiConnection = new WiFiConnection();

  while (!wifiConnection->connect(
    "your WiFi SSID", 
    "your WiFi password", 
    10
    ))
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("Got WiFi");

  Serial.println("Connecting MQTT");

  mqtt = new MQTT();

  while (!mqtt->connect(
           "Demo",  // device name
           "your mqtt host", // mqtt host
           "your mqtt username",       // username
           "your mqtt password",       // password
           "DemoIn",                   // topic subscribed to
           "DemoOut",                   // topic published to
           true,                      // true for secure
           10                          // timeout in secs
           )) {
    Serial.println(".");
  }

  unsigned char message [] = { 1,2,3,4,5,6 };
  
  mqtt->sendMessageToMQTT(message,sizeof(message));

  mqtt->bindToIncomingMQTTMessages(MQTTMessageReceiver);

  Serial.println("MQTT connected");
}

void loop() {
  mqtt->loop();
  delay(100);
}
