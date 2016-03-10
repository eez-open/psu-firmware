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
#define CONF_SLIDER_Y_STEP_PIXELS 1

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

static bool draw_scale_overflow;
static bool last_draw_scale_overflow;

////////////////////////////////////////////////////////////////////////////////

void enter_modal_mode(const WidgetCursor &widget_cursor) {
    if (page_index == 0) {
        psu::enterTimeCriticalMode();

        page_index = 1;

        data_cursor = widget_cursor.cursor;
        data_id = widget_cursor.widget->data;
        last_draw_y_offset = -1;
        scale = 0;

        data::Cursor saved_cursor = data::getCursor();
        data::setCursor(data_cursor);
        data_unit = data::getMin(data_id).getUnit();
        scale_min = data::getMin(data_id).getFloat();
        scale_max = data::getMax(data_id).getFloat();
        data::setCursor(saved_cursor);
        
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

float *get_scales(int *num_steps = 0) {
    static float steps_50[] = { 30.0f, 10.0f, 5.0f, 2.0f, 1.0f, 0.5f };
    static float steps_40[] = { 20.0f, 10.0f, 5.0f, 2.0f, 1.0f, 0.5f };
    static float steps_30[] = { 20.0f, 10.0f, 5.0f, 2.0f, 1.0f, 0.5f };
    static float steps_5[] = { 3.0f, 2.0f, 1.0f, 0.8f, 0.4f, 0.2f };
    static float steps_3[] = { 2.0f, 1.0f, 0.8f, 0.4f, 0.2f };

    float max_value = data::getMax(data_id).getFloat();

    if (max_value >= 50.0f) {
        if (num_steps) *num_steps = sizeof(steps_50) / sizeof(float);
        return steps_50;
    } else if (max_value >= 40.0f) {
        if (num_steps) *num_steps = sizeof(steps_40) / sizeof(float);
        return steps_40;
    } else if (max_value >= 30.0f) {
        if (num_steps) *num_steps = sizeof(steps_30) / sizeof(float);
        return steps_30;
    } else if (max_value >= 5.0f) {
        if (num_steps) *num_steps = sizeof(steps_5) / sizeof(float);
        return steps_5;
    } else {
        if (num_steps) *num_steps = sizeof(steps_3) / sizeof(float);
        return steps_3;
    }
}

int get_new_scale(int dx) {
    int num_steps;
    get_scales(&num_steps);

    int new_scale = start_scale - dx;

    draw_scale_overflow = false;

    if (new_scale < 0) {
        draw_scale_overflow = true;
        new_scale = 0;
    }

    if (new_scale > num_steps) {
        draw_scale_overflow = true;
        new_scale = num_steps;
    }

    return new_scale;
}

float get_scale_range() {
    if (scale == 0) {
        float min_value = data::getMin(data_id).getFloat();
        float max_value = data::getMax(data_id).getFloat();
        return max_value - min_value;
    } else {
        float *scales = get_scales();
        return scales[scale - 1];
    }
}

float get_scale_step() {
    if (scale == 0) {
        return 0;
    } else {
        float *scales = get_scales();
        return scales[scale - 1] / 10;
    }
}

float scale_floor(float value) {
    if (scale == 0) {
        return data::getMin(data_id).getFloat();
    }
    return get_scale_step() * floor(value / get_scale_step());
}

float scale_ceil(float value) {
    if (scale == 0) {
        return data::getMax(data_id).getFloat();
    }
    return get_scale_step() * ceil(value / get_scale_step());
}

////////////////////////////////////////////////////////////////////////////////

bool draw(uint8_t *document, Widget *widget, int x, int y, bool refresh, bool inverse) {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(data_cursor);

    bool changed;
    data::Value value = data::get(data_id, changed);

    if (changed || refresh || draw_scale_overflow != last_draw_scale_overflow) {
        last_draw_scale_overflow = draw_scale_overflow;

        // adjust scale min and max, if required, to fit value
        if (value.getFloat() < scale_min) {
            scale_min = scale_floor(value.getFloat());
            scale_max = scale_min + get_scale_range();
        } else if (value.getFloat() > scale_max) {
            scale_max = scale_ceil(value.getFloat());
            scale_min = scale_max - get_scale_range();
        }

        float min_value = data::getMin(data_id).getFloat();
        float max_value = data::getMax(data_id).getFloat();
        if (scale_min < min_value) {
            scale_min = min_value;
            scale_max = scale_min + get_scale_range();
        } else if (scale_max > max_value) {
            scale_max = max_value;
            scale_min = scale_max - get_scale_range();
        }

        // draw scale min and max
        data::Value min = data::Value(scale_min, data_unit);
        data::Value max = data::Value(scale_max, data_unit);

        char text[32];

        Style *style = (Style *)(document + widget->style);
        font::Font *font = styleGetFont(style);
        int fontHeight = font->getAscent() / DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;

        uint16_t saved_color = style->color;
        if (draw_scale_overflow) {
            style->color = VGA_RED;
        }

        max.toTextNoUnit(text);
        drawText(text, x, y, (int)widget->w, fontHeight, style, inverse);

        min.toTextNoUnit(text);
        drawText(text, x, y + (int)widget->h - fontHeight, (int)widget->w, fontHeight, style, inverse);

        style->color = saved_color;

        // draw value
        int y_offset = (int)round(y+ widget->h - 2 * fontHeight - (widget->h - 3 * fontHeight) * (value.getFloat() - scale_min) / (scale_max - scale_min));

        value.toTextNoUnit(text);
        drawText(text, x, y_offset , (int)widget->w, fontHeight, style, inverse);

        // draw slider decorations
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

    draw_scale_overflow = false;

    data::setCursor(saved_cursor);
}

////////////////////////////////////////////////////////////////////////////////

void touch_move() {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(data_cursor);

    int dy = ((start_y - touch::y) / CONF_SLIDER_Y_STEP_PIXELS) * CONF_SLIDER_Y_STEP_PIXELS;
    float value = start_value + dy *
        (scale_max - scale_min) / (CONF_SLIDER_PRECISION_FACTOR * height);

    // change scale if required
    int dx = (touch::x - start_x) / CONF_SLIDER_X_STEP_PIXELS;
    int new_scale = get_new_scale(dx);
    if (new_scale != scale) {
        float current_scale_range = get_scale_range();
        scale = new_scale;
        float new_scale_range = get_scale_range();

        float value_rel = (value - scale_min) / current_scale_range;
        scale_min = value - value_rel * new_scale_range;

        if (value_rel < 0.5) {
            scale_min = scale_floor(scale_min);
            scale_max = scale_min + new_scale_range;
        } else {
            scale_max = scale_ceil(scale_min + new_scale_range);
            scale_min = scale_max - new_scale_range;
        }

        start_value = value;
        start_y = touch::y;
    }
    
    float min_value = data::getMin(data_id).getFloat();
    float max_value = data::getMax(data_id).getFloat();
    if (value < min_value) value = min_value;
    if (value > max_value) value = max_value;

    data::set(data_id, data::Value(value, data_unit));
    data::setCursor(saved_cursor);
}

void touch_up() {
}

}
}
}
} // namespace eez::psu::gui::slider
