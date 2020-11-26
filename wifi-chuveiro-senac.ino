#include <SPI.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
 
String apiKey = "MIIP78TGGCQ8ZRG6"; 
const char *ssid = "Cavalcante";     
const char *pass = "15052406";
const char* server = "api.thingspeak.com";

LiquidCrystal_I2C lcd(0x3F, 16, 2);
#define SENSOR  2
 
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;
 
void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}
 
WiFiClient client;
 
void setup()
{
  Serial.begin(115200);

  delay(10);
 
  pinMode(SENSOR, INPUT_PULLUP);
 
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("-Chuveiro Wi-Fi-");
  lcd.setCursor(0, 1);
  lcd.print("Bem Vindo!");
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}
 
void loop()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) 
  {
    
    pulse1Sec = pulseCount;
    pulseCount = 0;
 
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
 
    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);
 
    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;
    
    Serial.print("Fuxo de Água: ");
    Serial.print(float(flowRate)); 
    Serial.print("L/min");
    Serial.print("\t");       

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fluxo: ");
    lcd.print(float(flowRate));
    lcd.print(" L/M");
 
    Serial.print("Consumo: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalLitres);
    Serial.println("L");

    lcd.setCursor(0, 1);
    lcd.print("Volume: ");
    lcd.print(totalLitres);
    lcd.print(" L");
 
  }
  
  if (client.connect(server, 80))
  {
    String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(float(flowRate));
      postStr += "&field2=";
      postStr += String(totalLitres);
      postStr += "\r\n\r\n";
    
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
   
  }
    client.stop();
}
