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
#include "font.h"
#include "touch.h"
#include "gesture.h"
#include "gui_data.h"

#include "channel.h"

namespace eez {
namespace psu {
namespace gui {

using namespace lcd;

#include "gui_view.h"

////////////////////////////////////////////////////////////////////////////////

bool styleHasBorder(Style *style) {
    return style->flags & STYLE_FLAGS_BORDER;
}

bool styleIsHorzAlignLeft(Style *style) {
    return (style->flags & STYLE_FLAGS_HORZ_ALIGN) == STYLE_FLAGS_HORZ_ALIGN_LEFT;
}

bool styleIsHorzAlignRight(Style *style) {
    return (style->flags & STYLE_FLAGS_HORZ_ALIGN) == STYLE_FLAGS_HORZ_ALIGN_RIGHT;
}

bool styleIsVertAlignTop(Style *style) {
    return (style->flags & STYLE_FLAGS_VERT_ALIGN) == STYLE_FLAGS_VERT_ALIGN_TOP;
}

bool styleIsVertAlignBottom(Style *style) {
    return (style->flags & STYLE_FLAGS_VERT_ALIGN) == STYLE_FLAGS_VERT_ALIGN_BOTTOM;
}

////////////////////////////////////////////////////////////////////////////////

void drawText(char *text, int x, int y, int w, int h, Style *style) {
    x *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    y *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    w *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    h *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;

    int x1 = x;
    int y1 = y;
    int x2 = x + w - 1;
    int y2 = y + h - 1;

    if (styleHasBorder(style)) {
        lcd::lcd.setColor(style->border_color);
        lcd::lcd.drawRect(x1, y1, x2, y2);
        ++x1;
        ++y1;
        --x2;
        --y2;
    }

    font::Font *font;
    if (style->font == LARGE_FONT) font = &font::large_font;
    else if (style->font == SMALL_FONT) font = &font::small_font;
    else font = &font::medium_font;
    
    int width = lcd::lcd.measureStr(text, *font);
    int height = font->getHeight();

    int x_offset;
    if (styleIsHorzAlignLeft(style)) x_offset = x1 + style->padding_horizontal;
    else if (styleIsHorzAlignRight(style)) x_offset = x2 - style->padding_horizontal - width;
    else x_offset = x1 + ((x2 - x1) - width) / 2;

    int y_offset;
    if (styleIsVertAlignTop(style)) y_offset = y1 + style->padding_vertical;
    else if (styleIsVertAlignBottom(style)) y_offset = y2 - style->padding_vertical -height;
    else y_offset = y1 + ((y2 - y1) - height) / 2;

    lcd::lcd.setColor(style->background_color);

    if (x1 <= x_offset && y1 <= y2)
        lcd::lcd.fillRect(x1, y1, x_offset, y2);
    if (x_offset + width <= x2 && y1 <= y2)
        lcd::lcd.fillRect(x_offset + width, y1, x2, y2);
    if (x_offset <= x_offset + width - 1 && y1 <= y_offset - 1)
        lcd::lcd.fillRect(x_offset, y1, x_offset + width - 1, y_offset - 1);
    if (x_offset <= x_offset + width - 1 && y_offset + height <= y2)
        lcd::lcd.fillRect(x_offset, y_offset + height, x_offset + width - 1, y2);

    lcd::lcd.setBackColor(style->background_color);
    lcd::lcd.setColor(style->color);
    lcd::lcd.drawStr(text, x_offset, y_offset, x1, y1, x2, y2, *font);
}

////////////////////////////////////////////////////////////////////////////////

void draw_widget(uint8_t *start, Widget *widget, int x, int y);
void draw_widgets(uint8_t *start, List widgets, int x, int y);

////////////////////////////////////////////////////////////////////////////////

void draw_list_widget(uint8_t *start, Widget *widget, int x, int y) {
    for (int index = 0; index < data::count(widget->data); ++index) {
        data::select(widget->data, index);

        draw_widgets(start, ((ContainerWidget *)(start + widget->specific))->widgets, x, y);

        x += widget->w;
        y += widget->h;
    }
}

void draw_container_widget(uint8_t *start, Widget *widget, int x, int y) {
    draw_widgets(start, ((ContainerWidget *)(start + widget->specific))->widgets, x, y);
}

void draw_select_widget(uint8_t *start, Widget *widget, int x, int y) {
    int index = (int)data::get(widget->data);
    data::select(widget->data, index);
    ContainerWidget *select_widget = ((ContainerWidget *)(start + widget->specific));
    Widget *selected_widget = (Widget *)(start + select_widget->widgets.first) + index;
    draw_widget(start, selected_widget, x, y);
}

void draw_edit_widget(uint8_t *start, Widget *widget, int x, int y) {
    char *text = data::get(widget->data);
    if (text) {
        drawText(text, x, y, (int)widget->w, (int)widget->h, (Style *)(start + widget->style));
    }
}

void draw_display_widget(uint8_t *start, Widget *widget, int x, int y) {
    char *text = data::get(widget->data);
    if (text) {
        drawText(text, x, y, (int)widget->w, (int)widget->h, (Style *)(start + widget->style));
    }
}

void draw_display_string_widget(uint8_t *start, Widget *widget, int x, int y) {
    char *text = (char *)(start + widget->data);
    // TODO improve performace because we are redrawing all the time
    drawText(text, x, y, (int)widget->w, (int)widget->h, (Style *)(start + widget->style));
}

void draw_display_string_select_widget(uint8_t *start, Widget *widget, int x, int y) {
    int state = (int)data::get(widget->data);
    if (state != 0) {
        DisplayStringSelectWidget *display_string_select_widget = ((DisplayStringSelectWidget *)(start + widget->specific));
        char *text;
        Style *style;
        if (state == 1) {
            text = (char *)(start + display_string_select_widget->text1);
            style = (Style *)(start + display_string_select_widget->style1);
        } else {
            text = (char *)(start + display_string_select_widget->text2);
            style = (Style *)(start + display_string_select_widget->style2);
        }
        drawText(text, x, y, (int)widget->w, (int)widget->h, style);
    }
}

void draw_three_state_indicator_widget(uint8_t *start, Widget *widget, int x, int y) {
    int state = (int)data::get(widget->data);
    if (state != 0) {
        ThreeStateIndicatorWidget *three_state_indicator_widget = ((ThreeStateIndicatorWidget *)(start + widget->specific));

        OBJ_OFFSET style;
        if (state == 1) style = widget->style;
        else if (state == 2) style = three_state_indicator_widget->style1;
        else style = three_state_indicator_widget->style2;

        char *text = (char *)(start + three_state_indicator_widget->text);
        drawText(text, x, y, (int)widget->w, (int)widget->h, (Style *)(start + style));
    }
}

////////////////////////////////////////////////////////////////////////////////

void draw_widget(uint8_t *start, Widget *widget, int x, int y) {
    if (widget->type == WIDGET_TYPE_LIST) {
        draw_list_widget(start, widget, x, y);
    } else if (widget->type == WIDGET_TYPE_CONTAINER) {
        draw_container_widget(start, widget, x, y);
    } else if (widget->type == WIDGET_TYPE_SELECT) {
        draw_select_widget(start, widget, x, y);
    } else if (widget->type == WIDGET_TYPE_DISPLAY) {
        draw_display_widget(start, widget, x, y);
    } else if (widget->type == WIDGET_TYPE_DISPLAY_STRING) {
        draw_display_string_widget(start, widget, x, y);
    } else if (widget->type == WIDGET_TYPE_EDIT) {
        draw_edit_widget(start, widget, x, y);
    } else if (widget->type == WIDGET_TYPE_THREE_STATE_INDICATOR) {
        draw_three_state_indicator_widget(start, widget, x, y);
    } else if (widget->type == WIDGET_TYPE_DISPLAY_STRING_SELECT) {
        draw_display_string_select_widget(start, widget, x, y);
    }
}

void draw_widgets(uint8_t *start, List widgets, int x, int y) {
    for (int i = 0; i < widgets.count; ++i) {
        Widget *widget = (Widget *)(start + widgets.first) + i;
        draw_widget(start, widget, x + (int)widget->x, y + (int)widget->y);
    }
}

////////////////////////////////////////////////////////////////////////////////

void draw_page(uint8_t *start, Page *page) {
    draw_widgets(start, page->widgets, 0, 0);
}

void draw() {
    Document *doc = (Document *)document;
    Page *page = (Page *)(document + doc->pages.first);
    draw_page(document, page);
}

////////////////////////////////////////////////////////////////////////////////

void init() {
    touch::init();

    lcd::init();
    lcd::lcd.setBackColor(VGA_WHITE);
    lcd::lcd.setColor(VGA_WHITE);
    lcd::lcd.fillRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);
}

////////////////////////////////////////////////////////////////////////////////

void tick(unsigned long tick_usec) {
    touch::tick();

    gesture::push_pointer(tick_usec, touch::is_down, touch::x, touch::y);

    draw();
}

}
}
} // namespace eez::psu::ui
