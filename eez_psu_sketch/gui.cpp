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

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

#define SMALL_FONT 1
#define MEDIUM_FONT 2
#define LARGE_FONT 3
#define STYLE_FLAGS_BORDER 1
#define STYLE_FLAGS_HORZ_ALIGN 6
#define STYLE_FLAGS_HORZ_ALIGN_LEFT 0
#define STYLE_FLAGS_HORZ_ALIGN_RIGHT 2
#define STYLE_FLAGS_HORZ_ALIGN_CENTER 4
#define STYLE_FLAGS_VERT_ALIGN 24
#define STYLE_FLAGS_VERT_ALIGN_TOP 0
#define STYLE_FLAGS_VERT_ALIGN_BOTTOM 8
#define STYLE_FLAGS_VERT_ALIGN_CENTER 16
#define WIDGET_TYPE_NONE 0
#define WIDGET_TYPE_CONTAINER 1
#define WIDGET_TYPE_LIST 2
#define WIDGET_TYPE_SELECT 3
#define WIDGET_TYPE_DISPLAY 4
#define WIDGET_TYPE_DISPLAY_STRING 5
#define WIDGET_TYPE_EDIT 6

typedef uint16_t OBJ_OFFSET;

struct List {
    uint16_t count;
    OBJ_OFFSET first;
};

struct Page {
    uint16_t w;
    uint16_t h;
    List widgets;
};

struct ContainerWidget {
    List widgets;
};

struct Style {
    uint8_t font;
    uint16_t flags;
    uint16_t background_color;
    uint16_t color;
    uint16_t border_color;
    uint16_t padding_horizontal;
    uint16_t padding_vertical;
};

struct Widget {
    uint8_t type;
    OBJ_OFFSET data;
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    OBJ_OFFSET style;
    OBJ_OFFSET specific;
};

struct Document {
    List styles;
    List pages;
};

#pragma pack(pop)

uint8_t document[250] = {
    0x04, 0x00, 0x08, 0x00, 0x01, 0x00, 0x3C, 0x00, 0x02, 0x15, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x9A,
    0xD6, 0x04, 0x00, 0x02, 0x00, 0x02, 0x15, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x9A, 0xD6, 0x04, 0x00,
    0x02, 0x00, 0x03, 0x15, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x9A, 0xD6, 0x04, 0x00, 0x02, 0x00, 0x02,
    0x15, 0x00, 0xFF, 0xFF, 0x00, 0xF8, 0x9A, 0xD6, 0x04, 0x00, 0x02, 0x00, 0xF0, 0x00, 0x40, 0x01,
    0x01, 0x00, 0x44, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x08,
    0x00, 0x53, 0x00, 0x01, 0x00, 0x57, 0x00, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00,
    0xA0, 0x00, 0x08, 0x00, 0x66, 0x00, 0x02, 0x00, 0x6A, 0x00, 0x05, 0x88, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xF0, 0x00, 0xA0, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xF0, 0x00, 0xA0, 0x00, 0x08, 0x00, 0x8C, 0x00, 0x4F, 0x46, 0x46, 0x00, 0x03, 0x00, 0x90, 0x00,
    0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x64, 0x00, 0x08, 0x00, 0xBD, 0x00, 0x06,
    0x06, 0x00, 0x00, 0x00, 0x64, 0x00, 0x78, 0x00, 0x3C, 0x00, 0x15, 0x00, 0x00, 0x00, 0x06, 0x07,
    0x00, 0x78, 0x00, 0x64, 0x00, 0x78, 0x00, 0x3C, 0x00, 0x15, 0x00, 0x00, 0x00, 0x03, 0x00, 0xC1,
    0x00, 0x04, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x64, 0x00, 0x22, 0x00, 0x00, 0x00,
    0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x64, 0x00, 0x22, 0x00, 0x00, 0x00, 0x05,
    0xEE, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x64, 0x00, 0x2F, 0x00, 0x00, 0x00, 0x55, 0x6E,
    0x72, 0x65, 0x67, 0x75, 0x6C, 0x61, 0x74, 0x65, 0x64, 0x00
};

////////////////////////////////////////////////////////////////////////////////

void draw_widgets(uint8_t *start, List widgets, int x, int y);

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

void drawText(char *text, int x, int y, int w, int h, Style *style) {
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

    lcd::lcd.fillRect(x1, y1, x_offset, y2);
    lcd::lcd.fillRect(x_offset + width, y1, x2, y2);
    lcd::lcd.fillRect(x_offset, y1, x_offset + width - 1, y_offset - 1);
    lcd::lcd.fillRect(x_offset, y_offset + height, x_offset + width - 1, y2);

    lcd::lcd.setBackColor(style->background_color);
    lcd::lcd.setColor(style->color);
    lcd::lcd.drawStr(text, x_offset, y_offset, *font);
}

void draw_widget(uint8_t *start, Widget *widget, int x, int y);

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
    }
}

void draw_widgets(uint8_t *start, List widgets, int x, int y) {
    for (int i = 0; i < widgets.count; ++i) {
        Widget *widget = (Widget *)(start + widgets.first) + i;
        draw_widget(start, widget, x + (int)widget->x, y + (int)widget->y);
    }
}

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
