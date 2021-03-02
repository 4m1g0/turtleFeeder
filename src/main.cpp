#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

Servo servo1;
Servo servo2;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
void open(int pin);
uint8_t mqttFailed = 0;


void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(9600);
  servo1.attach(D5);
  servo2.attach(D6);
  servo1.write(0);
  servo2.write(0);
  WiFi.mode(WIFI_STA);
  WiFi.begin("***", "***");
  if (!WiFi.getAutoConnect()) { WiFi.setAutoConnect(true); } 

  while (WiFi.status() != WL_CONNECTED)
  {
    static int i = 0;
    i++;
    if (i>120) // 1 min
      ESP.restart();
    Serial.print(".");
    delay(500);
  }

  Serial.println("Connected to network!");

  client.setServer("****", 1883);
  client.setCallback(callback);
  delay(100);
  reconnect();
}

void loop() {
  
  client.loop(); // MQTT update

  if (!client.connected()) { // Reconnect to MQTT if necesary
    reconnect();
  }

}


void reconnect() {
  static unsigned long lastConnection = 0;

  if (client.connected() || millis() - lastConnection < 3000)
    return;
  
  lastConnection = millis();

  wifiClient = WiFiClient();               // Wifi Client reconnect issue 4497 (https://github.com/esp8266/Arduino/issues/4497) (From Tasmota)
  client.setClient(wifiClient);

  if (client.connect(String(ESP.getChipId()).c_str(), "***", "***")) {
    Serial.println("MQTT connected");
    client.subscribe("turtleFeeder/open/1");
    client.subscribe("turtleFeeder/open/2");
    mqttFailed = 0;
    client.publish("turtleFeeder/ack/connected", "1");
  } 
  else {  
    Serial.println("MQTT Connection failed");
    if (mqttFailed > 20) { // if failed for 1 minute, reset
      ESP.restart();
    }
    mqttFailed++;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (!strcmp(topic, "turtleFeeder/open/1")) {
    Serial.println("open 1");
    open(1);
    return;
  } 
  if (!strcmp(topic, "turtleFeeder/open/2")) {
    Serial.println("open 2");
    open(2);
  } 
}

void open(int pin) {
  if (pin == 1) {
    servo1.write(170); 
    client.publish("turtleFeeder/ack/1", "1");
  }

  if (pin == 2) {
    servo2.write(180); 
    client.publish("turtleFeeder/ack/2", "2");
  }
}