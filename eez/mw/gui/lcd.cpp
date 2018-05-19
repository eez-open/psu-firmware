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
#include "eez/mw/util.h"

#define swap(type, i, j) {type t = i; i = j; j = t;}

namespace eez {
namespace mw {
namespace gui {
namespace lcd {

////////////////////////////////////////////////////////////////////////////////

uint16_t g_nativeDisplayWidth = 240;
uint16_t g_nativeDisplayHeight = 320;

#if DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_PORTRAIT
uint16_t g_displayWidth = g_nativeDisplayWidth;
uint16_t g_displayHeight = g_nativeDisplayHeight;
#else
uint16_t g_displayWidth = g_nativeDisplayHeight;
uint16_t g_displayHeight = g_nativeDisplayWidth;
#endif

static uint8_t g_colorCache[256][4];
uint8_t g_fch, g_fcl, g_bch, g_bcl;

static font::Font g_font;

void onLuminocityChanged() {
	// invalidate cache
	for (int i = 0; i < 256; ++i) {
		g_colorCache[i][0] = 0;
		g_colorCache[i][1] = 0;
		g_colorCache[i][2] = 0;
		g_colorCache[i][3] = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////

static void rgbToHsl(float r, float g, float b, float &h, float &s, float &l) {
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

static float hue2rgb(float p, float q, float t) {
	if (t < 0) t += 1;
	if (t > 1) t -= 1;
	if (t < 1.0f / 6) return p + (q - p) * 6 * t;
	if (t < 1.0f / 2) return q;
	if (t < 2.0f / 3) return p + (q - p) * (2.0f / 3 - t) * 6;
	return p;
}

static void hslToRgb(float h, float s, float l, float &r, float &g, float &b) {
	if (s == 0) {
		r = g = b = l; // achromatic
	} else {
		float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
		float p = 2 * l - q;

		r = hue2rgb(p, q, h + 1.0f / 3);
		g = hue2rgb(p, q, h);
		b = hue2rgb(p, q, h - 1.0f / 3);
	}

	r *= 255;
	g *= 255;
	b *= 255;
}

#define FLOAT_TO_COLOR_COMPONENT(F) ((F) < 0 ? 0 : (F) > 255 ? 255 : (uint8_t)(F))

void adjustColor(uint8_t &ch, uint8_t &cl) {
	if (getDisplayBackgroundLuminosityStep() == DISPLAY_BACKGROUND_LUMINOSITY_STEP_DEFAULT) {
		return;
	}

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

	float lNew = remap((float)getDisplayBackgroundLuminosityStep(),
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

static void adjustForegroundColor() {
	adjustColor(g_fch, g_fcl);
}

static void adjustBackgroundColor() {
	adjustColor(g_bch, g_bcl);
}

static int8_t drawGlyph(int pageId, int x1, int y1, int clip_x1, int clip_y1, int clip_x2, int clip_y2, uint8_t encoding, bool fill_background) {
	if (!executeCriticalTasks(pageId)) {
		return 0;
	}

	font::Glyph glyph;
	g_font.getGlyph(encoding, glyph);
	if (!glyph.isFound())
		return 0;

	int x2 = x1 + glyph.dx - 1;
	int y2 = y1 + g_font.getHeight() - 1;

	int x_glyph = x1 + glyph.x;
	int y_glyph = y1 + g_font.getAscent() - (glyph.y + glyph.height);

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
		doDrawGlyph(pageId, glyph, paintEnabled, x_glyph, y_glyph, width, height, offset, iStartByte, iStartCol, widthInBytes);
	}

	return glyph.dx;
}

static int8_t measureGlyph(uint8_t encoding) {
	font::Glyph glyph;
	g_font.getGlyph(encoding, glyph);
	if (!glyph.isFound())
		return 0;

	return glyph.dx;
}

int getDisplayWidth() {
	return g_displayWidth;
}

int getDisplayHeight() {
	return g_displayHeight;
}

void setColor(uint8_t r, uint8_t g, uint8_t b) {
    g_fch = RGB_TO_HIGH_BYTE(r, g, b);
    g_fcl = RGB_TO_LOW_BYTE(r, g, b);
    adjustForegroundColor();
}

void setColor(uint16_t color, bool ignoreLuminocity) {
    g_fch = uint8_t(color >> 8);
    g_fcl = uint8_t(color & 0xFF);
    if (!ignoreLuminocity) {
        adjustForegroundColor();
    }
}

uint16_t getColor() {
    return (g_fch << 8) | g_fcl;
}

void setBackColor(uint8_t r, uint8_t g, uint8_t b) {
    g_bch = RGB_TO_HIGH_BYTE(r, g, b);
    g_bcl = RGB_TO_LOW_BYTE(r, g, b);
    adjustBackgroundColor();
}

void setBackColor(uint16_t color, bool ignoreLuminocity) {
    g_bch = uint8_t(color >> 8);
    g_bcl = uint8_t(color & 0xFF);
    if (!ignoreLuminocity) {
        adjustBackgroundColor();
    }
}

uint16_t getBackColor() {
    return (g_bch << 8) | g_bcl;
}

void drawRect(int x1, int y1, int x2, int y2) {
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

void drawStr(int pageId, const char *text, int textLength, int x, int y, int clip_x1, int clip_y1, int clip_x2, int clip_y2, font::Font &font, bool fill_background) {
	g_font = font;

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

int measureStr(const char *text, int textLength, font::Font &font, int max_width) {
	g_font = font;

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

}
}
}
} // namespace eez::mw::gui::lcd

#endif
