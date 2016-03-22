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
#include "gui_data_snapshot.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_slider.h"
#include "gui_edit_mode_step.h"
#include "gui_edit_mode_keypad.h"
#include "gui_widget_button_group.h"

#include "channel.h"

#ifdef EEZ_PSU_SIMULATOR
#include "front_panel/control.h"
#endif

#define CONF_BLINK_TIME 400000UL
#define CONF_ENUM_WIDGETS_STACK_SIZE 5

namespace eez {
namespace psu {
namespace gui {

using namespace lcd;

////////////////////////////////////////////////////////////////////////////////

static int active_page_id;

static bool is_page_refresh;
static bool widget_refresh;

static Style *page_style;

static WidgetCursor selected_widget;
static WidgetCursor found_widget_at_down;

static void (*dialog_yes_callback)();
static void (*dialog_no_callback)();
static void (*dialog_cancel_callback)();

static bool wasBlinkTime;
static bool isBlinkTime;

////////////////////////////////////////////////////////////////////////////////

typedef bool(*EnumWidgetsCallback)(uint8_t *start, const WidgetCursor &widgetCursor, bool refresh);

//int g_sp_max_counter = 0;

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
            else if (widget->type == WIDGET_TYPE_VERTICAL_LIST || widget->type == WIDGET_TYPE_HORIZONTAL_LIST) {
                if (stack[stack_index].index < data::count(widget->data)) {
                    data::select(cursor, widget->data, stack[stack_index].index);

                    Widget *child_widget = (Widget *)(document + ((ListWidget *)(document + widget->specific))->item_widget);

                    ++stack[stack_index].index;

                    if (widget->type == WIDGET_TYPE_VERTICAL_LIST) {
                        int y = stack[stack_index].y;
                        stack[stack_index].y += child_widget->h;

                        if (!push(child_widget, stack[stack_index].x, y, stack[stack_index].refresh)) {
                            return true;
                        }
                    } else {
                        int x = stack[stack_index].x;
                        stack[stack_index].x += child_widget->w;

                        if (!push(child_widget, x, stack[stack_index].y, stack[stack_index].refresh)) {
                            return true;
                        }
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

    data::Cursor cursor;

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
        if (widget->type == WIDGET_TYPE_CONTAINER || widget->type == WIDGET_TYPE_VERTICAL_LIST || widget->type == WIDGET_TYPE_HORIZONTAL_LIST) {
             if (++stack_index == CONF_ENUM_WIDGETS_STACK_SIZE) {
                return false;
            }

            //if (stack_index > g_sp_max_counter) g_sp_max_counter = stack_index;

            stack[stack_index].widget = widget;
            stack[stack_index].index = 0;
            stack[stack_index].x = x + widget->x;
            stack[stack_index].y = y + widget->y;
            stack[stack_index].refresh = refresh;

            return true;
        }
        else if (widget->type == WIDGET_TYPE_SELECT) {
            int index = data::currentSnapshot.get(cursor, widget->data).getInt();
            data::select(cursor, widget->data, index);

            ContainerWidget *select_widget = ((ContainerWidget *)(document + widget->specific));
            Widget *selected_widget = (Widget *)(document + select_widget->widgets.first) + index;

            if (!refresh) {
                int previousIndex = data::previousSnapshot.get(cursor, widget->data).getInt();
                refresh = index != previousIndex;
            }

            return push(selected_widget, x, y, refresh);
        }
        else {
            return !callback(document, WidgetCursor(widget, x + widget->x, y + widget->y, cursor), refresh);
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
    else if (style->font == ICONS_FONT) font = &font::icons_font;
    else font = &font::medium_font;
    return font;
}

////////////////////////////////////////////////////////////////////////////////

//int draw_counter;

void drawText(char *text, int x, int y, int w, int h, Style *style, bool inverse) {
    //++draw_counter;

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

    font::Font *font = styleGetFont(style);
    
    int width = lcd::lcd.measureStr(text, *font, x2 - x1 + 1);
    int height = font->getHeight();

    int x_offset;
    if (styleIsHorzAlignLeft(style)) x_offset = x1 + style->padding_horizontal;
    else if (styleIsHorzAlignRight(style)) x_offset = x2 - style->padding_horizontal - width;
    else x_offset = x1 + ((x2 - x1) - width) / 2;
    if (x_offset < 0) x_offset = x1;

    int y_offset;
    if (styleIsVertAlignTop(style)) y_offset = y1 + style->padding_vertical;
    else if (styleIsVertAlignBottom(style)) y_offset = y2 - style->padding_vertical -height;
    else y_offset = y1 + ((y2 - y1) - height) / 2;
    if (y_offset < 0) y_offset = y1;

    uint16_t background_color = inverse ? style->color : style->background_color;
    lcd::lcd.setColor(background_color);

    if (widget_refresh) {
        lcd::lcd.fillRect(x1, y1, x2, y2);
    } else if (!is_page_refresh || page_style->background_color != background_color) {
        if (x1 <= x_offset - 1 && y1 <= y2)
            lcd::lcd.fillRect(x1, y1, x_offset - 1, y2);
        if (x_offset + width <= x2 && y1 <= y2)
            lcd::lcd.fillRect(x_offset + width, y1, x2, y2);

        int right = min(x_offset + width - 1, x2);

        if (x_offset <= right && y1 <= y_offset - 1)
            lcd::lcd.fillRect(x_offset, y1, right, y_offset - 1);
        if (x_offset <= right && y_offset + height <= y2)
            lcd::lcd.fillRect(x_offset, y_offset + height, right, y2);
    }

    if (inverse) {
        lcd::lcd.setBackColor(style->color);
        lcd::lcd.setColor(style->background_color);
    } else {
        lcd::lcd.setBackColor(style->background_color);
        lcd::lcd.setColor(style->color);
    }
    lcd::lcd.drawStr(text, x_offset, y_offset, x1, y1, x2, y2, *font, (!is_page_refresh && !widget_refresh) || page_style->background_color != background_color);
}

void drawMultilineText(char *text, int x, int y, int w, int h, Style *style, bool inverse) {
    //++draw_counter;

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

    font::Font *font = styleGetFont(style);
    int height = (int)(0.9 * font->getHeight());
    
    font::Glyph space_glyph;
    font->getGlyph(' ', space_glyph);
    int space_width = space_glyph.dx;

    bool clear_background = false;

    uint16_t background_color = inverse ? style->color : style->background_color;
    lcd::lcd.setColor(background_color);
    if (widget_refresh) {
        lcd::lcd.fillRect(x1, y1, x2, y2);
    } else {
        clear_background = !is_page_refresh || page_style->background_color != background_color;
        if (clear_background) {
            if (style->padding_horizontal > 0) {
                lcd::lcd.fillRect(x1, y1, x1 + style->padding_horizontal - 1, y2);
                lcd::lcd.fillRect(x2 - style->padding_horizontal + 1, y1, x2, y2);
            }
            if (y1 < y) {
                lcd::lcd.fillRect(x1, y1, x2, y1 + style->padding_vertical - 1);
                lcd::lcd.fillRect(x1, y2 - style->padding_vertical + 1, x2, y2);
            }
        }
    }

    x1 += style->padding_horizontal;
    x2 -= style->padding_horizontal;
    y1 += style->padding_vertical;
    y2 -= style->padding_vertical;

    x = x1;
    y = y1;

    int i = 0;
    while (true) {
        int j = i;
        while (text[i] != 0 && text[i] != ' ' && text[i] != '\n')
            ++i;

        char save = text[i];

        text[i] = 0;
        int width = lcd::lcd.measureStr(text + j, *font);
        text[i] = save;

        while (width > x2 - x + 1) {
            if (clear_background) {
                lcd::lcd.setColor(background_color);
                lcd::lcd.fillRect(x, y, x2, y + height - 1);
            }

            y += height;
            if (y + height > y2) {
                break;
            }

            x = x1;
        }

        if (y + height > y2) {
            break;
        }

        if (inverse) {
            lcd::lcd.setBackColor(style->color);
            lcd::lcd.setColor(style->background_color);
        }
        else {
            lcd::lcd.setBackColor(style->background_color);
            lcd::lcd.setColor(style->color);
        }

        text[i] = 0;
        lcd::lcd.drawStr(text + j, x, y, x1, y1, x2, y2, *font, (!is_page_refresh && !widget_refresh) || page_style->background_color != background_color);
        text[i] = save;

        x += width;

        while (text[i] == ' ') {
            if (clear_background) {
                lcd::lcd.setColor(background_color);
                lcd::lcd.fillRect(x, y, x + space_width - 1, y + height - 1);
            }
            x += space_width;
            ++i;
        }

        if (text[i] == 0 || text[i] == '\n') {
            if (clear_background) {
                lcd::lcd.setColor(background_color);
                lcd::lcd.fillRect(x, y, x2, y + height - 1);
            }

            y += height;

            if (text[i] == 0) {
                break;
            }

            ++i;

            int extraHeightBetweenParagraphs = (int)(0.2 * height);

            if (extraHeightBetweenParagraphs > 0 && clear_background) {
                lcd::lcd.setColor(background_color);
                lcd::lcd.fillRect(x1, y, x2, y + extraHeightBetweenParagraphs - 1);
            }

            y += extraHeightBetweenParagraphs;

            if (y + height > y2) {
                break;
            }
            x = x1;
        }
    }

    if (clear_background) {
        lcd::lcd.setColor(background_color);
        lcd::lcd.fillRect(x1, y, x2, y2);
    }
}

void fillRect(int x, int y, int w, int h) {
    x *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    y *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    w *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;
    h *= DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;

    lcd::lcd.fillRect(x, y, x + w - 1, y + h - 1);
}

////////////////////////////////////////////////////////////////////////////////

Style *get_active_style(Widget *widget) {
    int widget_style_id = (Style *)(document + widget->style) - (Style *)(document + ((Document *)document)->styles.first);
    int active_style_id = widget_style_id == STYLE_ID_EDIT_VALUE_SMALL ? STYLE_ID_EDIT_VALUE_ACTIVE_SMALL : STYLE_ID_EDIT_VALUE_ACTIVE;
    return (Style *)(document + ((Document *)document)->styles.first) + active_style_id;
}

bool draw_display_widget(uint8_t *document, const WidgetCursor &widgetCursor, bool refresh, bool inverse) {
    bool edit = edit_mode::isEditWidget(widgetCursor);
    if (edit && active_page_id == PAGE_ID_EDIT_MODE_KEYPAD) {
        char *text = data::currentSnapshot.editModeSnapshot.keypadText;
        if (!refresh) {
            char *previousText = data::previousSnapshot.editModeSnapshot.keypadText;
            refresh = strcmp(text, previousText) != 0;
        }
        if (refresh) {
            drawText(text, widgetCursor.x, widgetCursor.y, (int)widgetCursor.widget->w, (int)widgetCursor.widget->h, get_active_style(widgetCursor.widget), inverse);
            return true;
        }
        return false;
    }
    else {
        data::Value value = data::currentSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data);
        bool isBlinking = data::currentSnapshot.isBlinking(widgetCursor.cursor, widgetCursor.widget->data);
        if (isBlinkTime && isBlinking) {
            value = data::Value("");
        } else {
            value = data::currentSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data);
        }
        refresh = refresh || (wasBlinkTime != isBlinkTime);
        if (!refresh) {
            data::Value previousValue = data::previousSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data);
            refresh = value != previousValue;
        }
        if (refresh) {
            char text[32];
            value.toText(text, 32);

            Style *style;
            if (edit)
                style = get_active_style(widgetCursor.widget);
            else
                style = (Style *)(document + widgetCursor.widget->style);

            drawText(text, widgetCursor.x, widgetCursor.y, (int)widgetCursor.widget->w, (int)widgetCursor.widget->h, style, inverse);
            return true;
        }
        return false;
    }
}

bool draw_display_string_widget(uint8_t *document, const WidgetCursor &widgetCursor, bool refresh, bool inverse) {
    if (refresh) {
        DisplayStringWidget *display_string_widget = ((DisplayStringWidget *)(document + widgetCursor.widget->specific));
        char *text = (char *)(document + display_string_widget->text);
        drawText(text, widgetCursor.x, widgetCursor.y, (int)widgetCursor.widget->w, (int)widgetCursor.widget->h, (Style *)(document + widgetCursor.widget->style), inverse);
        return true;
    }
    return false;
}

bool draw_display_multiline_string_widget(uint8_t *document, const WidgetCursor &widgetCursor, bool refresh, bool inverse) {
    if (refresh) {
        DisplayStringWidget *display_string_widget = ((DisplayStringWidget *)(document + widgetCursor.widget->specific));
        char *text = (char *)(document + display_string_widget->text);
        drawMultilineText(text, widgetCursor.x, widgetCursor.y, (int)widgetCursor.widget->w, (int)widgetCursor.widget->h, (Style *)(document + widgetCursor.widget->style), inverse);
        return true;
    }
    return false;
}

bool draw_three_state_indicator_widget(uint8_t *document, const WidgetCursor &widgetCursor, bool refresh, bool inverse) {
    int state = data::currentSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data).getInt();
    if (!refresh) {
        int previousState = data::previousSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data).getInt();
        refresh = state != previousState;
    }
    if (refresh) {
        ThreeStateIndicatorWidget *three_state_indicator_widget = ((ThreeStateIndicatorWidget *)(document + widgetCursor.widget->specific));

        OBJ_OFFSET style;
        if (state == 1) style = widgetCursor.widget->style;
        else if (state == 2) style = three_state_indicator_widget->style1;
        else style = three_state_indicator_widget->style2;

        char *text = (char *)(document + three_state_indicator_widget->text);
        drawText(text, widgetCursor.x, widgetCursor.y, (int)widgetCursor.widget->w, (int)widgetCursor.widget->h, (Style *)(document + style), inverse);
        return true;
    }
    return false;
}

bool draw_display_string_select_widget(uint8_t *document, const WidgetCursor &widgetCursor, bool refresh, bool inverse) {
    int state = data::currentSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data).getInt();
    if (!refresh) {
        int previousState = data::previousSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data).getInt();
        refresh = state != previousState;
    }
    if (refresh) {
        DisplayStringSelectWidget *display_string_select_widget = ((DisplayStringSelectWidget *)(document + widgetCursor.widget->specific));
        char *text;
        Style *style;
        if (state == 1) {
            text = (char *)(document + display_string_select_widget->text1);
            style = (Style *)(document + display_string_select_widget->style1);
        } else {
            text = (char *)(document + display_string_select_widget->text2);
            style = (Style *)(document + display_string_select_widget->style2);
        }
        drawText(text, widgetCursor.x, widgetCursor.y, (int)widgetCursor.widget->w, (int)widgetCursor.widget->h, style, inverse);
        return true;
    }
    return false;
}

void draw_scale(Widget *widget, int y_from, int y_to, int y_min, int y_max, int y_value, int f, int d) {
    Style *style = (Style *)(document + widget->style);
    ScaleWidget *scale_widget = ((ScaleWidget *)(document + widget->specific));

    int x1 = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->x;
    int l1 = 5 * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->w / 12 - 1;

    int x2 = x1 + l1 + 2;
    int l2 = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->w - l1 - 3;

    int s = 10 * f / d;

    int y_offset = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * (widget->y + widget->h) - 1 -
        (DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widget->h - (y_max - y_min)) / 2 - scale_widget->needle_height / 2;

    for (int y_i = y_from; y_i <= y_to; ++y_i) {
        int y = y_offset - y_i;

        // draw ticks
        if (y_i >= y_min && y_i <= y_max) {
            if (y_i % s == 0) {
                lcd::lcd.setColor(style->border_color);
                lcd::lcd.drawHLine(x1, y, l1);
            }
            else if (y_i % (s / 2) == 0) {
                lcd::lcd.setColor(style->border_color);
                lcd::lcd.drawHLine(x1, y, l1 / 2);
            }
            else if (y_i % (s / 10) == 0) {
                lcd::lcd.setColor(style->border_color);
                lcd::lcd.drawHLine(x1, y, l1 / 4);
            }
            else {
                lcd::lcd.setColor(style->background_color);
                lcd::lcd.drawHLine(x1, y, l1);
            }
        }

        int d = abs(y_i - y_value);
        if (d <= scale_widget->needle_height / 2) {
            // draw thumb
            lcd::lcd.setColor(style->color);
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

bool draw_scale_widget(uint8_t *document, const WidgetCursor &widgetCursor, bool refresh, bool inverse) {
    data::Value value = data::currentSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data);
    if (!refresh) {
        data::Value previousValue = data::previousSnapshot.editModeSnapshot.editValue;
        refresh = previousValue != value;
    }

    if (refresh) {
        float min = data::getMin(widgetCursor.cursor, widgetCursor.widget->data).getFloat();
        float max = data::getMax(widgetCursor.cursor, widgetCursor.widget->data).getFloat();

        Style *style = (Style *)(document + widgetCursor.widget->style);
        font::Font *font = styleGetFont(style);
        int fontHeight = font->getAscent() / DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;

        ScaleWidget *scale_widget = ((ScaleWidget *)(document + widgetCursor.widget->specific));

        int f = (int)floor(DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * (widgetCursor.widget->h - scale_widget->needle_height) / max);
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
        int y_value = (int)round(value.getFloat() * f);

        int y_from_min = y_min - scale_widget->needle_height / 2;
        int y_from_max = y_max + scale_widget->needle_height / 2;

        static int edit_mode_slider_scale_last_y_value;

        if (is_page_refresh || widgetCursor.widget->data == DATA_ID_EDIT_VALUE) {
            // draw entire scale 
            draw_scale(widgetCursor.widget, y_from_min, y_from_max, y_min, y_max, y_value, f, d);
        }
        else {
            // optimization for the scale in edit with slider mode:
            // draw only part of the scale that is changed
            if (widgetCursor.widget->data == DATA_ID_EDIT_VALUE) {
                int last_y_value_from = edit_mode_slider_scale_last_y_value - scale_widget->needle_height / 2;
                int last_y_value_to = edit_mode_slider_scale_last_y_value + scale_widget->needle_height / 2;
                int y_value_from = y_value - scale_widget->needle_height / 2;
                int y_value_to = y_value + scale_widget->needle_height / 2;

                if (last_y_value_from != y_value_from) {
                    if (last_y_value_from > y_value_from) {
                        util_swap(int, last_y_value_from, y_value_from);
                        util_swap(int, last_y_value_to, y_value_to);
                    }

                    if (last_y_value_from < y_from_min)
                        last_y_value_from = y_from_min;

                    if (y_value_to > y_from_max)
                        y_value_to = y_from_max;

                    if (last_y_value_to + 1 < y_value_from) {
                        draw_scale(widgetCursor.widget, last_y_value_from, last_y_value_to, y_min, y_max, y_value, f, d);
                        draw_scale(widgetCursor.widget, y_value_from, y_value_to, y_min, y_max, y_value, f, d);
                    }
                    else {
                        draw_scale(widgetCursor.widget, last_y_value_from, y_value_to, y_min, y_max, y_value, f, d);
                    }
                }
            }
        }

        edit_mode_slider_scale_last_y_value = y_value;

        if (widgetCursor.widget->data == DATA_ID_EDIT_VALUE) {
            edit_mode_slider::scale_width = DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER * widgetCursor.widget->w;
            edit_mode_slider::scale_height = (max - min) * f;
        }

        return true;
    }

    return false;
}

bool draw_toggle_button_widget(uint8_t *document, const WidgetCursor &widgetCursor, bool refresh, bool inverse) {
    int state = data::currentSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data).getInt();
    if (!refresh) {
        int previousState = data::previousSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data).getInt();
        refresh = state != previousState;
    }
    if (refresh) {
        ToggleButtonWidget *toggle_button_widget = ((ToggleButtonWidget *)(document + widgetCursor.widget->specific));
        char *text = (char *)(document + (state == 0 ? toggle_button_widget->text1 : toggle_button_widget->text2));
        drawText(text, widgetCursor.x, widgetCursor.y, (int)widgetCursor.widget->w, (int)widgetCursor.widget->h, (Style *)(document + widgetCursor.widget->style), inverse);
        return true;
    }
    return false;
}

bool draw_widget(uint8_t *document, const WidgetCursor &widgetCursor, bool refresh) {
    bool inverse = selected_widget == widgetCursor;

    if (widgetCursor.widget->type == WIDGET_TYPE_DISPLAY) {
        return draw_display_widget(document, widgetCursor, refresh, inverse);
    } else if (widgetCursor.widget->type == WIDGET_TYPE_DISPLAY_STRING) {
        return draw_display_string_widget(document, widgetCursor, refresh, inverse);
    } else if (widgetCursor.widget->type == WIDGET_TYPE_THREE_STATE_INDICATOR) {
        return draw_three_state_indicator_widget(document, widgetCursor, refresh, inverse);
    } else if (widgetCursor.widget->type == WIDGET_TYPE_DISPLAY_STRING_SELECT) {
        return draw_display_string_select_widget(document, widgetCursor, refresh, inverse);
    } else if (widgetCursor.widget->type == WIDGET_TYPE_DISPLAY_MULTILINE_STRING) {
        return draw_display_multiline_string_widget(document, widgetCursor, refresh, inverse);
    } else if (widgetCursor.widget->type == WIDGET_TYPE_SCALE) {
        return draw_scale_widget(document, widgetCursor, refresh, inverse);
    } else if (widgetCursor.widget->type == WIDGET_TYPE_TOGGLE_BUTTON) {
        return draw_toggle_button_widget(document, widgetCursor, refresh, inverse);
    } else if (widgetCursor.widget->type == WIDGET_TYPE_BUTTON_GROUP) {
        return widget_button_group::draw(document, widgetCursor, refresh, inverse);
    }

    return false;
}

static EnumWidgets draw_enum_widgets(document, draw_widget);

void draw_tick(unsigned long tick_usec) {
    if (!draw_enum_widgets.next()) {
        wasBlinkTime = isBlinkTime;
        isBlinkTime = (micros() % (2 * CONF_BLINK_TIME)) > CONF_BLINK_TIME;

        data::previousSnapshot = data::currentSnapshot;
        data::currentSnapshot.takeSnapshot();

        draw_enum_widgets.start(active_page_id, 0, 0, false);

        //DebugTraceF("%d", draw_counter);
        //draw_counter = 0;

        is_page_refresh = false;
    }
}

void refresh_widget(WidgetCursor widget_cursor) {
    widget_refresh = true;
    draw_widget(document, widget_cursor, true);
    widget_refresh = false;
}

void refresh_page() {
    is_page_refresh = true;

    // clear screen with background color
    Widget *page = (Widget *)(document + ((Document *)document)->pages.first) + active_page_id;
    Style *style = (Style *)(document + page->style);
    page_style = style;
    lcd::lcd.setColor(style->background_color);
    lcd::lcd.fillRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);

    data::currentSnapshot.takeSnapshot();
    draw_enum_widgets.start(active_page_id, 0, 0, true);
}

int getActivePage() {
    return active_page_id;
}

void showPage(int index) {
    active_page_id = index;
    refresh_page();
}

////////////////////////////////////////////////////////////////////////////////

static int find_widget_at_x;
static int find_widget_at_y;
static WidgetCursor found_widget;

bool find_widget_step(uint8_t *start, const WidgetCursor &widgetCursor, bool refresh) {
    bool inside = 
        find_widget_at_x >= widgetCursor.x * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER &&
        find_widget_at_x < (widgetCursor.x + widgetCursor.widget->w) * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER &&
        find_widget_at_y >= widgetCursor.y * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER &&
        find_widget_at_y < (widgetCursor.y + widgetCursor.widget->h) * DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER;

    if (inside) {
        found_widget = widgetCursor;
        return true;
    }

    return false;
}

void find_widget(int x, int y) {
    found_widget = 0;

    find_widget_at_x = touch::x;
    find_widget_at_y = touch::y;
    EnumWidgets enum_widgets(document, find_widget_step);
    enum_widgets.start(active_page_id, 0, 0, true);
    enum_widgets.next();
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

void do_action(int action_id, WidgetCursor &widget_cursor) {
    if (edit_mode::doAction(action_id, widget_cursor)) {
        return;
    }

    if (action_id == ACTION_ID_TOUCH_SCREEN_CALIBRATION) {
        touch::calibration::enter_calibration_mode();
    } else if (action_id == ACTION_ID_YES) {
        dialog_yes_callback();
    } else if (action_id == ACTION_ID_NO) {
        dialog_no_callback();
    } else if (action_id == ACTION_ID_CANCEL) {
        dialog_cancel_callback();
    } else if (action_id == ACTION_ID_TOGGLE) {
        data::toggle(widget_cursor.widget->data);
    } else  {
        data::doAction(widget_cursor.cursor, action_id);
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
#ifdef EEZ_PSU_SIMULATOR
    if (!simulator::front_panel::isOpened()) {
        return;
    }
#endif

    touch::tick(tick_usec);

    if (touch::calibration::is_calibrated()) {
        if (touch::event_type == touch::TOUCH_DOWN) {
            find_widget(touch::x, touch::y);
            if (found_widget && found_widget.widget->action) {
                found_widget_at_down = found_widget;
            } else {
                found_widget_at_down = 0;
            }
            if (found_widget_at_down) {
                select_widget(found_widget_at_down);
            } else {
                if (found_widget && found_widget.widget->type == WIDGET_TYPE_BUTTON_GROUP) {
                    widget_button_group::onTouchDown(found_widget);
                } else if (active_page_id == PAGE_ID_EDIT_MODE_SLIDER) {
                    edit_mode_slider::onTouchDown();
                } else if (active_page_id == PAGE_ID_EDIT_MODE_STEP) {
                    edit_mode_step::onTouchDown();
                }
            }
        } else if (touch::event_type == touch::TOUCH_MOVE) {
            if (!found_widget_at_down) {
                if (active_page_id == PAGE_ID_EDIT_MODE_SLIDER) {
                    edit_mode_slider::onTouchMove();
                } else if (active_page_id == PAGE_ID_EDIT_MODE_STEP) {
                    edit_mode_step::onTouchMove();
                } else if (active_page_id == PAGE_ID_YES_NO) {
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
                if (active_page_id == PAGE_ID_EDIT_MODE_SLIDER) {
                    edit_mode_slider::onTouchUp();
                } else if (active_page_id == PAGE_ID_EDIT_MODE_STEP) {
                    edit_mode_step::onTouchUp();
                }
            }
        }

        draw_tick(tick_usec);
    } else {
        touch::calibration::tick(tick_usec);
    }
}

void yesNoDialog(const char *message PROGMEM, void (*yes_callback)(), void (*no_callback)(), void (*cancel_callback)()) {
    data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE, data::Value::ConstStr(message));

    dialog_yes_callback = yes_callback;
    dialog_no_callback = no_callback;
    dialog_cancel_callback = cancel_callback;

    active_page_id = PAGE_ID_YES_NO;
    refresh_page();
}

}
}
} // namespace eez::psu::gui
