# Secure sockets with MQTT

Now we need to make the MQTT server use secure sockets. We do this from a terminal connection. We are going to create some key files and add them to our Mosqitto installation. Then we are going to edit the configuration file of the MQTT broker to tell it to use these certificates to encrypt connections to remote devices. 
## Install openssl
 First we need openssl to make the broker certificates and keys:
```
sudo apt-get install openssl
```
We might not have to do this, it may be already installed - in which case we get told so by the installer.
## Make the certificate authority key
The certificate authority key is the daddy of our security. We are going to make a *self signed* key which we can use to create keys that can be used to encrypt conversations. Keys made from this key can't be used to authenticate an https site. If you want a "proper" certificate authority key that can be used to both encrpyt messages and authenticate hosts you should take a look at Let's Encrypt which you can find [here](https://letsencrypt.org/getting-started/).
```
openssl genrsa -des3 -out ca.key 2048
```
During the key creation process you will be asked to enter a key password which you will use each time you use this authority to make a new certificate. I used: 
```
great-big-green-banjo
```
.. or did i?

The output from the genrsa option to the openssl program is a file called **ca.key** (that's the name we asked for). Keep this file safe as it is the key to your security regime.
## Make a certificate using the authority key
Now we can use the certificate authority key to make a certificate:
```
openssl req -new -x509 -days 1826 -key ca.key -out ca.crt
```
The req option requests a new key, using our authority key to make it. You will now have a conversation with the option:
```
Enter pass phrase for ca.key:
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:UK
State or Province Name (full name) [Some-State]:Yorkshire
Locality Name (eg, city) []:Hull
Organization Name (eg, company) [Internet Widgits Pty Ltd]:Clever Little Boxes
Organizational Unit Name (eg, section) []:Devices
Common Name (e.g. server FQDN or YOUR name) []:ns349814.ip-178-32-221.eu
Email Address []:rob@robmiles.com
```
The output is stored in a file called **ca.crt** (again,that's what we asked for).

## Make a server key pair
We now have a certificate. Next thing is to make a server key pair that can be used by the broker.
``` 
openssl genrsa -out server.key 2048
```
This maks a file called **server.key** which we can use to make a certificate request file:
## Make a certificate request file
Now need to create a certificate request file:
```
openssl req -new -out server.csr -key server.key
```
You now get to have another chat with openssl:
```
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:UK
State or Province Name (full name) [Some-State]:Yorkshire
Locality Name (eg, city) []:Hull
Organization Name (eg, company) [Internet Widgits Pty Ltd]:Just a name
Organizational Unit Name (eg, section) []:Devices
Common Name (e.g. server FQDN or YOUR name) []:myserver.mydomain.com
Email Address []:name@yourmailaddress.com

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:
An optional company name []:
```
Note that you need to put theFQDN (fully qualified domain name) of your server where I indicated. The output is file **server.csr** (yet again - that's what you asked for). 
##Make a key file for Mosquitto
Now need to package this into a file that can be given to Mosquitto.
```
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 360
```
You’ll be asked for the CA password which in my case was all about green banjos. The output from this will be stored in a file called **server.crt**.
## Copy the certificate files into the Mosquitto installation
Now we need to copy the certificate files into the Mosquitto installation:
```
sudo cp ca.crt /etc/mosquitto/ca_certificates
sudo cp server.key /etc/mosquitto/certs/
sudo cp server.crt /etc/mosquitto/certs/
```
Next we edit the configuration file.  
```
sudo nano /etc/mosquitto/mosquitto.conf
```
This is the file I ended up with. You could do worse than make yours look the same. Note that I've got two listeners set up so that I can use insecure clients (such as the ESP8266 which doesn't have enough memory to do what I want and support SSL) alongside the secure ones.
```
# Place your local configuration in /etc/mosquitto/conf.d/
#
# A full description of the configuration file is at
# /usr/share/doc/mosquitto/examples/mosquitto.conf.example

pid_file /var/run/mosquitto.pid

persistence true
persistence_location /var/lib/mosquitto/

log_dest file /var/log/mosquitto/mosquitto.log
log_type error
log_type warning
log_type notice
log_type information

connection_messages true
log_timestamp true

password_file /etc/mosquitto/pwfile

# We allow non-ssl connections so that
# we can use ESP8266 devices as clients

listener 1883

# Also have settings for ssl

listener 8883
cafile /etc/mosquitto/ca_certificates/ca.crt
keyfile /etc/mosquitto/certs/server.key
certfile /etc/mosquitto/certs/server.crt
require_certificate false

# end of ssl settings

include_dir /etc/mosquitto/conf.d
```
Now we need to restart the Mosquitto broker to cause it to reload the settings. 
```
sudo /etc/init.d/mosquitto restart
```
# Client
Now we have to modify our client devices to support the use of secure sockets. 
## Arduino ESP32 and ESP8266
The WiFi libaries for these devices are similar, but not quite the same. I use the following to grab the right libaries for each:
```
#if defined(ARDUINO_ARCH_ESP32)

#include <Arduino.h>
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
```
Using this at the start of your program means that your C++ code can be mostly the same for both platforms. 

I use pubsubclient to make MQTT connections in Arduino C++ programs. You can find it [here](https://github.com/knolleary/pubsubclient). To make this work over secure sockets I have to ask it to use a secure socket.

```
PubSubClient *mqttPubSubClient = NULL;

// later in my setup code

if (mqttPubSubClient == NULL)
{
	if (mqttSettings.mqttSecureSockets)
	{
		WiFiClientSecure * secureClient = new WiFiClientSecure();
#if defined(ARDUINO_ARCH_ESP8266)
		secureClient->setInsecure();
#endif			
		mqttPubSubClient = new PubSubClient(*secureClient);
	}
	else
	{
		WiFiClient * espClient = new WiFiClient();
		mqttPubSubClient = new PubSubClient(*espClient);
	}

	mqttPubSubClient->setBufferSize(MQTT_BUFFER_SIZE_MAX);

	mqttPubSubClient->setServer(mqttSettings.mqttServer, mqttSettings.mqttPort);
	mqttPubSubClient->setCallback(callback);
}
```
Note the extra line to explicitly set insecure mode when using the ESP8266 device. This means that the SSL will work with self-signed certificates like ours. The ESP32 has this option set by default. This means that our connection will be encrypted but that the client will not validate the identity of the server.  
## MicroPython
If you want to use secure sockets with MicroPython you seem to have to download a certificate file into your device for it to use when it makes a connection. This makes the connection more secure, but it is much harder to use. 
```
def connectMQTT():
    from umqtt.simple import MQTTClient
    CERT_PATH = "certificate.cer"
    print('getting cert')
    with open(CERT_PATH, 'r') as f:
        cert = f.read()
    print('got cert')
    sslparams = {'cert':cert}
    CLIENT_ID='MicroPythonTest'
    Username='username here'
    Password='password here'
    mqtt=MQTTClient(client_id=CLIENT_ID,server='server here',port=8883,user=Username,password=Password, keepalive=4000, ssl=True, ssl_params=sslparams)
    mqtt.set_callback(callback)
    mqtt.connect(False)
    mqtt.subscribe('some topic')
    return mqtt
```
The function above reads a certificate file and then adds this into the client connection to build an mqtt connection client you can use in your MicroPython program. 
To make a certificate file you use this command on your server, in the same folder where your other certificate files are stored:
```
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out certificate.cer -days 360
```
Then copy the certificate.cer file onto your microPython device to use in the connection. I use the program Thonny to connect to my MicroPython devices and transfer files into them. You can find this program [here](https://thonny.org/)
