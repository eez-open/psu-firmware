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

#define CONF_SLIDER_PRECISION_FACTOR 2

namespace eez {
namespace psu {
namespace gui {
namespace slider {

static data::Cursor data_cursor;
static int data_id;
static int height;

static int start_x;
static int start_y;
static data::Value start_value;

static int last_draw_y_offset;

////////////////////////////////////////////////////////////////////////////////

void enter_modal_mode(const WidgetCursor &widget_cursor) {
    if (page_index == 0) {
        page_index = 1;
        data_cursor = widget_cursor.cursor;
        data_id = widget_cursor.widget->data;
        last_draw_y_offset = -1;
        refresh_page();
    }
}

void exit_modal_mode() {
    if (page_index == 1) {
        page_index = 0;
        refresh_page();
    }
}

////////////////////////////////////////////////////////////////////////////////

bool draw(uint8_t *document, Widget *widget, int x, int y, bool refresh, bool inverse) {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(data_cursor);

    bool changed;
    data::Value value = data::get(data_id, changed);
    if (changed || refresh) {
        data::Value minValue = data::getMin(data_id);
        data::Value maxValue = data::getMax(data_id);

        char text[32];

        Style *style = (Style *)(document + widget->style);
        font::Font *font = styleGetFont(style);
        int fontHeight = font->getAscent() / DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;

        maxValue.toTextNoUnit(text);
        drawText(text, x, y, (int)widget->w, fontHeight, style, inverse);

        minValue.toTextNoUnit(text);
        drawText(text, x, y + (int)widget->h - fontHeight, (int)widget->w, fontHeight, style, inverse);

        int y_offset = (int)round(y+ widget->h - 2 * fontHeight - (widget->h - 3 * fontHeight) * (value.getFloat() - minValue.getFloat()) / (maxValue.getFloat() - minValue.getFloat()));

        value.toTextNoUnit(text);
        drawText(text, x, y_offset , (int)widget->w, fontHeight, style, inverse);

        if (inverse) {
            lcd::lcd.setColor(style->color);
        } else {
            lcd::lcd.setColor(style->background_color);
        }
        
        if (y + fontHeight < y_offset) {
            lcd::lcd.setColor(0xD0, 0xD0, 0xD0);
            if (last_draw_y_offset == -1) {
                fill_rect(x, y + fontHeight, (int)widget->w, y_offset - (y + fontHeight));
            } else {
                if (y_offset > last_draw_y_offset) {
                    fill_rect(x, y + last_draw_y_offset, (int)widget->w, y_offset - (y + last_draw_y_offset));
                }
            }
        }

        if (y_offset + fontHeight < y + (int)widget->h - fontHeight) {
            lcd::lcd.setColor(0x50, 0x50, 0x50);
            if (last_draw_y_offset == -1) {
                fill_rect(x, y_offset + fontHeight, (int)widget->w, y + (int)widget->h - fontHeight - (y_offset + fontHeight));
            } else {
                if (y_offset < last_draw_y_offset) {
                    fill_rect(x, y_offset + fontHeight, (int)widget->w, last_draw_y_offset - y_offset);
                }
            }
        }

        last_draw_y_offset = y_offset;

        height = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * (widget->h - 3 * fontHeight);

        return true;
    }

    data::setCursor(saved_cursor);

    return false;
}

////////////////////////////////////////////////////////////////////////////////

void touch_down() {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(data_cursor);

    bool changed;
    start_value = data::get(data_id, changed);
    start_x = touch::x;
    start_y = touch::y;

    data::setCursor(saved_cursor);
}

void touch_move() {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(data_cursor);

    data::Value min_value = data::getMin(data_id);
    data::Value max_value = data::getMax(data_id);
    float value = start_value.getFloat() + (start_y - touch::y) *
        (max_value.getFloat() - min_value.getFloat()) / (CONF_SLIDER_PRECISION_FACTOR * height);
    if (value < min_value.getFloat()) value = min_value.getFloat();
    if (value > max_value.getFloat()) value = max_value.getFloat();
    data::set(data_id, data::Value(value, min_value.getUnit()));

    data::setCursor(saved_cursor);
}

void touch_up() {
}

}
}
}
} // namespace eez::psu::gui::slider
