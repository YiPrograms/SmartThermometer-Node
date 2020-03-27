#define IsWithin(x, a, b) ((x >= a) && (x <= b))
#include <avr/pgmspace.h>

const char Mobile_KB[3][13] PROGMEM = {
    {0, 13, 10, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
    {1, 12, 9, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'},
    {3, 10, 7, 'z', 'x', 'c', 'v', 'b', 'n', 'm'},
};

const char Mobile_KB_Cap[3][13] PROGMEM = {
    {0, 13, 10, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
    {1, 12, 9, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'},
    {3, 10, 7, 'Z', 'X', 'C', 'V', 'B', 'N', 'M'},
};

const char Mobile_NumKeys[3][13] PROGMEM = {
    {0, 13, 10, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
    {0, 13, 10, '-', '/', ':', ';', '(', ')', '$', '&', '@', '"'},
    {5, 8, 5, '.', '\,', '?', '!', '\''}
};

const char Mobile_SymKeys[3][13] PROGMEM = {
    {0, 13, 10, '[', ']', '{', '}', '#', '%', '^', '*', '+', '='},
    {4, 9, 6, '_', '\\', '|', '\'', '<', '>'},  // 4
    {5, 8, 5, '.', '\,', '?', '!', '\''}
};

const char Mobile_NumPad[3][13] PROGMEM = {
    {7, 6, 3, '7', '8', '9'},
    {7, 6, 3, '4', '5', '6'},
    {7, 6, 3, '1', '2', '3'}
};


#include "Roboto_Mono_Medium_23.h"

byte TouchButton(int x, int y, int w, int h) {
    int X, Y;
    // Retrieve a point
    TS_Point p = ts.getPoint();
    X = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    Y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    return (IsWithin(X, x, x + w) & IsWithin(Y, y, y + h));
}

void drawButton(int x, int y, int w, int h) {
    // white
    tft.fillRoundRect(x, y, w, h, 3, 0xffff);  // outter button color

    // red
    tft.fillRoundRect(x + 1, y + 1, w - 1 * 2, h - 1 * 2, 3,
                      0xf800);  // inner button color
}


bool shift = false, special = false, back = false, lastSp = false,
                lastSh = false, touch = false, numpad = false;

void MakeKB_Button(const char type[][13]) {
    tft.setFont(&Roboto_Mono_Medium_23);
    //tft.setTextSize(3);
    tft.setTextColor(0xffff, 0xf000);
    for (int y = 0; y < 3; y++) {
        int ShiftRight = 15 * pgm_read_byte(&(type[y][0]));
        for (int x = 3; x < 13; x++) {
            if (x >= pgm_read_byte(&(type[y][1]))) break;

            drawButton(10 + (30 * (x - 3)) + ShiftRight, 100 + (35 * y), 27, 30);
            tft.setCursor(16 + (30 * (x - 3)) + ShiftRight, 124 + (35 * y));
            tft.print(char(pgm_read_byte(&(type[y][x]))));
        }
    }
    
    if (!numpad) {
        // ShiftKey
        drawButton(10, 170, 40, 30);
        tft.setCursor(23, 198);
        tft.print('^');

        // Special Characters
        drawButton(10, 205, 54, 30);
        tft.setCursor(15, 229);
        tft.print(F("123"));
    } else {
        // 0
        drawButton(145, 205, 27, 30);
        tft.setCursor(150, 229);
        tft.print(F("0"));
    }

    // BackSpace
    drawButton(265, 170, 40, 30);
    tft.setCursor(272, 193);
    tft.print(F("<-"));

    // Return
    drawButton(260, 205, 45, 30);
    tft.setCursor(268, 229);
    tft.print(F("OK"));
}

void CleanKeyboard(bool full) {
    if (full)
        tft.fillRoundRect(8, 95, 304, 145, 0, ILI9341_BLACK);
    else
        tft.fillRoundRect(8, 95, 304, 110, 0, ILI9341_BLACK);
}

byte GetKeyPress() {

    byte result = 0;

    if (ts.touched()) {
        if (!touch) {
            touch = true;

            if (!numpad) {
                // ShiftKey
                if (TouchButton(10, 170, 40, 30)) {
                    shift = !shift;
                }

                // Special Characters
                if (TouchButton(10, 205, 54, 30)) {
                    special = !special;
                }
            } else {
                // 0
                if (TouchButton(145, 205, 27, 30)) {
                    result = '0';
                }
            }

            for (int y = 0; y < 3; y++) {
                int ShiftRight;
                if (numpad) {
                    ShiftRight = 15 * pgm_read_byte(&(Mobile_NumPad[y][0]));
                } else if (special) {
                    if (shift)
                        ShiftRight = 15 * pgm_read_byte(&(Mobile_SymKeys[y][0]));
                    else
                        ShiftRight = 15 * pgm_read_byte(&(Mobile_NumKeys[y][0]));
                } else
                    ShiftRight = 15 * pgm_read_byte(&(Mobile_KB[y][0]));

                for (int x = 3; x < 13; x++) {
                    if (x >= (numpad? pgm_read_byte(&Mobile_NumPad[y][1])
                            :(special ? (shift ? pgm_read_byte(&(Mobile_SymKeys[y][1]))
                                         : pgm_read_byte(&(Mobile_NumKeys[y][1])))
                              : pgm_read_byte(&(Mobile_KB[y][1])))))
                        break;

                    if (TouchButton(10 + (30 * (x - 3)) + ShiftRight, 100 + (35 * y), 27, 30)) {
                        if (numpad) {
                            result = pgm_read_byte(&(Mobile_NumPad[y][x]));
                        } else if (special) {
                            if (shift)
                                result = pgm_read_byte(&(Mobile_SymKeys[y][x]));
                            else
                                result = pgm_read_byte(&(Mobile_NumKeys[y][x]));
                        } else {
                            if (shift) {
                                result = pgm_read_byte(&(Mobile_KB_Cap[y][x]));
                                shift = false;
                            }
                            else
                                result = pgm_read_byte(&(Mobile_KB[y][x]));
                        }
                    
                        break;
                    }
                }
            }

            if (!numpad && (special != lastSp || shift != lastSh)) {
                if (special || special != lastSp)
                    CleanKeyboard(false);
                if (special) {
                    if (shift) {
                        MakeKB_Button(Mobile_SymKeys);
                    } else {
                        MakeKB_Button(Mobile_NumKeys);
                    }
                } else {
                    if (shift)
                        MakeKB_Button(Mobile_KB_Cap);
                    else
                        MakeKB_Button(Mobile_KB);
                    tft.setTextColor(0xffff, 0xf800);
                }


                if (special)
                    tft.setTextColor(0x0FF0, 0xf800);
                else
                    tft.setTextColor(0xFFFF, 0xf800);

                tft.setCursor(15, 229);
                tft.print(F("123"));

                if (shift)
                    tft.setTextColor(0x0FF0, 0xf800);
                else
                    tft.setTextColor(0xffff, 0xf800);

                tft.setCursor(23, 198);
                tft.print('^');

                lastSh = shift;

                lastSp = special;
                lastSh = shift;
            }


            // BackSpace
            if (TouchButton(265, 170, 40, 30)) {
                result = 1;
            }

            // Return
            if (TouchButton(260, 205, 45, 30)) {
                result = 2;
            }
        }
    } else {
        touch = false;
    }
    return result;
}

