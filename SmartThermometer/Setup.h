const uint8_t SD3 = 10;

#include <Wire.h> // I2C (Display, Temperature, Proximity)
#include <SPI.h> // SPI (Display, Touch, RFID)

// Display
#ifndef LCD
// TFT Display
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <XPT2046_Touchscreen.h>
#include "GfxUi.h" // Additional UI functions

#define TFT_CS SD3
#define TFT_DC D4
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
GfxUi ui = GfxUi(&tft);

// Fonts
#include "DejaVu_Sans_35.h"
#include "DejaVu_Sans_Bold_12.h"
#include "DejaVu_Sans_Bold_23.h"
#include "Lato_Regular_160.h"

// Touchscreen
#define TOUCH_CS_PIN D0
#define TS_MINX 323
#define TS_MINY 195
#define TS_MAXX 3947
#define TS_MAXY 3824
TS_Point touch_point;
XPT2046_Touchscreen ts(TOUCH_CS_PIN);

#include "Keyboard.h"

#else
// LCD Display
#include <Adafruit_ILI9341.h> // For colors
#include <LiquidCrystal_I2C.h>
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

#endif

// Buzzer   
#define SPK_PIN D8

// WifiManager
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// RFID
#include <MFRC522.h>
#define SS_PIN D4
MFRC522 mfrc522(SS_PIN, -1);


// Temperature
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Proximity
#include <APDS9930.h>
APDS9930 apds = APDS9930();
#define APDS9930_INT    D3
#define PROX_INT_HIGH   530 // Proximity level for interrupt
#define PROX_INT_LOW    0  // No far interrupt

// Server
#include "Credentials.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
HTTPClient https;
StaticJsonDocument<200> doc;


void ScreenSetup() {

    pinMode(SPK_PIN, OUTPUT); // Speaker

#ifndef LCD
    if (!ts.begin()) {
        Serial.println("TouchSensor not found?");
    }

    tft.begin();
    tft.setRotation(3);

    ts.begin();
#else
    lcd.begin();
    lcd.backlight();
#endif
}

void RFIDSetup() {
    SPI.begin();
    mfrc522.PCD_Init();
    delay(4);
    Serial.print("RFID: ");
    mfrc522.PCD_DumpVersionToSerial();
}


volatile bool promixity = false;
volatile bool measuring = false;
volatile bool premeasuring = false;

ICACHE_RAM_ATTR void PromixityInterrupt() {
    promixity = true;
}

#ifdef LCD
void PrintLCD(int row, String text) {
    int spaces = int(16 - text.length())/2;
    lcd.setCursor(0, row);
    
    for (int i = 0; i < spaces; i++)
        lcd.print(" ");
    lcd.print(text);
    for (int i = 0; i < spaces; i++)
        lcd.print(" ");
}
#endif

void TemperatureSetup() {
    mlx.begin();
}

void ProximitySetup() {
    pinMode(APDS9930_INT, INPUT);
    attachInterrupt(digitalPinToInterrupt(APDS9930_INT), PromixityInterrupt, FALLING);
    // Initialize APDS-9930 (configure I2C and initial values)
    if ( apds.init() ) {
        Serial.println(F("APDS-9930 initialization complete"));
    } else {
        Serial.println(F("Something went wrong during APDS-9930 init!"));
    }

    // Set proximity interrupt thresholds
    if ( !apds.setProximityIntLowThreshold(PROX_INT_LOW) ) {
        Serial.println(F("Error writing low threshold"));
    }
    if ( !apds.setProximityIntHighThreshold(PROX_INT_HIGH) ) {
        Serial.println(F("Error writing high threshold"));
    }

    // Start running the APDS-9930 proximity sensor (interrupts)
    if ( apds.enableProximitySensor(true) ) {
        Serial.println(F("Proximity sensor is now running"));
    } else {
        Serial.println(F("Something went wrong during sensor init!"));
    }
}

// Called if WiFi has not been configured yet
void ConfigModeCallback(WiFiManager *myWiFiManager) {
#ifndef LCD
    ui.setTextAlignment(CENTER);
    tft.setFont(&DejaVu_Sans_Bold_12);
    tft.setTextColor(ILI9341_CYAN);
    ui.drawString(160, 28, "WiFi Manager");
    ui.drawString(160, 44, "Please connect to AP");
    tft.setTextColor(ILI9341_WHITE);
    ui.drawString(160, 60, myWiFiManager->getConfigPortalSSID());
    tft.setTextColor(ILI9341_CYAN);
    ui.drawString(160, 76, "To setup Wifi Configuration");
#else
    PrintLCD(0, "AP: " + myWiFiManager->getConfigPortalSSID());
#endif
}

void WifiSetup() {
    WiFiManager ESP_wifiManager;
    ESP_wifiManager.setDebugOutput(true);
#ifndef LCD
    tft.fillScreen(ILI9341_BLACK);
    tft.setFont(&DejaVu_Sans_Bold_23);
    ui.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    ui.setTextAlignment(CENTER);
    ui.drawString(160, 120, "Connecting to WiFi...");
#else
    PrintLCD(0, "Connecting...   ");
#endif

    ESP_wifiManager.setAPCallback(ConfigModeCallback);

    ESP_wifiManager.autoConnect();

    Serial.print("WiFi Connected! IP: ");
    Serial.println(WiFi.localIP());
#ifndef LCD
    tft.fillRoundRect(0, 0, 320, 180, 0, ILI9341_BLACK);
#else
    lcd.clear();
#endif

    Serial.println("Request test:");
    https.begin("http://www.google.com/");
    Serial.println(https.GET());

    client->setFingerprint(FINGERPRINT);
}
