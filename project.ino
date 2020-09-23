#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans18pt7b.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#define OLED_RESET LED_BUILTIN  //4
#define RED_PIN D0
#define YELLOW_PIN D4
#define GREEN_PIN D3

Adafruit_SSD1306 display(OLED_RESET);

String url = "http://webrates.truefx.com/rates/connect.html?f=html";
String pay = "";
String payload = "";
String cur = "EUR/USD";
float exchange = -1;
float prevExchange = -1;
int val = 0, s1 = 53, s2 = 66, e1 = 57, e2 = 69;
const char* ssid = "bachawat(alien broadband)";
const char* password = "bachawat1234";

int ledPin = 13;

int timezone = 7 * 3600;
int dst = 0;

void setup() {
    pinMode(D8, INPUT);
    pinMode(D5, INPUT_PULLUP);
    pinMode(D6, INPUT);
    pinMode(D7, INPUT);
 
    attachInterrupt(D7, IntCallback1, RISING);
    attachInterrupt(D6, IntCallback2, RISING);
    attachInterrupt(D5, IntCallback3, FALLING);
    attachInterrupt(D8, IntCallback, RISING);
    
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RED_PIN,OUTPUT);
    pinMode(YELLOW_PIN,OUTPUT);
    pinMode(GREEN_PIN,OUTPUT);
    
    digitalWrite(LED_BUILTIN, HIGH); // switch it OFF. (LOW means OFF for the ESP8266)
    digitalWrite(RED_PIN,LOW);
    digitalWrite(YELLOW_PIN,LOW);
    digitalWrite(GREEN_PIN,LOW);
    
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the buffer.
  display.clearDisplay();
  display.setFont();
  display.setTextSize(1);
  display.display();
  
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,LOW);

  Serial.begin(115200);

  display.setFont();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0,0);
  display.println("Connecting to WiFi:");
  display.println( ssid );

  WiFiManager wifi;
  display.println("\nConnecting");

  display.display();

  wifi.autoConnect("ForexTracker");

  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  
  display.println("Wifi Connected!");
  display.print("IP:");
  display.println(WiFi.localIP() );

  display.display();

  configTime(timezone, dst, "pool.ntp.org","time-d-b.nist.gov");
  display.println("\nWaiting for NTP...");

  while(!time(nullptr)){
     Serial.print("*");
     delay(1000);
  }
  display.println("\nTime response....OK"); 
  display.display();  
  delay(500);
  display.clearDisplay();
  display.display();
  val = 0;
  
}

void loop() {
  
  if (val == 0) {
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if(httpCode == HTTP_CODE_OK) {
            Serial.print("Received Response\n");
            payload = http.getString();
            Serial.println(payload);
            pay = payload.substring(s1, e1) + payload.substring(s2, e2);
            exchange = pay.toFloat();
            sendMessage(exchange-prevExchange);
            
            Serial.print("\n\n");
            Serial.flush();
            prevExchange = exchange;
          }
      else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          val = 6;
      }
      http.end();
  }
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  p_tm->tm_sec -= 5400;
  mktime(p_tm);

  display.clearDisplay();
  display.setFont();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print(cur+" ");
  
  display.setTextSize(1);
  display.setCursor(94,0);
  display.print(p_tm->tm_hour);
  display.print(":");
  if( p_tm->tm_min <10)
    display.print("0"); 
  display.print(p_tm->tm_min);

  display.setTextSize(1);
  display.setCursor(94,10);
  display.print(p_tm->tm_mday);
  display.print("/");
  display.print(p_tm->tm_mon + 1);

  display.setFont(&FreeSans18pt7b);
  display.setTextSize(1);
  display.setCursor(0,55);
  display.print(pay);
  
  display.display();
  val = (val + 1) % 7;
  delay(1000); // update every 1 sec
}

void sendMessage(float in){
  if(in > 0){
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(RED_PIN, LOW);
    }
    else if(in == 0){
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(YELLOW_PIN, HIGH);
      digitalWrite(RED_PIN, LOW);
    }
    else if(in < 0){
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(RED_PIN, HIGH);
    }
}

void IntCallback(){
  set(0);
  disp();
}

void IntCallback1(){
  set(1);
  disp();
}

void IntCallback2(){
  set(2);
  disp();
}

void IntCallback3(){
  set(3);
  disp();
}

void disp() {
  display.clearDisplay();
  display.setFont();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print(cur+" ");
  display.setFont(&FreeSans18pt7b);
  display.setTextSize(1);
  display.setCursor(0,55);
  display.print(pay);
  display.display();
}

void set(int a) {
  a = 7 + 145 * a;
  cur = payload.substring(a+8,a+15);
  s1 = a + 46;
  s2 = a + 59;
  e1 = s1 + 4;
  e2 = s2 + 3;
  pay = payload.substring(s1, e1) + payload.substring(s2, e2);
  exchange = pay.toFloat();
  sendMessage(0);
}
