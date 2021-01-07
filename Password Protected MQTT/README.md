# Add passwords to an MQTT broker
If you want to add passwords to your MQTT installation, this is how you do it. You need to configure your Mosquitto broker and then add the username and passwords to your device programs.
## Mosquitto Config

To perform this step you will have to be logged into the machine that is running the MQTT broker.
### Adding a password to an MQTT Broker
The password for an MQTT broker is held in a file which is then specified in the MQTT configuration file.
#### Create the password
Start by creating the password:
```
sudo mosquitto_passwd -c /etc/mosquitto/pwfile myMQTTuser
```
This statement creates a new password file called pwfile in the mosquitto folder. The username being created is myMQTTuser. You will be prompted twice to enter the password. Note that this password text will not be stored anywhere, so you need to make a copy of it.
#### Add the password to the Mosquitto configuration
Now that you have a password file you must tell the Mosquitto broker to use the file. Start by opening the configuration file in the nano editor:
```
sudo nano /etc/mosquitto/mosquitto.conf
```
Now add the line:
```
password_file /etc/mosquitto/pwfile
```
Write out the modified file (CTRL+O), exit the editor (CTRL+X) and restart the broker. 
```
sudo /etc/init.d/mosquitto restart
```
Now your clients will have to give the username and password when they connect. See the section on Client config to discover how to do this in MicroPython and Arduino C++.
## Client 
Your client devices will have to give the username and password that you set up when they make their connections. 
### Arduino C++
I use pubsubclient to make MQTT connections in Arduino C++ programs. You can find it [here](https://github.com/knolleary/pubsubclient).
```
mqttPubSubClient->connect(mqttSettings.mqttDeviceName, mqttSettings.mqttUser, mqttSettings.mqttPassword)
```
In the call of the connect function you give the name your device will have on the broker, the username and the password.
### Micro Python
I use the umqttrobust library you can find [here](https://github.com/micropython/micropython-lib/tree/master/umqtt.robust): You specify the username and password when you connect:
```
from umqtt.robust import MQTTClient

client = MQTTClient(client_id=devname,server=host,user=username,password=password, keepalive=4000)
```
The host is the url of the MQTT broker, the client_id is the name your device will have on the broker. Fill the username and password with the values that you used set for the broker.