# Smart Therometer NodeMCU Client

## Variants
- A Variant (with TFT & Touch screen)
	- With register card support
	
- B Variant (LCD screen only)
	- Cheaper, smaller solution

## Libraries

- MFRC522 by GithubCommunity
- WifiManager by tzapu
- Adafruit ILI9341
- XPT2046_Touchscreen
- ArduinoJson
- Adafruit MLX90614 Library

- [APDS-9930](https://github.com/Depau/APDS9930)
- [LiquidCrystal-I2C](https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library)

### Library Modifications
- `APDS9930/src/APDS9930.h`:
	- Change `#define DEFAULT_PGAIN` from `PGAIN_8X` to `PGAIN_1X`
	
	
## Hardware

- NodeMCU (ESP8266)
- RFID Scanner: MFRC522
- Temperature Sensor: GY-906 (MLX90614)
- Proximity Sensor: APDS-9930
- Buzzer

- Screen:
	- A Variant: ILI9341 2.4' TFT with XPT2046 touch screen
	- B Variant: LCD1602A with I2C adapter board

## Pins

| NodeMCU | MFRC522 | MLX90614 | APDS9930 | Buzzer | ILI9341 w/ XPT2046 | LCD1602 I2C |
|	D0		|		|			|			|		|T_CS				|				|
|	D1 (I2C SCL)|	|SCL		|SCL		|		|					|SCL			|
|	D2 (I2C SDA)|	|SDA		|SDA		|		|					|SDA			|
|	D3		|		|			|INT		|		|					|				|
|	D4		|NSS (CS)|			|			|		|DC					|				|
|	D5 (SPI SCK)|SCK|			|			|		|SCK, T_CLK			|				|
|	D6 (SPI MISO)|MISO|			|			|		|SDO(MISO), T_DO	|				|
|	D7 (SPI MOSI)|MOSI|			|			|		|SDI(MOSI), T_DIN	|				|
|	D8		|		|			|			|	+	|					|				|
|	SD3		|		|			|			|		|CS					|				|
|	RST		|RST	|			|			|		|RESET				|				|
|	3V3		|VCC	|VCC		|VCC, VL	|		|VCC, LED			|VCC			|
|	GND		|GND	|GND		|GND		|	-	|GND				|GND, LCD1602: K|
|	Vin (5V)|		|			|			|		|					|LCD1602: VDD, A|

