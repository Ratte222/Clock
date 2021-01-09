// подключение библиотек
//#include <Wire.h> 
//#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;
//LiquidCrystal_I2C lcd(0x3F, 20, 4);
void setup() {
  // инициализация LCD
  //lcd.begin();
  // Включить подсветку и написать текст.
  //lcd.backlight();
  //lcd.print("Hello GEEKMATIC!");
    USE_SERIAL.begin(115200);
    //Serial1.begin(115200);
   // USE_SERIAL.setDebugOutput(true);
    /*
    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();
    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }
    */
    USE_SERIAL.setTimeout(500);
    WiFiMulti.addAP("MAC", "117192017");
    WiFiMulti.addAP("Ya", "117192017");
}
void loop() {
  //getWeather2021-01-03
    // ожидание WiFi соединения
    uint8_t handleMessage = 0;
    if(USE_SERIAL.available() > 0)
    { 
      String UARTdat = USE_SERIAL.readString();
      //wifiMulti.run();
      if((WiFiMulti.run() == WL_CONNECTED)) {
          HTTPClient http;
          //USE_SERIAL.print("[HTTP] begin...\n");
          //http.begin("VASH_DOMEN.COM", 80, "/vasha_stranica.html"); //HTTP
          http.begin("http://192.168.0.104/"+UARTdat);
          //USE_SERIAL.print("[HTTP] GET...\n");
          // начало соединения и отсылка HTTP заголовка
          int httpCode = http.GET();
          if(httpCode) {
              // HTTP header has been send and Server response header has been handled
              //USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
              // файл найден на сервере
              if(httpCode == 200) {
                 USE_SERIAL.print(http.getString()+"\r\n");
                 
              }
              else { USE_SERIAL.println(http.getString());}
          } else {
              USE_SERIAL.print("Errore 1\r\n");
          }
          http.end();
      }
      else
      {
        USE_SERIAL.print("Errore 2\r\n");
      }
      //USE_SERIAL.print(UARTdat);
      /*
      if(UARTdat.indexOf("getDateTime") > -1)
      {
        if((WiFiMulti.run() == WL_CONNECTED)) {
          HTTPClient http;
          //USE_SERIAL.print("[HTTP] begin...\n");
          //http.begin("VASH_DOMEN.COM", 80, "/vasha_stranica.html"); //HTTP
          http.begin("http://192.168.0.104/GetDateTimePC");
          //USE_SERIAL.print("[HTTP] GET...\n");
          // начало соединения и отсылка HTTP заголовка
          int httpCode = http.GET();
          if(httpCode) {
              // HTTP header has been send and Server response header has been handled
              //USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
              // файл найден на сервере
              if(httpCode == 200) {
                 USE_SERIAL.print(http.getString()+"\r\n");
                 
              }
              else { USE_SERIAL.println(http.getString());}
          } else {
              //USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
          }
          http.end();
        }
      }
      else if(UARTdat.indexOf("getWeather") > -1)
      {
        if((WiFiMulti.run() == WL_CONNECTED)) {
          HTTPClient http;
          //USE_SERIAL.print("[HTTP] begin...\n");
          //http.begin("VASH_DOMEN.COM", 80, "/vasha_stranica.html"); //HTTP
          http.begin("http://192.168.0.104/GetWeather");
          //USE_SERIAL.print("[HTTP] GET...\n");
          // начало соединения и отсылка HTTP заголовка
          int httpCode = http.GET();
          if(httpCode) {
              // HTTP header has been send and Server response header has been handled
              //USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
              // файл найден на сервере
              if(httpCode == 200) {
                 USE_SERIAL.print(http.getString()+"\r\n");
                 
                 
                 //lcd.clear();
                 //lcd.print("Temp:"+payload.substring(payload.indexOf('temp', 135)+3,payload.indexOf('pres',150)-5)+" C");
                 //lcd.setCursor(0, 1);
                 //lcd.print("Vlazhnost:"+payload.substring(payload.indexOf('humid', 150)+6,payload.indexOf('temp_min',155)-9)+" %");
              }else { USE_SERIAL.println(http.getString());}
          } else {
              //USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
          }
          http.end();
        }
      }
      else if(UARTdat.indexOf("getWeatherToday") > -1)
      {
        if((WiFiMulti.run() == WL_CONNECTED)) {
          HTTPClient http;
          USE_SERIAL.print("[HTTP] begin...\n");
          //http.begin("VASH_DOMEN.COM", 80, "/vasha_stranica.html"); //HTTP
          http.begin("http://api.openweathermap.org/data/2.5/weather?lat=51.6743721&lon=33.9105138&appid=01b822b1051ee607269fc87c780aba06");
          USE_SERIAL.print("[HTTP] GET...\n");
          // начало соединения и отсылка HTTP заголовка
          int httpCode = http.GET();
          if(httpCode) {
              // HTTP header has been send and Server response header has been handled
              USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
              // файл найден на сервере
              if(httpCode == 200) {
                 String response = http.getString();
                 //USE_SERIAL.println(payload.substring(145,230));
                 int indexStart = response.indexOf("description")+14, indexStop = response.indexOf("\"", indexStart);
                 USE_SERIAL.println("Weather "+response.substring(indexStart, indexStop));
                 
                 //lcd.clear();
                 //lcd.print("Temp:"+payload.substring(payload.indexOf('temp', 135)+3,payload.indexOf('pres',150)-5)+" C");
                 //lcd.setCursor(0, 1);
                 //lcd.print("Vlazhnost:"+payload.substring(payload.indexOf('humid', 150)+6,payload.indexOf('temp_min',155)-9)+" %");
              }
              else { USE_SERIAL.println(http.getString());}
          } else {
              USE_SERIAL.println("[HTTP] GET... failed, no connection or no HTTP server\n");
              
          }
          http.end();
        }
        
      }
      */
    }
    //delay(10000);
}
