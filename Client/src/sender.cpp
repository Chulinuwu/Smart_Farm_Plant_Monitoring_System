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
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <HTTPClient.h>
#include <ThingESP.h>

const int lightPin = 34; // Define pin that use adc1 
const int dhtPin = 18;   // Define pin for DHT sensor

DHT_Unified dht(dhtPin, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize the I2C LCD with the I2C address

ThingESP32 thing(THINGESP_USERNAME, TWILILO_PROJECTNAME, TWILILO_DEVICENAME);
// Timer variables
unsigned long previousMillis = 0;
const long INTERVAL = 6000; 

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *serverAddress = SERVER_ADDRESS;
const int serverPort = SERVER_PORT;
const char *googleScriptUrl = APPSCRIPTS_URL;

WiFiClient TCPclient;
HTTPClient http;

bool isConnected = false;

float soil = 0;
float water = 0;
float lighteiei = 0;

void sendDataToGoogleSheet(float humidity, float temperature, float waterLevel, float soilHumidity)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    http.begin(googleScriptUrl);

    StaticJsonDocument<200> jsonDoc;

    jsonDoc["humidity"] = humidity;
    jsonDoc["temperature"] = temperature;
    jsonDoc["waterLevel"] = waterLevel;
    jsonDoc["soilHumidity"] = soilHumidity;

    String jsonString;

    serializeJson(jsonDoc, jsonString);

    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode == 302)
    {
      String redirectUrl = http.getLocation();
      http.end();

      http.begin(redirectUrl);
      http.addHeader("Content-Type", "application/json");
      httpResponseCode = http.POST(jsonString);
    }

    // if (httpResponseCode > 0) {
    //   String response = http.getString();
    //   Serial.println("HTTP Response code: " + String(httpResponseCode));
    //   Serial.println("Response: " + response);
    // } else {
    //   Serial.println("Error on sending POST: " + String(httpResponseCode));
    // }
    // Serial.print("sending data HTTP response: " + String(httpResponseCode));

    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }
}

String HandleResponse(String query)
{
  // Fetch temperature event
  sensors_event_t event;
  dht.temperature().getEvent(&event);

  float temperature = event.temperature; // Celsius
  // Fetch humidity event
  dht.humidity().getEvent(&event);

  float humidity = event.relative_humidity; // Percentage

  // Fetch water level, light, and soil humidity
  float waterLevelwhatapp = water; 
  float lightwhatapp = lighteiei;  
  float soilHumiditywhatapp = soil; 

  // Check if the values are valid
  if (isnan(temperature) || isnan(humidity) || isnan(waterLevelwhatapp) || isnan(lightwhatapp) || isnan(soilHumiditywhatapp))
  {
    Serial.println("Failed to read from sensors!");
    return "Failed to read from sensors.";
  }

  if (waterLevelwhatapp < 50)
  {
    Serial.println("Your water level is below 50 percent now");
    return "Your water level is below 50 percent now";
  }

  //Response String
  String temp = "Temperature: " + String(temperature) + "°C\n";
  String humid = "Humidity: " + String(humidity) + " %\n";
  String water = "Water Level: " + String(waterLevelwhatapp) + " %\n";
  String lightStr = "Light: " + String(lightwhatapp) + " lux\n";
  String soilHumid = "Soil Humidity: " + String(soilHumiditywhatapp) + " %\n";

  if (query == "temperature")
  {
    return temp;
  }
  else if (query == "humidity")
  {
    return humid;
  }
  else if (query == "water level")
  {
    return water;
  }
  else if (query == "light")
  {
    return lightStr;
  }
  else if (query == "soil humidity")
  {
    return soilHumid;
  }
  else
  {
    return "Your query was invalid.";
  }
}

void sendTemperatureAndHumidity()
{
  sensors_event_t tempEvent, humidEvent;
  dht.temperature().getEvent(&tempEvent);
  dht.humidity().getEvent(&humidEvent);

  float temperature = tempEvent.temperature;
  float humidity = humidEvent.relative_humidity;
  float waterLevelwhatapp = 100 - water; 
  float lightwhatapp = lighteiei;      
  float soilHumiditywhatapp = soil;     

  // Validate readings
  if (isnan(temperature) || isnan(humidity) || isnan(waterLevelwhatapp) || isnan(lightwhatapp) || isnan(soilHumiditywhatapp))
  {
    Serial.println("Failed to read from sensors!");
    return; 
  }

  String data = "Temperature: " + String(temperature) + "°C\n" +
                "Humidity: " + String(humidity) + " %\n" +
                "Water Level: " + String(waterLevelwhatapp) + " %\n" +
                "Light: " + String(lightwhatapp) + " lux\n" +
                "Soil Humidity: " + String(soilHumiditywhatapp) + " %\n";

  String response = HandleResponse(data);
  if (response == "Your device is offline.")
  {
    Serial.println("Error: Device is offline. Check your internet connection.");
  }
  else
  {
    Serial.println("Message prepared: ");
    Serial.println(data);
  }
}

void setup()
{
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(9600);
  pinMode(lightPin, INPUT);

  Wire.begin(21, 22);
  lcd.begin(16, 2); 
  lcd.backlight();  

  thing.SetWiFi(WIFI_SSID, WIFI_PASSWORD);
  thing.initDevice();

  dht.begin();

  Serial.println("DHT sensor initialized.");
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);

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

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.print("Connecting to ");
  Serial.print(SERVER_ADDRESS);
  Serial.print(':');
  Serial.println(SERVER_PORT);

  if (!TCPclient.connect(SERVER_ADDRESS, SERVER_PORT))
  {
    Serial.println("Connection failed.");
    isConnected = false;
  }
  else
  {
    Serial.println("Connected to server.");
    isConnected = true;
  }

  Serial.println("Connected to server.");
}

String data;
String waterLevelStr;
String soilHumidityStr;

float waterLevel;
float soilHumidity;
void loop()
{
  thing.Handle();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= INTERVAL)
  {
    previousMillis = currentMillis;
    sendTemperatureAndHumidity();
  }
  if (!isConnected)
  {
    if (TCPclient.connect(SERVER_ADDRESS, SERVER_PORT))
    {
      Serial.println("Reconnected to server.");
      isConnected = true;
    }
    else
    {
      Serial.println("Reconnection failed.");
      delay(5000);
    }
  }
  Blynk.run();

  int light = 0;
  int lightStatus = digitalRead(lightPin);
  if (lightStatus == HIGH)
  {
    light = 0;
    Serial.println("Light is off");
  }
  else
  {
    light = 100;
    Serial.println("Light is on");
  }
  Serial.print("Light: ");
  Serial.println(light);

  sensors_event_t event;

  dht.humidity().getEvent(&event);
  float humidity = event.relative_humidity;
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  dht.temperature().getEvent(&event);
  float temperature = event.temperature;
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  if (TCPclient.available())
  {
    data = TCPclient.readStringUntil('\n'); 
    Serial.println("Received from server: " + data);

    int commaIndex = data.indexOf(',');
    String waterLevelStr = data.substring(1, commaIndex);
    String soilHumidityStr = data.substring(commaIndex + 1, data.length() - 2);

    waterLevelStr.trim();   
    soilHumidityStr.trim(); 

    waterLevel = waterLevelStr.toFloat();
    soilHumidity = soilHumidityStr.toFloat();

    Serial.print("Parsed Water Level: ");
    Serial.println(waterLevel);
    Serial.print("Parsed Soil Humidity: ");
    Serial.println(soilHumidity);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");
  delay(250);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");
  delay(250);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water Level: ");
  lcd.setCursor(0, 1);
  float maxWaterLevel = 12.0; 
  float waterLevelPercentage = (waterLevel / maxWaterLevel) * 100;
  String waterLevelStr = String(waterLevelPercentage, 1); 
  waterLevelStr.remove(waterLevelStr.length() - 1);  
  lcd.print(waterLevelPercentage);
  lcd.print(" %");
  delay(250);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Soil Humidity: ");
  lcd.setCursor(0, 1);
  lcd.print(soilHumidity);
  lcd.print(" %");
  delay(250);

  soil = soilHumidity;
  water = 100 - waterLevelPercentage;
  lighteiei = light;

  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V4, soilHumidity);
  Blynk.virtualWrite(V5, light);
  Blynk.virtualWrite(V6, 100 - waterLevelPercentage);

  if (waterLevelPercentage < 50)
  {
    Blynk.logEvent("waterbelow50");
    Serial.println("sent notification to blynk");
  }
  sendDataToGoogleSheet(humidity, temperature, waterLevel, soilHumidity);

  delay(250);
}

BLYNK_WRITE(V3)
{
  int pinValue = param.asInt();
  if (TCPclient.connected())
  {
    if (pinValue == 1)
    {
      TCPclient.write('1');
      TCPclient.print("\n");
      Serial.println("- Virtual button pressed, sent command: 1");
    }
    else
    {
      TCPclient.write('0');
      TCPclient.print("\n");
      Serial.println("- Virtual button released, sent command: 0");
    }

    TCPclient.flush();
    
    if (TCPclient.available())
    {
      String response = TCPclient.readStringUntil('\n');
      Serial.print("Response from server: ");
      Serial.println(response);
    }
  }
  else
  {
    Serial.println("TCP client not connected");
  }
}
