//#include <LiquidCrystal.h>
////#include "DHT.h"
//
////#define DHTPIN 10
////#define DHTTYPE DHT22
////DHT dht(DHTPIN, DHTTYPE);
//
////dht DHT;
////#define DHT11_PIN 7
//
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//
//
//
////void setup() {
////
//// Serial.begin(9600);
////  Serial.println("setup begin");
////  
////  lcd.begin(16, 2);
////  lcd.print("hello, world!");
////
//// Serial.println(F("DHTxx test!"));
////
////  dht.begin();
////
////  Serial.println("setup end");
////}
//
////void loop() {
////
////   int chk = DHT.read11(DHT11_PIN);
////  Serial.print("Temperature = ");
////  Serial.println(DHT.temperature);
////  Serial.print("Humidity = ");
////  Serial.println(DHT.humidity);
////   Serial.print("----------");
//// 
////  delay(1000);
////
////
////
////  lcd.print("what the");
////  lcd.display();  
////}
//
////
////void processThermostat() {
////
////  delay(2000);
////
////  float h = dht.readHumidity();
////  float t = dht.readTemperature();
////  float f = dht.readTemperature(true);
////
////  if (isnan(h) || isnan(t) || isnan(f)) {
////    Serial.println(F("Failed to read from DHT sensor!"));
////    return;
////  }
////
////  float hif = dht.computeHeatIndex(f, h);
////  float hic = dht.computeHeatIndex(t, h, false);
////
////  Serial.print(F("Humidity: "));
////  Serial.print(h);
////  Serial.print(F("%  Temperature: "));
////  Serial.print(t);
////  Serial.print(F("°C "));
////  Serial.print(f);
////  Serial.print(F("°F  Heat index: "));
////  Serial.print(hic);
////  Serial.print(F("°C "));
////  Serial.print(hif);
////  Serial.println(F("°F"));
////}
////}
////
