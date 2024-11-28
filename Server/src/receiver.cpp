#define TRIGGER_PIN 12
#define ECHO_PIN 14
#define RELAY_PIN 4
const int SOIL_PIN = 34;
float soilMoisture, sensor_analog;

#include "secrets.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_I2CDevice.h>
#include "FS.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <Arduino.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

WiFiServer server(SERVER_PORT);
WiFiClient client;

bool commandpump = false;
long duration;

enum SensorState
{
  IDLE,
  TRIGGER,
  WAIT_FOR_ECHO,
  CALCULATE_DISTANCE
};
SensorState sensorState = IDLE;
unsigned long triggerTime = 0;
unsigned long echoStartTime = 0;
unsigned long echoEndTime = 0;
float distance = 0.0;
unsigned long previousMillis = 0;
const long interval = 2000;

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(9600);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Server started");
}

void loop()
{
  client = server.available();
  if (client)
  {
    Serial.println("Client connected");
    while (client.connected())
    {
      unsigned long currentMillis = millis();

      switch (sensorState)
      {
      case IDLE:
        digitalWrite(TRIGGER_PIN, LOW);
        triggerTime = micros();
        sensorState = TRIGGER;
        break;

      case TRIGGER:
        if (micros() - triggerTime >= 2)
        {
          digitalWrite(TRIGGER_PIN, HIGH);
          triggerTime = micros();
          sensorState = WAIT_FOR_ECHO;
        }
        break;

      case WAIT_FOR_ECHO:
        if (micros() - triggerTime >= 10)
        {
          digitalWrite(TRIGGER_PIN, LOW);
          duration = pulseIn(ECHO_PIN, HIGH, 30000);
          sensorState = CALCULATE_DISTANCE;
        }
        break;

      case CALCULATE_DISTANCE:
        if (duration > 0)
        {
          distance = (duration / 2.0) / 29.1;
          Serial.print("Distance: ");
          Serial.print(distance);
          Serial.println(" cm");
        }
        else
        {
          Serial.println("No echo received");
          distance = 0.0;
        }
        sensorState = IDLE;
        sensor_analog = analogRead(SOIL_PIN);
        soilMoisture = (100 - ((sensor_analog / 4095.00) * 100));
        Serial.print("Moisture = ");
        Serial.print(soilMoisture);
        Serial.println("%");
        break;
      }

      if (client.available())
      {
        Serial.println("Client available");
        char command = client.read();
        Serial.print("Received command: ");
        Serial.println(command);

        if (command == '1')
        {
          commandpump = true;
          digitalWrite(RELAY_PIN, HIGH);
          Serial.println("Relay triggered");
        }
        else if (command == '0')
        {
          commandpump = false;
          digitalWrite(RELAY_PIN, LOW);
          Serial.println("Relay turned off");
        }

        Serial.print("Distance sent to server. = ");
        client.println(distance);
      }
      else
      {
        Serial.print("No data available from client (command pump : ");
        Serial.print(commandpump);
        Serial.println(" )");
        if (commandpump)
        {
          digitalWrite(RELAY_PIN, HIGH);
        }
        else
        {
          digitalWrite(RELAY_PIN, LOW);
        }
        delay(500);
      }
      if (currentMillis - previousMillis >= interval)
      {
        previousMillis = currentMillis;
        client.print("(");
        client.print(distance);
        client.print(",");
        client.print(soilMoisture);
        client.println(")");
        Serial.print("Distance to client: ");
        Serial.print("(");
        Serial.print(distance);
        Serial.print(",");
        Serial.print(soilMoisture);
        Serial.println(")");
      }
      delay(1000);
    }
    client.stop();
    Serial.println("Client disconnected");
  }
  delay(1000);
}