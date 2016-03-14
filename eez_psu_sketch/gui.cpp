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
#include "gui.h"
#include "gui_internal.h"
#include "gui_slider.h"

#include "channel.h"

#define CONF_ENUM_WIDGETS_STACK_SIZE 5

namespace eez {
namespace psu {
namespace gui {

using namespace lcd;

////////////////////////////////////////////////////////////////////////////////

bool is_page_refresh;
static bool widget_refresh;
int page_index;
Style *page_style;

static WidgetCursor selected_widget;
static WidgetCursor found_widget_at_down;

static void (*dialog_yes_callback)();
static void (*dialog_no_callback)();
static void (*dialog_cancel_callback)();

static int edit_mode_page_index = PAGE_ID_EDIT_WITH_SLIDER;
data::Cursor edit_data_cursor;
int edit_data_id;

////////////////////////////////////////////////////////////////////////////////

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
        if (stack_index == CONF_ENUM_WIDGETS_STACK_SIZE) {
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

    StackItem stack[CONF_ENUM_WIDGETS_STACK_SIZE];
    int stack_index;

    bool push(Widget *widget, int x, int y, bool refresh) {
        if (widget->type == WIDGET_TYPE_CONTAINER || widget->type == WIDGET_TYPE_VERTICAL_LIST) {
            if (++stack_index == CONF_ENUM_WIDGETS_STACK_SIZE) {
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

            return push(selected_widget, x, y, changed || refresh);
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

////////////////////////////////////////////////////////////////////////////////

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
    } else if (!is_page_refresh || page_style->background_color != background_color) {
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
    lcd::lcd.drawStr(text, x_offset, y_offset, x1, y1, x2, y2, *font, !is_page_refresh && !widget_refresh || page_style->background_color != background_color);
}

void fill_rect(int x, int y, int w, int h) {
    x *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    y *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    w *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    h *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;

    lcd::lcd.fillRect(x, y, x + w - 1, y + h - 1);
}

////////////////////////////////////////////////////////////////////////////////

bool draw_display_widget(uint8_t *document, Widget *widget, int x, int y, bool refresh, bool inverse) {
    bool changed;
    data::Value value = data::get(widget->data, changed);
    if (changed || refresh) {
        char text[32];
        value.toText(text, 32);
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

bool draw_widget(uint8_t *document, Widget *widget, int x, int y, bool refresh) {
    bool inverse = selected_widget == widget;

    if (widget->type == WIDGET_TYPE_DISPLAY) {
        return draw_display_widget(document, widget, x, y, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_DISPLAY_STRING) {
        return draw_display_string_widget(document, widget, x, y, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_THREE_STATE_INDICATOR) {
        return draw_three_state_indicator_widget(document, widget, x, y, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_DISPLAY_STRING_SELECT) {
        return draw_display_string_select_widget(document, widget, x, y, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_VERTICAL_SLIDER) {
        return slider::draw(document, widget, x, y, refresh, inverse);
    }

    return false;
}

static EnumWidgets draw_enum_widgets(document, draw_widget);

void draw() {
    if (!draw_enum_widgets.next()) {
        draw_enum_widgets.start(page_index, 0, 0, false);
        is_page_refresh = false;
    }
}

void refresh_widget(WidgetCursor widget_cursor) {
    data::setCursor(widget_cursor.cursor);
    widget_refresh = true;
    draw_widget(document, widget_cursor.widget, widget_cursor.x, widget_cursor.y, true);
    widget_refresh = false;
}

void refresh_page() {
    is_page_refresh = true;

    // clear screen with background color
    Widget *page = (Widget *)(document + ((Document *)document)->pages.first) + page_index;
    Style *style = (Style *)(document + page->style);
    page_style = style;
    lcd::lcd.setColor(style->background_color);
    lcd::lcd.fillRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);

    draw_enum_widgets.start(page_index, 0, 0, true);
}

////////////////////////////////////////////////////////////////////////////////

static int find_widget_at_x;
static int find_widget_at_y;
static WidgetCursor found_widget;

bool find_widget_step(uint8_t *start, Widget *widget, int x, int y, bool refresh) {
    if (find_widget_at_x >= x * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER && find_widget_at_x < (x + widget->w) * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER
        && find_widget_at_y >= y * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER && find_widget_at_y < (y + widget->h) * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER) 
    {
        found_widget.widget = widget;
        found_widget.x = x;
        found_widget.y = y;
        found_widget.cursor = data::getCursor();
        return true;
    }
    return false;
}

void find_widget(int x, int y) {
    found_widget = 0;

    data::Cursor saved_cursor = data::getCursor();

    find_widget_at_x = touch::x;
    find_widget_at_y = touch::y;
    EnumWidgets enum_widgets(document, find_widget_step);
    enum_widgets.start(page_index, 0, 0, true);
    enum_widgets.next();

    data::setCursor(saved_cursor);
}

////////////////////////////////////////////////////////////////////////////////

void select_widget(WidgetCursor &widget_cursor) {
    selected_widget = widget_cursor;
    refresh_widget(selected_widget);
}

void deselect_widget() {
    WidgetCursor old_selected_widget = selected_widget;
    selected_widget = 0;
    refresh_widget(old_selected_widget);
}

////////////////////////////////////////////////////////////////////////////////

void enter_edit_mode(const WidgetCursor &widget_cursor) {
    if (page_index != edit_mode_page_index) {
        if (page_index == PAGE_ID_MAIN) {
            edit_data_cursor = widget_cursor.cursor;
            edit_data_id = widget_cursor.widget->data;
        }

        page_index = edit_mode_page_index;

        if (page_index == ACTION_ID_EDIT_WITH_SLIDER) {
            psu::enterTimeCriticalMode();
        }

        refresh_page();
    }
}

void exit_edit_mode() {
    if (page_index != PAGE_ID_MAIN) {
        if (page_index == ACTION_ID_EDIT_WITH_SLIDER) {
            psu::leaveTimeCriticalMode();
        }
        page_index = PAGE_ID_MAIN;
        refresh_page();
    }
}

////////////////////////////////////////////////////////////////////////////////

void do_action(int action_id, const WidgetCursor &widget_cursor) {
    if (action_id == ACTION_ID_EDIT) {
        enter_edit_mode(widget_cursor);
    } if (action_id == ACTION_ID_EDIT_WITH_SLIDER) {
        edit_mode_page_index = PAGE_ID_EDIT_WITH_SLIDER;
        enter_edit_mode(widget_cursor);
    } if (action_id == ACTION_ID_EDIT_WITH_STEP) {
        edit_mode_page_index = PAGE_ID_EDIT_WITH_STEP;
        enter_edit_mode(widget_cursor);
    } if (action_id == ACTION_ID_EDIT_WITH_KEYPAD) {
        edit_mode_page_index = PAGE_ID_EDIT_WITH_KEYPAD;
        enter_edit_mode(widget_cursor);
    } else if (action_id == ACTION_ID_EXIT) {
        exit_edit_mode();
    } else if (action_id == ACTION_ID_TOUCH_SCREEN_CALIBRATION) {
        touch::calibration::enter_calibration_mode();
    } else if (action_id == ACTION_ID_YES) {
        dialog_yes_callback();
    } else if (action_id == ACTION_ID_NO) {
        dialog_no_callback();
    } else if (action_id == ACTION_ID_CANCEL) {
        dialog_cancel_callback();
    } else {
        data::Cursor saved_cursor = data::getCursor();
        data::setCursor(widget_cursor.cursor);

        data::do_action(action_id);

        data::setCursor(saved_cursor);
    }
}

////////////////////////////////////////////////////////////////////////////////

void init() {
    lcd::init();

    touch::init();

    if (touch::calibration::is_calibrated()) {
        refresh_page();
    }
}

void tick(unsigned long tick_usec) {
    touch::tick(tick_usec);

    if (touch::calibration::is_calibrated()) {
        if (touch::event_type == touch::TOUCH_DOWN) {
            find_widget(touch::x, touch::y);
            if (found_widget && !found_widget.widget->action) {
                found_widget = 0;
            }
            found_widget_at_down = found_widget;
            if (found_widget_at_down) {
                select_widget(found_widget_at_down);
            } else {
                if (page_index == PAGE_ID_EDIT_WITH_SLIDER) {
                    slider::touch_down();
                }
            }
        } else if (touch::event_type == touch::TOUCH_MOVE) {
            if (!found_widget_at_down) {
                if (page_index == PAGE_ID_EDIT_WITH_SLIDER) {
                    slider::touch_move();
                } else if (page_index == PAGE_ID_YES_NO) {
#ifdef CONF_DEBUG
                    lcd::lcd.setColor(VGA_WHITE);

                    int x = touch::x;
                    if (x < 1) x = 1;
                    else if (x > lcd::lcd.getDisplayXSize() - 2) x = lcd::lcd.getDisplayXSize() - 2;

                    int y = touch::y;
                    if (y < 1) y = 1;
                    else if (y > lcd::lcd.getDisplayYSize() - 2) y = lcd::lcd.getDisplayYSize() - 2;

                    lcd::lcd.fillRect(touch::x-1, touch::y-1, touch::x+1, touch::y+1);
#endif
                }
            }
        } else if (touch::event_type == touch::TOUCH_UP) {
            if (found_widget_at_down) {
                deselect_widget();
                do_action(found_widget_at_down.widget->action, found_widget_at_down);
                found_widget_at_down = 0;
            } else {
                if (page_index == PAGE_ID_EDIT_WITH_SLIDER) {
                    slider::touch_up();
                }
            }
        }

        draw();
    } else {
        touch::calibration::tick(tick_usec);
    }
}

void yesNoDialog(const char *message PROGMEM, void (*yes_callback)(), void (*no_callback)(), void (*cancel_callback)()) {
    data::set(DATA_ID_ALERT_MESSAGE, data::Value(message));

    dialog_yes_callback = yes_callback;
    dialog_no_callback = no_callback;
    dialog_cancel_callback = cancel_callback;

    page_index = PAGE_ID_YES_NO;
    refresh_page();
}

}
}
} // namespace eez::psu::ui
