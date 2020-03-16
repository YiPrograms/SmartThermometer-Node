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
#include "Lato_Regular_160.h"


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
#define PROX_IRQ D3
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


volatile bool promixity = false;
volatile bool measuring = false;
volatile bool premeasuring = false;

ICACHE_RAM_ATTR void PromixityInterrupt() {
    if (!measuring && !premeasuring)
        promixity = true;
}

void TemperatureSetup() {
    mlx.begin();

    pinMode(PROX_IRQ, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PROX_IRQ), PromixityInterrupt, FALLING);
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

bool initial = true;


int lastTrigger = 0;


String cardUID = "";
int lastCard = 0;

String lastTitle = "";

int measureTime = 0;
double measureSum = 0;
int measureCount = 0;
int measureRetries = 0;

double measureMax = -1;
double measureMin = 1e6;

double bodyTemp = 0;
int lastBodyTemp = 0;

String roomTemp = "";
int lastRoomTemp = 0;

void DrawTitle(String title, int color) {
    if (lastTitle == title) return;
    lastTitle = title;

    tft.fillRoundRect(0, 0, 320, 65, 0, ILI9341_BLACK);
    tft.setFont(&DejaVu_Sans_35);
    ui.setTextColor(color, ILI9341_BLACK);
    ui.drawString(160, 45, title);
}

void DrawTemperature(String text, int color) {
    tft.fillRoundRect(0, 70, 320, 125, 0, ILI9341_BLACK);
    tft.setFont(&Lato_Regular_160);
    ui.setTextColor(color, ILI9341_BLACK);
    ui.drawString(154, 188, text);
}

void DrawRoomTemperature(bool init) {
    ui.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
    
    if (init) {
        tft.setFont(&DejaVu_Sans_Bold_23);
        ui.drawString(95, 235, "Room:         C");
        tft.setFont(&DejaVu_Sans_Bold_12);
        ui.drawString(167, 225, "o");
    }
    
    tft.setFont(&DejaVu_Sans_Bold_23);
    tft.fillRoundRect(95, 215, 62, 22, 0, ILI9341_BLACK);
    ui.drawString(125, 235, roomTemp);
}


void MeasureBodyTemp() {
    //Serial.print("Object Temp: ");
    //Serial.println(mlx.readObjectTempC());

    if (promixity) {                                                    // Start premeasuring
        promixity = false;
        if (int(millis()) - lastBodyTemp > BODY_TEMP_INTERVAL_MS) {
            premeasuring = true;
            lastTrigger = millis();
        }
    }

    if (premeasuring) {
        if (digitalRead(PROX_IRQ) == HIGH) {                                 // The person has left
            premeasuring = false;
        } else if (int(millis()) - lastTrigger > BODY_TEMP_WAIT_MS) {        // Start measuring
            premeasuring = false;
            measuring = true;
            measureTime = millis();
            
            if (lastBodyTemp) {
                lastBodyTemp = 0;
                DrawTemperature("", ILI9341_BLACK);
            }
            
            tone(SPK_PIN, BEEP_FREQUENCY, BODY_TEMP_MEASURE_START_BEEP_MS);
            DrawTitle(MEASURING_PROMPT, 0x04FF);
                 
            measureSum = 0;
            measureCount = 0;
            measureMax = -1;
            measureMin = 1e6;
            measureRetries = 0;
        }
    }

    if (measuring) {                                                             // Measure body temperature
        int measurePass = int(millis()) - measureTime;
        
        if (measurePass > BODY_TEMP_MEASURE_BEGIN_MS) {
            double measureResult = mlx.readObjectTempC();
            measureMax = max(measureMax, measureResult);
            measureMin = min(measureMin, measureResult);
    
            if (measureMax - measureMin > BODY_TEMP_REMEASURE_THEREHOLD_C) {      // Remeasure
                measureSum = 0;
                measureCount = 0;
                measureMax = -1;
                measureMin = 1e6;
                
                if (measureRetries > BODY_TEMP_MAX_REMEASURE_TIMES) {            // Measure Failed
                    measuring = false;
                    lastBodyTemp = millis();
                    DrawTitle(MEASURE_FAILED_PROMPT, ILI9341_RED);
                    tone(SPK_PIN, BEEP_FREQUENCY, BODY_TEMP_MEASURE_STOP_BEEP_MS*2);
                } else {
                    measureTime = millis() + BODY_TEMP_MEASURE_BEGIN_MS;
                    measureRetries++;
                }
            } else {
                measureSum += measureResult;
                measureCount++;

                if (measurePass > BODY_TEMP_MEASURE_END_MS) {
                    if (measureCount < BODY_TEMP_MIN_SAMPLES)                                      // Too few samples
                        measureTime += BODY_TEMP_MEASURE_BEGIN_MS;
                    else {                                                     
                        bodyTemp = measureSum / measureCount;
                        
                        if (digitalRead(PROX_IRQ)==HIGH) {                                         // The person left
                            measuring = false;
                            lastBodyTemp = millis();
                            DrawTitle(MEASURE_LEAVE_PROMPT, ILI9341_RED);
                            tone(SPK_PIN, BEEP_FREQUENCY, BODY_TEMP_MEASURE_STOP_BEEP_MS*2);
                        } else {                                                                    // Measure Sucess
                            int color = 0;
    
                            if (bodyTemp < 35.8) color = 0x04FF;
                            else if (bodyTemp < 37) color = 0x07FF;
                            else if (bodyTemp < 37.25) color = ILI9341_YELLOW;
                            else if (bodyTemp < 37.5) color = ILI9341_ORANGE;
                            else color = ILI9341_RED;
                            
                            DrawTemperature(String(bodyTemp, 1), color);
                            lastBodyTemp = millis();
                            tone(SPK_PIN, BEEP_FREQUENCY, BODY_TEMP_MEASURE_STOP_BEEP_MS);
        
                            if (cardUID != "") {                                         // Upload result
                                DrawTitle("Sending as " + cardUID + "...", 0x04FF);
                                delay(1000);
                                DrawTitle("Sent as " + cardUID + "!", ILI9341_GREEN);
                                cardUID = "";
                            } else {
                                DrawTitle(TAP_CARD_PROMPT, ILI9341_ORANGE);
                            }
        
                            measuring = false;
                        }
                    }
                }
            }
        }
    }

    if (!measuring && lastBodyTemp && int(millis()) - lastBodyTemp > 1000 * BODY_TEMP_STAY_SEC) {
        lastBodyTemp = 0;
        DrawTemperature("", ILI9341_BLACK);
        if (cardUID == "")
            DrawTitle(TAP_CARD_PROMPT, ILI9341_ORANGE);
    }
}

void UpdateRoomTemp() {
    if (initial) {
        roomTemp = String(mlx.readAmbientTempC(), 1);
        DrawRoomTemperature(true);
        lastRoomTemp = millis();
    }
    if (int(millis()) - lastRoomTemp > 1000 * ROOM_TEMP_UPDATE_SEC) {
        String newTemp = String(mlx.readAmbientTempC(), 1);
        
        if (roomTemp != newTemp) {
            roomTemp = newTemp;
            DrawRoomTemperature(false);
            lastRoomTemp = millis();
        }
    }
}

void ScanCard() {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) { // Scan card
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
            if (!measuring || measuring && cardUID == "") {
                cardUID = newCardUID;
                lastCard = millis();
                DrawTitle(cardUID, ILI9341_GREEN);
                
                tone(SPK_PIN, BEEP_FREQUENCY, CARD_BEEP_MS);
            }
        }
    }

    if (initial || !measuring && cardUID != "" && int(millis()) - lastCard > 1000 * CARD_TIMEOUT_SEC) {   // Card Timeout
        cardUID = "";
        DrawTitle(TAP_CARD_PROMPT, ILI9341_ORANGE);
    }
}

void RegScreen() {
    
}

void loop() {
    if (regMode) {


        initial = false;
    } else {
        MeasureBodyTemp();
        ScanCard();
        UpdateRoomTemp();
        initial = false;
    }
}
