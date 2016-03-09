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
#define CONF_SLIDER_X_STEP_PIXELS 20
#define CONF_SLIDER_Y_STEP_PIXELS 5

namespace eez {
namespace psu {
namespace gui {
namespace slider {

static data::Cursor data_cursor;
static int data_id;
data::Unit data_unit;
static int height;

static int start_x;
static int start_y;
static float start_value;

static int start_scale = 1;
static int scale = 1;
static float scale_min;
static float scale_max;

static int last_draw_y_offset;

////////////////////////////////////////////////////////////////////////////////

void enter_modal_mode(const WidgetCursor &widget_cursor) {
    if (page_index == 0) {
        psu::enterTimeCriticalMode();
        page_index = 1;
        data_cursor = widget_cursor.cursor;
        data_id = widget_cursor.widget->data;
        last_draw_y_offset = -1;
        scale = 1;
        data_unit = data::getMin(data_id).getUnit();
        scale_min = data::getMin(data_id).getFloat();
        scale_max = data::getMax(data_id).getFloat();
        refresh_page();
    }
}

void exit_modal_mode() {
    if (page_index == 1) {
        psu::leaveTimeCriticalMode();
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
        data::Value minValue = data::Value(scale_min, data_unit);
        data::Value maxValue = data::Value(scale_max, data_unit);

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
    start_value = data::get(data_id, changed).getFloat();
    start_scale = scale;
    start_x = touch::x;
    start_y = touch::y;

    data::setCursor(saved_cursor);
}

void touch_move() {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(data_cursor);

    float min_value = data::getMin(data_id).getFloat();
    float max_value = data::getMax(data_id).getFloat();
    float max_range = max_value - min_value;

    int dy = ((start_y - touch::y) / CONF_SLIDER_Y_STEP_PIXELS) * CONF_SLIDER_Y_STEP_PIXELS;

    float value = start_value + dy *
        (scale_max - scale_min) / (CONF_SLIDER_PRECISION_FACTOR * height);

    float scale_range = scale_max - scale_min;

    int dx = (touch::x - start_x) / CONF_SLIDER_X_STEP_PIXELS;
    int new_scale;
    if (dx > 0) {
        new_scale = start_scale >> dx;
    } else {
        new_scale = start_scale << (-dx);
    }
    if (new_scale < 1) new_scale = 1;
    if (new_scale > max_range) new_scale = (int)(2 * max_range);
    if (new_scale != scale) {
        float new_scale_range = (scale_max - scale_min) * scale / new_scale;

        scale_min = value - (value - scale_min) * new_scale_range / scale_range;
        scale_max = scale_min + new_scale_range;

        scale = new_scale;
        scale_range = new_scale_range;

        start_value = value;
        start_y = touch::y;
    }
    
    if (value < min_value) value = min_value;
    if (value > max_value) value = max_value;

    if (value < scale_min) {
        scale_min = value - scale_range / 8;
        scale_max = scale_min + scale_range;
    } else if (value > scale_max) {
        scale_max = value + scale_range / 8;
        scale_min = scale_max - scale_range;
    }

    if (scale_min < min_value) {
        scale_min = min_value;
        scale_max = scale_min + scale_range;
    } else if (scale_max > max_value) {
        scale_max = max_value;
        scale_min = scale_max - scale_range;
    }

    data::set(data_id, data::Value(value, data_unit));

    data::setCursor(saved_cursor);
}

void touch_up() {
}

}
}
}
} // namespace eez::psu::gui::slider
