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

#include "channel.h"

namespace eez {
namespace psu {
namespace ui {

using namespace lcd;

#define BOX_FLAGS_BORDER            1 << 0

#define BOX_FLAGS_HORZ_ALIGN        3 << 1
#define BOX_FLAGS_HORZ_ALIGN_LEFT   0 << 1
#define BOX_FLAGS_HORZ_ALIGN_RIGHT  1 << 1
#define BOX_FLAGS_HORZ_ALIGN_CENTER 2 << 1

#define BOX_FLAGS_VERT_ALIGN        3 << 3
#define BOX_FLAGS_VERT_ALIGN_TOP    0 << 3
#define BOX_FLAGS_VERT_ALIGN_BOTTOM 1 << 3
#define BOX_FLAGS_VERT_ALIGN_CENTER 2 << 3

struct Style {
    uint16_t flags;
    int horz_padding;
    int vert_padding;
    uint16_t back_color;
    uint16_t border_color;
    uint16_t color;
    font::Font *font;

    bool hasBorder() {
        return flags & BOX_FLAGS_BORDER;
    }

    bool isHorzAlignLeft() {
        return (flags & BOX_FLAGS_HORZ_ALIGN) == BOX_FLAGS_HORZ_ALIGN_LEFT;
    }

    bool isHorzAlignRight() {
        return (flags & BOX_FLAGS_HORZ_ALIGN) == BOX_FLAGS_HORZ_ALIGN_RIGHT;
    }

    bool isVertAlignTop() {
        return (flags & BOX_FLAGS_VERT_ALIGN) == BOX_FLAGS_VERT_ALIGN_TOP;
    }

    bool isVertAlignBottom() {
        return (flags & BOX_FLAGS_VERT_ALIGN) == BOX_FLAGS_VERT_ALIGN_BOTTOM;
    }
};

struct Box {
    Style *style;
    int x1;
    int y1;
    int x2;
    int y2;
};

struct State {
    char old_text[16];
    int old_x_offset;
    int old_y_offset;
    int old_width;
    char text[16];
};

////////////////////////////////////////////////////////////////////////////////

Style style1 = {
    BOX_FLAGS_BORDER | BOX_FLAGS_HORZ_ALIGN_CENTER | BOX_FLAGS_VERT_ALIGN_CENTER,
    8,
    4,
    VGA_WHITE,
    VGA_GRAY,
    VGA_BLACK,
    &font::MEDIUM_FONT
};

Style style2 = {
    BOX_FLAGS_BORDER | BOX_FLAGS_HORZ_ALIGN_CENTER | BOX_FLAGS_VERT_ALIGN_CENTER,
    8,
    4,
    VGA_WHITE,
    VGA_GRAY,
    VGA_BLACK,
    &font::MEDIUM_FONT
};

Box mon_box = {
    &style1,
    0,
    0,
    240 - 1,
    80 - 1
};

Box u_set_box = {
    &style2,
    0,
    80,
    120 - 1,
    160 - 1
};

Box i_set_box = {
    &style2,
    120,
    80,
    240 - 1,
    160 - 1
};

Box *channel_controls[] = { &mon_box, &u_set_box, &i_set_box };

State channel_state[2][3];

////////////////////////////////////////////////////////////////////////////////

void draw(int x, int y, Box *box, State *state) {
    Style *style = box->style;

    const int x1 = x + box->x1;
    const int y1 = y + box->y1;
    const int x2 = x + box->x2;
    const int y2 = y + box->y2;
    
    char *old_text = state->old_text;
    char *text = state->text;

    int width = lcd::lcd.measureStr(text, *style->font);
    int height = style->font->getHeight();

    int x_offset;
    if (style->isHorzAlignLeft()) x_offset = x1 + style->horz_padding;
    else if (style->isHorzAlignRight()) x_offset = x2 - style->horz_padding - width;
    else x_offset = x1 + ((x2 - x1) - width) / 2;

    int y_offset;
    if (style->isVertAlignTop()) y_offset = y1 + style->vert_padding;
    else if (style->isVertAlignBottom()) y_offset = y2 - style->vert_padding -height;
    else y_offset = y1 + ((y2 - y1) - height) / 2;

    if (state->old_width == 0) {
        lcd::lcd.setColor(style->back_color);
        lcd::lcd.fillRect(x1, y1, x2, y2);
        
        if (style->hasBorder()) {
            lcd::lcd.setColor(style->border_color);
            lcd::lcd.drawRect(x1, y1, x2, y2);
        }
    } else {
        lcd::lcd.setColor(style->back_color);
        lcd::lcd.fillRect(state->old_x_offset, state->old_y_offset, state->old_x_offset + state->old_width - 1, state->old_y_offset + height - 1);
    }

    lcd::lcd.setBackColor(style->back_color);
    lcd::lcd.setColor(style->color);
    lcd::lcd.drawStr(x_offset, y_offset + style->font->getAscent(), text, *style->font);

    strcpy(old_text, text);
    state->old_x_offset = x_offset;
    state->old_y_offset = y_offset;
    state->old_width = width;
}

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

typedef uint16_t OBJ_OFFSET;

struct List {
    uint16_t count;
    OBJ_OFFSET first;
};

struct Document {
    List pages;
};

struct Widget {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    List widgets;
};

struct Page {
    uint16_t width;
    uint16_t height;
    List widgets;
};

uint8_t document[] = {
    0x01, 0x00, 0x04, 0x00, 0xF0, 0x00, 0x40, 0x01, 0x01, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xF0, 0x00, 0x40, 0x01, 0x03, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x50, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x78, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x78, 0x00, 0x50, 0x00, 0x78, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00
};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

void dump_widgets(uint8_t *start, List widgets) {
    for (int i = 0; i < widgets.count; ++i) {
        Widget *widget = (Widget *)(start + widgets.first) + i;
        DebugTrace("%d", (int)widget->x);
        DebugTrace("%d", (int)widget->y);
        DebugTrace("%d", (int)widget->w);
        DebugTrace("%d", (int)widget->h);
        dump_widgets(start, widget->widgets);
    }
}

void dump() {
    Document *doc = (Document *)document;
    DebugTrace("%d", (int)doc->pages.count);

    Page *page = (Page *)(document + doc->pages.first);
    DebugTrace("%d", (int)page->width);
    DebugTrace("%d", (int)page->height);

    dump_widgets(document, page->widgets);
}

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

    /*
    for (int i = 0; i < CH_NUM; ++i) {
        Channel &channel = Channel::get(i);

        for (int j = 0; j < 3; ++j) {
            channel_state[i][j].text[0] = 0;
        }

        if (channel.isCvMode()) {
            util::strcatCurrent(channel_state[i][0].text, channel.i.mon);
        } else {
            util::strcatVoltage(channel_state[i][0].text, channel.u.mon);
        }

        util::strcatVoltage(channel_state[i][1].text, channel.u.set);
        util::strcatCurrent(channel_state[i][2].text, channel.i.set);

        for (int j = 0; j < 3; ++j) {
            if (strcmp(channel_state[i][j].old_text, channel_state[i][j].text) != 0) {
                draw(0, i * 160, channel_controls[j], &channel_state[i][j]);
            }
        }
    }
    */
}

}
}
} // namespace eez::psu::ui
