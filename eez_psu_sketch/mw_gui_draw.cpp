/*
* EEZ Middleware
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

#include "mw_mw.h"

#if OPTION_DISPLAY

#include "mw_util.h"
#include "mw_gui_gui.h"
#include "mw_gui_draw.h"

namespace eez {
namespace mw {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

bool styleHasBorder(const Style *style) {
	return style->flags & STYLE_FLAGS_BORDER ? true : false;
}

bool styleIsHorzAlignLeft(const Style *style) {
	return (style->flags & STYLE_FLAGS_HORZ_ALIGN) == STYLE_FLAGS_HORZ_ALIGN_LEFT;
}

bool styleIsHorzAlignRight(const Style *style) {
	return (style->flags & STYLE_FLAGS_HORZ_ALIGN) == STYLE_FLAGS_HORZ_ALIGN_RIGHT;
}

bool styleIsVertAlignTop(const Style *style) {
	return (style->flags & STYLE_FLAGS_VERT_ALIGN) == STYLE_FLAGS_VERT_ALIGN_TOP;
}

bool styleIsVertAlignBottom(const Style *style) {
	return (style->flags & STYLE_FLAGS_VERT_ALIGN) == STYLE_FLAGS_VERT_ALIGN_BOTTOM;
}

font::Font styleGetFont(const Style *style) {
	return font::Font(style->font > 0 ? fonts[style->font - 1] : 0);
}

bool styleIsBlink(const Style *style) {
	return style->flags & STYLE_FLAGS_BLINK ? true : false;
}

////////////////////////////////////////////////////////////////////////////////

void drawText(int pageId, const char *text, int textLength, int x, int y, int w, int h, const Style *style, bool inverse, bool blink, bool ignoreLuminocity, uint16_t *overrideBackgroundColor) {
	int x1 = x;
	int y1 = y;
	int x2 = x + w - 1;
	int y2 = y + h - 1;

	if (styleHasBorder(style)) {
		lcd::setColor(style->border_color);
		lcd::drawRect(x1, y1, x2, y2);
		++x1;
		++y1;
		--x2;
		--y2;
	}

	font::Font font = styleGetFont(style);

	int width = lcd::measureStr(text, textLength, font, x2 - x1 + 1);
	int height = font.getHeight();

	int x_offset;
	if (styleIsHorzAlignLeft(style)) x_offset = x1 + style->padding_horizontal;
	else if (styleIsHorzAlignRight(style)) x_offset = x2 - style->padding_horizontal - width;
	else x_offset = x1 + ((x2 - x1) - width) / 2;
	if (x_offset < 0) x_offset = x1;

	int y_offset;
	if (styleIsVertAlignTop(style)) y_offset = y1 + style->padding_vertical;
	else if (styleIsVertAlignBottom(style)) y_offset = y2 - style->padding_vertical - height;
	else y_offset = y1 + ((y2 - y1) - height) / 2;
	if (y_offset < 0) y_offset = y1;

	uint16_t backgroundColor;
	if (overrideBackgroundColor) {
		backgroundColor = *overrideBackgroundColor;
	} else {
		backgroundColor = style->background_color;
	}

	// fill background
	if (inverse || blink) {
		lcd::setColor(style->color, ignoreLuminocity);
	} else {
		lcd::setColor(backgroundColor, ignoreLuminocity);
	}

	if (x1 <= x_offset - 1 && y1 <= y2)
		lcd::fillRect(x1, y1, x_offset - 1, y2);
	if (x_offset + width <= x2 && y1 <= y2)
		lcd::fillRect(x_offset + width, y1, x2, y2);

	int right = MIN(x_offset + width - 1, x2);

	if (x_offset <= right && y1 <= y_offset - 1)
		lcd::fillRect(x_offset, y1, right, y_offset - 1);
	if (x_offset <= right && y_offset + height <= y2)
		lcd::fillRect(x_offset, y_offset + height, right, y2);

	// draw text
	if (inverse || blink) {
		lcd::setBackColor(style->color, ignoreLuminocity);
		lcd::setColor(backgroundColor, ignoreLuminocity);
	} else {
		lcd::setBackColor(backgroundColor, ignoreLuminocity);
		lcd::setColor(style->color, ignoreLuminocity);
	}
	lcd::drawStr(pageId, text, textLength, x_offset, y_offset, x1, y1, x2, y2, font, true);
}

void drawMultilineText(int pageId, const char *text, int x, int y, int w, int h, const Style *style, bool inverse) {
	int x1 = x;
	int y1 = y;
	int x2 = x + w - 1;
	int y2 = y + h - 1;

	if (styleHasBorder(style)) {
		lcd::setColor(style->border_color);
		lcd::drawRect(x1, y1, x2, y2);
		++x1;
		++y1;
		--x2;
		--y2;
	}

	font::Font font = styleGetFont(style);
	int height = (int)(0.9 * font.getHeight());

	font::Glyph space_glyph;
	font.getGlyph(' ', space_glyph);
	int space_width = space_glyph.dx;

	bool clear_background = false;

	uint16_t background_color = inverse ? style->color : style->background_color;
	lcd::setColor(background_color);
	lcd::fillRect(x1, y1, x2, y2);

	x1 += style->padding_horizontal;
	x2 -= style->padding_horizontal;
	y1 += style->padding_vertical;
	y2 -= style->padding_vertical;

	x = x1;
	y = y1;

	int i = 0;
	while (true) {
		int j = i;
		while (text[i] != 0 && text[i] != ' ' && text[i] != '\n')
			++i;

		int width = lcd::measureStr(text + j, i - j, font);

		while (width > x2 - x + 1) {
			if (clear_background) {
				lcd::setColor(background_color);
				lcd::fillRect(x, y, x2, y + height - 1);
			}

			y += height;
			if (y + height > y2) {
				break;
			}

			x = x1;
		}

		if (y + height > y2) {
			break;
		}

		if (inverse) {
			lcd::setBackColor(style->color);
			lcd::setColor(style->background_color);
		} else {
			lcd::setBackColor(style->background_color);
			lcd::setColor(style->color);
		}

		lcd::drawStr(pageId, text + j, i - j, x, y, x1, y1, x2, y2, font, false);

		x += width;

		while (text[i] == ' ') {
			if (clear_background) {
				lcd::setColor(background_color);
				lcd::fillRect(x, y, x + space_width - 1, y + height - 1);
			}
			x += space_width;
			++i;
		}

		if (text[i] == 0 || text[i] == '\n') {
			if (clear_background) {
				lcd::setColor(background_color);
				lcd::fillRect(x, y, x2, y + height - 1);
			}

			y += height;

			if (text[i] == 0) {
				break;
			}

			++i;

			int extraHeightBetweenParagraphs = (int)(0.2 * height);

			if (extraHeightBetweenParagraphs > 0 && clear_background) {
				lcd::setColor(background_color);
				lcd::fillRect(x1, y, x2, y + extraHeightBetweenParagraphs - 1);
			}

			y += extraHeightBetweenParagraphs;

			if (y + height > y2) {
				break;
			}
			x = x1;
		}
	}

	if (clear_background) {
		lcd::setColor(background_color);
		lcd::fillRect(x1, y, x2, y2);
	}
}

void drawBitmap(uint8_t bitmapIndex, int x, int y, int w, int h, const Style *style, bool inverse) {
	if (bitmapIndex == 0) {
		return;
	}
	Bitmap &bitmap = bitmaps[bitmapIndex - 1];

	int x1 = x;
	int y1 = y;
	int x2 = x + w - 1;
	int y2 = y + h - 1;

	if (styleHasBorder(style)) {
		lcd::setColor(style->border_color);
		lcd::drawRect(x1, y1, x2, y2);
		++x1;
		++y1;
		--x2;
		--y2;
	}

	int width = bitmap.w;
	int height = bitmap.h;

	int x_offset;
	if (styleIsHorzAlignLeft(style)) x_offset = x1 + style->padding_horizontal;
	else if (styleIsHorzAlignRight(style)) x_offset = x2 - style->padding_horizontal - width;
	else x_offset = x1 + ((x2 - x1) - width) / 2;
	if (x_offset < 0) x_offset = x1;

	int y_offset;
	if (styleIsVertAlignTop(style)) y_offset = y1 + style->padding_vertical;
	else if (styleIsVertAlignBottom(style)) y_offset = y2 - style->padding_vertical - height;
	else y_offset = y1 + ((y2 - y1) - height) / 2;
	if (y_offset < 0) y_offset = y1;

	uint16_t background_color = inverse ? style->color : style->background_color;
	lcd::setColor(background_color);

	// fill background
	if (x1 <= x_offset - 1 && y1 <= y2)
		lcd::fillRect(x1, y1, x_offset - 1, y2);
	if (x_offset + width <= x2 && y1 <= y2)
		lcd::fillRect(x_offset + width, y1, x2, y2);

	int right = MIN(x_offset + width - 1, x2);

	if (x_offset <= right && y1 <= y_offset - 1)
		lcd::fillRect(x_offset, y1, right, y_offset - 1);
	if (x_offset <= right && y_offset + height <= y2)
		lcd::fillRect(x_offset, y_offset + height, right, y2);

	// draw bitmap
	if (inverse) {
		lcd::setBackColor(style->color);
		lcd::setColor(style->background_color);
	} else {
		lcd::setBackColor(style->background_color);
		lcd::setColor(style->color);
	}
	lcd::drawBitmap(x_offset, y_offset, width, height, (uint16_t*)bitmap.pixels);
}

void drawRectangle(int x, int y, int w, int h, const Style *style, bool inverse, bool ignoreLuminocity) {
	if (w > 0 && h > 0) {
		int x1 = x;
		int y1 = y;
		int x2 = x + w - 1;
		int y2 = y + h - 1;

		if (styleHasBorder(style)) {
			lcd::setColor(style->border_color);
			lcd::drawRect(x1, y1, x2, y2);
			++x1;
			++y1;
			--x2;
			--y2;
		}

		uint16_t color = inverse ? style->background_color : style->color;
		lcd::setColor(color, ignoreLuminocity);
		lcd::fillRect(x1, y1, x2, y2);
	}
}

}
}
} // namespace eez::mw::gui

#endif
