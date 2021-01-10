#include "MQTT.h"


void (*MQTT::boundMessageReceiver)(unsigned char* buffer, unsigned int length);


MQTT::MQTT()
{
	boundMessageReceiver = NULL;
	mqttState = turnedOff;
}

void MQTT::mqttCallback(char* topic, byte* payload, unsigned int length)
{
	Serial.println("Got a message");

	sendMessageFromMQTT((unsigned char *) payload, length);
}

bool MQTT::connect(char * device, char* host, char* username, char* password, char* receiveTopic, char* publishTopic, bool secure, int timeOutInSeconds)
{
	unsigned long timeOutInMillis = timeOutInSeconds * 10000;
	unsigned long startTimeMillis = millis();

	int port;

	mqttState = mqttDisconnected;

	if (mqttPubSubClient == NULL)
	{
		if (secure)
		{
      WiFiClientSecure *secureClient = new WiFiClientSecure();
#if defined(ARDUINO_ARCH_ESP8266)
      secureClient->setInsecure();
#endif
    mqttPubSubClient = new PubSubClient(*secureClient);
		}
		else
		{
      WiFiClient *espClient = new WiFiClient();
      mqttPubSubClient = new PubSubClient(*espClient);
		}
	}

	if (secure)
	{
		port = 8883;
	}
	else
	{
		port = 1883;
	}

	mqttPubSubClient->setServer(host, port);
	mqttPubSubClient->setCallback(mqttCallback);

	while (!mqttPubSubClient->connect(device, username, password))
	{
		unsigned long elapsedTimeMillis = millis() - startTimeMillis;

		if (elapsedTimeMillis > timeOutInMillis)
		{
			mqttPubSubClient->disconnect();
			return false;
		}

		delay(100);
	}

	snprintf(mqttReceiveTopic, MAX_MQTT_TOPIC_LENGTH, "%s", receiveTopic);

	snprintf(mqttPublishTopic, MAX_MQTT_TOPIC_LENGTH, "%s", publishTopic);

	Serial.printf("MQTT subscribed to %s\n", mqttReceiveTopic);
	Serial.printf("MQTT publish to to %s\n", mqttPublishTopic);

	mqttPubSubClient->subscribe(mqttReceiveTopic);

	mqttState = mqttConnected;

	return true;
}

void MQTT::bindToIncomingMQTTMessages(void (*receiveMessage)(unsigned char* buffer, unsigned int length))
{
	boundMessageReceiver = receiveMessage;
}

void MQTT::sendMessageFromMQTT(unsigned char* buffer, unsigned int length)
{
	if (boundMessageReceiver != NULL)
		boundMessageReceiver(buffer, length);
}


bool MQTT::sendMessageToMQTT(unsigned char* buffer, unsigned int length)
{
	return mqttPubSubClient->publish(mqttPublishTopic, buffer, length);
}

void MQTT::loop()
{
	mqttPubSubClient->loop();
}


bool MQTT::connected()
{
	return mqttState == mqttConnected;;
}
