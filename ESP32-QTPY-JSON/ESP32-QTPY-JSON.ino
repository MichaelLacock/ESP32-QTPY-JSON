// ESP32-QTPY JSON Example
// Originating from example code from Adafruit.
// Modified demo by Michael Lacock, 2021.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <jsonlib.h>    // https://github.com/wyolum/jsonlib
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1327.h>
#include "secrets.h"    // Local file in same directory as this file.

#define Display_available   1   // (1 for "128x128 SSD1327" i2c OLED, 0 for no display.)

#define DEFAULT_I2C_PORT &Wire

#if defined(ARDUINO_ADAFRUIT_KB2040_RP2040) \
    || defined(ARDUINO_ADAFRUIT_QTPY_ESP32S2)
  #define SECONDARY_I2C_PORT &Wire1
#endif

#if Display_available == 1
  #define SCREEN_WIDTH 128 
  #define SCREEN_HEIGHT 128 
  #define OLED_RESET -1
#else
  #define SCREEN_WIDTH 0 
  #define SCREEN_HEIGHT 0 
  #define OLED_RESET -1
#endif

Adafruit_SSD1327 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET, 1000000);

// From "secrets.h" file.
const char* ssid  = SECRET_SSID;
const char* password = SECRET_PASS;

//#define SERVER "worldtimeapi.org"  // Set "SSL_Site" to 0.
//#define PATH   "/api/timezone/America/Chicago"

#define SERVER "zenquotes.io"
#define PATH   "/api/random"

const char* host = SERVER;
#define SSL_Site    1   // (1 for HTTPS, and 0 for HTTP)

#if SSL_Site == 1
    WiFiClientSecure client;
    int httpPort = 443;
    String url = "https://";
#else
    WiFiClient client;
    int httpPort = 80;
    String url = "http://";
#endif

String HTML_data = ("");
String contents = ("");
String full_contents = ("");

void setup() {
    Serial.begin(115200);
    Serial.println("---ON---");
    Serial.println("");

    #if defined(ARDUINO_ADAFRUIT_QTPY_ESP32S2)
        Wire1.setPins(SDA1, SCL1);
    #endif

    if (Display_available == 1) {
      if ( ! display.begin(0x3D) ) {
          Serial.println("Unable to initialize OLED");
          while (1) yield();
      }
      display.display();
      display.setTextWrap(true);
      delay(2000);
      display.clearDisplay();
    }
  
    // Wifi Setup
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
  
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    if (Display_available == 1) {
        display.clearDisplay();
        display.setTextSize(1);          
        display.setTextColor(SSD1327_WHITE);      
        display.setCursor(20,40);      
        display.println(F("WiFi connected."));
        display.display();
    }
}

void loop() {
    Serial.println("\nStarting connection to server...");
  
    HTML_data = ("");
    contents = ("");
    full_contents = ("");

    if (SSL_Site == 1) {
        // You might need to comment out "client.setInsecure();" if 
        // "SSL_Site == 0". The compiler can be strange sometimes.
        client.setInsecure();  // Makes it easier, no fingerprint required. However, NOT ENCRYPTED.
        url = ("https://");
    } else {
        url = ("http://");
    }
  
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }
  
    url += SERVER;
    url += PATH;
  
    Serial.print("Requesting URL: ");
    Serial.println(url);
  
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }
  
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        HTML_data += line;
    }
  
    String json = HTML_data;
    
    String quote = jsonExtract(json, "q");  // Assign to different values to extract from json data.
    String author = jsonExtract(json, "a");
    
    Serial.print("[-] "); Serial.println(quote);
    Serial.print("   - "); Serial.println(author);
    
    if (Display_available == 1) {
       display.clearDisplay();
       display.setTextSize(1);          
       display.setTextColor(SSD1327_WHITE);      
       display.setCursor(10,20);      
       display.println(quote);
       display.setCursor(5,95);
       display.println(author);
       display.display();
    }

    // For World Time API.
//    String datetime = jsonExtract(json, "datetime");
//    Serial.print("[-] "); Serial.println(datetime);
//    if (Display_available == 1) {
//       display.clearDisplay();
//       display.setTextSize(1);          
//       display.setTextColor(SSD1327_WHITE);      
//       display.setCursor(10,20);      
//       display.println(datetime);
//       display.display();
//    }
    
    client.stop();
  
    delay(10000);
}
