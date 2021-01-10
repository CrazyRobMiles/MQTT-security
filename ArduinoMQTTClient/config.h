#pragma once

#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32)

#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#include <WebServer.h>

#endif

#if defined(ARDUINO_ARCH_ESP8266)

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>

#endif
