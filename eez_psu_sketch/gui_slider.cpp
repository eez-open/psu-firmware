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
#include "gui_slider.h"

#define CONF_SLIDER_THUMB_HEIGHT 9
#define CONF_SLIDER_THUMB_COLOR VGA_BLACK
#define CONF_SLIDER_MIN_MAX_MARKS_COLOR VGA_RED
#define CONF_SLIDER_TICKS_COLOR 0xE0, 0xE0, 0xE0

namespace eez {
namespace psu {
namespace gui {
namespace slider {

static int width;
static float height;

static int start_y;
static float start_value;

static int last_y_value;

static int last_scale;

////////////////////////////////////////////////////////////////////////////////

void draw_scale(Widget *widget, int y_from, int y_to, int y_min, int y_max, int y_value, int f, int d) {
    Style *style = (Style *)(document + widget->style);

    int x1 = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->x;
    int l1 = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->w / 2;

    int x2 = x1 + l1;
    int l2 = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->w - l1;

    int s = 10 * f / d;

    for (int y_i = y_from; y_i <= y_to; ++y_i) {
        int y = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * (widget->y + widget->h) - 1 - y_i;

        // draw ticks
        if (y_i == y_min || y_i == y_max) {
            lcd::lcd.setColor(CONF_SLIDER_MIN_MAX_MARKS_COLOR);
            lcd::lcd.drawHLine(x1, y, l1);
        }
        else if (y_i % s == 0) {
            lcd::lcd.setColor(CONF_SLIDER_TICKS_COLOR);
            lcd::lcd.drawHLine(x1, y, l1);
        }
        else if (y_i % (s / 2) == 0) {
            lcd::lcd.setColor(CONF_SLIDER_TICKS_COLOR);
            lcd::lcd.drawHLine(x1, y, l1 / 2);
        }
        else if (y_i % (s / 10) == 0) {
            lcd::lcd.setColor(CONF_SLIDER_TICKS_COLOR);
            lcd::lcd.drawHLine(x1, y, l1 / 4);
        }
        else {
            lcd::lcd.setColor(style->background_color);
            lcd::lcd.drawHLine(x1, y, l1);
        }

        int d = abs(y_i - y_value);
        if (d <= CONF_SLIDER_THUMB_HEIGHT / 2) {
            // draw thumb
            lcd::lcd.setColor(CONF_SLIDER_THUMB_COLOR);
            lcd::lcd.drawHLine(x2 + d, y, l2 - d);
            if (y_i != y_value) {
                lcd::lcd.setColor(style->background_color);
                lcd::lcd.drawHLine(x2, y, d);
            }
        }
        else {
            // erase
            lcd::lcd.setColor(style->background_color);
            lcd::lcd.drawHLine(x2, y, l2);
        }
    }
}

bool draw(uint8_t *document, Widget *widget, int x, int y, bool refresh, bool inverse) {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(edit_data_cursor);

    bool changed;
    float value = data::get(edit_data_id, changed).getFloat();


    float min = data::getMin(edit_data_id).getFloat();
    float max = data::getMax(edit_data_id).getFloat();

    edit_data_cursor = data::getCursor();
    data::setCursor(saved_cursor);

    if (changed || refresh) {

        Style *style = (Style *)(document + widget->style);
        font::Font *font = styleGetFont(style);
        int fontHeight = font->getAscent() / DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;

        int f = (int)floor(DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->h / max);
        int d;
        if (max > 10) {
            d = 1;
        }
        else {
            f = 10 * (f / 10);
            d = 10;
        }

        int y_min = (int)round(min * f);
        int y_max = (int)round(max * f);
        int y_value = (int)round(value * f);

        if (is_page_refresh) {
            draw_scale(widget, 0, DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->h, y_min, y_max, y_value, f, d);
        } else {
            int last_y_value_from = last_y_value - CONF_SLIDER_THUMB_HEIGHT / 2;
            int last_y_value_to = last_y_value + CONF_SLIDER_THUMB_HEIGHT / 2;
            int y_value_from = y_value - CONF_SLIDER_THUMB_HEIGHT / 2;
            int y_value_to = y_value + CONF_SLIDER_THUMB_HEIGHT / 2;

            if (last_y_value_from != y_value_from) {
                if (last_y_value_from > y_value_from) {
                    util_swap(int, last_y_value_from, y_value_from);
                    util_swap(int, last_y_value_to, y_value_to);
                }

                if (last_y_value_from < 0) 
                    last_y_value_from = 0;

                if (y_value_to > DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->h - 1)
                    y_value_to = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->h - 1;

                if (last_y_value_to + 1 < y_value_from) {
                    draw_scale(widget, last_y_value_from, last_y_value_to, y_min, y_max, y_value, f, d);
                    draw_scale(widget, y_value_from, y_value_to, y_min, y_max, y_value, f, d);
                } else {
                    draw_scale(widget, last_y_value_from, y_value_to, y_min, y_max, y_value, f, d);
                }
            }
        }

        last_y_value = y_value;

        width = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->w;
        height = (max - min) * f;

        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////

void touch_down() {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(edit_data_cursor);

    bool changed;
    start_value = data::get(edit_data_id, changed).getFloat();
    start_y = touch::y;

    last_scale = 1;

    data::setCursor(saved_cursor);
}

////////////////////////////////////////////////////////////////////////////////

void touch_move() {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(edit_data_cursor);

    data::Value minValue = data::getMin(edit_data_id);
    float min = minValue.getFloat();
    float max = data::getMax(edit_data_id).getFloat();

    int scale;

    int x = (touch::x / 20) * 20;
    if (x < width) {
        scale = 1;
    }
    else {
        int num_bars = (max - min) >= 10 ? 9 : 5;
        int bar_width = (240 - width) / num_bars;
        scale = 1 << (1 + (x - width) / bar_width);
    }

    if (scale != last_scale) {
        bool changed;
        start_value = data::get(edit_data_id, changed).getFloat();
        start_y = touch::y;
        last_scale = scale;
    }

    float value = start_value + (start_y - touch::y) * (max - min) / (scale * height);

    if (value < min) value = min;
    if (value > max) value = max;

    data::set(edit_data_id, data::Value(value, minValue.getUnit()));

    data::setCursor(saved_cursor);
}

void touch_up() {
}

}
}
}
} // namespace eez::psu::gui::slider
