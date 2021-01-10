#pragma once

#include "config.h"

class WiFiConnection
{
public:
	WiFiConnection();

	bool connect(char* ssid, char* password, int timeOutInSeconds);

};
