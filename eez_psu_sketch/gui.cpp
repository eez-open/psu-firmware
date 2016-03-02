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

static bool page_refresh;
static bool widget_refresh;

struct WidgetCursor {
    Widget *widget;
    int x;
    int y;
    data::Cursor cursor;

    WidgetCursor& operator=(int) {
        widget = 0;
        return *this;
    }

    bool operator != (const Widget* rhs) const {
        return widget != rhs || cursor != data::getCursor();
    }
    bool operator == (const Widget* rhs) const {
        return !(*this != rhs);
    }

    bool operator != (const WidgetCursor& rhs) const {
        return widget != rhs.widget || x != rhs.x || y != rhs.y || cursor != rhs.cursor;
    }

    operator bool() {
        return widget != 0;
    }
};

static WidgetCursor selected_widget;

////////////////////////////////////////////////////////////////////////////////

#define ENUM_WIDGETS_STACK_SIZE 5

typedef bool(*EnumWidgetsCallback)(uint8_t *start, Widget *widget, int x, int y, bool refresh);

int g_sp = 0;

class EnumWidgets {
public:
    EnumWidgets(uint8_t *document, EnumWidgetsCallback callback) {
        this->callback = callback;
        this->document = document;
    }

    void start(bool refresh) {
        stack[0].widget = (Widget *)(document + ((Document *)document)->pages.first);
        stack[0].index = 0;
        stack_index = 0;
        stack[0].x = 0;
        stack[0].y = 0;
        stack[0].refresh = refresh;
    }

    bool next() {
        if (stack_index == ENUM_WIDGETS_STACK_SIZE) {
            return false;
        }

        while (true) {
            Widget *widget = stack[stack_index].widget;

            if (widget->type == WIDGET_TYPE_CONTAINER) {
                ContainerWidget *container = (ContainerWidget *)(document + widget->specific);
                if (stack[stack_index].index < container->widgets.count) {
                    Widget *child_widget = (Widget *)(document + container->widgets.first) + stack[stack_index].index;

                    ++stack[stack_index].index;

                    if (!push(child_widget, stack[stack_index].x, stack[stack_index].y, stack[stack_index].refresh)) {
                        return true;
                    }
                }
                else {
                    if (!pop()) {
                        return false;
                    }
                }
            }
            else if (widget->type == WIDGET_TYPE_VERTICAL_LIST) {
                if (stack[stack_index].index < data::count(widget->data)) {
                    data::select(widget->data, stack[stack_index].index);

                    Widget *child_widget = (Widget *)(document + ((VerticalListWidget *)(document + widget->specific))->item_widget);

                    ++stack[stack_index].index;

                    int y = stack[stack_index].y;
                    stack[stack_index].y += child_widget->h;

                    if (!push(child_widget, stack[stack_index].x, y, stack[stack_index].refresh)) {
                        return true;
                    }
                }
                else {
                    if (!pop()) {
                        return false;
                    }
                }
            }
        }
    }

private:
    uint8_t *document;

    EnumWidgetsCallback callback;

    struct StackItem {
        Widget *widget;
        int index;
        int x;
        int y;
        bool refresh;
    };

    StackItem stack[ENUM_WIDGETS_STACK_SIZE];
    int stack_index;

    bool push(Widget *widget, int x, int y, bool refresh) {
        if (widget->type == WIDGET_TYPE_CONTAINER || widget->type == WIDGET_TYPE_VERTICAL_LIST) {
            if (++stack_index == ENUM_WIDGETS_STACK_SIZE) {
                return false;
            }

            if (stack_index > g_sp) g_sp = stack_index;

            stack[stack_index].widget = widget;
            stack[stack_index].index = 0;
            stack[stack_index].x = x;
            stack[stack_index].y = y;
            stack[stack_index].refresh = refresh;

            return true;
        }
        else if (widget->type == WIDGET_TYPE_SELECT) {
            bool changed;

            int index = data::get(widget->data, changed).getInt();
            data::select(widget->data, index);

            ContainerWidget *select_widget = ((ContainerWidget *)(document + widget->specific));
            Widget *selected_widget = (Widget *)(document + select_widget->widgets.first) + index;

            return push(selected_widget, x, y, changed || stack[stack_index].refresh);
        }
        else {
            return !callback(document, widget, x + widget->x, y + widget->y, refresh);
        }
    }

    bool pop() {
        if (stack_index == 0) {
            return false;
        }
        --stack_index;
        return true;
    }
};

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

void drawText(char *text, int x, int y, int w, int h, Style *style, bool inverse) {
    x *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    y *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    w *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    h *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;

    int x1 = x;
    int y1 = y;
    int x2 = x + w - 1;
    int y2 = y + h - 1;

    if (styleHasBorder(style)) {
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

    if (inverse) {
        lcd::lcd.setColor(style->color);
    } else {
        lcd::lcd.setColor(style->background_color);
    }

    if (widget_refresh) {
        lcd::lcd.fillRect(x1, y1, x2, y2);
    }

    if (!page_refresh && !widget_refresh) {
        if (x1 <= x_offset && y1 <= y2)
            lcd::lcd.fillRect(x1, y1, x_offset, y2);
        if (x_offset + width <= x2 && y1 <= y2)
            lcd::lcd.fillRect(x_offset + width, y1, x2, y2);
        if (x_offset <= x_offset + width - 1 && y1 <= y_offset - 1)
            lcd::lcd.fillRect(x_offset, y1, x_offset + width - 1, y_offset - 1);
        if (x_offset <= x_offset + width - 1 && y_offset + height <= y2)
            lcd::lcd.fillRect(x_offset, y_offset + height, x_offset + width - 1, y2);
    }

    if (inverse) {
        lcd::lcd.setBackColor(style->color);
        lcd::lcd.setColor(style->background_color);
    } else {
        lcd::lcd.setBackColor(style->background_color);
        lcd::lcd.setColor(style->color);
    }
    lcd::lcd.drawStr(text, x_offset, y_offset, x1, y1, x2, y2, *font, !page_refresh && !widget_refresh);
}

bool draw_edit_widget(uint8_t *document, Widget *widget, int x, int y, bool refresh, bool inverse) {
    bool changed;
    data::Value value = data::get(widget->data, changed);
    if (changed || refresh) {
        char text[32];
        value.toText(text);
        drawText(text, x, y, (int)widget->w, (int)widget->h, (Style *)(document + widget->style), inverse);
        return true;
    }
    return false;
}

bool draw_display_widget(uint8_t *document, Widget *widget, int x, int y, bool refresh, bool inverse) {
    bool changed;
    data::Value value = data::get(widget->data, changed);
    if (changed || refresh) {
        char text[32];
        value.toText(text);
        drawText(text, x, y, (int)widget->w, (int)widget->h, (Style *)(document + widget->style), inverse);
        return true;
    }
    return false;
}

bool draw_display_string_widget(uint8_t *document, Widget *widget, int x, int y, bool refresh, bool inverse) {
    if (refresh) {
        DisplayStringWidget *display_string_widget = ((DisplayStringWidget *)(document + widget->specific));
        char *text = (char *)(document + display_string_widget->text);
        drawText(text, x, y, (int)widget->w, (int)widget->h, (Style *)(document + widget->style), inverse);
        return true;
    }
    return false;
}

bool draw_display_string_select_widget(uint8_t *document, Widget *widget, int x, int y, bool refresh, bool inverse) {
    bool changed;
    int state = data::get(widget->data, changed).getInt();
    if (changed || refresh) {
        DisplayStringSelectWidget *display_string_select_widget = ((DisplayStringSelectWidget *)(document + widget->specific));
        char *text;
        Style *style;
        if (state == 1) {
            text = (char *)(document + display_string_select_widget->text1);
            style = (Style *)(document + display_string_select_widget->style1);
        } else {
            text = (char *)(document + display_string_select_widget->text2);
            style = (Style *)(document + display_string_select_widget->style2);
        }
        drawText(text, x, y, (int)widget->w, (int)widget->h, style, inverse);
        return true;
    }
    return false;
}

bool draw_three_state_indicator_widget(uint8_t *document, Widget *widget, int x, int y, bool refresh, bool inverse) {
    bool changed;
    int state = data::get(widget->data, changed).getInt();
    if (changed || refresh) {
        ThreeStateIndicatorWidget *three_state_indicator_widget = ((ThreeStateIndicatorWidget *)(document + widget->specific));

        OBJ_OFFSET style;
        if (state == 1) style = widget->style;
        else if (state == 2) style = three_state_indicator_widget->style1;
        else style = three_state_indicator_widget->style2;

        char *text = (char *)(document + three_state_indicator_widget->text);
        drawText(text, x, y, (int)widget->w, (int)widget->h, (Style *)(document + style), inverse);
        return true;
    }
    return false;
}

bool draw_widget(uint8_t *document, Widget *widget, int x, int y, bool refresh) {
    bool inverse = selected_widget == widget;

    if (widget->type == WIDGET_TYPE_DISPLAY) {
        return draw_display_widget(document, widget, x, y, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_DISPLAY_STRING) {
        return draw_display_string_widget(document, widget, x, y, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_EDIT) {
        return draw_edit_widget(document, widget, x, y, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_THREE_STATE_INDICATOR) {
        return draw_three_state_indicator_widget(document, widget, x, y, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_DISPLAY_STRING_SELECT) {
        return draw_display_string_select_widget(document, widget, x, y, refresh, inverse);
    }

    return false;
}

static EnumWidgets draw_enum_widgets(document, draw_widget);

void draw() {
    if (!draw_enum_widgets.next()) {
        draw_enum_widgets.start(false);
        page_refresh = false;
    }
}

void refreshPage() {
    page_refresh = true;

    // clear screen with background color
    Widget *page = (Widget *)(document + ((Document *)document)->pages.first);
    Style *style = (Style *)(document + page->style);
    lcd::lcd.setColor(style->background_color);
    lcd::lcd.fillRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);

    draw_enum_widgets.start(true);
}

////////////////////////////////////////////////////////////////////////////////

bool find_widget(uint8_t *start, Widget *widget, int x, int y, bool refresh) {
    if (touch::x >= x * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER && touch::x < (x + widget->w) * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER
        && touch::y >= y * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER && touch::y < (y + widget->h) * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER) 
    {
        selected_widget.widget = widget;
        selected_widget.x = x;
        selected_widget.y = y;
        selected_widget.cursor = data::getCursor();
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void init() {
    touch::init();

    lcd::init();
    lcd::lcd.setBackColor(VGA_WHITE);
    lcd::lcd.setColor(VGA_WHITE);
    lcd::lcd.fillRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);

    refreshPage();
}

void tick(unsigned long tick_usec) {
    touch::tick();

    gesture::push_pointer(tick_usec, touch::is_down, touch::x, touch::y);

    WidgetCursor old_selected_widget = selected_widget;

    if (touch::is_down) {
        EnumWidgets enum_widgets(document, find_widget);
        enum_widgets.start(true);
        enum_widgets.next();
    } else if (gesture::gesture == gesture::GESTURE_UP) {
        selected_widget = 0;
    }

    if (old_selected_widget != selected_widget) {
        data::Cursor saved_cursor = data::getCursor();

        if (old_selected_widget) {
            data::setCursor(old_selected_widget.cursor);

            Widget *widget = old_selected_widget.widget;
            int x = old_selected_widget.x;
            int y = old_selected_widget.y;

            old_selected_widget = 0;

            widget_refresh = true;
            draw_widget(document, widget, x, y, true);
            widget_refresh = false;
        }

        if (selected_widget) {
            data::setCursor(selected_widget.cursor);

            widget_refresh = true;
            draw_widget(document, selected_widget.widget, selected_widget.x, selected_widget.y, true);
            widget_refresh = false;
        }

        data::setCursor(saved_cursor);
    } else {
        draw();
    }
}

}
}
} // namespace eez::psu::ui
