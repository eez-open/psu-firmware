/*
 * EEZ Middleware
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

#include "eez/mw/mw.h"

#if OPTION_DISPLAY

#include "eez/mw/gui/lcd.h"
#include "eez/mw/gui/hooks.h"
#include "eez/mw/gui/font.h"

#if defined(EEZ_PLATFORM_STM32)
#include "stm32f4xx.h"
#include "stm32f429i_discovery_lcd.h"
#endif

namespace eez {
namespace mw {
namespace gui {
namespace lcd {

////////////////////////////////////////////////////////////////////////////////

static uint16_t *g_buffer;

static uint16_t x, y, x1, y1, x2, y2;

void init() {
#if defined(EEZ_PLATFORM_SIMULATOR)
	g_buffer = new uint16_t[g_displayWidth * g_displayHeight];
#endif

#if defined(EEZ_PLATFORM_STM32)
	g_buffer = (uint16_t *)(LCD_FRAME_BUFFER);
#endif
}

void tick(uint32_t tickCount) {
}

bool isOn() {
	return true;
}

void turnOn(bool withoutTransition) {
}

void turnOff() {
	setColor(COLOR_BLACK);
	fillRect(0, 0, g_displayWidth - 1, g_displayHeight - 1);
}

void updateBrightness() {
}

void setXY(uint16_t x1_, uint16_t y1_, uint16_t x2_, uint16_t y2_) {
	x1 = x1_;
	y1 = y1_;
	x2 = x2_;
	y2 = y2_;

	x = x1;
	y = y1;
}

void setPixel(uint16_t color) {
#if defined(EEZ_PLATFORM_SIMULATOR)
	if (x >= 0 && x < g_displayWidth && y >= 0 && y < g_displayHeight) {
		*(g_buffer + y * g_displayWidth + x) = color;
	}

	if (++x > x2) {
		x = x1;
		if (++y > y2) {
			y = y1;
		}
	}
#endif

#if defined(EEZ_PLATFORM_STM32)
	if (x >= 0 && x < g_displayWidth && y >= 0 && y < g_displayHeight) {
		*(g_buffer + x * g_displayHeight + g_displayHeight - 1 - y) = color;
	}

	if (++x > x2) {
		x = x1;
		if (++y > y2) {
			y = y1;
		}
	}
#endif
}

void doDrawGlyph(int pageId, const font::Glyph& glyph, bool paintEnabled, int x_glyph, int y_glyph, int width, int height, int offset, int iStartByte, int iStartCol, int widthInBytes) {
	uint16_t fc = (g_fch << 8) | g_fcl;
	uint16_t bc = (g_bch << 8) | g_bcl;

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
}

uint16_t *getBuffer() {
	return g_buffer;
}

void drawPixel(int x, int y) {
    setXY(x, y, x, y);
    setPixel((g_fch << 8) | g_fcl);
}

void fillRect(int x1, int y1, int x2, int y2) {
    setXY(x1, y1, x2, y2);
    int n = (x2 - x1 + 1) * (y2 - y1 + 1);
    for (int i = 0; i < n; ++i) {
        setPixel((g_fch << 8) | g_fcl);
    }
}

void drawHLine(int x, int y, int l) {
    setXY(x, y, x + l, y);
    for (int i = 0; i < l + 1; ++i) {
        setPixel((g_fch << 8) | g_fcl);
    }
}

void drawVLine(int x, int y, int l) {
    setXY(x, y, x, y + l);
    for (int i = 0; i < l + 1; ++i) {
        setPixel((g_fch << 8) | g_fcl);
    }
}

void drawBitmap(int x, int y, int sx, int sy, uint16_t *data) {
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
}

}
}
}
} // namespace eez::mw::gui::lcd

#endif
