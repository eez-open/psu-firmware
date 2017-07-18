/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "psu.h"

#if OPTION_DISPLAY

#include "util.h"
#include "lcd.h"
#include "arduino_util.h"
#include "gui_internal.h"

#define CONF_LCD_ON_OFF_TRANSITION_TIME 1000000L

#ifdef EEZ_PSU_SIMULATOR
#define cbi(reg, bitmask)
#define sbi(reg, bitmask)
#define pulse_high(reg, bitmask)
#define pulse_low(reg, bitmask)
#else
#define cbi(reg, bitmask) *reg &= ~bitmask
#define sbi(reg, bitmask) *reg |= bitmask
#define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
#define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);
#endif

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define DISPLAY_MODEL_SSD1289    0
#define DISPLAY_MODEL_ILI9341_16 1

#define RGB_TO_HIGH_BYTE(R, G, B) (((R) & 248) | (G) >> 5)
#define RGB_TO_LOW_BYTE(R, G, B) (((G) & 28) << 3 | (B) >> 3)

namespace eez {
namespace psu {
namespace gui {
namespace lcd {

////////////////////////////////////////////////////////////////////////////////

void rgbToHsl(float r, float g, float b, float &h, float &s, float &l) {
    r /= 255;
    g /= 255;
    b /= 255;

    float min = r;
    float mid = g;
    float max = b;

    if (min > mid) {
        swap(float, min, mid);
    }
    if (mid > max) {
        swap(float, mid, max);
    }
    if (min > mid) {
        swap(float, min, mid);
    }

    l = (max + min) / 2;

    if (max == min) {
        h = s = 0; // achromatic
    } else {
        float d = max - min;
        s = l > 0.5 ? d / (2 - max - min) : d / (max + min);

        if (max == r) {
            h = (g - b) / d + (g < b ? 6 : 0);
        } else if (max == g) {
            h = (b - r) / d + 2;
        } else if (max == b) {
            h = (r - g) / d + 4;
        }

        h /= 6;
    }
}

float hue2rgb(float p, float q, float t) {
    if (t < 0) t += 1;
    if (t > 1) t -= 1;
    if (t < 1.0f/6) return p + (q - p) * 6 * t;
    if (t < 1.0f/2) return q;
    if (t < 2.0f/3) return p + (q - p) * (2.0f/3 - t) * 6;
    return p;
}

void hslToRgb(float h, float s, float l, float &r, float &g, float &b) {
    if (s == 0) {
        r = g = b = l; // achromatic
    } else {
        float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;

        r = hue2rgb(p, q, h + 1.0f/3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0f/3);
    }

    r *= 255;
    g *= 255;
    b *= 255;
}

////////////////////////////////////////////////////////////////////////////////

#define TFT_320QVT_1289 DISPLAY_MODEL_SSD1289
#define TFT_320QVT_9341 DISPLAY_MODEL_ILI9341_16

LCD lcd(DISPLAY_TYPE, LCD_RS, LCD_WR, LCD_CS, LCD_RESET);

////////////////////////////////////////////////////////////////////////////////

LCD::LCD(uint8_t model, uint8_t RS, uint8_t WR, uint8_t CS, uint8_t RST) {
    displayWidth = 240;
    displayHeight = 320;

#ifdef EEZ_PSU_SIMULATOR
    buffer = new uint16_t[displayWidth * displayHeight];
#else
    display_model = model;

    this->RS = RS;
    this->WR = WR;
    this->CS = CS;
    this->RST = RST;

    setDirectionRegisters();

    P_RS = portOutputRegister(digitalPinToPort(RS));
    P_WR = portOutputRegister(digitalPinToPort(WR));
    P_CS = portOutputRegister(digitalPinToPort(CS));
    P_RST = portOutputRegister(digitalPinToPort(RST));

    B_RS = digitalPinToBitMask(RS);
    B_WR = digitalPinToBitMask(WR);
    B_CS = digitalPinToBitMask(CS);
    B_RST = digitalPinToBitMask(RST);
#endif
}

LCD::~LCD() {
#ifdef EEZ_PSU_SIMULATOR
    delete [] buffer;
#endif
}

#if !defined(EEZ_PSU_SIMULATOR)
void LCD::write(char VH, char VL) {
    REG_PIOA_CODR = 0x0000C080;
    REG_PIOC_CODR = 0x0000003E;
    REG_PIOD_CODR = 0x0000064F;
    REG_PIOA_SODR = ((VH & 0x06) << 13) | ((VL & 0x40) << 1);
    (VH & 0x01) ? REG_PIOB_SODR = 0x4000000 : REG_PIOB_CODR = 0x4000000;
    REG_PIOC_SODR = ((VL & 0x01) << 5) | ((VL & 0x02) << 3) | ((VL & 0x04) << 1) | ((VL & 0x08) >> 1) | ((VL & 0x10) >> 3);
    REG_PIOD_SODR = ((VH & 0x78) >> 3) | ((VH & 0x80) >> 1) | ((VL & 0x20) << 5) | ((VL & 0x80) << 2);
    pulse_low(P_WR, B_WR);
}

void LCD::writeCom(char VL) {
    cbi(P_RS, B_RS);
    write(0x00, VL);
}

void LCD::writeData(char VH, char VL) {
    sbi(P_RS, B_RS);
    write(VH, VL);
}

void LCD::writeData(char VL) {
    sbi(P_RS, B_RS);
    write(0x00, VL);
}

void LCD::writeComData(char com1, int dat1) {
    writeCom(com1);
    writeData(dat1 >> 8, dat1);
}

void LCD::setDirectionRegisters() {
    REG_PIOA_OER = 0x0000c000; //PA14,PA15 enable
    REG_PIOB_OER = 0x04000000; //PB26 enable
    REG_PIOD_OER = 0x0000064f; //PD0-3,PD6,PD9-10 enable
    REG_PIOA_OER = 0x00000080; //PA7 enable
    REG_PIOC_OER = 0x0000003e; //PC1 - PC5 enable
}

void LCD::fastFill16(int ch, int cl, long numPixels) {
    long blocks;

    REG_PIOA_CODR = 0x0000C080;
    REG_PIOC_CODR = 0x0000003E;
    REG_PIOD_CODR = 0x0000064F;
    REG_PIOA_SODR = ((ch & 0x06) << 13) | ((cl & 0x40) << 1);
    (ch & 0x01) ? REG_PIOB_SODR = 0x4000000 : REG_PIOB_CODR = 0x4000000;
    REG_PIOC_SODR = ((cl & 0x01) << 5) | ((cl & 0x02) << 3) | ((cl & 0x04) << 1) | ((cl & 0x08) >> 1) | ((cl & 0x10) >> 3);
    REG_PIOD_SODR = ((ch & 0x78) >> 3) | ((ch & 0x80) >> 1) | ((cl & 0x20) << 5) | ((cl & 0x80) << 2);

    blocks = numPixels / 16;
    for (int i = 0; i < blocks; i++) {
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
        pulse_low(P_WR, B_WR);
    }
    if ((numPixels % 16) != 0) {
        for (int i = 0; i < (numPixels % 16) + 1; i++) {
            pulse_low(P_WR, B_WR);
        }
    }
}
#endif

void LCD::init(uint8_t orientation) {
    this->orientation = orientation;

#if !defined(EEZ_PSU_SIMULATOR)
    pinMode(RS, OUTPUT);
    pinMode(WR, OUTPUT);
    pinMode(CS, OUTPUT);
    pinMode(RST, OUTPUT);

    setDirectionRegisters();

    sbi(P_RST, B_RST);
    delay(5);
    cbi(P_RST, B_RST);
    delay(15);
    sbi(P_RST, B_RST);
    delay(15);

    cbi(P_CS, B_CS);

    switch (display_model) {
    case DISPLAY_MODEL_SSD1289:
        writeComData(0x00, 0x0001);
        writeComData(0x03, 0xA8A4);
        writeComData(0x0C, 0x0000);
        writeComData(0x0D, 0x080C);
        writeComData(0x0E, 0x2B00);
        writeComData(0x1E, 0x00B7);
        writeComData(0x01, 0x2B3F);
        writeComData(0x02, 0x0600);
        writeComData(0x10, 0x0000);
        writeComData(0x11, 0x6070);
        writeComData(0x05, 0x0000);
        writeComData(0x06, 0x0000);
        writeComData(0x16, 0xEF1C);
        writeComData(0x17, 0x0003);
        writeComData(0x07, 0x0233);
        writeComData(0x0B, 0x0000);
        writeComData(0x0F, 0x0000);
        writeComData(0x41, 0x0000);
        writeComData(0x42, 0x0000);
        writeComData(0x48, 0x0000);
        writeComData(0x49, 0x013F);
        writeComData(0x4A, 0x0000);
        writeComData(0x4B, 0x0000);
        writeComData(0x44, 0xEF00);
        writeComData(0x45, 0x0000);
        writeComData(0x46, 0x013F);
        writeComData(0x30, 0x0707);
        writeComData(0x31, 0x0204);
        writeComData(0x32, 0x0204);
        writeComData(0x33, 0x0502);
        writeComData(0x34, 0x0507);
        writeComData(0x35, 0x0204);
        writeComData(0x36, 0x0204);
        writeComData(0x37, 0x0502);
        writeComData(0x3A, 0x0302);
        writeComData(0x3B, 0x0302);
        writeComData(0x23, 0x0000);
        writeComData(0x24, 0x0000);
        writeComData(0x25, 0x8000);
        writeComData(0x4f, 0x0000);
        writeComData(0x4e, 0x0000);
        writeCom(0x22);
        break;

    case DISPLAY_MODEL_ILI9341_16:
        writeCom(0xcf);
        writeData(0x00);
        writeData(0xc1);
        writeData(0x30);

        writeCom(0xed);
        writeData(0x64);
        writeData(0x03);
        writeData(0x12);
        writeData(0x81);

        writeCom(0xcb);
        writeData(0x39);
        writeData(0x2c);
        writeData(0x00);
        writeData(0x34);
        writeData(0x02);

        writeCom(0xea);
        writeData(0x00);
        writeData(0x00);

        writeCom(0xe8);
        writeData(0x85);
        writeData(0x10);
        writeData(0x79);

        writeCom(0xC0); //Power control
        writeData(0x23); //VRH[5:0]

        writeCom(0xC1); //Power control
        writeData(0x11); //SAP[2:0];BT[3:0]

        writeCom(0xC2);
        writeData(0x11);

        writeCom(0xC5); //VCM control
        writeData(0x3d);
        writeData(0x30);

        writeCom(0xc7);
        writeData(0xaa);

        writeCom(0x3A);
        writeData(0x55);

        writeCom(0x36); // Memory Access Control
        writeData(0x08);

        writeCom(0xB1); // Frame Rate Control
        writeData(0x00);
        writeData(0x11);

        writeCom(0xB6); // Display Function Control
        writeData(0x0a);
        writeData(0xa2);

        writeCom(0xF2); // 3Gamma Function Disable
        writeData(0x00);

        writeCom(0xF7);
        writeData(0x20);

        writeCom(0xF1);
        writeData(0x01);
        writeData(0x30);

        writeCom(0x26); //Gamma curve selected
        writeData(0x01);

        writeCom(0xE0); //Set Gamma
        writeData(0x0f);
        writeData(0x3f);
        writeData(0x2f);
        writeData(0x0c);
        writeData(0x10);
        writeData(0x0a);
        writeData(0x53);
        writeData(0xd5);
        writeData(0x40);
        writeData(0x0a);
        writeData(0x13);
        writeData(0x03);
        writeData(0x08);
        writeData(0x03);
        writeData(0x00);

        writeCom(0xE1); //Set Gamma
        writeData(0x00);
        writeData(0x00);
        writeData(0x10);
        writeData(0x03);
        writeData(0x0f);
        writeData(0x05);
        writeData(0x2c);
        writeData(0xa2);
        writeData(0x3f);
        writeData(0x05);
        writeData(0x0e);
        writeData(0x0c);
        writeData(0x37);
        writeData(0x3c);
        writeData(0x0F);
        writeCom(0x11); //Exit Sleep
        delay(120);
        writeCom(0x29); //display on
        delay(50);
        break;
    }

    sbi(P_CS, B_CS);
#endif
}

void LCD::setXY(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
#ifdef EEZ_PSU_SIMULATOR
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;

    if (orientation == DISPLAY_ORIENTATION_PORTRAIT) {
        x = x1;
        y = y1;
    } else {
        x = x2;
        y = y1;
    }
#else
    if (orientation == DISPLAY_ORIENTATION_LANDSCAPE) {
        swap(uint16_t, x1, y1);
        swap(uint16_t, x2, y2);
        y1 = displayHeight - 1 - y1;
        y2 = displayHeight - 1 - y2;
        swap(uint16_t, y1, y2)
    }

    switch (display_model) {
    case DISPLAY_MODEL_SSD1289:
        writeComData(0x44, (x2 << 8) + x1);
        writeComData(0x45, y1);
        writeComData(0x46, y2);
        writeComData(0x4e, x1);
        writeComData(0x4f, y1);
        writeCom(0x22);
        break;

    case DISPLAY_MODEL_ILI9341_16:
        writeCom(0x2A);
        writeData(x1 >> 8);
        writeData(x1);
        writeData(x2 >> 8);
        writeData(x2);
        writeCom(0x2B);
        writeData(y1 >> 8);
        writeData(y1);
        writeData(y2 >> 8);
        writeData(y2);
        writeCom(0x2c);
        break;
    }
#endif
}

#define FLOAT_TO_COLOR_COMPONENT(F) ((F) < 0 ? 0 : (F) > 255 ? 255 : (uint8_t)(F))

static uint8_t g_colorCache[256][4];

void LCD::onLuminocityChanged() {
    // invalidate cache
    for (int i = 0; i < 256; ++i) {
        g_colorCache[i][0] = 0;
        g_colorCache[i][1] = 0;
        g_colorCache[i][2] = 0;
        g_colorCache[i][3] = 0;
    }
}

void LCD::adjustColor(uint8_t &ch, uint8_t &cl) {
    int i = (ch & 0xF0) | (cl & 0x0F);
    if (ch == g_colorCache[i][0] && cl == g_colorCache[i][1]) {
        // cache hit!
        ch = g_colorCache[i][2];
        cl = g_colorCache[i][3];
        return;
    }

    uint8_t r, g, b;
    r = ch & 248;
    g = ((ch << 5) | (cl >> 3)) & 252;
    b = cl << 3;

    float h, s, l;
    rgbToHsl(r, g, b, h, s, l);

    float a = l < 0.5 ? l : 1 - l;
    if (a > 0.3f) {
        a = 0.3f;
    }
    float lmin = l - a;
    float lmax = l + a;

    float lNew = util::remap((float)persist_conf::devConf2.displayBackgroundLuminosityStep,
        (float)DISPLAY_BACKGROUND_LUMINOSITY_STEP_MIN,
        lmin,
        (float)DISPLAY_BACKGROUND_LUMINOSITY_STEP_MAX,
        lmax);

    float floatR, floatG, floatB;
    hslToRgb(h, s, lNew, floatR, floatG, floatB);

    r = FLOAT_TO_COLOR_COMPONENT(floatR);
    g = FLOAT_TO_COLOR_COMPONENT(floatG);
    b = FLOAT_TO_COLOR_COMPONENT(floatB);

    uint8_t chNew = RGB_TO_HIGH_BYTE(r, g, b);
    uint8_t clNew = RGB_TO_LOW_BYTE(r, g, b);

    // store new color in the cache
    g_colorCache[i][0] = ch;
    g_colorCache[i][1] = cl;
    g_colorCache[i][2] = chNew;
    g_colorCache[i][3] = clNew;

    ch = chNew;
    cl = clNew;
}

void LCD::adjustForegroundColor() {
    adjustColor(fch, fcl);
}

void LCD::adjustBackgroundColor() {
    adjustColor(bch, bcl);
}

void LCD::setColor(uint8_t r, uint8_t g, uint8_t b) {
    fch = RGB_TO_HIGH_BYTE(r, g, b);
    fcl = RGB_TO_LOW_BYTE(r, g, b);
    adjustForegroundColor();
}

void LCD::setColor(uint16_t color, bool ignoreLuminocity) {
    fch = uint8_t(color >> 8);
    fcl = uint8_t(color & 0xFF);
    if (!ignoreLuminocity) {
        adjustForegroundColor();
    }
}

uint16_t LCD::getColor() {
    return (fch << 8) | fcl;
}

void LCD::setBackColor(uint8_t r, uint8_t g, uint8_t b) {
    bch = RGB_TO_HIGH_BYTE(r, g, b);
    bcl = RGB_TO_LOW_BYTE(r, g, b);
    adjustBackgroundColor();
}

void LCD::setBackColor(uint16_t color, bool ignoreLuminocity) {
    bch = bch = uint8_t(color >> 8);
    bcl = bcl = uint8_t(color & 0xFF);
    if (!ignoreLuminocity) {
        adjustBackgroundColor();
    }
}

uint16_t LCD::getBackColor() {
    return (bch << 8) | bcl;
}

int LCD::getDisplayWidth() {
    return orientation == DISPLAY_ORIENTATION_PORTRAIT ? displayWidth : displayHeight;
}

int LCD::getDisplayHeight() {
    return orientation == DISPLAY_ORIENTATION_PORTRAIT ? displayHeight : displayWidth;
}

void LCD::setPixel(uint16_t color) {
#ifdef EEZ_PSU_SIMULATOR
    if (orientation == DISPLAY_ORIENTATION_PORTRAIT) {
        if (x >= 0 && x < displayWidth && y >= 0 && y < displayHeight) {
            *(buffer + y * displayWidth + x) = color;
        }
        if (++x > x2) {
            x = x1;
            if (++y > y2) {
                y = y1;
            }
        }
    } else {
        if (x >= 0 && x < displayHeight && y >= 0 && y < displayWidth) {
            *(buffer + y * displayHeight + x) = color;
        }

        if (--x < x1) {
            x = x2;
        }
    }
#else
    writeData((color >> 8), (color & 0xFF));	// rrrrrggggggbbbbb
#endif
}

void LCD::drawPixel(int x, int y) {
    cbi(P_CS, B_CS);
    setXY(x, y, x, y);
    setPixel((fch << 8) | fcl);
    sbi(P_CS, B_CS);
}

void LCD::drawRect(int x1, int y1, int x2, int y2) {
    if (x1 > x2) {
        swap(int, x1, x2);
    }
    if (y1 > y2) {
        swap(int, y1, y2);
    }

    drawHLine(x1, y1, x2 - x1);
    drawHLine(x1, y2, x2 - x1);
    drawVLine(x1, y1, y2 - y1);
    drawVLine(x2, y1, y2 - y1);
}

void LCD::fillRect(int x1, int y1, int x2, int y2) {
#ifdef EEZ_PSU_SIMULATOR
    if (orientation == DISPLAY_ORIENTATION_PORTRAIT) {
        setXY(x1, y1, x2, y2);
        for (int i = 0; i < (x2 - x1 + 1) * (y2 - y1 + 1); ++i) {
            setPixel((fch << 8) | fcl);
        }
    } else {
        for (int iy = y1; iy <= y2; ++iy) {
            setXY(x1, iy, x2, iy);
            for (int ix = x2; ix >= x1; --ix) {
                setPixel((fch << 8) | fcl);
            }
        }
    }
#else
    if (x1 > x2) {
        swap(int, x1, x2);
    }
    if (y1 > y2) {
        swap(int, y1, y2);
    }
    cbi(P_CS, B_CS);
    setXY(x1, y1, x2, y2);
    sbi(P_RS, B_RS);
    fastFill16(fch, fcl, (long(x2 - x1) + 1) * (long(y2 - y1) + 1));
    sbi(P_CS, B_CS);
#endif
}

void LCD::drawHLine(int x, int y, int l) {
#ifdef EEZ_PSU_SIMULATOR
    setXY(x, y, x + l, y);
    for (int i = 0; i < l + 1; ++i) {
        setPixel((fch << 8) | fcl);
    }
#else
    if (l < 0) {
        l = -l;
        x -= l;
    }
    cbi(P_CS, B_CS);
    setXY(x, y, x + l, y);
    sbi(P_RS, B_RS);
    fastFill16(fch, fcl, l);
    sbi(P_CS, B_CS);
#endif
}

void LCD::drawVLine(int x, int y, int l) {
#ifdef EEZ_PSU_SIMULATOR
    if (orientation == DISPLAY_ORIENTATION_PORTRAIT) {
        setXY(x, y, x, y + l);
        for (int i = 0; i < l + 1; ++i) {
            setPixel((fch << 8) | fcl);
        }
    } else {
        for (int i = 0; i < l +1 ; ++i) {
            setXY(x, y + i, x, y + i);
            setPixel((fch << 8) | fcl);
        }
    }
#else
    if (l < 0) {
        l = -l;
        y -= l;
    }
    cbi(P_CS, B_CS);
    setXY(x, y, x, y + l);
    sbi(P_RS, B_RS);
    fastFill16(fch, fcl, l);
    sbi(P_CS, B_CS);
#endif
}

void LCD::drawBitmap(int x, int y, int sx, int sy, uint16_t *data) {
#ifdef EEZ_PSU_SIMULATOR
    if (orientation == DISPLAY_ORIENTATION_PORTRAIT) {
        setXY(x, y, x + sx - 1, y + sy - 1);
        for (int i = 0; i < sx * sy; ++i) {
            uint8_t l = *(((uint8_t *)data) + 2 * i + 0);
            uint8_t h = *(((uint8_t *)data) + 2 * i + 1);
            if (h == RGB_TO_HIGH_BYTE(DISPLAY_BACKGROUND_COLOR_R, DISPLAY_BACKGROUND_COLOR_G, DISPLAY_BACKGROUND_COLOR_B) &&
                l == RGB_TO_LOW_BYTE(DISPLAY_BACKGROUND_COLOR_R, DISPLAY_BACKGROUND_COLOR_G, DISPLAY_BACKGROUND_COLOR_B)) {
                adjustColor(h, l);
            }
            setPixel((h << 8) + l);
        }
    } else {
        for (int iy = 0; iy < sy; ++iy) {
            setXY(x, y + iy, x + sx - 1, y + iy);
            for (int ix = sx - 1; ix >= 0; --ix) {
                uint8_t l = *(((uint8_t *)data) + 2 * (iy * sx + ix) + 0);
                uint8_t h = *(((uint8_t *)data) + 2 * (iy * sx + ix) + 1);
                if (h == RGB_TO_HIGH_BYTE(DISPLAY_BACKGROUND_COLOR_R, DISPLAY_BACKGROUND_COLOR_G, DISPLAY_BACKGROUND_COLOR_B) &&
                    l == RGB_TO_LOW_BYTE(DISPLAY_BACKGROUND_COLOR_R, DISPLAY_BACKGROUND_COLOR_G, DISPLAY_BACKGROUND_COLOR_B)) {
                    adjustColor(h, l);
                }
                setPixel((h << 8) + l);
            }
        }
    }
#else
    unsigned int color;
    int tx, ty, tc, tsx, tsy;

    if (orientation == DISPLAY_ORIENTATION_PORTRAIT) {
        cbi(P_CS, B_CS);
        setXY(x, y, x + sx - 1, y + sy - 1);
        for (tc = 0; tc < (sx*sy); tc++) {
            color = pgm_read_word(&data[tc]);
            uint8_t h = uint8_t(color >> 8);
            uint8_t l = uint8_t(color & 0xFF);
            if (h == RGB_TO_HIGH_BYTE(DISPLAY_BACKGROUND_COLOR_R, DISPLAY_BACKGROUND_COLOR_G, DISPLAY_BACKGROUND_COLOR_B) &&
                l == RGB_TO_LOW_BYTE(DISPLAY_BACKGROUND_COLOR_R, DISPLAY_BACKGROUND_COLOR_G, DISPLAY_BACKGROUND_COLOR_B)) {
                adjustColor(h, l);
            }
            writeData(h, l);
        }
        sbi(P_CS, B_CS);
    } else {
        cbi(P_CS, B_CS);
        for (ty = 0; ty < sy; ty++) {
            setXY(x, y + ty, x + sx - 1, y + ty);
            for (tx = sx - 1; tx >= 0; tx--) {
                color = pgm_read_word(&data[(ty*sx) + tx]);
                uint8_t h = uint8_t(color >> 8);
                uint8_t l = uint8_t(color & 0xFF);
                if (h == RGB_TO_HIGH_BYTE(DISPLAY_BACKGROUND_COLOR_R, DISPLAY_BACKGROUND_COLOR_G, DISPLAY_BACKGROUND_COLOR_B) &&
                    l == RGB_TO_LOW_BYTE(DISPLAY_BACKGROUND_COLOR_R, DISPLAY_BACKGROUND_COLOR_G, DISPLAY_BACKGROUND_COLOR_B)) {
                    adjustColor(h, l);
                }
                writeData(h, l);
            }
        }
        sbi(P_CS, B_CS);
    }
#endif
}

int8_t LCD::drawGlyph(int pageId, int x1, int y1, int clip_x1, int clip_y1, int clip_x2, int clip_y2, uint8_t encoding, bool fill_background) {
    if (!psu::criticalTick(pageId)) {
        return 0;
    }

	font::Glyph glyph;
	font.getGlyph(encoding, glyph);
	if (!glyph.isFound())
		return 0;

    int x2 = x1 + glyph.dx - 1;
    int y2 = y1 + font.getHeight() - 1;

    int x_glyph = x1 + glyph.x;
    int y_glyph = y1 + font.getAscent() - (glyph.y + glyph.height);

    if (fill_background) {
        // clear pixels around glyph
        uint16_t color = getColor();

        setColor(getBackColor(), true);

        if (x1 < clip_x1) x1 = clip_x1;
        if (y1 < clip_y1) y1 = clip_y1;
        if (x2 > clip_x2) x2 = clip_x2;
        if (y2 > clip_y2) y2 = clip_y2;

        if (x1 < x_glyph && y1 <= y2) {
            fillRect(x1, y1, x_glyph - 1, y2);
        }

        if (x_glyph + glyph.width <= x2 && y1 <= y2) {
            fillRect(x_glyph + glyph.width, y1, x2, y2);
        }

        if (x1 <= x2 && y1 < y_glyph) {
            fillRect(x1, y1, x2, y_glyph - 1);
        }

        if (x1 <= x2 && y_glyph + glyph.height <= y2) {
            fillRect(x1, y_glyph + glyph.height, x2, y2);
        }

        setColor(color, true);
    }
    
    // draw glyph pixels
	uint8_t widthInBytes = (glyph.width + 7) / 8;

    int iStartByte = 0;
    int iStartCol = 0;
    if (x_glyph < clip_x1) {
        int dx_off = clip_x1 - x_glyph;
        iStartByte = dx_off / 8;
        iStartCol = dx_off % 8;
        x_glyph = clip_x1;
    }

    int offset = font::GLYPH_HEADER_SIZE;
    if (y_glyph < clip_y1) {
        int dy_off = clip_y1 - y_glyph;
        offset += dy_off * widthInBytes;
        y_glyph = clip_y1;
    }

    bool paintEnabled = true;

    int width;
    if (x_glyph + glyph.width - 1 > clip_x2) {
        width = clip_x2 - x_glyph + 1;
        // if glyph doesn't fit, don't paint it, i.e. paint background
        paintEnabled = false;
    } else {
        width = glyph.width;
    }

    int height;
    if (y_glyph + glyph.height - 1 > clip_y2) {
        height = clip_y2 - y_glyph + 1;
    } else {
        height = glyph.height;
    }

    if (width > 0 && height > 0) {
#if !defined(EEZ_PSU_SIMULATOR)
        clear_bit(P_CS, B_CS);
#endif

#if DISPLAY_TYPE == TFT_320QVT_9341 && !defined(EEZ_PSU_SIMULATOR)
#if DISPLAY_ORIENTATION != DISPLAY_ORIENTATION_LANDSCAPE
#error "Only LANDSCAPE display orientation is supported on ITDB32S_V2"
#endif
        // optimized drawing for ILI9341

#define PIXEL_ON { \
    sbi(P_RS, B_RS); \
    REG_PIOA_CODR=0x0000C080; \
    REG_PIOC_CODR=0x0000003E; \
    REG_PIOD_CODR=0x0000064F; \
    REG_PIOA_SODR=REG_PIOA_SODR_FG; \
    FG_TEST ? REG_PIOB_SODR = 0x4000000 : REG_PIOB_CODR = 0x4000000; \
    REG_PIOC_SODR=REG_PIOC_SODR_FG; \
    REG_PIOD_SODR=REG_PIOD_SODR_FG; \
    pulse_low(P_WR, B_WR); \
}

#define PIXEL_OFF { \
    sbi(P_RS, B_RS); \
    REG_PIOA_CODR=0x0000C080; \
    REG_PIOC_CODR=0x0000003E; \
    REG_PIOD_CODR=0x0000064F; \
    REG_PIOA_SODR=REG_PIOA_SODR_BG; \
    BG_TEST ? REG_PIOB_SODR = 0x4000000 : REG_PIOB_CODR = 0x4000000; \
    REG_PIOC_SODR=REG_PIOC_SODR_BG; \
    REG_PIOD_SODR=REG_PIOD_SODR_BG; \
    pulse_low(P_WR, B_WR); \
}

#define SET_PIXEL(C) \
    if (C) { \
        if (LAST_PIXEL == 1) { \
            pulse_low(P_WR, B_WR); \
        } else { \
            PIXEL_ON; \
            LAST_PIXEL = 1; \
        } \
    } else { \
        if (LAST_PIXEL == 0) { \
            pulse_low(P_WR, B_WR); \
        } else { \
            PIXEL_OFF; \
            LAST_PIXEL = 0; \
        } \
    }

        uint32_t REG_PIOA_SODR_FG =((fch & 0x06)<<13) | ((fcl & 0x40)<<1);
        uint32_t REG_PIOC_SODR_FG =((fcl & 0x01)<<5) | ((fcl & 0x02)<<3) | ((fcl & 0x04)<<1) | ((fcl & 0x08)>>1) | ((fcl & 0x10)>>3);
        uint32_t REG_PIOD_SODR_FG =((fch & 0x78)>>3) | ((fch & 0x80)>>1) | ((fcl & 0x20)<<5) | ((fcl & 0x80)<<2);
        int FG_TEST = fch & 0x01;

        uint32_t REG_PIOA_SODR_BG =((bch & 0x06)<<13) | ((bcl & 0x40)<<1);
        uint32_t REG_PIOC_SODR_BG =((bcl & 0x01)<<5) | ((bcl & 0x02)<<3) | ((bcl & 0x04)<<1) | ((bcl & 0x08)>>1) | ((bcl & 0x10)>>3);
        uint32_t REG_PIOD_SODR_BG =((bch & 0x78)>>3) | ((bch & 0x80)>>1) | ((bcl & 0x20)<<5) | ((bcl & 0x80)<<2);
        int BG_TEST = bch & 0x01;

        int numPixels = 0;

        int iEndByte = iStartByte + (width + 7) / 8;
        int x1_glyph = x_glyph;
        int x2_glyph = x_glyph + width - 1;

        if (paintEnabled) {
            while (height--) {
                setXY(x1_glyph, y_glyph, x2_glyph, y_glyph);
                ++y_glyph;

                const uint8_t *p_data = glyph.data + offset + iEndByte;
                for (int iByte = iEndByte; iByte >= iStartByte; --iByte, numPixels += 8) {
                    if (numPixels % 40 == 0) {
                        if (!psu::criticalTick(pageId)) {
                            return 0;
                        }
                    }

                    uint8_t data = *p_data--;

                    int LAST_PIXEL = -1;

                    int iPixel = (iByte << 3) + 7;
                    if (iPixel - 7 >= iStartCol && iPixel < width) {
                        SET_PIXEL(data & (0x80 >> 7));
                        SET_PIXEL(data & (0x80 >> 6));
                        SET_PIXEL(data & (0x80 >> 5));
                        SET_PIXEL(data & (0x80 >> 4));
                        SET_PIXEL(data & (0x80 >> 3));
                        SET_PIXEL(data & (0x80 >> 2));
                        SET_PIXEL(data & (0x80 >> 1));
                        SET_PIXEL(data & (0x80 >> 0));
                    } else {
                                    if (iPixel >= iStartCol && iPixel < width) SET_PIXEL(data & (0x80 >> 7));
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) SET_PIXEL(data & (0x80 >> 6));
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) SET_PIXEL(data & (0x80 >> 5));
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) SET_PIXEL(data & (0x80 >> 4));
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) SET_PIXEL(data & (0x80 >> 3));
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) SET_PIXEL(data & (0x80 >> 2));
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) SET_PIXEL(data & (0x80 >> 1));
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) SET_PIXEL(data & (0x80 >> 0));
                    }
                }

                offset += widthInBytes;
            }
        } else {
            while (height--) {
                setXY(x1_glyph, y_glyph, x2_glyph, y_glyph);
                ++y_glyph;

                const uint8_t *p_data = glyph.data + offset + iEndByte;
                for (int iByte = iEndByte; iByte >= iStartByte; --iByte, numPixels += 8) {
                    if (numPixels % 120 == 0) {
                        if (!psu::criticalTick(pageId)) {
                            return 0;
                        }
                    }

                    uint8_t data = *p_data--;

                    int LAST_PIXEL = -1;

                    int iPixel = (iByte << 3) + 7;
                    if (iPixel - 7 >= iStartCol && iPixel < width) {
                        PIXEL_OFF;
                        PIXEL_OFF;
                        PIXEL_OFF;
                        PIXEL_OFF;
                        PIXEL_OFF;
                        PIXEL_OFF;
                        PIXEL_OFF;
                        PIXEL_OFF;
                    } else {
                                    if (iPixel >= iStartCol && iPixel < width) PIXEL_OFF;
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) PIXEL_OFF;
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) PIXEL_OFF;
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) PIXEL_OFF;
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) PIXEL_OFF;
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) PIXEL_OFF;
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) PIXEL_OFF;
                        --iPixel; if (iPixel >= iStartCol && iPixel < width) PIXEL_OFF;
                    }
                }

                offset += widthInBytes;
            }
        }
#else
        uint16_t fc = (fch << 8) | fcl;
        uint16_t bc = (bch << 8) | bcl;

	    if (orientation == DISPLAY_ORIENTATION_PORTRAIT) {
            int numPixels = 0;

            setXY(x_glyph, y_glyph, x_glyph + width - 1, y_glyph + height - 1);
		    for (int iRow = 0; iRow < height; ++iRow, offset += widthInBytes) {
			    for (int iByte = iStartByte, iCol = iStartCol; iByte < widthInBytes; ++iByte, numPixels += 8) {
                    if (numPixels % 120 == 0) {
                        if (!psu::criticalTick(pageId)) {
                            return 0;
                        }
                    }

                    uint8_t data = arduino_util::prog_read_byte(glyph.data + offset + iByte);
                    if (paintEnabled) {
                        if (iCol + 8 <= width) {
                            setPixel(data & 0x80 ? fc : bc);
                            setPixel(data & 0x40 ? fc : bc);
                            setPixel(data & 0x20 ? fc : bc);
                            setPixel(data & 0x10 ? fc : bc);
                            setPixel(data & 0x08 ? fc : bc);
                            setPixel(data & 0x04 ? fc : bc);
                            setPixel(data & 0x02 ? fc : bc);
                            setPixel(data & 0x01 ? fc : bc);
                            iCol += 8;
                        } else {
                            if (iCol++ >= width) break; setPixel(data & 0x80 ? fc : bc);
                            if (iCol++ >= width) break; setPixel(data & 0x40 ? fc : bc);
                            if (iCol++ >= width) break; setPixel(data & 0x20 ? fc : bc);
                            if (iCol++ >= width) break; setPixel(data & 0x10 ? fc : bc);
                            if (iCol++ >= width) break; setPixel(data & 0x08 ? fc : bc);
                            if (iCol++ >= width) break; setPixel(data & 0x04 ? fc : bc);
                            if (iCol++ >= width) break; setPixel(data & 0x02 ? fc : bc);
                            if (iCol++ >= width) break; setPixel(data & 0x01 ? fc : bc);
                        }
                    } else {
                        if (iCol + 8 <= width) {
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            iCol += 8;
                        } else {
                            if (iCol++ >= width) break; setPixel(bc);
                            if (iCol++ >= width) break; setPixel(bc);
                            if (iCol++ >= width) break; setPixel(bc);
                            if (iCol++ >= width) break; setPixel(bc);
                            if (iCol++ >= width) break; setPixel(bc);
                            if (iCol++ >= width) break; setPixel(bc);
                            if (iCol++ >= width) break; setPixel(bc);
                            if (iCol++ >= width) break; setPixel(bc);
                        }
                    }
			    }
		    }
	    }
	    else {
            int numPixels = 0;

		    for (int iRow = 0; iRow < height; ++iRow) {
			    setXY(x_glyph, y_glyph + iRow, x_glyph + width - 1, y_glyph + iRow);
			    for (int iByte = iStartByte + (width + 7) / 8; iByte >= iStartByte; --iByte, numPixels += 8) {
                    if (numPixels % 120 == 0) {
                        if (!psu::criticalTick(pageId)) {
                            return 0;
                        }
                    }

#if defined(EEZ_PSU_ARDUINO_DUE)
				    uint8_t data = *(glyph.data + offset + iByte);
#else
				    uint8_t data = arduino_util::prog_read_byte(glyph.data + offset + iByte);
#endif

                    int iPixel = iByte * 8 + 7;
                    if (iPixel - 7 >= iStartCol && iPixel < width) {
                        if (paintEnabled) {
                            setPixel((data & (0x80 >> 7)) ? fc : bc);
                            setPixel((data & (0x80 >> 6)) ? fc : bc);
                            setPixel((data & (0x80 >> 5)) ? fc : bc);
                            setPixel((data & (0x80 >> 4)) ? fc : bc);
                            setPixel((data & (0x80 >> 3)) ? fc : bc);
                            setPixel((data & (0x80 >> 2)) ? fc : bc);
                            setPixel((data & (0x80 >> 1)) ? fc : bc);
                            setPixel((data & (0x80 >> 0)) ? fc : bc);
                        } else {
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                            setPixel(bc);
                        }
                    } else {
                        if (paintEnabled) {
					                  if (iPixel >= iStartCol && iPixel < width) setPixel((data & (0x80 >> 7)) ? fc : bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel((data & (0x80 >> 6)) ? fc : bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel((data & (0x80 >> 5)) ? fc : bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel((data & (0x80 >> 4)) ? fc : bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel((data & (0x80 >> 3)) ? fc : bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel((data & (0x80 >> 2)) ? fc : bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel((data & (0x80 >> 1)) ? fc : bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel((data & (0x80 >> 0)) ? fc : bc);
                        } else {
                                      if (iPixel >= iStartCol && iPixel < width) setPixel(bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel(bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel(bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel(bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel(bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel(bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel(bc);
                            --iPixel; if (iPixel >= iStartCol && iPixel < width) setPixel(bc);
                        }
                    }
                }

			    offset += widthInBytes;
		    }
	    }
#endif

#if !defined(EEZ_PSU_SIMULATOR)
	    set_bit(P_CS, B_CS);
#endif
    }

	return glyph.dx;
}

void LCD::drawStr(int pageId, const char *text, int textLength, int x, int y, int clip_x1, int clip_y1, int clip_x2, int clip_y2, font::Font &font, bool fill_background) {
	this->font = font;

    if (textLength == -1) {
	    char encoding;
	    while ((encoding = *text++) != 0) {
		    x += drawGlyph(pageId, x, y, clip_x1, clip_y1, clip_x2, clip_y2, encoding, fill_background);
	    }
    } else {
        for (int i = 0; i < textLength && text[i]; ++i) {
            char encoding = text[i];
		    x += drawGlyph(pageId, x, y, clip_x1, clip_y1, clip_x2, clip_y2, encoding, fill_background);
	    }
    }
}

int8_t LCD::measureGlyph(uint8_t encoding) {
    font::Glyph glyph;
	font.getGlyph(encoding, glyph);
	if (!glyph.isFound())
		return 0;

	return glyph.dx;
}

int LCD::measureStr(const char *text, int textLength, font::Font &font, int max_width) {
	this->font = font;

	int width = 0;

    if (textLength == -1) {
    	char encoding;
	    while ((encoding = *text++) != 0) {
		    int glyph_width = measureGlyph(encoding);
            if (max_width > 0 && width + glyph_width > max_width) {
                break;
            }
            width += glyph_width;
	    }
    } else {
        for (int i = 0; i < textLength && text[i]; ++i) {
            char encoding = text[i];
		    int glyph_width = measureGlyph(encoding);
            if (max_width > 0 && width + glyph_width > max_width) {
                break;
            }
            width += glyph_width;
        }
    }

	return width;
}

////////////////////////////////////////////////////////////////////////////////

static bool g_isOn = false;
static bool g_onOffTransition;
static uint32_t g_onOffTransitionStart;
static uint8_t g_onOffTransitionFromBrightness;
static uint8_t g_onOffTransitionToBrightness;

void init() {
	lcd.init(DISPLAY_ORIENTATION);
    
    // make sure the screen is off on the beginning
    g_isOn = true;
	turnOff();
}

void tick(uint32_t tickCount) {
    if (g_onOffTransition) {
        int32_t diff = tickCount - g_onOffTransitionStart;
        if (diff > CONF_LCD_ON_OFF_TRANSITION_TIME) {
            g_onOffTransition = false;
            if (!g_isOn) {
                lcd.setColor(COLOR_BLACK);
                lcd.fillRect(0, 0, lcd.getDisplayWidth()-1, lcd.getDisplayHeight()-1);
            }
            updateBrightness();
        } else {
            uint8_t value = (uint8_t)round(util::remap((float)diff, 0, (float)g_onOffTransitionFromBrightness, (float)CONF_LCD_ON_OFF_TRANSITION_TIME, (float)g_onOffTransitionToBrightness));
            analogWrite(LCD_BRIGHTNESS, value);
        }
    }
}

bool isOn() {
    return g_isOn;
}

static uint8_t getBrightness() {
    return (uint8_t)round(util::remapQuad(persist_conf::devConf2.displayBrightness, 1, 196, 20, 106));
}

void turnOn(bool withoutTransition) {
    if (!g_isOn) {
        g_isOn = true;
        if (withoutTransition) {
            analogWrite(LCD_BRIGHTNESS, getBrightness());
        } else {
            g_onOffTransition = true;
            g_onOffTransitionStart = micros();
            g_onOffTransitionFromBrightness = 255;
            g_onOffTransitionToBrightness = getBrightness();
        }
    }
}

void turnOff() {
    if (g_isOn) {
        g_isOn = false;
        g_onOffTransition = true;
        g_onOffTransitionStart = micros();
        g_onOffTransitionFromBrightness = getBrightness();
        g_onOffTransitionToBrightness = 255;
    }
}

void updateBrightness() {
    if (g_isOn) {
        analogWrite(LCD_BRIGHTNESS, getBrightness());
    } else {
        analogWrite(LCD_BRIGHTNESS, 255);
    }
}

}
}
}
} // namespace eez::psu::ui::lcd

#endif