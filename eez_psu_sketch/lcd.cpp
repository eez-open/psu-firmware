/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
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

#include "lcd.h"
#include "arduino_util.h"

namespace eez {
namespace psu {
namespace gui {
namespace lcd {

////////////////////////////////////////////////////////////////////////////////

#define TFT_320QVT_1289 ITDB32S
#define TFT_320QVT_9341 ITDB32S_V2

EEZ_UTFT lcd(DISPLAY_TYPE, LCD_RS, LCD_WR, LCD_CS, LCD_RESET);

////////////////////////////////////////////////////////////////////////////////

EEZ_UTFT::EEZ_UTFT(byte model, int RS, int WR, int CS, int RST, int SER)
	: UTFT(model, RS, WR, CS, RST, SER)
{
}

int8_t EEZ_UTFT::drawGlyph(int x1, int y1, int clip_x1, int clip_y1, int clip_x2, int clip_y2, uint8_t encoding, bool fill_background) {
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
        word color = getColor();

        setColor(getBackColor());

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

        setColor(color);
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
	    clear_bit(P_CS, B_CS);

        word fc = (fch << 8) | fcl;
        word bc = (bch << 8) | bcl;

	    if (orient == PORTRAIT) {
		    setXY(x_glyph, y_glyph, x_glyph + width - 1, y_glyph + height - 1);
		    for (int iRow = 0; iRow < height; ++iRow, offset += widthInBytes) {
			    for (int iByte = iStartByte, iCol = iStartCol; iByte < widthInBytes; ++iByte) {
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
		    for (int iRow = 0; iRow < height; ++iRow) {
			    setXY(x_glyph, y_glyph + iRow, x_glyph + width - 1, y_glyph + iRow);
			    for (int iByte = iStartByte + (width + 7) / 8; iByte >= iStartByte; --iByte) {
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

	    set_bit(P_CS, B_CS);
	    clrXY();
    }

	return glyph.dx;
}

void EEZ_UTFT::drawStr(const char *text, int textLength, int x, int y, int clip_x1, int clip_y1, int clip_x2, int clip_y2, font::Font &font, bool fill_background) {
	this->font = font;

    if (textLength == -1) {
	    char encoding;
	    while ((encoding = *text++) != 0) {
		    x += drawGlyph(x, y, clip_x1, clip_y1, clip_x2, clip_y2, encoding, fill_background);
	    }
    } else {
        for (int i = 0; i < textLength && text[i]; ++i) {
            char encoding = text[i];
		    x += drawGlyph(x, y, clip_x1, clip_y1, clip_x2, clip_y2, encoding, fill_background);
	    }
    }
}

int8_t EEZ_UTFT::measureGlyph(uint8_t encoding) {
    font::Glyph glyph;
	font.getGlyph(encoding, glyph);
	if (!glyph.isFound())
		return 0;

	return glyph.dx;
}

int EEZ_UTFT::measureStr(const char *text, int textLength, font::Font &font, int max_width) {
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

void init() {
	lcd.InitLCD(DISPLAY_ORIENTATION);
    
    // make sure the screen is off on the beginning
    g_isOn = true;
	turnOff();
}

void turnOn() {
    if (!g_isOn) {
	    lcd.setContrast(64);
	    analogWrite(LCD_BRIGHTNESS, 50);
        g_isOn = true;
    }
}

void turnOff() {
    if (g_isOn) {
	    lcd.setContrast(0);
	    analogWrite(LCD_BRIGHTNESS, 255);
        lcd.setColor(VGA_BLACK);
        lcd.fillRect(0, 0, lcd.getDisplayXSize()-1, lcd.getDisplayYSize()-1);
        g_isOn = false;
    }
}

}
}
}
} // namespace eez::psu::ui::lcd

#endif