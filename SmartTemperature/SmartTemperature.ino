#include <Arduino.h>
#include "Settings.h"


// Display
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <Wire.h>  // required even though we do not use I2C 
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "GfxUi.h" // Additional UI functions

static uint8_t SD3 = 10;
#define TFT_CS SD3
#define TFT_DC D4
#define BL_LED D8
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
GfxUi ui = GfxUi(&tft);

#define SPK_PIN D1

// Fonts
#include "ArialRoundedMTBold_14.h"
#include "ArialRoundedMTBold_36.h"
#include "DejaVu_Sans_35.h"


// WifiManager
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>


// Touchscreen
#define TOUCH_CS_PIN D3
#define TOUCH_IRQ_PIN D2
#define TS_MINX 323
#define TS_MINY 195
#define TS_MAXX 3947
#define TS_MAXY 3824
TS_Point touch_point;
XPT2046_Touchscreen ts(TOUCH_CS_PIN);


// RFID
//#include <SPI.h>  // Included
#include <MFRC522.h>
#define RST_PIN D0
#define SS_PIN D4
MFRC522 mfrc522(SS_PIN, RST_PIN);

void ScreenSetup() {
    pinMode(BL_LED, OUTPUT); // BackLight
    digitalWrite(BL_LED, HIGH); // Backlight on

    pinMode(SPK_PIN,OUTPUT); // Speaker

    if (!ts.begin()) {
        Serial.println("TouchSensor not found?");
    }

    tft.begin();
    tft.setRotation(3);

    ts.begin();
}

void RFIDSetup() {
    SPI.begin();
    mfrc522.PCD_Init();
    delay(4);
    Serial.print("RFID: ");
    mfrc522.PCD_DumpVersionToSerial();
}

// Called if WiFi has not been configured yet
void ConfigModeCallback(WiFiManager *myWiFiManager) {
    ui.setTextAlignment(CENTER);
    tft.setFont(&ArialRoundedMTBold_14);
    tft.setTextColor(ILI9341_CYAN);
    ui.drawString(160, 28, "WiFi Manager");
    ui.drawString(160, 44, "Please connect to AP");
    tft.setTextColor(ILI9341_WHITE);
    ui.drawString(160, 60, myWiFiManager->getConfigPortalSSID());
    tft.setTextColor(ILI9341_CYAN);
    ui.drawString(160, 76, "To setup Wifi Configuration");
}

void WifiSetup() {
    WiFiManager ESP_wifiManager;
    ESP_wifiManager.setDebugOutput(true);

    tft.fillScreen(ILI9341_BLACK);
    tft.setFont(&ArialRoundedMTBold_14);
    ui.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    ui.setTextAlignment(CENTER);
    ui.drawString(160, 120, "Connecting to WiFi...");

    ESP_wifiManager.setAPCallback(ConfigModeCallback);

    ESP_wifiManager.autoConnect();

    Serial.print("WiFi Connected! IP: ");
    Serial.println(WiFi.localIP());
    tft.fillRoundRect(40, 0, 300, 180, 0, ILI9341_BLACK);
}

void setup() {
    Serial.begin(115200);

    ScreenSetup();
    RFIDSetup();
    WifiSetup();
}

bool regMode = false;

String cardUID = "XXXX";
int lastTitle = 0;
int titleTimeout = 1;

double bodyTemp = 32.5;
int lastTemp = millis();


void UpdateMeasureScreen() {

    if (titleTimeout && millis() - lastTitle > 1000 * titleTimeout) {
        titleTimeout = 0;
        cardUID = "";
        tft.fillRoundRect(0, 0, 320, 65, 0, ILI9341_BLACK);
        tft.setFont(&DejaVu_Sans_35);
        tft.setCursor(18,100);
        ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
        ui.drawString(160, 45, "Tap Your Card...");
    }
    
}

void ScanCard() {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        Serial.print("Card approach: ");
        byte *id = mfrc522.uid.uidByte;
        byte idSize = mfrc522.uid.size;
        String newCardUID = "";
        for (byte i = 0; i < idSize; i++) {
            if (mfrc522.uid.uidByte[i] < 0x10) newCardUID += "0";
            newCardUID += String(mfrc522.uid.uidByte[i], HEX);
        }
        Serial.println(newCardUID);

        if (newCardUID != cardUID) {
            cardUID = newCardUID;
            lastTitle = millis();
            titleTimeout = CARD_STAY_SEC;
            tft.fillRoundRect(0, 0, 320, 65, 0, ILI9341_BLACK);
            tft.setFont(&DejaVu_Sans_35);
            tft.setCursor(18,100);
            ui.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
            ui.drawString(160, 45, cardUID);

            tone(SPK_PIN, 1488, 100);
        }
    }
}

void ScanTemperature() {
    
}

void RegScreen() {
    
}

void loop() {
    if (regMode) {
        
    } else {
        UpdateMeasureScreen();
        ScanCard();
    }
}
