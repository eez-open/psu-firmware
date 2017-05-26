/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
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

#if OPTION_DISPLAY

#include "gui_internal.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_slider.h"
#include "gui_widget_button_group.h"
#include "gui_page.h"

#ifdef EEZ_PSU_SIMULATOR
#include "front_panel/control.h"
#endif

#define CONF_GUI_ENUM_WIDGETS_STACK_SIZE 5
#define CONF_GUI_BLINK_TIME 400000UL // 400ms
#define CONF_GUI_YT_GRAPH_BLANK_PIXELS_AFTER_CURSOR 10

#define CONF_MAX_STATE_SIZE 2048

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

static bool g_widgetRefresh;
static WidgetCursor g_selectedWidget;
static bool g_isBlinkTime;
static bool g_wasBlinkTime;

static uint8_t g_stateBuffer[2][CONF_MAX_STATE_SIZE];
WidgetState *g_previousState;
WidgetState *g_currentState;

////////////////////////////////////////////////////////////////////////////////

int getCurrentStateBufferIndex() {
    return (uint8_t *)g_currentState == &g_stateBuffer[0][0] ? 0 : 1;
}

////////////////////////////////////////////////////////////////////////////////

bool styleHasBorder(const Style *style) {
    return style->flags & STYLE_FLAGS_BORDER;
}

bool styleIsHorzAlignLeft(const Style *style) {
    return (style->flags & STYLE_FLAGS_HORZ_ALIGN) == STYLE_FLAGS_HORZ_ALIGN_LEFT;
}

bool styleIsHorzAlignRight(const Style *style) {
    return (style->flags & STYLE_FLAGS_HORZ_ALIGN) == STYLE_FLAGS_HORZ_ALIGN_RIGHT;
}

bool styleIsVertAlignTop(const Style *style) {
    return (style->flags & STYLE_FLAGS_VERT_ALIGN) == STYLE_FLAGS_VERT_ALIGN_TOP;
}

bool styleIsVertAlignBottom(const Style *style) {
    return (style->flags & STYLE_FLAGS_VERT_ALIGN) == STYLE_FLAGS_VERT_ALIGN_BOTTOM;
}

font::Font styleGetFont(const Style *style) {
    return font::Font(style->font > 0 ? fonts[style->font - 1] : 0);
}

////////////////////////////////////////////////////////////////////////////////

void drawText(const char *text, int textLength, int x, int y, int w, int h, const Style *style, bool inverse, bool blink) {
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

    font::Font font = styleGetFont(style);
    
    int width = lcd::lcd.measureStr(text, textLength, font, x2 - x1 + 1);
    int height = font.getHeight();

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

    if (inverse || blink) {
        lcd::lcd.setColor(style->color);
    } else {
        lcd::lcd.setColor(style->background_color);
    }
    if (g_widgetRefresh) {
        lcd::lcd.fillRect(x1, y1, x2, y2);
    } else {
        if (x1 <= x_offset - 1 && y1 <= y2)
            lcd::lcd.fillRect(x1, y1, x_offset - 1, y2);
        if (x_offset + width <= x2 && y1 <= y2)
            lcd::lcd.fillRect(x_offset + width, y1, x2, y2);

        int right = MIN(x_offset + width - 1, x2);

        if (x_offset <= right && y1 <= y_offset - 1)
            lcd::lcd.fillRect(x_offset, y1, right, y_offset - 1);
        if (x_offset <= right && y_offset + height <= y2)
            lcd::lcd.fillRect(x_offset, y_offset + height, right, y2);
    }

    if (inverse || blink) {
        lcd::lcd.setBackColor(style->color);
        lcd::lcd.setColor(style->background_color);
    } else {
        lcd::lcd.setBackColor(style->background_color);
        lcd::lcd.setColor(style->color);
    }
    lcd::lcd.drawStr(text, textLength, x_offset, y_offset, x1, y1, x2, y2, font, !g_widgetRefresh);
}

void drawMultilineText(const char *text, int x, int y, int w, int h, const Style *style, bool inverse) {
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

    font::Font font = styleGetFont(style);
    int height = (int)(0.9 * font.getHeight());
    
    font::Glyph space_glyph;
    font.getGlyph(' ', space_glyph);
    int space_width = space_glyph.dx;

    bool clear_background = false;

    uint16_t background_color = inverse ? style->color : style->background_color;
    lcd::lcd.setColor(background_color);
    lcd::lcd.fillRect(x1, y1, x2, y2);

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

        int width = lcd::lcd.measureStr(text + j, i - j, font);

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

        lcd::lcd.drawStr(text + j, i - j, x, y, x1, y1, x2, y2, font, false);

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
    lcd::lcd.fillRect(x, y, x + w - 1, y + h - 1);
}

void drawBitmap(uint8_t bitmapIndex, int x, int y, int w, int h, const Style *style, bool inverse) {
    if (bitmapIndex == 0) {
        return;
    }
    Bitmap &bitmap = bitmaps[bitmapIndex - 1];

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

    int width = bitmap.w;
    int height = bitmap.h;

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

    if (g_widgetRefresh) {
        lcd::lcd.fillRect(x1, y1, x2, y2);
    } else {
        if (x1 <= x_offset - 1 && y1 <= y2)
            lcd::lcd.fillRect(x1, y1, x_offset - 1, y2);
        if (x_offset + width <= x2 && y1 <= y2)
            lcd::lcd.fillRect(x_offset + width, y1, x2, y2);

        int right = MIN(x_offset + width - 1, x2);

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
    lcd::lcd.drawBitmap(x_offset, y_offset, width, height, (uint16_t*)bitmap.pixels);
}

void drawRectangle(int x, int y, int w, int h, const Style *style, bool inverse) {
    if (w > 0 && h > 0) {
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

        uint16_t color = inverse ? style->background_color : style->color;
        lcd::lcd.setColor(color);
        lcd::lcd.fillRect(x1, y1, x2, y2);
    }
}

////////////////////////////////////////////////////////////////////////////////

void drawDisplayDataWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    DECL_WIDGET_SPECIFIC(DisplayDataWidget, display_data_widget, widget);

    widgetCursor.currentState->size = sizeof(WidgetState);
    widgetCursor.currentState->flags.focused = isFocusWidget(widgetCursor);
    widgetCursor.currentState->flags.blinking = data::isBlinking(widgetCursor.cursor, widget->data) && g_isBlinkTime;
    widgetCursor.currentState->data = data::get(widgetCursor.cursor, 
        widgetCursor.currentState->flags.focused && getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD ? DATA_ID_KEYPAD_TEXT : widget->data);

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.focused != widgetCursor.currentState->flags.focused ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
        widgetCursor.previousState->flags.blinking != widgetCursor.currentState->flags.blinking ||
        widgetCursor.previousState->data != widgetCursor.currentState->data;

    if (refresh) {
        char text[64];
        widgetCursor.currentState->data.toText(text, sizeof(text));

        DECL_STYLE(style, widgetCursor.currentState->flags.focused ? display_data_widget->activeStyle : widget->style);

        drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
            widgetCursor.currentState->flags.pressed,
            widgetCursor.currentState->flags.blinking);
    }
}

void drawTextWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    widgetCursor.currentState->size = sizeof(WidgetState);
    widgetCursor.currentState->data = widget->data ? data::get(widgetCursor.cursor, widget->data) : 0;

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
        widgetCursor.previousState->data != widgetCursor.currentState->data;

    if (refresh) {
        DECL_WIDGET_STYLE(style, widget);

        if (widget->data) {
            if (widgetCursor.currentState->data.isString()) {
                drawText(widgetCursor.currentState->data.asString(), -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
                    widgetCursor.currentState->flags.pressed);
            } else {
                char text[64];
                widgetCursor.currentState->data.toText(text, sizeof(text));
                drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
                    widgetCursor.currentState->flags.pressed);
            }
        } else {
            DECL_WIDGET_SPECIFIC(TextWidget, display_string_widget, widget);
            DECL_STRING(text, display_string_widget->text);
            drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
                widgetCursor.currentState->flags.pressed);
        }
    }
}

void drawMultilineTextWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    widgetCursor.currentState->size = sizeof(WidgetState);
    widgetCursor.currentState->data = widget->data ? data::get(widgetCursor.cursor, widget->data) : 0;

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
        widgetCursor.previousState->data != widgetCursor.currentState->data;

    if (refresh) {
        DECL_WIDGET_STYLE(style, widget);

        if (widget->data) {
            if (widgetCursor.currentState->data.isString()) {
                drawMultilineText(widgetCursor.currentState->data.asString(), widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
                    widgetCursor.currentState->flags.pressed);
            } else {
                char text[64];
                widgetCursor.currentState->data.toText(text, sizeof(text));
                drawMultilineText(text, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
                    widgetCursor.currentState->flags.pressed);
            }
        } else {
            DECL_WIDGET_SPECIFIC(MultilineTextWidget, display_string_widget, widget);
            DECL_STRING(text, display_string_widget->text);
            drawMultilineText(text, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
                widgetCursor.currentState->flags.pressed);
        }
    }
}

void drawScale(const Widget *widget, const ScaleWidget *scale_widget, const Style* style, int y_from, int y_to, int y_min, int y_max, int y_value, int f, int d, bool drawTicks) {
    bool vertical = scale_widget->needle_position == SCALE_NEEDLE_POSITION_LEFT ||
        scale_widget->needle_position == SCALE_NEEDLE_POSITION_RIGHT;
    bool flip = scale_widget->needle_position == SCALE_NEEDLE_POSITION_LEFT ||
        scale_widget->needle_position == SCALE_NEEDLE_POSITION_TOP;

    int needleSize;

    int x1, l1, x2, l2;

    if (vertical) {
        needleSize = scale_widget->needle_height;

        if (flip) {
            x1 = widget->x + scale_widget->needle_width + 2;
            l1 = widget->w - (scale_widget->needle_width + 2);
            x2 = widget->x;
            l2 = scale_widget->needle_width;
        } else {
            x1 = widget->x;
            l1 = widget->w - (scale_widget->needle_width + 2);
            x2 = widget->x + widget->w - scale_widget->needle_width;
            l2 = scale_widget->needle_width;
        }
    } else {
        needleSize = scale_widget->needle_width;

        if (flip) {
            x1 = widget->y + scale_widget->needle_height + 2;
            l1 = widget->h - (scale_widget->needle_height + 2);
            x2 = widget->y;
            l2 = scale_widget->needle_height;
        } else {
            x1 = widget->y;
            l1 = widget->h - scale_widget->needle_height - 2;
            x2 = widget->y + widget->h - scale_widget->needle_height;
            l2 = scale_widget->needle_height;
        }
    }

    int s = 10 * f / d;

    int y_offset;
    if (vertical) {
        y_offset = int((widget->y + widget->h) - 1 - (widget->h - (y_max - y_min)) / 2);
    } else {
        y_offset = int(widget->x + (widget->w - (y_max - y_min)) / 2);
    }

    for (int y_i = y_from; y_i <= y_to; ++y_i) {
        int y;

        if (vertical) {
            y = y_offset - y_i;
        } else {
            y = y_offset + y_i;
        }

        if (drawTicks) {
            // draw ticks
            if (y_i >= y_min && y_i <= y_max) {
                if (y_i % s == 0) {
                    lcd::lcd.setColor(style->border_color);
                    if (vertical) {
                        lcd::lcd.drawHLine(x1, y, l1 - 1);
                    } else {
                        lcd::lcd.drawVLine(y, x1, l1 - 1);
                    }
                }
                else if (y_i % (s / 2) == 0) {
                    lcd::lcd.setColor(style->border_color);
                    if (vertical) {
                        if (flip) {
                            lcd::lcd.drawHLine(x1 + l1 / 2, y, l1 / 2);
                        } else {
                            lcd::lcd.drawHLine(x1, y, l1 / 2);
                        }
                    } else {
                        if (flip) {
                            lcd::lcd.drawVLine(y, x1 + l1 / 2, l1 / 2);
                        } else {
                            lcd::lcd.drawVLine(y, x1, l1 / 2);
                        }
                    }
                }
                else if (y_i % (s / 10) == 0) {
                    lcd::lcd.setColor(style->border_color);
                    if (vertical) {
                        if (flip) {
                            lcd::lcd.drawHLine(x1 + l1 - l1 / 4, y, l1 / 4);
                        } else {
                            lcd::lcd.drawHLine(x1, y, l1 / 4);
                        }
                    } else {
                        if (flip) {
                            lcd::lcd.drawVLine(y, x1 + l1 - l1 / 4, l1 / 4);
                        } else {
                            lcd::lcd.drawVLine(y, x1, l1 / 4);
                        }
                    }
                }
                else {
                    lcd::lcd.setColor(style->background_color);
                    if (vertical) {
                        lcd::lcd.drawHLine(x1, y, l1 - 1);
                    } else {
                        lcd::lcd.drawVLine(y, x1, l1 - 1);
                    }
                }
            }
        }

        int d = abs(y_i - y_value);
        if (d <= int(needleSize / 2)) {
            // draw thumb
            lcd::lcd.setColor(style->color);
            if (vertical) {
                if (flip) {
                    lcd::lcd.drawHLine(x2, y, l2 - d - 1);
                } else {
                    lcd::lcd.drawHLine(x2 + d, y, l2 - d - 1);
                }
            } else {
                if (flip) {
                    lcd::lcd.drawVLine(y, x2, l2 - d - 1);
                } else {
                    lcd::lcd.drawVLine(y, x2 + d, l2 - d - 1);
                }
            }

            if (y_i != y_value) {
                lcd::lcd.setColor(style->background_color);
                if (vertical) {
                    if (flip) {
                        lcd::lcd.drawHLine(x2 + l2 - d, y, d - 1);
                    } else {
                        lcd::lcd.drawHLine(x2, y, d - 1);
                    }
                } else {
                    if (flip) {
                        lcd::lcd.drawVLine(y, x2 + l2 - d, d - 1);
                    } else {
                        lcd::lcd.drawVLine(y, x2, d - 1);
                    }
                }
            }
        }
        else {
            // erase
            lcd::lcd.setColor(style->background_color);
            if (vertical) {
                lcd::lcd.drawHLine(x2, y, l2 - 1);
            } else {
                lcd::lcd.drawVLine(y, x2, l2 - 1);
            }
        }
    }
}

void drawScaleWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    widgetCursor.currentState->size = sizeof(WidgetState);
    widgetCursor.currentState->data = data::get(widgetCursor.cursor, widget->data);

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->data != widgetCursor.currentState->data;

    if (refresh) {
        float min = data::getMin(widgetCursor.cursor, widget->data).getFloat();
        float max = data::getMax(widgetCursor.cursor, widget->data).getFloat();

        DECL_WIDGET_STYLE(style, widget);
        font::Font font = styleGetFont(style);
        int fontHeight = font.getAscent();

        DECL_WIDGET_SPECIFIC(ScaleWidget, scale_widget, widget);

        bool vertical = scale_widget->needle_position == SCALE_NEEDLE_POSITION_LEFT ||
            scale_widget->needle_position == SCALE_NEEDLE_POSITION_RIGHT;

        int f;
        int needleSize;
        if (vertical) {
            needleSize = scale_widget->needle_height;
            f = (int)floor((widget->h - needleSize) / max);
        } else {
            needleSize = scale_widget->needle_width;
            f = (int)floor((widget->w - needleSize) / max);
        }

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
        int y_value = (int)round(widgetCursor.currentState->data.getFloat() * f);

        int y_from_min = y_min - needleSize / 2;
        int y_from_max = y_max + needleSize / 2;

        static int edit_mode_slider_scale_last_y_value;

        if (widget->data != DATA_ID_EDIT_VALUE) {
            // draw entire scale 
            drawScale(widget, scale_widget, style, y_from_min, y_from_max, y_min, y_max, y_value, f, d, true);
        }
        else {
            // optimization for the scale in edit with slider mode:
            // draw only part of the scale that is changed
            if (widget->data == DATA_ID_EDIT_VALUE && edit_mode_slider_scale_last_y_value != y_value) {
                int last_y_value_from = edit_mode_slider_scale_last_y_value - needleSize / 2;
                int last_y_value_to = edit_mode_slider_scale_last_y_value + needleSize / 2;
                int y_value_from = y_value - needleSize / 2;
                int y_value_to = y_value + needleSize / 2;

                if (last_y_value_from > y_value_from) {
                    util_swap(int, last_y_value_from, y_value_from);
                    util_swap(int, last_y_value_to, y_value_to);
                }

                if (last_y_value_from < y_from_min) {
                    last_y_value_from = y_from_min;
                }

                if (y_value_to > y_from_max) {
                    y_value_to = y_from_max;
                }

                if (0 && last_y_value_to + 1 < y_value_from) {
                    drawScale(widget, scale_widget, style, last_y_value_from, last_y_value_to, y_min, y_max, y_value, f, d, false);
                    drawScale(widget, scale_widget, style, y_value_from, y_value_to, y_min, y_max, y_value, f, d, false);
                }
                else {
                    drawScale(widget, scale_widget, style, last_y_value_from, y_value_to, y_min, y_max, y_value, f, d, false);
                }
            }
        }

        edit_mode_slider_scale_last_y_value = y_value;

        if (widget->data == DATA_ID_EDIT_VALUE) {
            edit_mode_slider::scale_is_vertical = vertical;
            edit_mode_slider::scale_width = vertical ? widget->w : widget->h;
            edit_mode_slider::scale_height = (max - min) * f; 
        }
    }
}

void drawButtonWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    DECL_WIDGET_SPECIFIC(ButtonWidget, button_widget, widget);

    widgetCursor.currentState->size = sizeof(WidgetState);
    widgetCursor.currentState->flags.enabled = data::get(widgetCursor.cursor, button_widget->enabled).getInt();
    widgetCursor.currentState->flags.blinking = data::isBlinking(widgetCursor.cursor, widget->data) && g_isBlinkTime;
    widgetCursor.currentState->data = widget->data ? data::get(widgetCursor.cursor, widget->data) : 0;

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
        widgetCursor.previousState->flags.enabled != widgetCursor.currentState->flags.enabled ||
        widgetCursor.previousState->flags.blinking != widgetCursor.currentState->flags.blinking ||
        widgetCursor.previousState->data != widgetCursor.currentState->data;

    if (refresh) {
        if (widget->data) {
            DECL_STYLE(style, widgetCursor.currentState->flags.enabled ? widget->style : button_widget->disabledStyle);

            if (widgetCursor.currentState->data.isString()) {
                drawText(widgetCursor.currentState->data.asString(), -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
                    widgetCursor.currentState->flags.pressed,
                    widgetCursor.currentState->flags.blinking);
            } else if (widgetCursor.currentState->data.isConstString()) {
                char text[64];
                widgetCursor.currentState->data.toText(text, sizeof(text));
                drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
                    widgetCursor.currentState->flags.pressed,
                    widgetCursor.currentState->flags.blinking);
            } else {
                DECL_STRING(text, button_widget->text);
                drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
                    widgetCursor.currentState->flags.pressed,
                    widgetCursor.currentState->flags.blinking);
            }
        } else {
            DECL_STRING(text, button_widget->text);
            DECL_STYLE(style, widgetCursor.currentState->flags.enabled ? widget->style : button_widget->disabledStyle);
            drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
                widgetCursor.currentState->flags.pressed,
                widgetCursor.currentState->flags.blinking);
        }
    }
}

void drawToggleButtonWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    widgetCursor.currentState->size = sizeof(WidgetState);
    widgetCursor.currentState->flags.enabled = data::get(widgetCursor.cursor, widget->data).getInt();

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
        widgetCursor.previousState->flags.enabled != widgetCursor.currentState->flags.enabled;

    if (refresh) {
        DECL_WIDGET_SPECIFIC(ToggleButtonWidget, toggle_button_widget, widget);
        DECL_STRING(text, widgetCursor.currentState->flags.enabled ? toggle_button_widget->text2 : toggle_button_widget->text1);
        DECL_WIDGET_STYLE(style, widget);
        drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
            widgetCursor.currentState->flags.pressed);
    }
}

void drawRectangleWidget(const WidgetCursor &widgetCursor) {
    widgetCursor.currentState->size = sizeof(WidgetState);

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed;

    if (refresh) {
        DECL_WIDGET(widget, widgetCursor.widgetOffset);
        DECL_WIDGET_STYLE(style, widget);
        drawRectangle(widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
            widgetCursor.currentState->flags.pressed);
    }
}

void drawBitmapWidget(const WidgetCursor &widgetCursor) {
    widgetCursor.currentState->size = sizeof(WidgetState);

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed;

    if (refresh) {
        DECL_WIDGET(widget, widgetCursor.widgetOffset);
        DECL_WIDGET_SPECIFIC(BitmapWidget, display_bitmap_widget, widget);
        DECL_WIDGET_STYLE(style, widget);
        drawBitmap(display_bitmap_widget->bitmap, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
            widgetCursor.currentState->flags.pressed);
    }
}

int calcValuePosInBarGraphWidget(data::Value &value, float min, float max, int d) {
    int p = (int)roundf((value.getFloat() - min) * d / (max - min));

    if (p < 0) {
        p = 0;
    } else if (p >= d) {
        p = d - 1;
    }

    return p;
}

void drawLineInBarGraphWidget(const BarGraphWidget *barGraphWidget, int p, OBJ_OFFSET lineStyle, int x, int y, int w, int h) {
    DECL_STYLE(style, lineStyle);

    lcd::lcd.setColor(style->color);
    if (barGraphWidget->orientation == BAR_GRAPH_ORIENTATION_LEFT_RIGHT) {
        lcd::lcd.drawVLine(x + p, y, h - 1);
    } else if (barGraphWidget->orientation == BAR_GRAPH_ORIENTATION_RIGHT_LEFT) {
        lcd::lcd.drawVLine(x - p, y, h - 1);
    } else if (barGraphWidget->orientation == BAR_GRAPH_ORIENTATION_TOP_BOTTOM) {
        lcd::lcd.drawHLine(x, y + p, w - 1);
    } else {
        lcd::lcd.drawHLine(x, y - p, w - 1);
    }
}

void drawBarGraphWidget(const WidgetCursor &widgetCursor) {
    bool fullScale = true;

    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    DECL_WIDGET_SPECIFIC(BarGraphWidget, barGraphWidget, widget);

    widgetCursor.currentState->size = sizeof(BarGraphWidgetState);
    widgetCursor.currentState->data = data::get(widgetCursor.cursor, widget->data);
    ((BarGraphWidgetState *)widgetCursor.currentState)->line1Data = data::get(widgetCursor.cursor, barGraphWidget->line1Data);
    ((BarGraphWidgetState *)widgetCursor.currentState)->line2Data = data::get(widgetCursor.cursor, barGraphWidget->line2Data);

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
        widgetCursor.previousState->data != widgetCursor.currentState->data ||
        ((BarGraphWidgetState *)widgetCursor.previousState)->line1Data != ((BarGraphWidgetState *)widgetCursor.currentState)->line1Data ||
        ((BarGraphWidgetState *)widgetCursor.previousState)->line2Data != ((BarGraphWidgetState *)widgetCursor.currentState)->line2Data;

    if (refresh) {
        int x = widgetCursor.x;
        int y = widgetCursor.y;
        const int w = widget->w;
        const int h = widget->h;

        float min = data::getMin(widgetCursor.cursor, widget->data).getFloat();
        float max = fullScale ? 
            ((BarGraphWidgetState *)widgetCursor.currentState)->line2Data.getFloat() :
            data::getMax(widgetCursor.cursor, widget->data).getFloat();

        bool horizontal = barGraphWidget->orientation == BAR_GRAPH_ORIENTATION_LEFT_RIGHT || barGraphWidget->orientation == BAR_GRAPH_ORIENTATION_RIGHT_LEFT;

        int d = horizontal ? w : h;

        // calc bar  position (monitored value)
        int pValue = calcValuePosInBarGraphWidget(widgetCursor.currentState->data, min, max, d);

        // calc line 1 position (set value) 
        int pLine1 = calcValuePosInBarGraphWidget(((BarGraphWidgetState *)widgetCursor.currentState)->line1Data, min, max, d);

        int pLine2;
        if (!fullScale) {
            // calc line 2 position (limit value) 
            pLine2 = calcValuePosInBarGraphWidget(((BarGraphWidgetState *)widgetCursor.currentState)->line2Data, min, max, d);

            // make sure line positions don't overlap
            if (pLine1 == pLine2) {
                pLine1 = pLine2 - 1;
            }

            // make sure all lines are visible
            if (pLine1 < 0) {
                pLine2 -= pLine1;
                pLine1 = 0;
            }
        }

        DECL_WIDGET_STYLE(style, widget);

        Style textStyle;

        uint16_t inverseColor;
        if (barGraphWidget->textStyle) {
            DECL_STYLE(textStyleInner, barGraphWidget->textStyle);
            memcpy(&textStyle, textStyleInner, sizeof(Style));
    
            inverseColor = textStyle.background_color;
        } else {
            inverseColor = style->background_color;
        }

        uint16_t fg = widgetCursor.currentState->flags.pressed ? inverseColor : style->color;
        uint16_t bg = widgetCursor.currentState->flags.pressed ? inverseColor : style->background_color;

        if (horizontal) {
            // calc text position
            char valueText[64];
            int pText = 0;
            int wText = 0;
            if (barGraphWidget->textStyle) {
                font::Font font = styleGetFont(&textStyle);
    
                widgetCursor.currentState->data.toText(valueText, sizeof(valueText));
                wText = lcd::lcd.measureStr(valueText, -1, font, w);
                
                int padding = textStyle.padding_horizontal;
                wText += padding;

                if (pValue + wText <= d) {
                    textStyle.background_color = bg;
                    pText = pValue;
                } else {
                    textStyle.background_color = fg;
                    wText += padding;
                    pText = pValue - wText;
                }
            }

            if (barGraphWidget->orientation == BAR_GRAPH_ORIENTATION_LEFT_RIGHT) {
                // draw bar
                if (pText > 0) {
                    lcd::lcd.setColor(fg);
                    lcd::lcd.fillRect(x, y, x + pText - 1, y + h - 1);
                }
    
                drawText(valueText, -1, x + pText, y, wText, h, &textStyle, false);

                // draw background, but do not draw over line 1 and line 2
                lcd::lcd.setColor(bg);

                int pBackground = pText + wText;

                if (pBackground <= pLine1) {
                    if (pBackground < pLine1) {
                        lcd::lcd.fillRect(x + pBackground, y, x + pLine1 - 1, y + h - 1);
                    }
                    pBackground = pLine1 + 1;
                }

                if (!fullScale) {
                    if (pBackground <= pLine2) {
                        if (pBackground < pLine2) {
                            lcd::lcd.fillRect(x + pBackground, y, x + pLine2 - 1, y + h - 1);
                        }
                        pBackground = pLine2 + 1;
                    }
                }

                if (pBackground < d) {
                    lcd::lcd.fillRect(x + pBackground, y, x + d - 1, y + h - 1);
                }
            } else {
                x += w - 1;

                // draw bar
                if (pText > 0) {
                    lcd::lcd.setColor(fg);
                    lcd::lcd.fillRect(x - (pText - 1), y, x, y + h - 1);
                }
    
                drawText(valueText, -1, x - (pText + wText - 1), y, wText, h, &textStyle, false);

                // draw background, but do not draw over line 1 and line 2
                lcd::lcd.setColor(bg);

                int pBackground = pText + wText;

                if (pBackground <= pLine1) {
                    if (pBackground < pLine1) {
                        lcd::lcd.fillRect(x - (pLine1 - 1), y, x - pBackground, y + h - 1);
                    }
                    pBackground = pLine1 + 1;
                }

                if (!fullScale) {
                    if (pBackground <= pLine2) {
                        if (pBackground < pLine2) {
                            lcd::lcd.fillRect(x - (pLine2 - 1), y, x - pBackground, y + h - 1);
                        }
                        pBackground = pLine2 + 1;
                    }
                }

                if (pBackground < d) {
                    lcd::lcd.fillRect(x - (d - 1), y, x - pBackground, y + h - 1);
                }
            }

            drawLineInBarGraphWidget(barGraphWidget, pLine1, barGraphWidget->line1Style, x, y, w, h);
            if (!fullScale) {
                drawLineInBarGraphWidget(barGraphWidget, pLine2, barGraphWidget->line2Style, x, y, w, h);
            }
        } else {
            // calc text position
            char valueText[64];
            int pText = 0;
            int hText = 0;
            if (barGraphWidget->textStyle) {
                font::Font font = styleGetFont(&textStyle);
    
                widgetCursor.currentState->data.toText(valueText, sizeof(valueText));
                hText = font.getHeight();
                
                int padding = textStyle.padding_vertical;
                hText += padding;

                if (pValue + hText <= d) {
                    textStyle.background_color = bg;
                    pText = pValue;
                } else {
                    textStyle.background_color = fg;
                    hText += padding;
                    pText = pValue - hText;
                }
            }

            if (barGraphWidget->orientation == BAR_GRAPH_ORIENTATION_TOP_BOTTOM) {
                // draw bar
                if (pText > 0) {
                    lcd::lcd.setColor(fg);
                    lcd::lcd.fillRect(x, y, x + w - 1, y + pText - 1);
                }
    
                drawText(valueText, -1, x, y + pText, w, hText, &textStyle, false);

                // draw background, but do not draw over line 1 and line 2
                lcd::lcd.setColor(bg);

                int pBackground = pText + hText;

                if (pBackground <= pLine1) {
                    if (pBackground < pLine1) {
                        lcd::lcd.fillRect(x, y + pBackground, x + w - 1, y + pLine1 - 1);
                    }
                    pBackground = pLine1 + 1;
                }

                if (!fullScale) {
                    if (pBackground <= pLine2) {
                        if (pBackground < pLine2) {
                            lcd::lcd.fillRect(x, y + pBackground, x + w - 1, y + pLine2 - 1);
                        }
                        pBackground = pLine2 + 1;
                    }
                }

                if (pBackground < d) {
                    lcd::lcd.fillRect(x, y + pBackground, x + w - 1, y + d - 1);
                }
            } else {
                y += h - 1;

                // draw bar
                if (pText > 0) {
                    lcd::lcd.setColor(fg);
                    lcd::lcd.fillRect(x, y - (pText - 1), x + w - 1, y);
                }
    
                drawText(valueText, -1, x, y - (pText + hText - 1), w, hText, &textStyle, false);

                // draw background, but do not draw over line 1 and line 2
                lcd::lcd.setColor(bg);

                int pBackground = pText + hText;

                if (pBackground <= pLine1) {
                    if (pBackground < pLine1) {
                        lcd::lcd.fillRect(x, y - (pLine1 - 1), x + w - 1, y - pBackground);
                    }
                    pBackground = pLine1 + 1;
                }

                if (!fullScale) {
                    if (pBackground <= pLine2) {
                        if (pBackground < pLine2) {
                            lcd::lcd.fillRect(x, y - (pLine2 - 1), x + w - 1, y - (pBackground));
                        }
                        pBackground = pLine2 + 1;
                    }
                }

                if (pBackground < d) {
                    lcd::lcd.fillRect(x, y - (d - 1), x + w - 1, y - pBackground);
                }
            }

            drawLineInBarGraphWidget(barGraphWidget, pLine1, barGraphWidget->line1Style, x, y, w, h);
            if (!fullScale) {
                drawLineInBarGraphWidget(barGraphWidget, pLine2, barGraphWidget->line2Style, x, y, w, h);
            }
        }
    }
}

int getYValue(
    const WidgetCursor &widgetCursor, const Widget *widget,
    uint8_t data, float min, float max,
    int position
    ) 
{
    float value = data::getHistoryValue(widgetCursor.cursor, data, position).getFloat();
    int y = (int)floor(widget->h * (value - min) / (max - min));
    if (y < 0) y = 0;
    if (y >= widget->h) y = widget->h - 1;
    return widget->h - 1 - y;
}

void drawYTGraph(
    const WidgetCursor &widgetCursor, const Widget *widget,
    int startPosition, int endPosition, int numPositions,
    int currentHistoryValuePosition,
    int xGraphOffset, int graphWidth,
    uint8_t data1, float min1, float max1, uint16_t data1Color,
    uint8_t data2, float min2, float max2, uint16_t data2Color,
    uint16_t color, uint16_t backgroundColor
    ) 
{
    for (int position = startPosition; position < endPosition; ++position) {
        if (position < graphWidth) {
            int x = widgetCursor.x + xGraphOffset + position;

            lcd::lcd.setColor(color);
            lcd::lcd.drawVLine(x, widgetCursor.y, widget->h - 1);

            int y1 = getYValue(widgetCursor, widget, data1, min1, max1, position);
            int y1Prev = getYValue(widgetCursor, widget, data1, min1, max1, position == 0 ? position : position - 1);

            int y2 = getYValue(widgetCursor, widget, data2, min2, max2, position);
            int y2Prev = getYValue(widgetCursor, widget, data2, min2, max2, position == 0 ? position : position - 1);

            if (abs(y1Prev - y1) <= 1 && abs(y2Prev - y2) <= 1) {
                if (y1 == y2) {
                    lcd::lcd.setColor(position % 2 ? data2Color : data1Color);
                    lcd::lcd.drawPixel(x, widgetCursor.y + y1);
                } else {
                    lcd::lcd.setColor(data1Color);
                    lcd::lcd.drawPixel(x, widgetCursor.y + y1);

                    lcd::lcd.setColor(data2Color);
                    lcd::lcd.drawPixel(x, widgetCursor.y + y2);
                }
            } else {
                lcd::lcd.setColor(data1Color);
                if (abs(y1Prev - y1) <= 1) {
                    lcd::lcd.drawPixel(x, widgetCursor.y + y1);
                } else {
                    if (y1Prev < y1) {
                        lcd::lcd.drawVLine(x, widgetCursor.y + y1Prev + 1, y1 - y1Prev - 1);
                    } else {
                        lcd::lcd.drawVLine(x, widgetCursor.y + y1, y1Prev - y1 - 1);
                    }
                }

                lcd::lcd.setColor(data2Color);
                if (abs(y2Prev - y2) <= 1) {
                    lcd::lcd.drawPixel(x, widgetCursor.y + y2);
                } else {
                    if (y2Prev < y2) {
                        lcd::lcd.drawVLine(x, widgetCursor.y + y2Prev + 1, y2 - y2Prev - 1);
                    } else {
                        lcd::lcd.drawVLine(x, widgetCursor.y + y2, y2Prev - y2 - 1);
                    }
                }
            }
        }
    }
}

void drawYTGraphWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    DECL_WIDGET_SPECIFIC(YTGraphWidget, ytGraphWidget, widget);
    DECL_WIDGET_STYLE(style, widget);
    DECL_STYLE(y1Style, ytGraphWidget->y1Style);
    DECL_STYLE(y2Style, ytGraphWidget->y2Style);

    widgetCursor.currentState->size = sizeof(YTGraphWidgetState);
    widgetCursor.currentState->data = data::get(widgetCursor.cursor, widget->data);
    ((YTGraphWidgetState *)widgetCursor.currentState)->y2Data = data::get(widgetCursor.cursor, ytGraphWidget->y2Data);

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed;

    if (refresh) {
        // draw background
        uint16_t color = widgetCursor.currentState->flags.pressed ? style->color : style->background_color;
        lcd::lcd.setColor(color);
        lcd::lcd.fillRect(widgetCursor.x, widgetCursor.y, widgetCursor.x + (int)widget->w - 1, widgetCursor.y + (int)widget->h - 1);
    }

    int textWidth = 62; // TODO this is hardcoded value
    int textHeight = widget->h / 2;

    // draw first value text
    bool refreshText = !widgetCursor.previousState || widgetCursor.previousState->data != widgetCursor.currentState->data;

    if (refresh || refreshText) {
        char text[64];
        widgetCursor.currentState->data.toText(text, sizeof(text));

        drawText(text, -1, widgetCursor.x, widgetCursor.y, textWidth, textHeight, y1Style,
            widgetCursor.currentState->flags.pressed);
    }

    // draw second value text
    refreshText = !widgetCursor.previousState || ((YTGraphWidgetState *)widgetCursor.previousState)->y2Data != ((YTGraphWidgetState *)widgetCursor.currentState)->y2Data;

    if (refresh || refreshText) {
        char text[64];
        ((YTGraphWidgetState *)widgetCursor.currentState)->y2Data.toText(text, sizeof(text));

        drawText(text, -1, widgetCursor.x, widgetCursor.y + textHeight, textWidth, textHeight, y2Style,
            widgetCursor.currentState->flags.pressed);
    }

    // draw graph
    int graphWidth = widget->w - textWidth;

    int numHistoryValues = data::getNumHistoryValues(widget->data);
    int currentHistoryValuePosition = data::getCurrentHistoryValuePosition(widgetCursor.cursor, widget->data);

    static int lastPosition[CH_MAX];

    float min1 = data::getMin(widgetCursor.cursor, widget->data).getFloat();
    float max1 = data::getLimit(widgetCursor.cursor, widget->data).getFloat();

    float min2 = data::getMin(widgetCursor.cursor, ytGraphWidget->y2Data).getFloat();
    float max2 = data::getLimit(widgetCursor.cursor, ytGraphWidget->y2Data).getFloat();

    int iChannel = widgetCursor.cursor.i >= 0 ? widgetCursor.cursor.i : 0;

    int startPosition;
    int endPosition;
    if (refresh) {
        startPosition = 0;
        endPosition = numHistoryValues;
    } else {
        startPosition = lastPosition[iChannel];
        if (startPosition == currentHistoryValuePosition) {
            return;
        }
        endPosition = currentHistoryValuePosition;
    }

    if (startPosition < endPosition) {
        drawYTGraph(widgetCursor, widget,
            startPosition, endPosition,
            currentHistoryValuePosition, numHistoryValues,
            textWidth, graphWidth,
            widget->data, min1, max1, y1Style->color, 
            ytGraphWidget->y2Data, min2, max2, y2Style->color,
            widgetCursor.currentState->flags.pressed ? style->color: style->background_color,
            widgetCursor.currentState->flags.pressed ? style->background_color : style->color);
    } else {
        drawYTGraph(widgetCursor, widget, 
            startPosition, numHistoryValues,
            currentHistoryValuePosition, numHistoryValues,
            textWidth, graphWidth,
            widget->data, min1, max1, y1Style->color,
            ytGraphWidget->y2Data, min2, max2, y2Style->color,
            widgetCursor.currentState->flags.pressed ? style->color: style->background_color,
            widgetCursor.currentState->flags.pressed ? style->background_color : style->color);

        drawYTGraph(widgetCursor, widget,
            0, endPosition,
            currentHistoryValuePosition, numHistoryValues,
            textWidth, graphWidth,
            widget->data, min1, max1, y1Style->color,
            ytGraphWidget->y2Data, min2, max2, y2Style->color,
            widgetCursor.currentState->flags.pressed ? style->color: style->background_color,
            widgetCursor.currentState->flags.pressed ? style->background_color : style->color);
    }

    int x = widgetCursor.x + textWidth;

    // draw cursor
    lcd::lcd.setColor(style->color);
    lcd::lcd.drawVLine(x + currentHistoryValuePosition, widgetCursor.y, (int)widget->h - 1);

    // draw blank lines
    int x1 = x + (currentHistoryValuePosition + 1) % numHistoryValues;
    int x2 = x + (currentHistoryValuePosition + CONF_GUI_YT_GRAPH_BLANK_PIXELS_AFTER_CURSOR) % numHistoryValues;

    lcd::lcd.setColor(style->background_color);
    if (x1 < x2) {
        lcd::lcd.fillRect(x1, widgetCursor.y, x2, widgetCursor.y + (int)widget->h - 1);
    } else {
        lcd::lcd.fillRect(x1, widgetCursor.y, x + graphWidth - 1, widgetCursor.y + (int)widget->h - 1);
        lcd::lcd.fillRect(x, widgetCursor.y, x2, widgetCursor.y + (int)widget->h - 1);
    }

    lastPosition[iChannel] = currentHistoryValuePosition;
}

void drawUpDownWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    DECL_WIDGET_SPECIFIC(UpDownWidget, upDownWidget, widget);

    widgetCursor.currentState->size = sizeof(WidgetState);
    widgetCursor.currentState->data = data::get(widgetCursor.cursor, widget->data);

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
        widgetCursor.previousState->data != widgetCursor.currentState->data;

    if (refresh) {
        DECL_STRING(downButtonText, upDownWidget->downButtonText);
        DECL_STYLE(buttonsStyle, upDownWidget->buttonsStyle);

        font::Font buttonsFont = styleGetFont(buttonsStyle);
        int buttonWidth = buttonsFont.getHeight();

        drawText(downButtonText, -1, widgetCursor.x, widgetCursor.y, buttonWidth, (int)widget->h, buttonsStyle,
            (widgetCursor.currentState->flags.pressed || g_selectedWidget == widgetCursor) && g_selectedWidget.segment == UP_DOWN_WIDGET_SEGMENT_DOWN_BUTTON);

        char text[64];
        widgetCursor.currentState->data.toText(text, sizeof(text));
        DECL_STYLE(style, widget->style);
        drawText(text, -1, widgetCursor.x + buttonWidth, widgetCursor.y, (int)(widget->w - 2 * buttonWidth), (int)widget->h, style, false);

        DECL_STRING(upButtonText, upDownWidget->upButtonText);
        drawText(upButtonText, -1, widgetCursor.x + widget->w - buttonWidth, widgetCursor.y, buttonWidth, (int)widget->h, buttonsStyle,
            (widgetCursor.currentState->flags.pressed || g_selectedWidget == widgetCursor) && g_selectedWidget.segment == UP_DOWN_WIDGET_SEGMENT_UP_BUTTON);
    }
}

void upDown() {
    if (g_foundWidgetAtDown) {
        DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
        if (widget->type == WIDGET_TYPE_UP_DOWN) {
            int value = data::get(g_foundWidgetAtDown.cursor, widget->data).getInt();

            int newValue = value;

            if (g_foundWidgetAtDown.segment == UP_DOWN_WIDGET_SEGMENT_DOWN_BUTTON) {
                --newValue;
            } else if (g_foundWidgetAtDown.segment == UP_DOWN_WIDGET_SEGMENT_UP_BUTTON) {
                ++newValue;
            }

            int min = data::getMin(g_foundWidgetAtDown.cursor, widget->data).getInt();
            if (newValue < min) {
                newValue = min;
            }

            int max = data::getMax(g_foundWidgetAtDown.cursor, widget->data).getInt();
            if (newValue > max) {
                newValue = max;
            }

            if (newValue != value) {
                data::set(g_foundWidgetAtDown.cursor, widget->data, newValue, 0);
            }
        }
    }
}

void drawListGraphWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    DECL_WIDGET_SPECIFIC(ListGraphWidget, listGraphWidget, widget);
    DECL_WIDGET_STYLE(style, widget);
    DECL_STYLE(y1Style, listGraphWidget->y1Style);
    DECL_STYLE(y2Style, listGraphWidget->y2Style);
    DECL_STYLE(cursorStyle, listGraphWidget->cursorStyle);

    widgetCursor.currentState->size = sizeof(ListGraphWidgetState);
    widgetCursor.currentState->data = data::get(widgetCursor.cursor, widget->data);
    ((ListGraphWidgetState *)widgetCursor.currentState)->cursorData = data::get(widgetCursor.cursor, listGraphWidget->cursorData);

    bool refreshAll = !widgetCursor.previousState ||
        widgetCursor.previousState->data != widgetCursor.currentState->data;
    bool refresh = refreshAll;

    int iPrevCursor = -1;
    int iPrevRow = -1;
    if (widgetCursor.previousState) {
        iPrevCursor = ((ListGraphWidgetState *)widgetCursor.previousState)->cursorData.getInt();
        iPrevRow = iPrevCursor / 3;
    }

    int iCursor = ((ListGraphWidgetState *)widgetCursor.currentState)->cursorData.getInt();
    int iRow = iCursor / 3;

    if (!refreshAll) {
        refresh = iPrevCursor != iCursor;
    }

    if (refresh) {
        // draw background
        if (refreshAll) {
            lcd::lcd.setColor(style->background_color);
            lcd::lcd.fillRect(widgetCursor.x, widgetCursor.y, widgetCursor.x + (int)widget->w - 1, widgetCursor.y + (int)widget->h - 1);
        }

        int dwellListLength = data::getListLength(listGraphWidget->dwellData);
        if (dwellListLength > 0) {
            float *dwellList = data::getFloatList(listGraphWidget->dwellData);
        
            const Style *styles[2] = {
                y1Style,
                y2Style
            };

            int listLength[2] = {
                data::getListLength(listGraphWidget->y1Data),
                data::getListLength(listGraphWidget->y2Data)
            };
            
            float *list[2] = {
                data::getFloatList(listGraphWidget->y1Data),
                data::getFloatList(listGraphWidget->y2Data)
            };
            
            float min[2] = {
                data::getMin(widgetCursor.cursor, listGraphWidget->y1Data).getFloat(),
                data::getMin(widgetCursor.cursor, listGraphWidget->y2Data).getFloat()
            };
            
            float max[2] = {
                data::getMax(widgetCursor.cursor, listGraphWidget->y1Data).getFloat(),
                data::getMax(widgetCursor.cursor, listGraphWidget->y2Data).getFloat()
            };

            int maxListLength = data::getListLength(widget->data);

            float dwellSum = 0;
            for (int i = 0; i < maxListLength; ++i) {
                if (i < dwellListLength) {
                    dwellSum += dwellList[i];
                } else {
                    dwellSum += dwellList[dwellListLength - 1];
                }
            }

            float currentDwellSum = 0;
            int xPrev = widgetCursor.x;
            int yPrev[2];
            for (int i = 0; i < maxListLength; ++i) {
                currentDwellSum += i < dwellListLength ? dwellList[i] : dwellList[dwellListLength - 1];
                int x1 = xPrev;
                int x2;
                if (i == maxListLength - 1) {
                    x2 = widgetCursor.x + (int)widget->w - 1;
                } else {
                    x2 = widgetCursor.x + int(currentDwellSum * (int)widget->w / dwellSum);
                }
                if (x2 < x1) x2 = x1;
                if (x2 >= widgetCursor.x + (int)widget->w) x2 = widgetCursor.x + (int)widget->w - 1;

                bool skipDraw = false;

                if (!refreshAll) {
                    if (i < iPrevRow - 1 && i > iPrevRow + 1 && i < iRow - 1 && i > iRow + 1) {
                        skipDraw = true;
                    }
                }

                if (!skipDraw) {
                    if (!refreshAll && i == iPrevRow) {
                        lcd::lcd.setColor(style->background_color);
                        lcd::lcd.fillRect(x1, widgetCursor.y, x2 - 1, widgetCursor.y + (int)widget->h - 1);
                    }

                    if (i == iRow) {
                        lcd::lcd.setColor(cursorStyle->background_color);
                        lcd::lcd.fillRect(x1, widgetCursor.y, x2 - 1, widgetCursor.y + (int)widget->h - 1);
                    }
                }

                for (int k = 0; k < 2; ++k) {
                    int j = iCursor % 3 == 2 ? k : 1 - k;

                    if (listLength[j] > 0) {
                        if (!skipDraw) {
                            lcd::lcd.setColor(styles[j]->color);
                        }

                        float value = i < listLength[j] ? list[j][i] : list[j][listLength[j] - 1];
                        int y = int((value - min[j]) * widget->h / (max[j] - min[j]));
                        if (y < 0) y = 0;
                        if (y >= (int)widget->h) y = (int)widget->h - 1;

                        y = widgetCursor.y + ((int)widget->h - 1) - y;

                        if (!skipDraw) {
                            if (i > 0 && abs(yPrev[j] - y) > 1) {
                                if (yPrev[j] < y) {
                                    lcd::lcd.drawVLine(x1, yPrev[j] + 1, y - yPrev[j] - 1);
                                } else {
                                    lcd::lcd.drawVLine(x1, y, yPrev[j] - y - 1);
                                }
                            }
                        }

                        yPrev[j] = y;

                        if (!skipDraw) {
                            lcd::lcd.drawHLine(x1, y, x2 - x1);
                        }
                    }
                }

                xPrev = x2;
            }
        }
    }
}

void onTouchListGraph(const WidgetCursor &widgetCursor, int xTouch, int yTouch) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    DECL_WIDGET_SPECIFIC(ListGraphWidget, listGraphWidget, widget);

    if (xTouch < widgetCursor.x || xTouch >= widgetCursor.x + (int)widget->w) return;
    if (yTouch < widgetCursor.y || yTouch >= widgetCursor.y + (int)widget->h) return;

    int dwellListLength = data::getListLength(listGraphWidget->dwellData);
    if (dwellListLength > 0) {
        int iCursor = -1;

        float *dwellList = data::getFloatList(listGraphWidget->dwellData);

        int maxListLength = data::getListLength(widget->data);

        float dwellSum = 0;
        for (int i = 0; i < maxListLength; ++i) {
            if (i < dwellListLength) {
                dwellSum += dwellList[i];
            } else {
                dwellSum += dwellList[dwellListLength - 1];
            }
        }

        float currentDwellSum = 0;
        int xPrev = widgetCursor.x;
        for (int i = 0; i < maxListLength; ++i) {
            currentDwellSum += i < dwellListLength ? dwellList[i] : dwellList[dwellListLength - 1];
            int x1 = xPrev;
            int x2;
            if (i == maxListLength - 1) {
                x2 = widgetCursor.x + (int)widget->w - 1;
            } else {
                x2 = widgetCursor.x + int(currentDwellSum * (int)widget->w / dwellSum);
            }
            if (x2 < x1) x2 = x1;
            if (x2 >= widgetCursor.x + (int)widget->w) x2 = widgetCursor.x + (int)widget->w - 1;

            if (xTouch >= x1 && xTouch < x2) {
                int iCurrentCursor = data::get(widgetCursor.cursor, listGraphWidget->cursorData).getInt();
                iCursor = i * 3 + iCurrentCursor % 3;
                break;
            }
        }

        if (iCursor >= 0) {
            data::set(widgetCursor.cursor, listGraphWidget->cursorData, data::Value(iCursor), 0);
        }
    }
}

void drawWidget(const WidgetCursor &widgetCursor_) {
    WidgetCursor widgetCursor = widgetCursor_;

    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    uint8_t state[128];
    if (widgetCursor.currentState == 0) {
        widgetCursor.currentState = (WidgetState *)&state;
    } else {
        //static uint16_t g_maxStateSize = 0; 
        //uint16_t stateSize = (uint8_t *)widgetCursor.currentState - (getCurrentStateBufferIndex() == 0 ? &g_stateBuffer[0][0] : &g_stateBuffer[1][0]);
        //if (stateSize > g_maxStateSize) {
        //    g_maxStateSize = stateSize;
        //    DebugTraceF("%d", (int)g_maxStateSize);
        //}
    }

    widgetCursor.currentState->flags.pressed = g_selectedWidget == widgetCursor;

    if (widget->type == WIDGET_TYPE_DISPLAY_DATA) {
        drawDisplayDataWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_TEXT) {
        drawTextWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_MULTILINE_TEXT) {
        drawMultilineTextWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_RECTANGLE) {
        drawRectangleWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_BITMAP) {
        drawBitmapWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_BUTTON) {
        drawButtonWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_TOGGLE_BUTTON) {
        drawToggleButtonWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_BUTTON_GROUP) {
        widgetButtonGroup::draw(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_SCALE) {
        drawScaleWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_BAR_GRAPH) {
        drawBarGraphWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_YT_GRAPH) {
        drawYTGraphWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_UP_DOWN) {
        drawUpDownWidget(widgetCursor);
    } else if (widget->type == WIDGET_TYPE_LIST_GRAPH) {
        drawListGraphWidget(widgetCursor);
    } 
}

void refreshWidget(WidgetCursor widgetCursor) {
    if (isActivePageInternal()) {
        ((InternalPage *)getActivePage())->drawWidget(widgetCursor, widgetCursor == g_selectedWidget);
    } else {
        g_widgetRefresh = true;
        drawWidget(widgetCursor);
        g_widgetRefresh = false;
    }
}

void selectWidget(WidgetCursor &widgetCursor) {
    g_selectedWidget = widgetCursor;
    refreshWidget(g_selectedWidget);
}

void deselectWidget() {
    WidgetCursor old_selected_widget = g_selectedWidget;
    g_selectedWidget = 0;
    refreshWidget(old_selected_widget);
}

////////////////////////////////////////////////////////////////////////////////

WidgetState *next(WidgetState *p) {
    return p ? (WidgetState *)(((uint8_t *)p) + p->size) : 0;
}

void enumWidget(OBJ_OFFSET widgetOffset, int x, int y, data::Cursor &cursor, WidgetState *previousState, WidgetState *currentState, EnumWidgetsCallback callback);

void enumContainer(List widgets, int x, int y, data::Cursor &cursor, WidgetState *previousState, WidgetState *currentState, EnumWidgetsCallback callback) {
    WidgetState *savedCurrentState = currentState;

    WidgetState *endOfContainerInPreviousState;
    if (previousState) endOfContainerInPreviousState = next(previousState);

    // move to the first child widget state
    if (previousState) ++previousState;
    if (currentState) ++currentState;

    for (int index = 0; index < widgets.count; ++index) {
        OBJ_OFFSET childWidgetOffset = getListItemOffset(widgets, index, sizeof(Widget));
        enumWidget(childWidgetOffset, x, y, cursor, previousState, currentState, callback);

        if (previousState) {
            previousState = next(previousState);
            if (previousState >= endOfContainerInPreviousState) previousState = 0;
        }
        
        currentState = next(currentState);
    }

    if (currentState) {
        savedCurrentState->size = ((uint8_t *)currentState) - ((uint8_t *)savedCurrentState);
    }
}

void enumWidget(OBJ_OFFSET widgetOffset, int x, int y, data::Cursor &cursor, WidgetState *previousState, WidgetState *currentState, EnumWidgetsCallback callback) {
    psu::criticalTick();

    DECL_WIDGET(widget, widgetOffset);

    x += widget->x;
    y += widget->y;

    if (widget->type == WIDGET_TYPE_CONTAINER) {
        DECL_WIDGET_SPECIFIC(ContainerWidget, container, widget);
        enumContainer(container->widgets, x, y, cursor, previousState, currentState, callback);
    }
    else if (widget->type == WIDGET_TYPE_CUSTOM) {
        DECL_WIDGET_SPECIFIC(CustomWidgetSpecific, customWidgetSpecific, widget);
        DECL_CUSTOM_WIDGET(customWidget, customWidgetSpecific->customWidget);
        enumContainer(customWidget->widgets, x, y, cursor, previousState, currentState, callback);
    }
    else if (widget->type == WIDGET_TYPE_LIST) {
        WidgetState *savedCurrentState = currentState;

        WidgetState *endOfContainerInPreviousState;
        if (previousState) endOfContainerInPreviousState = next(previousState);

        // move to the first child widget state
        if (previousState) ++previousState;
        if (currentState) ++currentState;

        int xOffset = 0;
        int yOffset = 0;
        for (int index = 0; index < data::count(widget->data); ++index) {
            data::select(cursor, widget->data, index);

            DECL_WIDGET_SPECIFIC(ListWidget, listWidget, widget);
            OBJ_OFFSET childWidgetOffset = listWidget->item_widget;

            if (listWidget->listType == LIST_TYPE_VERTICAL) {
                DECL_WIDGET(childWidget, childWidgetOffset);
                if (yOffset < widget->h) {
                    enumWidget(childWidgetOffset, x + xOffset, y + yOffset, cursor, previousState, currentState, callback);
                    yOffset += childWidget->h;
                } else {
                    // TODO: add vertical scroll
                    break;
                }
            } else {
                DECL_WIDGET(childWidget, childWidgetOffset);
                if (xOffset < widget->w) {
                    enumWidget(childWidgetOffset, x + xOffset, y + yOffset, cursor, previousState, currentState, callback);
                    xOffset += childWidget->w;
                } else {
                    // TODO: add horizontal scroll
                    break;
                }
            }

            if (previousState) {
                previousState = next(previousState);
                if (previousState >= endOfContainerInPreviousState) previousState = 0;
            }
        
            currentState = next(currentState);
        }

        if (currentState) {
            savedCurrentState->size = ((uint8_t *)currentState) - ((uint8_t *)savedCurrentState);
        }

        data::select(cursor, widget->data, -1);
    }
    else if (widget->type == WIDGET_TYPE_SELECT) {
        data::Value indexValue = data::get(cursor, widget->data);

        if (currentState) {
            currentState->data = indexValue;
        }

        if (previousState && previousState->data != currentState->data) {
            previousState = 0;
        }

        WidgetState *savedCurrentState = currentState;

        // move to the selected widget state
        if (previousState) ++previousState;
        if (currentState) ++currentState;

        int index = indexValue.getInt();
        DECL_WIDGET_SPECIFIC(ContainerWidget, containerWidget, widget);
        OBJ_OFFSET selectedWidgetOffset = getListItemOffset(containerWidget->widgets, index, sizeof(Widget));

        enumWidget(selectedWidgetOffset, x, y, cursor, previousState, currentState, callback);

        if (currentState) {
            savedCurrentState->size = sizeof(WidgetState) + currentState->size;
        }
    }
    else {
        callback(WidgetCursor(widgetOffset, x, y, cursor, previousState, currentState));
    }
}

void enumWidgets(int pageIndex, WidgetState *previousState, WidgetState *currentState, EnumWidgetsCallback callback) {
    data::Cursor cursor;
    cursor.reset();
    enumWidget(getPageOffset(pageIndex), 0, 0, cursor, previousState, currentState, callback);
}

////////////////////////////////////////////////////////////////////////////////

void clearBackground() {
    // clear screen with background color
    DECL_WIDGET(page, getPageOffset(getActivePageId()));

    DECL_WIDGET_STYLE(style, page);
    lcd::lcd.setColor(style->background_color);

    if (getPreviousActivePageId() == PAGE_ID_INFO_ALERT || getPreviousActivePageId() == PAGE_ID_ERROR_ALERT || getPreviousActivePageId() == PAGE_ID_YES_NO) {
        DECL_WIDGET(page, getPageOffset(getPreviousActivePageId()));
        lcd::lcd.fillRect(page->x, page->y, page->x + page->w - 1, page->y + page->h - 1);
    }

    if (getActivePageId() == PAGE_ID_WELCOME) {
        lcd::lcd.fillRect(page->x, page->y, page->x + page->w - 1, page->y + page->h - 1);
    } else {
        DECL_WIDGET_SPECIFIC(PageWidget, pageSpecific, page);
        for (int i = 0; i < pageSpecific->transparentRectangles.count; ++i) {
            OBJ_OFFSET rectOffset = getListItemOffset(pageSpecific->transparentRectangles, i, sizeof(Rect));
            DECL_STRUCT_WITH_OFFSET(Rect, rect, rectOffset);
            lcd::lcd.fillRect(rect->x, rect->y, rect->x + rect->w - 1, rect->y + rect->h - 1);
        }
    }
}

void drawActivePage(bool refresh) {
    g_wasBlinkTime = g_isBlinkTime;
    g_isBlinkTime = (micros() % (2 * CONF_GUI_BLINK_TIME)) > CONF_GUI_BLINK_TIME && touch::event_type == touch::TOUCH_NONE;

    if (refresh) {
        g_previousState = 0;
        g_currentState = (WidgetState *)(&g_stateBuffer[0][0]);
    } else {
        g_previousState = g_currentState;
        g_currentState = (WidgetState *)(&g_stateBuffer[getCurrentStateBufferIndex() == 0 ? 1 : 0][0]);
    }

    enumWidgets(getActivePageId(), g_previousState, g_currentState, drawWidget);
}

static bool g_refreshPageOnNextTick;

void drawTick() {
    if (isActivePageInternal()) {
        ((InternalPage *)getActivePage())->drawTick();
    } else {
        if (g_refreshPageOnNextTick) {
            g_refreshPageOnNextTick = false;
            clearBackground();
            drawActivePage(true);
        } else {
            drawActivePage(false);
        }
    }
}


void refreshPage() {
    if (isActivePageInternal()) {
        ((InternalPage *)getActivePage())->refresh();
    } else {
        g_refreshPageOnNextTick = true;
    }
}

void flush() {
    drawTick();

#ifdef EEZ_PSU_SIMULATOR
    if (simulator::front_panel::isOpened()) {
        simulator::front_panel::tick();
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

static int g_find_widget_at_x;
static int g_find_widget_at_y;
static WidgetCursor g_foundWidget;

void findWidgetStep(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    bool inside = 
        g_find_widget_at_x >= widgetCursor.x &&
        g_find_widget_at_x < widgetCursor.x + (int)widget->w &&
        g_find_widget_at_y >= widgetCursor.y &&
        g_find_widget_at_y < widgetCursor.y + (int)widget->h;

    if (inside) {
        g_foundWidget = widgetCursor;

        if (widget->type == WIDGET_TYPE_UP_DOWN) {
            if (g_find_widget_at_x < widgetCursor.x + widget->w / 2) {
                g_foundWidget.segment = UP_DOWN_WIDGET_SEGMENT_DOWN_BUTTON;
            } else {
                g_foundWidget.segment = UP_DOWN_WIDGET_SEGMENT_UP_BUTTON;
            }
        }
    }
}

WidgetCursor findWidget(int x, int y) {
    if (isActivePageInternal()) {
        return ((InternalPage *)getActivePage())->findWidget(x, y);
    } else {
        g_foundWidget = 0;

        g_find_widget_at_x = x;
        g_find_widget_at_y = y;
        enumWidgets(getActivePageId(), 0, 0, findWidgetStep);

        return g_foundWidget;
    }
}

}
}
} // namespace eez::psu::gui

#endif