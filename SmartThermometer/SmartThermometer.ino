#include <Arduino.h>
#include "Settings.h"

const uint8_t SD3 = 10;

#include <Wire.h> // I2C (Display, Temperature)
#include <SPI.h> // SPI (Display, Touch, RFID)

// Display
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <XPT2046_Touchscreen.h>
#include "GfxUi.h" // Additional UI functions

#define TFT_CS SD3
#define TFT_DC D4
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
GfxUi ui = GfxUi(&tft);

#define SPK_PIN D8

// Fonts
#include "ArialRoundedMTBold_14.h"
#include "ArialRoundedMTBold_36.h"
#include "DejaVu_Sans_35.h"
#include "DejaVu_Sans_Bold_12.h"
#include "DejaVu_Sans_Bold_23.h"
#include "TomThumb.h"


// WifiManager
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>


// Touchscreen
#define TOUCH_CS_PIN D0
#define TS_MINX 323
#define TS_MINY 195
#define TS_MAXX 3947
#define TS_MAXY 3824
TS_Point touch_point;
XPT2046_Touchscreen ts(TOUCH_CS_PIN);


// RFID
#include <MFRC522.h>
#define SS_PIN D4
MFRC522 mfrc522(SS_PIN, -1);


// Temperature
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void ScreenSetup() {
    
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

ICACHE_RAM_ATTR void MeasureTemperature() {
    tone(SPK_PIN, CARD_BEEP_FREQUENCY);
}

void TemperatureSetup() {
    mlx.begin();

    pinMode(D3, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(D3), MeasureTemperature, FALLING);
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
    WifiSetup();
    RFIDSetup();
    TemperatureSetup();
}



bool regMode = false;

String cardUID = "";
int lastTitle = 0;
int titleTimeout = 1;

volatile double bodyTemp = 0;
volatile int lastBodyTemp = 0;

String roomTemp = "";
int lastRoomTemp = -1000;

void DrawTitle(String title, int color) {
    tft.fillRoundRect(0, 0, 320, 65, 0, ILI9341_BLACK);
    tft.setFont(&DejaVu_Sans_35);
    ui.setTextColor(color, ILI9341_BLACK);
    ui.drawString(160, 45, title);
}

void DrawRoomTemperature(bool init) {
    ui.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
    if (init) {
        tft.setFont(&DejaVu_Sans_Bold_23);
        ui.drawString(95, 235, "Room:         C");
        tft.setFont(&DejaVu_Sans_Bold_12);
        ui.drawString(167, 225, "o");
    }
    String newTemp = String(mlx.readAmbientTempC(), 1);
    if (roomTemp != newTemp) {
        roomTemp = newTemp;
        tft.setFont(&DejaVu_Sans_Bold_23);
        tft.fillRoundRect(95, 215, 62, 22, 0, ILI9341_BLACK);
        ui.drawString(125, 235, roomTemp);
    }
}

void UpdateMeasureScreen() {

    if (titleTimeout && millis() - lastTitle > 1000 * titleTimeout) {
        titleTimeout = 0;
        cardUID = "";
        DrawTitle(TAP_CARD_PROMPT, ILI9341_ORANGE);
    }

    if (millis() - lastRoomTemp > 1000 * ROOM_TEMP_UPDATE_SEC) {
        if (lastRoomTemp == -1000) {
            DrawRoomTemperature(true);
        } else {
            DrawRoomTemperature(false);
        }
        lastRoomTemp = millis();
    }
    
}

void ScanCard() {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        Serial.print("Card approach: ");
        byte *id = mfrc522.uid.uidByte;
        byte idSize = mfrc522.uid.size;
        String newCardUID = "";
        for (byte i = 0; i < idSize; i++) {
            if (mfrc522.uid.uidByte[i] < 0x10)
                newCardUID += "0";
            newCardUID += String(mfrc522.uid.uidByte[i], HEX);
        }
        Serial.println(newCardUID);

        if (newCardUID != cardUID) {
            cardUID = newCardUID;
            lastTitle = millis();
            titleTimeout = CARD_STAY_SEC;
            DrawTitle(cardUID, ILI9341_GREEN);

            if (CARD_BEEP_FREQUENCY)
                tone(SPK_PIN, CARD_BEEP_FREQUENCY, CARD_BEEP_MS);
        }
    }
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
