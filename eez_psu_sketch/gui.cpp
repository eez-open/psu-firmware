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
static int page_index = 0;
Style *page_style;
static data::Cursor slider_data_cursor;
static int slider_data_id;
static int SLIDER_HEIGHT = 320;

struct WidgetCursor {
    Widget *widget;
    int x;
    int y;
    data::Cursor cursor;

    WidgetCursor() : widget(0) {}

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

    void start(int pageIndex, int x, int y, bool refresh) {
        stack[0].widget = (Widget *)(document + ((Document *)document)->pages.first) + pageIndex;
        stack[0].index = 0;
        stack_index = 0;
        stack[0].x = x;
        stack[0].y = y;
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

font::Font *styleGetFont(Style *style) {
    font::Font *font;
    if (style->font == LARGE_FONT) font = &font::large_font;
    else if (style->font == SMALL_FONT) font = &font::small_font;
    else font = &font::medium_font;
    return font;
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

    font::Font *font = styleGetFont(style);
    
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

    uint16_t background_color;

    if (inverse) {
        background_color = style->color;
    } else {
        background_color = style->background_color;
    }

    lcd::lcd.setColor(background_color);

    if (widget_refresh) {
        lcd::lcd.fillRect(x1, y1, x2, y2);
    } else if (!page_refresh || page_style->background_color != background_color) {
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
    lcd::lcd.drawStr(text, x_offset, y_offset, x1, y1, x2, y2, *font, !page_refresh && !widget_refresh || page_style->background_color != background_color);
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

void fill_rect(int x, int y, int w, int h) {
    x *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    y *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    w *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    h *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;

    lcd::lcd.fillRect(x, y, x + w - 1, y + h - 1);
}

bool draw_vertical_slider_widget(uint8_t *document, Widget *widget, int x, int y, bool refresh, bool inverse) {
    data::Cursor saved_cursor = data::getCursor();
    data::setCursor(slider_data_cursor);

    bool changed;
    data::Value value = data::get(slider_data_id, changed);
    if (changed || refresh) {
        data::Value minValue = data::getMin(slider_data_id);
        data::Value maxValue = data::getMax(slider_data_id);

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
            fill_rect(x, y + fontHeight, (int)widget->w, y_offset - (y + fontHeight));
        }

        if (y_offset + fontHeight < y + (int)widget->h - fontHeight) {
            lcd::lcd.setColor(0x50, 0x50, 0x50);
            fill_rect(x, y_offset + fontHeight, (int)widget->w, y + (int)widget->h - fontHeight - (y_offset + fontHeight));
        }

        SLIDER_HEIGHT = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * (widget->h - 3 * fontHeight);

        return true;
    }

    data::setCursor(saved_cursor);

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
    } else if (widget->type == WIDGET_TYPE_VERTICAL_SLIDER) {
        return draw_vertical_slider_widget(document, widget, x, y, refresh, inverse);
    }

    return false;
}

static EnumWidgets draw_enum_widgets(document, draw_widget);

void draw() {
    if (!draw_enum_widgets.next()) {
        draw_enum_widgets.start(page_index, 0, 0, false);
        page_refresh = false;
    }
}

void refresh_widget(WidgetCursor widget_cursor) {
    data::setCursor(widget_cursor.cursor);
    widget_refresh = true;
    draw_widget(document, widget_cursor.widget, widget_cursor.x, widget_cursor.y, true);
    widget_refresh = false;
}

void refresh_page() {
    page_refresh = true;

    // clear screen with background color
    Widget *page = (Widget *)(document + ((Document *)document)->pages.first) + page_index;
    Style *style = (Style *)(document + page->style);
    page_style = style;
    lcd::lcd.setColor(style->background_color);
    lcd::lcd.fillRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);

    draw_enum_widgets.start(page_index, 0, 0, true);
}

////////////////////////////////////////////////////////////////////////////////

static int find_widget_x;
static int find_widget_y;

bool find_widget(uint8_t *start, Widget *widget, int x, int y, bool refresh) {
    if (find_widget_x >= x * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER && find_widget_x < (x + widget->w) * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER
        && find_widget_y >= y * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER && find_widget_y < (y + widget->h) * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER) 
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

    refresh_page();
}

void tick(unsigned long tick_usec) {
    touch::tick(tick_usec);

    /*
    WidgetCursor old_selected_widget = selected_widget;

    if (touch::x != -1 && touch::y != -1) {
        find_widget_x = touch::x;
        find_widget_y = touch::y;
        EnumWidgets enum_widgets(document, find_widget);
        enum_widgets.start(true);
        enum_widgets.next();
    } else {
        selected_widget = 0;
    }

    if (old_selected_widget != selected_widget) {
        data::Cursor saved_cursor = data::getCursor();

        if (old_selected_widget) {
            refresh_widget(old_selected_widget);
        }

        if (selected_widget) {
            refresh_widget(selected_widget);
        }

        data::setCursor(saved_cursor);
    } else {
        draw();
    }

    return;
    */

    if (page_index == 1) {
        static int start_x;
        static int start_y;
        static data::Value start_value;

        data::Cursor saved_cursor = data::getCursor();
        data::setCursor(slider_data_cursor);

        if (touch::event_type == touch::TOUCH_DOWN) {
            bool changed;
            start_value = data::get(slider_data_id, changed);
            start_x = touch::x;
            start_y = touch::y;
        } else if (touch::event_type == touch::TOUCH_MOVE) {
            data::Value min_value = data::getMin(slider_data_id);
            data::Value max_value = data::getMax(slider_data_id);
            float value = start_value.getFloat() + (start_y - touch::y) * (max_value.getFloat() - min_value.getFloat()) / SLIDER_HEIGHT;
            if (value < min_value.getFloat()) value = min_value.getFloat();
            if (value > max_value.getFloat()) value = max_value.getFloat();
            data::set(slider_data_id, data::Value(value, min_value.getUnit()));
        }

        data::setCursor(saved_cursor);
    }
    
    gesture::tick(tick_usec);

    static unsigned long tap_time;

    data::Cursor saved_cursor = data::getCursor();

    if (gesture::gesture_type == gesture::GESTURE_TAP) {
        if (selected_widget) {
            WidgetCursor old_selected_widget = selected_widget;
            selected_widget = 0;
            refresh_widget(old_selected_widget);
        }

        find_widget_x = gesture::start_x;
        find_widget_y = gesture::start_y;
        EnumWidgets enum_widgets(document, find_widget);
        enum_widgets.start(page_index, 0, 0, true);
        enum_widgets.next();

        if (selected_widget) {
            if (page_index == 0 || selected_widget.widget->type == WIDGET_TYPE_DISPLAY_STRING && DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * selected_widget.widget->y == 276) {
                refresh_widget(selected_widget);
                tap_time = micros();
            }
        }
    } else {
        if (selected_widget) {
            if (tick_usec - tap_time > 100000) {
                WidgetCursor old_selected_widget = selected_widget;

                selected_widget = 0;

                if (page_index == 0) {
                    if (old_selected_widget.widget->type == WIDGET_TYPE_EDIT) {
                        page_index = 1;
                        slider_data_cursor = old_selected_widget.cursor;
                        slider_data_id = old_selected_widget.widget->data;
                        refresh_page();
                    } else {
                        refresh_widget(old_selected_widget);
                    }
                } else {
                    if (old_selected_widget.widget->type == WIDGET_TYPE_DISPLAY_STRING && DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * old_selected_widget.widget->y == 276) {
                        page_index = 0;
                        refresh_page();
                    }
                }
            }
        }
    }

    data::setCursor(saved_cursor);

    draw();
}

}
}
} // namespace eez::psu::ui
