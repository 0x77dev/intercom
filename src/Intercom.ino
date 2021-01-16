/*
 * Project Intercom
 * Description: Intercom assistant, built for integrate your old-school intercom with Particle Cloud and Home Assistant
 * Author: Mikhail Marynenko <0x77dev@protonmail.com>
 */

#include "MQTT.h"

// Hang-up/down relay pin
int relay1 = D0;

// Open Button relay pin
int relay2 = D1;

// Intercom line analog pin
int line = A0;

// check for line voltage change
bool useLine = false;

byte mqtt_server[] = {192, 168, 31, 164};
MQTT client(mqtt_server, 1883, mqttCallback);

int open(String command)
{
  client.publish("homeassistant/intercom/state", "UNLOCK");
  digitalWrite(relay1, HIGH); // Hangup
  Particle.publish("intercom/status", "hangup");
  delay(1500);                // waits for 1500mS
  digitalWrite(relay2, HIGH); // press button
  Particle.publish("intercom/status", "press_button");
  delay(1350);               // waits for 1350mS
  digitalWrite(relay2, LOW); // release button
  Particle.publish("intercom/status", "release_button");
  delay(1200);               // waits for 1200mS
  digitalWrite(relay1, LOW); // Hangdown
  Particle.publish("intercom/status", "hangdown");
  client.publish("homeassistant/intercom/state", "LOCK");
  return 1;
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;

  if (strcmp(p, "UNLOCK"))
    open("");
}

int toggleLine(String command)
{
  useLine = !useLine;
  return 1;
}

void setup()
{
  // Setup Pins
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  // Setup Particle Cloud
  Particle.function("open", open);
  Particle.function("toggleLine", toggleLine);
  Particle.variable("useLine", useLine);

  // Setup MQTT
  Particle.publish("intercom/mqtt/status", "connecting");

  client.connect("intercom", "mqtt", "mqttpassword");

  if (client.isConnected())
  {
    Serial.print("\nMQTT Connected");
    Particle.publish("intercom/mqtt/status", "connected");
    client.publish("homeassistant/intercom/state", "online");
    client.subscribe("homeassistant/intercom/set");
  }
}

void loop()
{
  if (useLine)
  {
    Particle.publish("intercom/line", String(digitalRead(line)));
    while (digitalRead(line) == HIGH)
    {
      delay(1000);
      if (digitalRead(line) == HIGH)
      {
        Particle.publish("intercom/status", "line_active");
        open("");
      }
      else
      {
        Particle.publish("intercom/status", "line_not_active");
      }
    }
    delay(1500);
    Particle.publish("status", "intercom/standby");
  }
}
