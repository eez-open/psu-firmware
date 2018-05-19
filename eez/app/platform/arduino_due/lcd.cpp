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

#include "eez/app/psu.h"

#if OPTION_DISPLAY

#include "eez/mw/gui/lcd.h"
#include "eez/mw/gui/hooks.h"
#include "eez/mw/gui/font.h"

#define CONF_LCD_ON_OFF_TRANSITION_TIME 1000000L

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define cbi(reg, bitmask) *reg &= ~bitmask
#define sbi(reg, bitmask) *reg |= bitmask
#define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
#define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);

#define TFT_320QVT_1289 0
#define TFT_320QVT_9341 1

uint8_t RS, WR, CS, RST;
volatile uint32_t *P_RS, *P_WR, *P_CS, *P_RST;
uint16_t B_RS, B_WR, B_CS, B_RST;

static void write(char VH, char VL) {
	REG_PIOA_CODR = 0x0000C080;
	REG_PIOC_CODR = 0x0000003E;
	REG_PIOD_CODR = 0x0000064F;
	REG_PIOA_SODR = ((VH & 0x06) << 13) | ((VL & 0x40) << 1);
	(VH & 0x01) ? REG_PIOB_SODR = 0x4000000 : REG_PIOB_CODR = 0x4000000;
	REG_PIOC_SODR = ((VL & 0x01) << 5) | ((VL & 0x02) << 3) | ((VL & 0x04) << 1) | ((VL & 0x08) >> 1) | ((VL & 0x10) >> 3);
	REG_PIOD_SODR = ((VH & 0x78) >> 3) | ((VH & 0x80) >> 1) | ((VL & 0x20) << 5) | ((VL & 0x80) << 2);
	pulse_low(P_WR, B_WR);
}

static void writeCom(char VL) {
	cbi(P_RS, B_RS);
	write(0x00, VL);
}

static void writeData(char VH, char VL) {
	sbi(P_RS, B_RS);
	write(VH, VL);
}

static void writeData(char VL) {
	sbi(P_RS, B_RS);
	write(0x00, VL);
}

static void writeComData(char com1, int dat1) {
	writeCom(com1);
	writeData(dat1 >> 8, dat1);
}

static void setDirectionRegisters() {
	REG_PIOA_OER = 0x0000c000; //PA14,PA15 enable
	REG_PIOB_OER = 0x04000000; //PB26 enable
	REG_PIOD_OER = 0x0000064f; //PD0-3,PD6,PD9-10 enable
	REG_PIOA_OER = 0x00000080; //PA7 enable
	REG_PIOC_OER = 0x0000003e; //PC1 - PC5 enable
}

static void fastFill16(int ch, int cl, long numPixels) {
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

namespace eez {
namespace mw {
namespace gui {
namespace lcd {

////////////////////////////////////////////////////////////////////////////////

static bool g_isOn = false;
static bool g_onOffTransition;
static uint32_t g_onOffTransitionStart;
static uint8_t g_onOffTransitionFromBrightness;
static uint8_t g_onOffTransitionToBrightness;

void init() {
	RS = LCD_RS;
	WR = LCD_WR;
	CS = LCD_CS;
	RST = LCD_RESET;

	setDirectionRegisters();

	P_RS = portOutputRegister(digitalPinToPort(RS));
	P_WR = portOutputRegister(digitalPinToPort(WR));
	P_CS = portOutputRegister(digitalPinToPort(CS));
	P_RST = portOutputRegister(digitalPinToPort(RST));

	B_RS = digitalPinToBitMask(RS);
	B_WR = digitalPinToBitMask(WR);
	B_CS = digitalPinToBitMask(CS);
	B_RST = digitalPinToBitMask(RST);

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

	#if DISPLAY_TYPE == TFT_320QVT_1289
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
	#endif // TFT_320QVT_1289

	#if DISPLAY_TYPE == TFT_320QVT_9341
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
	#endif // TFT_320QVT_9341

	sbi(P_CS, B_CS);

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
				setColor(COLOR_BLACK);
				fillRect(0, 0, g_displayWidth - 1, g_displayHeight - 1);
			}
			updateBrightness();
		} else {
#if defined(EEZ_PLATFORM_ARDUINO_DUE)
			uint8_t value = (uint8_t)round(remap((float)diff, 0, (float)g_onOffTransitionFromBrightness, (float)CONF_LCD_ON_OFF_TRANSITION_TIME, (float)g_onOffTransitionToBrightness));
			analogWrite(LCD_BRIGHTNESS, value);
#endif
		}
	}
}

bool isOn() {
	return g_isOn;
}

void turnOn(bool withoutTransition) {
	if (!g_isOn) {
		g_isOn = true;
		if (withoutTransition) {
#if defined(EEZ_PLATFORM_ARDUINO_DUE)
			analogWrite(LCD_BRIGHTNESS, getBrightness());
#endif
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
#if defined(EEZ_PLATFORM_ARDUINO_DUE)
		analogWrite(LCD_BRIGHTNESS, getBrightness());
#endif
	} else {
#if defined(EEZ_PLATFORM_ARDUINO_DUE)
		analogWrite(LCD_BRIGHTNESS, 255);
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////

void setXY(uint16_t x1_, uint16_t y1_, uint16_t x2_, uint16_t y2_) {
#if DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_LANDSCAPE
	swap(uint16_t, x1_, y1_);
	swap(uint16_t, x2_, y2_);
	y1_ = g_nativeDisplayHeight - 1 - y1_;
	y2_ = g_nativeDisplayHeight - 1 - y2_;
	swap(uint16_t, y1_, y2_)
#endif // DISPLAY_ORIENTATION_PORTRAIT

#if DISPLAY_TYPE == TFT_320QVT_1289
	writeComData(0x44, (x2_ << 8) + x1_);
	writeComData(0x45, y1_);
	writeComData(0x46, y2_);
	writeComData(0x4e, x1_);
	writeComData(0x4f, y1_);
	writeCom(0x22);
#endif // TFT_320QVT_1289

#if DISPLAY_TYPE == TFT_320QVT_9341
	writeCom(0x2A);
	writeData(x1_ >> 8);
	writeData(x1_);
	writeData(x2_ >> 8);
	writeData(x2_);
	writeCom(0x2B);
	writeData(y1_ >> 8);
	writeData(y1_);
	writeData(y2_ >> 8);
	writeData(y2_);
	writeCom(0x2c);
#endif // TFT_320QVT_9341
}

void setPixel(uint16_t color) {
	writeData((color >> 8), (color & 0xFF));	// rrrrrggggggbbbbb
}

void doDrawGlyph(int pageId, const font::Glyph& glyph, bool paintEnabled, int x_glyph, int y_glyph, int width, int height, int offset, int iStartByte, int iStartCol, int widthInBytes) {
	clear_bit(P_CS, B_CS);

#if DISPLAY_TYPE == TFT_320QVT_9341
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

	uint32_t REG_PIOA_SODR_FG = ((g_fch & 0x06) << 13) | ((g_fcl & 0x40) << 1);
	uint32_t REG_PIOC_SODR_FG = ((g_fcl & 0x01) << 5) | ((g_fcl & 0x02) << 3) | ((g_fcl & 0x04) << 1) | ((g_fcl & 0x08) >> 1) | ((g_fcl & 0x10) >> 3);
	uint32_t REG_PIOD_SODR_FG = ((g_fch & 0x78) >> 3) | ((g_fch & 0x80) >> 1) | ((g_fcl & 0x20) << 5) | ((g_fcl & 0x80) << 2);
	int FG_TEST = g_fch & 0x01;

	uint32_t REG_PIOA_SODR_BG = ((g_bch & 0x06) << 13) | ((g_bcl & 0x40) << 1);
	uint32_t REG_PIOC_SODR_BG = ((g_bcl & 0x01) << 5) | ((g_bcl & 0x02) << 3) | ((g_bcl & 0x04) << 1) | ((g_bcl & 0x08) >> 1) | ((g_bcl & 0x10) >> 3);
	uint32_t REG_PIOD_SODR_BG = ((g_bch & 0x78) >> 3) | ((g_bch & 0x80) >> 1) | ((g_bcl & 0x20) << 5) | ((g_bcl & 0x80) << 2);
	int BG_TEST = g_bch & 0x01;

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
					if (!executeCriticalTasks(pageId)) {
						return;
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
					if (iPixel >= iStartCol && iPixel < width) { SET_PIXEL(data & (0x80 >> 7)); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { SET_PIXEL(data & (0x80 >> 6)); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { SET_PIXEL(data & (0x80 >> 5)); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { SET_PIXEL(data & (0x80 >> 4)); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { SET_PIXEL(data & (0x80 >> 3)); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { SET_PIXEL(data & (0x80 >> 2)); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { SET_PIXEL(data & (0x80 >> 1)); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { SET_PIXEL(data & (0x80 >> 0)); }
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
					if (!executeCriticalTasks(pageId)) {
						return;
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
					if (iPixel >= iStartCol && iPixel < width) { PIXEL_OFF; }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { PIXEL_OFF; }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { PIXEL_OFF; }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { PIXEL_OFF; }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { PIXEL_OFF; }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { PIXEL_OFF; }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { PIXEL_OFF; }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { PIXEL_OFF; }
				}
			}

			offset += widthInBytes;
		}
	}
#endif // TFT_320QVT_9341

#if DISPLAY_TYPE == TFT_320QVT_1289
	uint16_t fc = (g_fch << 8) | g_fcl;
	uint16_t bc = (g_bch << 8) | g_bcl;

#if DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_PORTRAIT
	int numPixels = 0;

	setXY(x_glyph, y_glyph, x_glyph + width - 1, y_glyph + height - 1);
	for (int iRow = 0; iRow < height; ++iRow, offset += widthInBytes) {
		for (int iByte = iStartByte, iCol = iStartCol; iByte < widthInBytes; ++iByte, numPixels += 8) {
			if (numPixels % 120 == 0) {
				if (!executeCriticalTasks(pageId)) {
					return;
				}
			}

			uint8_t data = *(glyph.data + offset + iByte);
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
					if (iCol++ >= width) { break; } setPixel(data & 0x80 ? fc : bc);
					if (iCol++ >= width) { break; } setPixel(data & 0x40 ? fc : bc);
					if (iCol++ >= width) { break; } setPixel(data & 0x20 ? fc : bc);
					if (iCol++ >= width) { break; } setPixel(data & 0x10 ? fc : bc);
					if (iCol++ >= width) { break; } setPixel(data & 0x08 ? fc : bc);
					if (iCol++ >= width) { break; } setPixel(data & 0x04 ? fc : bc);
					if (iCol++ >= width) { break; } setPixel(data & 0x02 ? fc : bc);
					if (iCol++ >= width) { break; } setPixel(data & 0x01 ? fc : bc);
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
					if (iCol++ >= width) { break; } setPixel(bc);
					if (iCol++ >= width) { break; } setPixel(bc);
					if (iCol++ >= width) { break; } setPixel(bc);
					if (iCol++ >= width) { break; } setPixel(bc);
					if (iCol++ >= width) { break; } setPixel(bc);
					if (iCol++ >= width) { break; } setPixel(bc);
					if (iCol++ >= width) { break; } setPixel(bc);
					if (iCol++ >= width) { break; } setPixel(bc);
				}
			}
		}
	}

#endif // DISPLAY_ORIENTATION_PORTRAIT

#if DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_LANDSCAPE

	int numPixels = 0;

	for (int iRow = 0; iRow < height; ++iRow) {
		setXY(x_glyph, y_glyph + iRow, x_glyph + width - 1, y_glyph + iRow);
		for (int iByte = iStartByte + (width + 7) / 8; iByte >= iStartByte; --iByte, numPixels += 8) {
			if (numPixels % 120 == 0) {
				if (!executeCriticalTasks(pageId)) {
					return;
				}
			}

			uint8_t data = *(glyph.data + offset + iByte);

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
					if (iPixel >= iStartCol && iPixel < width) { setPixel((data & (0x80 >> 7)) ? fc : bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel((data & (0x80 >> 6)) ? fc : bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel((data & (0x80 >> 5)) ? fc : bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel((data & (0x80 >> 4)) ? fc : bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel((data & (0x80 >> 3)) ? fc : bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel((data & (0x80 >> 2)) ? fc : bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel((data & (0x80 >> 1)) ? fc : bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel((data & (0x80 >> 0)) ? fc : bc); }
				} else {
					if (iPixel >= iStartCol && iPixel < width) { setPixel(bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel(bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel(bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel(bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel(bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel(bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel(bc); }
					--iPixel; if (iPixel >= iStartCol && iPixel < width) { setPixel(bc); }
				}
			}
		}

		offset += widthInBytes;
	}

#endif // DISPLAY_ORIENTATION_LANDSCAPE

#endif // TFT_320QVT_1289
	set_bit(P_CS, B_CS);
}

void drawPixel(int x, int y) {
	cbi(P_CS, B_CS);

    setXY(x, y, x, y);
    setPixel((g_fch << 8) | g_fcl);

    sbi(P_CS, B_CS);
}

void fillRect(int x1, int y1, int x2, int y2) {
    if (x1 > x2) {
        swap(int, x1, x2);
    }
    if (y1 > y2) {
        swap(int, y1, y2);
    }
    cbi(P_CS, B_CS);
    setXY(x1, y1, x2, y2);
    sbi(P_RS, B_RS);
    fastFill16(g_fch, g_fcl, (long(x2 - x1) + 1) * (long(y2 - y1) + 1));
    sbi(P_CS, B_CS);
}

void drawHLine(int x, int y, int l) {
    if (l < 0) {
        l = -l;
        x -= l;
    }
    cbi(P_CS, B_CS);
    setXY(x, y, x + l, y);
    sbi(P_RS, B_RS);
    fastFill16(g_fch, g_fcl, l);
    sbi(P_CS, B_CS);
}

void drawVLine(int x, int y, int l) {
    if (l < 0) {
        l = -l;
        y -= l;
    }
    cbi(P_CS, B_CS);
    setXY(x, y, x, y + l);
    sbi(P_RS, B_RS);
    fastFill16(g_fch, g_fcl, l);
    sbi(P_CS, B_CS);
}

void drawBitmap(int x, int y, int sx, int sy, uint16_t *data) {
    unsigned int color;
    int tx, ty, tc, tsx, tsy;

	#if DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_PORTRAIT
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
	#endif // DISPLAY_ORIENTATION_PORTRAIT

	#if DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_LANDSCAPE
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
	#endif // DISPLAY_ORIENTATION_LANDSCAPE
}

}
}
}
} // namespace eez::mw::gui::lcd

#endif
