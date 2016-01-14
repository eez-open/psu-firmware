/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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
#include "lcd.h"
#include "arduino_util.h"

namespace eez {
namespace psu {
namespace ui {
namespace lcd {

////////////////////////////////////////////////////////////////////////////////

EEZ_UTFT lcd(ITDB32S, LCD_RS, LCD_WR, LCD_CS, LCD_RESET);

////////////////////////////////////////////////////////////////////////////////

EEZ_UTFT::EEZ_UTFT(byte model, int RS, int WR, int CS, int RST, int SER)
	: UTFT(model, RS, WR, CS, RST, SER)
	, p_font(&font::MEDIUM_FONT)
{
}

int8_t EEZ_UTFT::drawGlyph(int x, int y, uint8_t encoding) {
	font::Glyph glyph;
	p_font->getGlyph(encoding, glyph);
	if (!glyph.isFound())
		return 0;

	x += glyph.x;
	y -= glyph.y + glyph.height;

	uint8_t widthInBytes = (glyph.width + 7) / 8;

	clear_bit(P_CS, B_CS);

	if (!_transparent) {
		if (orient == PORTRAIT) {
			setXY(x, y, x + glyph.width - 1, y + glyph.height - 1);
			for (int iRow = 0, offset = font::GLYPH_HEADER_SIZE; iRow < glyph.height; ++iRow) {
				for (int iByte = 0, iCol = 0; iByte < widthInBytes; ++iByte, ++offset) {
					uint8_t data = arduino_util::prog_read_byte(glyph.data + offset);
					for (uint8_t mask = 0x80; mask != 0 && iCol < glyph.width; mask >>= 1, ++iCol) {
						if (data & mask) {
							setPixel((fch << 8) | fcl);
						}
						else {
							setPixel((bch << 8) | bcl);
						}
					}
				}
			}
		}
		else {
			for (int iRow = 0, offset = font::GLYPH_HEADER_SIZE; iRow < glyph.height; ++iRow) {
				setXY(x, y + iRow, x + glyph.width - 1, y + iRow);
				for (int iByte = widthInBytes - 1; iByte >= 0; --iByte) {
					uint8_t data = arduino_util::prog_read_byte(glyph.data + offset + iByte);
					for (int iBit = 7; iBit >= 0; --iBit) {
						if (iByte * 8 + iBit < glyph.width) {
							if (data & (0x80 >> iBit)) {
								setPixel((fch << 8) | fcl);
							}
							else {
								setPixel((bch << 8) | bcl);
							}
						}
					}
				}

				offset += widthInBytes;
			}
		}
	}
	else {
		for (int iRow = 0, offset = font::GLYPH_HEADER_SIZE; iRow < glyph.height; ++iRow) {
			for (int iByte = 0, iCol = 0; iByte < widthInBytes; ++iByte, ++offset) {
				uint8_t data = arduino_util::prog_read_byte(glyph.data + offset);
				for (uint8_t mask = 0x80; mask != 0 && iCol < glyph.width; mask >>= 1, ++iCol) {
					if (data & mask) {
						setXY(x + iCol, y + iRow, x + iCol + 1, y + iRow + 1);
						setPixel((fch << 8) | fcl);
					}
				}
			}
		}
	}

	set_bit(P_CS, B_CS);
	clrXY();

	return glyph.dx;
}

void EEZ_UTFT::drawStr(int x, int y, const char *text, font::Font &font) {
	p_font = &font;

	char encoding;
	while ((encoding = *text++) != 0) {
		x += drawGlyph(x, y, encoding);
	}
}

int8_t EEZ_UTFT::measureGlyph(uint8_t encoding) {
    font::Glyph glyph;
	p_font->getGlyph(encoding, glyph);
	if (!glyph.isFound())
		return 0;

	return glyph.dx;
}

int EEZ_UTFT::measureStr(const char *text, font::Font &font) {
	p_font = &font;

	int width = 0;

	char encoding;
	while ((encoding = *text++) != 0) {
		width += measureGlyph(encoding);
	}

	return width;
}

////////////////////////////////////////////////////////////////////////////////

void init() {
	lcd.InitLCD(PORTRAIT);

	lcd.setContrast(64);
	lcd.setBrightness(16);
}

}
}
}
} // namespace eez::psu::ui::lcd
