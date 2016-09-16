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

#include "channel.h"
#include "actions.h"
#include "devices.h"
#include "sound.h"
#include "event_queue.h"

#include "gui.h"
#include "gui_internal.h"
#include "gui_data_snapshot.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_slider.h"
#include "gui_edit_mode_step.h"
#include "gui_edit_mode_keypad.h"
#include "gui_widget_button_group.h"
#include "gui_page_self_test_result.h"
#include "gui_page_main.h"
#include "gui_page_event_queue.h"
#include "gui_page_ch_settings_protection.h"
#include "gui_page_ch_settings_adv.h"
#include "gui_page_ch_settings_info.h"
#include "gui_page_sys_info.h"

#ifdef EEZ_PSU_SIMULATOR
#include "front_panel/control.h"
#endif

#define CONF_GUI_BLINK_TIME 400000UL
#define CONF_GUI_ENUM_WIDGETS_STACK_SIZE 5
#define CONF_GUI_STANDBY_PAGE_TIMEOUT 10000000UL
#define CONF_GUI_ENTERING_STANDBY_PAGE_TIMEOUT 5000000UL
#define CONF_GUI_WELCOME_PAGE_TIMEOUT 2000000UL
#define CONF_GUI_LONG_PRESS_TIMEOUT 1000000UL
#define CONF_GUI_DRAW_TICK_ITERATIONS 100
#define CONF_GUI_PAGE_NAVIGATION_STACK_SIZE 5

namespace eez {
namespace psu {
namespace gui {

using namespace lcd;

////////////////////////////////////////////////////////////////////////////////

Document *g_doc;
#if defined(EEZ_PSU_ARDUINO_MEGA)
static Document g_docBuffer;
#endif

static int g_activePageId;
static Page *g_activePage;

static int g_lastActivePageId = -1;
static Page *g_lastActivePage;
static bool g_lastPushActivePageOnStack;

static bool g_pushActivePageOnStack;

static int g_pageNavigationStack[CONF_GUI_PAGE_NAVIGATION_STACK_SIZE];
static int g_pageNavigationStackPointer = 0;

static bool g_widgetRefresh;

static WidgetCursor g_selectedWidget;
WidgetCursor g_foundWidgetAtDown;

static void (*g_dialogYesCallback)();
static void (*g_dialogNoCallback)();
static void (*g_dialogCancelCallback)();

static bool g_isBlinkTime;
static bool g_wasBlinkTime;

static unsigned long g_showPageTime;
static unsigned long g_timeOfLastActivity;
static unsigned long g_touchDownTime;
static bool g_touchActionExecuted;

Channel *g_channel;

Page *createPageFromId(int pageId) {
	switch (pageId) {
	case PAGE_ID_SELF_TEST_RESULT: return new SelfTestResultPage();
	case PAGE_ID_MAIN: return new MainPage();
	case PAGE_ID_EVENT_QUEUE: return new EventQueuePage();
	case PAGE_ID_CH_SETTINGS_PROT: return new ChSettingsProtectionPage();
	case PAGE_ID_CH_SETTINGS_PROT_OVP: return new ChSettingsOvpProtectionPage();
	case PAGE_ID_CH_SETTINGS_PROT_OCP: return new ChSettingsOcpProtectionPage();
	case PAGE_ID_CH_SETTINGS_PROT_OPP: return new ChSettingsOppProtectionPage();
	case PAGE_ID_CH_SETTINGS_PROT_OTP: return new ChSettingsOtpProtectionPage();
	case PAGE_ID_CH_SETTINGS_ADV: return new ChSettingsAdvPage();
	case PAGE_ID_CH_SETTINGS_ADV_LRIPPLE: return new ChSettingsAdvLRipplePage();
	case PAGE_ID_CH_SETTINGS_ADV_RSENSE: return new ChSettingsAdvRSensePage();
	case PAGE_ID_CH_SETTINGS_ADV_RPROG: return new ChSettingsAdvRProgPage();
	case PAGE_ID_CH_SETTINGS_INFO: return new ChSettingsInfoPage();
	case PAGE_ID_SYS_INFO:
	case PAGE_ID_SYS_INFO2: return new SysInfoPage();
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

typedef bool(*EnumWidgetsCallback)(const WidgetCursor &widgetCursor, bool refresh);

//int g_sp_max_counter = 0;

class EnumWidgets {
public:
    EnumWidgets(EnumWidgetsCallback callback) {
        this->callback = callback;
    }

    void start(int pageIndex, int x, int y, bool refresh) {
		cursor.iChannel = -1;
        stack[0].widgetOffset = getPageOffset(pageIndex);
        stack[0].index = 0;
        stack_index = 0;
        stack[0].x = x;
        stack[0].y = y;
        stack[0].refresh = refresh;
    }

    bool next() {
        if (stack_index == CONF_GUI_ENUM_WIDGETS_STACK_SIZE) {
            return false;
        }

        while (true) {
            DECL_WIDGET(widget, stack[stack_index].widgetOffset);

            if (widget->type == WIDGET_TYPE_CONTAINER) {
                DECL_WIDGET_SPECIFIC(ContainerWidget, container, widget);
                if (stack[stack_index].index < container->widgets.count) {
                    OBJ_OFFSET childWidgetOffset = getListItemOffset(container->widgets, stack[stack_index].index, sizeof(Widget));
                    ++stack[stack_index].index;
                    if (!push(childWidgetOffset, stack[stack_index].x, stack[stack_index].y, stack[stack_index].refresh)) {
                        return true;
                    }
                }
                else {
                    if (!pop()) {
                        return false;
                    }
                }
            }
            else if (widget->type == WIDGET_TYPE_LIST) {
                if (stack[stack_index].index < data::count(widget->data)) {
                    data::select(cursor, widget->data, stack[stack_index].index);

                    DECL_WIDGET_SPECIFIC(ListWidget, listWidget, widget);
                    OBJ_OFFSET childWidgetOffset = listWidget->item_widget;

                    ++stack[stack_index].index;

                    if (listWidget->listType == LIST_TYPE_VERTICAL) {
                        int y = stack[stack_index].y;

						DECL_WIDGET(childWidget, childWidgetOffset);
						if ((stack[stack_index].index - 1) * childWidget->h < widget->h) {
							stack[stack_index].y += childWidget->h;

							if (!push(childWidgetOffset, stack[stack_index].x, y, stack[stack_index].refresh)) {
								return true;
							}
						} else {
							// TODO: add vertical scroll

							// stop iteration
							stack[stack_index].index = data::count(widget->data);
						}
                    } else {
                        int x = stack[stack_index].x;

						DECL_WIDGET(childWidget, childWidgetOffset);
						if ((stack[stack_index].index - 1) * childWidget->w < widget->w) {
							stack[stack_index].x += childWidget->w;

							if (!push(childWidgetOffset, x, stack[stack_index].y, stack[stack_index].refresh)) {
								return true;
							}
						} else {
							// TODO: add horizontal scroll
							
							// stop iteration
							stack[stack_index].index = data::count(widget->data);
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
    data::Cursor cursor;

    EnumWidgetsCallback callback;

    struct StackItem {
        OBJ_OFFSET widgetOffset;
        int index;
        int x;
        int y;
        bool refresh;
    };

    StackItem stack[CONF_GUI_ENUM_WIDGETS_STACK_SIZE];
    int stack_index;

    bool push(OBJ_OFFSET widgetOffset, int x, int y, bool refresh) {
        DECL_WIDGET(widget, widgetOffset);

        if (widget->type == WIDGET_TYPE_CONTAINER || widget->type == WIDGET_TYPE_LIST) {
			if (++stack_index == CONF_GUI_ENUM_WIDGETS_STACK_SIZE) {
                return false;
            }

            //if (stack_index > g_sp_max_counter) g_sp_max_counter = stack_index;

            stack[stack_index].widgetOffset = widgetOffset;
            stack[stack_index].index = 0;
            stack[stack_index].x = x + widget->x;
            stack[stack_index].y = y + widget->y;
            stack[stack_index].refresh = refresh;

            return true;
        }
        else if (widget->type == WIDGET_TYPE_SELECT) {
            int index = data::currentSnapshot.get(cursor, widget->data).getInt();
            data::select(cursor, widget->data, index);

            DECL_WIDGET_SPECIFIC(ContainerWidget, containerWidget, widget);
            OBJ_OFFSET selectedWidgetOffset = getListItemOffset(containerWidget->widgets, index, sizeof(Widget));

            if (!refresh) {
                int previousIndex = data::previousSnapshot.get(cursor, widget->data).getInt();
                refresh = index != previousIndex;
            }

            return push(selectedWidgetOffset, x + widget->x, y + widget->y, refresh);
        }
        else {
            return !callback(WidgetCursor(widgetOffset, x + widget->x, y + widget->y, cursor), refresh);
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

//int draw_counter;

void drawText(const char *text, int textLength, int x, int y, int w, int h, const Style *style, bool inverse) {
    //++draw_counter;

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

    uint16_t background_color = inverse ? style->color : style->background_color;
    lcd::lcd.setColor(background_color);

    if (g_widgetRefresh) {
        lcd::lcd.fillRect(x1, y1, x2, y2);
    } else {
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
    lcd::lcd.drawStr(text, textLength, x_offset, y_offset, x1, y1, x2, y2, font, !g_widgetRefresh);
}

void drawMultilineText(const char *text, int x, int y, int w, int h, const Style *style, bool inverse) {
    //++draw_counter;

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
    //++draw_counter;

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
    lcd::lcd.drawBitmap(x_offset, y_offset, width, height, (bitmapdatatype)bitmap.pixels, 1);
}

void drawRectangle(int x, int y, int w, int h, const Style *style, bool inverse) {
    //++draw_counter;

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

bool draw_display_data_widget(const WidgetCursor &widgetCursor, const Widget *widget, bool refresh, bool inverse) {
	DECL_WIDGET_SPECIFIC(DisplayDataWidget, display_data_widget, widget);

	bool edit = edit_mode::isEditWidget(widgetCursor);
    if (edit && g_activePageId == PAGE_ID_EDIT_MODE_KEYPAD || widget->data == DATA_ID_KEYPAD_TEXT) {
        char *text = data::currentSnapshot.keypadSnapshot.text;
        if (!refresh) {
            char *previousText = data::previousSnapshot.keypadSnapshot.text;
            refresh = strcmp(text, previousText) != 0;
        }
        if (refresh) {
            DECL_STYLE_WITH_OFFSET(style, display_data_widget->editStyle);
            drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
            return true;
        }
        return false;
    }
    else {
        data::Value value;

        bool isBlinking = data::currentSnapshot.isBlinking(widgetCursor.cursor, widget->data);
        if (isBlinking) {
            if (g_isBlinkTime) {
                value = data::Value("");
            } else {
                value = data::currentSnapshot.get(widgetCursor.cursor, widget->data);
            }
            refresh = refresh || (g_isBlinkTime != g_wasBlinkTime);
        } else {
            value = data::currentSnapshot.get(widgetCursor.cursor, widget->data);
        }

		data::Value previousValue;
        if (!refresh) {
            bool wasBlinking = data::previousSnapshot.isBlinking(widgetCursor.cursor, widget->data);
            refresh = isBlinking != wasBlinking;
            if (!refresh) {
                previousValue = data::previousSnapshot.get(widgetCursor.cursor, widget->data);
                refresh = value != previousValue;
            }
        }

        if (refresh) {
            char text[64];
            value.toText(text, sizeof(text));

            if (edit) {
                DECL_STYLE_WITH_OFFSET(style, display_data_widget->editStyle);
                drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
            }
            else {
                DECL_WIDGET_STYLE(style, widget);
                drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
            }

            return true;
        }

        return false;
    }
}

bool draw_text_widget(const WidgetCursor &widgetCursor, const Widget *widget, bool refresh, bool inverse) {
    if (refresh) {
        DECL_WIDGET_STYLE(style, widget);

		if (widget->data) {
			data::Value value = data::currentSnapshot.get(widgetCursor.cursor, widget->data);
			if (value.isString()) {
	            drawText(value.asString(), -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
			} else {
				char text[64];
				value.toText(text, sizeof(text));
				drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
			}
		} else {
			DECL_WIDGET_SPECIFIC(TextWidget, display_string_widget, widget);
			DECL_STRING(text, display_string_widget->text);
			drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
		}

		return true;
    }
    return false;
}

bool draw_multiline_text_widget(const WidgetCursor &widgetCursor, const Widget *widget, bool refresh, bool inverse) {
    if (refresh) {
        DECL_WIDGET_STYLE(style, widget);

		if (widget->data) {
			data::Value value = data::currentSnapshot.get(widgetCursor.cursor, widget->data);
			if (value.isString()) {
				drawMultilineText(value.asString(), widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
			} else {
				char text[64];
				value.toText(text, sizeof(text));
				drawMultilineText(text, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
			}
		} else {
			DECL_WIDGET_SPECIFIC(MultilineTextWidget, display_string_widget, widget);
			DECL_STRING(text, display_string_widget->text);
			drawMultilineText(text, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
		}

		return true;
    }
    return false;
}

void drawScale(const Widget *widget, const ScaleWidget *scale_widget, const Style* style, int y_from, int y_to, int y_min, int y_max, int y_value, int f, int d, bool drawTicks) {
    //++draw_counter;

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

bool draw_scale_widget(const WidgetCursor &widgetCursor, const Widget *widget, bool refresh, bool inverse) {
    data::Value value = data::currentSnapshot.get(widgetCursor.cursor, widget->data);
    if (!refresh) {
        data::Value previousValue = data::previousSnapshot.editModeSnapshot.editValue;
        refresh = previousValue != value;
    }

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
        int y_value = (int)round(value.getFloat() * f);

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

        return true;
    }

    return false;
}

bool draw_button_widget(const WidgetCursor &widgetCursor, const Widget *widget, bool refresh, bool inverse) {
    DECL_WIDGET_SPECIFIC(ButtonWidget, button_widget, widget);

	int state = data::currentSnapshot.get(widgetCursor.cursor, button_widget->enabled).getInt();
    if (!refresh) {
        int previousState = data::previousSnapshot.get(widgetCursor.cursor, button_widget->enabled).getInt();
        refresh = state != previousState;
    }

	if (refresh) {
        DECL_STRING(text, button_widget->text);
        DECL_STYLE_WITH_OFFSET(style, state ? widget->style : button_widget->disabledStyle);
        drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
        return true;
    }
    
	return false;
}

bool draw_toggle_button_widget(const WidgetCursor &widgetCursor, const Widget *widget, bool refresh, bool inverse) {
    int state = data::currentSnapshot.get(widgetCursor.cursor, widget->data).getInt();
    if (!refresh) {
        int previousState = data::previousSnapshot.get(widgetCursor.cursor, widget->data).getInt();
        refresh = state != previousState;
    }
    if (refresh) {
        DECL_WIDGET_SPECIFIC(ToggleButtonWidget, toggle_button_widget, widget);
        DECL_STRING(text, (state == 0 ? toggle_button_widget->text1 : toggle_button_widget->text2));
        DECL_WIDGET_STYLE(style, widget);
        drawText(text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
        return true;
    }
    return false;
}

bool draw_rectangle_widget(const WidgetCursor &widgetCursor, const Widget *widget, bool refresh, bool inverse) {
    if (refresh) {
        DECL_WIDGET_STYLE(style, widget);
        drawRectangle(widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
        return true;
    }
    return false;
}

bool draw_bitmap_widget(const WidgetCursor &widgetCursor, const Widget *widget, bool refresh, bool inverse) {
    if (refresh) {
        DECL_WIDGET_SPECIFIC(BitmapWidget, display_bitmap_widget, widget);
        DECL_WIDGET_STYLE(style, widget);
        drawBitmap(display_bitmap_widget->bitmap, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style, inverse);
        return true;
    }
    return false;
}

bool draw_widget(const WidgetCursor &widgetCursor, bool refresh) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    bool inverse = g_selectedWidget == widgetCursor;

    if (widget->type == WIDGET_TYPE_DISPLAY_DATA) {
        return draw_display_data_widget(widgetCursor, widget, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_TEXT) {
        return draw_text_widget(widgetCursor, widget, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_MULTILINE_TEXT) {
        return draw_multiline_text_widget(widgetCursor, widget, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_RECTANGLE) {
        return draw_rectangle_widget(widgetCursor, widget, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_BITMAP) {
        return draw_bitmap_widget(widgetCursor, widget, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_BUTTON) {
        return draw_button_widget(widgetCursor, widget, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_TOGGLE_BUTTON) {
        return draw_toggle_button_widget(widgetCursor, widget, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_BUTTON_GROUP) {
        return widget_button_group::draw(widgetCursor, widget, refresh, inverse);
    } else if (widget->type == WIDGET_TYPE_SCALE) {
        return draw_scale_widget(widgetCursor, widget, refresh, inverse);
    } 

    return false;
}

static EnumWidgets g_drawEnumWidgets(draw_widget);

bool draw_tick() {
    for (int i = 0; i < CONF_GUI_DRAW_TICK_ITERATIONS; ++i) {
        if (!g_drawEnumWidgets.next()) {
            g_wasBlinkTime = g_isBlinkTime;
            g_isBlinkTime = (micros() % (2 * CONF_GUI_BLINK_TIME)) > CONF_GUI_BLINK_TIME && touch::event_type == touch::TOUCH_NONE;

            data::previousSnapshot = data::currentSnapshot;
            data::currentSnapshot.takeSnapshot();

		    DECL_WIDGET(page, getPageOffset(g_activePageId));
            g_drawEnumWidgets.start(g_activePageId, page->x, page->y, false);

            //DebugTraceF("%d", draw_counter);
            //draw_counter = 0;

            return false;
        }
    }
	return true;
}

void refresh_widget(WidgetCursor widget_cursor) {
    g_widgetRefresh = true;
    draw_widget(widget_cursor, true);
    g_widgetRefresh = false;
}

void refreshPage() {
	DECL_WIDGET(page, getPageOffset(g_activePageId));

	// clear screen with background color
    DECL_WIDGET_SPECIFIC(PageWidget, pageSpecific, page);
    DECL_WIDGET_STYLE(style, page);
    lcd::lcd.setColor(style->background_color);
	if (g_activePageId == PAGE_ID_WELCOME) {
		lcd::lcd.fillRect(page->x, page->y, page->x + page->w - 1, page->y + page->h - 1);
	} else {
		for (int i = 0; i < pageSpecific->transparentRectangles.count; ++i) {
			OBJ_OFFSET rectOffset = getListItemOffset(pageSpecific->transparentRectangles, i, sizeof(Rect));
			DECL_STRUCT_WITH_OFFSET(Rect, rect, rectOffset);
			lcd::lcd.fillRect(rect->x, rect->y, rect->x + rect->w - 1, rect->y + rect->h - 1);
		}
	}

    data::currentSnapshot.takeSnapshot();
    g_drawEnumWidgets.start(g_activePageId, page->x, page->y, true);
}

void flush() {
    while (draw_tick());

#ifdef EEZ_PSU_SIMULATOR
    if (simulator::front_panel::isOpened()) {
        simulator::front_panel::tick();
    }
#endif
}

int getActivePageId() {
    return g_activePageId;
}

Page *getActivePage() {
    return g_activePage;
}

void doShowPage(int index) {
    lcd::turnOn();

	if (g_activePage) {
		g_activePage->pageDidDisappear();
		delete g_activePage;
		g_activePage = 0;
	}

	g_activePageId = index;

	g_activePage = createPageFromId(g_activePageId);

	if (g_activePage) {
		g_activePage->pageWillAppear();
	}

    g_showPageTime = micros();
	g_timeOfLastActivity = millis();
    refreshPage();
}

void showPage(int index, bool pushOnStack) {
	if (g_lastActivePageId != -1) {
		g_activePageId = g_lastActivePageId;
		g_activePage = g_lastActivePage;
		g_pushActivePageOnStack = g_lastPushActivePageOnStack;

		g_lastActivePageId = -1;
	}

	if (index == PAGE_ID_MAIN) {
		g_pageNavigationStackPointer = -1;
	} else if (g_activePageId == PAGE_ID_MAIN) {
		g_pageNavigationStackPointer = 0;
	} else if (g_pageNavigationStackPointer >= 0 && g_pushActivePageOnStack) {
		if (g_pageNavigationStackPointer == CONF_GUI_PAGE_NAVIGATION_STACK_SIZE) {
			for (int i = 1; i < CONF_GUI_PAGE_NAVIGATION_STACK_SIZE; ++i) {
				g_pageNavigationStack[i - 1] = g_pageNavigationStack[i];
			}
			--g_pageNavigationStackPointer;
		}
		g_pageNavigationStack[g_pageNavigationStackPointer++] = g_activePageId;
	}

	g_pushActivePageOnStack = pushOnStack;

	doShowPage(index);
}

void showAuxPage(int index) {
	g_lastActivePageId = g_activePageId;
	g_lastActivePage = g_activePage;
	g_lastPushActivePageOnStack = g_pushActivePageOnStack;

    g_activePageId = index;
	g_activePage = 0;
    
	refreshPage();
}

void showPreviousPage() {
	if (g_lastActivePageId != -1) {
		g_activePageId = g_lastActivePageId;
		g_activePage = g_lastActivePage;
		g_pushActivePageOnStack = g_lastPushActivePageOnStack;
		
		g_lastActivePageId = -1;

		refreshPage();
	} else if (g_pageNavigationStackPointer >= 0) {
		--g_pageNavigationStackPointer;
		if (g_pageNavigationStackPointer >= 0) {
			g_pushActivePageOnStack = true;
			doShowPage(g_pageNavigationStack[g_pageNavigationStackPointer]);
		} else {
			doShowPage(PAGE_ID_MAIN);
		}
	}
}

void showWelcomePage() {
    showPage(PAGE_ID_WELCOME);
    flush();
}

void showSelfTestResultPage() {
    showPage(PAGE_ID_SELF_TEST_RESULT);
    flush();
}

void showStandbyPage() {
    showPage(PAGE_ID_STANDBY);
    flush();
}

void showEnteringStandbyPage() {
    showPage(PAGE_ID_ENTERING_STANDBY);
    flush();
}

////////////////////////////////////////////////////////////////////////////////

void dialogYes() {
	if (g_dialogYesCallback) {
		g_dialogYesCallback();
	}
}

void dialogNo() {
	if (g_dialogNoCallback) {
		g_dialogNoCallback();
	}
}

void dialogCancel() {
	if (g_dialogCancelCallback) {
		g_dialogCancelCallback();
	}
}

void dialogOk() {
	showPreviousPage();

	if (g_dialogYesCallback) {
		g_dialogYesCallback();
	}
}

void alertMessage(int alertPageId, data::Value message, void (*ok_callback)()) {
    data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE, message, 0);

    g_dialogYesCallback = ok_callback;

	showAuxPage(alertPageId);

	if (alertPageId == PAGE_ID_ERROR_ALERT) {
		sound::playBeep();
	}
}

void infoMessage(data::Value value, void (*ok_callback)()) {
	alertMessage(PAGE_ID_INFO_ALERT, value, ok_callback);
}

void infoMessageP(const char *message PROGMEM, void (*ok_callback)()) {
	alertMessage(PAGE_ID_INFO_ALERT, data::Value::ProgmemStr(message), ok_callback);
}

void errorMessage(data::Value value, void (*ok_callback)()) {
	alertMessage(PAGE_ID_ERROR_ALERT, value, ok_callback);
}

void errorMessageP(const char *message PROGMEM, void (*ok_callback)()) {
	alertMessage(PAGE_ID_ERROR_ALERT, data::Value::ProgmemStr(message), ok_callback);
}

void yesNoDialog(int yesNoPageId, const char *message PROGMEM, void (*yes_callback)(), void (*no_callback)(), void (*cancel_callback)()) {
    data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE, data::Value::ProgmemStr(message), 0);

    g_dialogYesCallback = yes_callback;
    g_dialogNoCallback = no_callback;
    g_dialogCancelCallback = cancel_callback;

	showPage(yesNoPageId);
}

////////////////////////////////////////////////////////////////////////////////

void selectChannel() {
	g_channel = &Channel::get(g_foundWidgetAtDown.cursor.iChannel);
}

////////////////////////////////////////////////////////////////////////////////

static int find_widget_at_x;
static int find_widget_at_y;
static WidgetCursor found_widget;

bool find_widget_step(const WidgetCursor &widgetCursor, bool refresh) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    bool inside = 
        find_widget_at_x >= widgetCursor.x &&
        find_widget_at_x < widgetCursor.x + (int)widget->w &&
        find_widget_at_y >= widgetCursor.y &&
        find_widget_at_y < widgetCursor.y + (int)widget->h;

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
    EnumWidgets enum_widgets(find_widget_step);
    DECL_WIDGET(page, getPageOffset(g_activePageId));
    enum_widgets.start(g_activePageId, page->x, page->y, true);
    enum_widgets.next();
}

////////////////////////////////////////////////////////////////////////////////

void select_widget(WidgetCursor &widget_cursor) {
    g_selectedWidget = widget_cursor;
    refresh_widget(g_selectedWidget);
}

void deselect_widget() {
    WidgetCursor old_selected_widget = g_selectedWidget;
    g_selectedWidget = 0;
    refresh_widget(old_selected_widget);
}

////////////////////////////////////////////////////////////////////////////////

void do_action(int action_id) {
	actions[action_id]();
}

////////////////////////////////////////////////////////////////////////////////

void standbyTouchHandling(unsigned long tick_usec) {
    // touch handling in power off:
    // wait for long press anywhere on the screen and then turn power on
    if (touch::event_type == touch::TOUCH_DOWN) {
        g_touchDownTime = tick_usec;
        g_touchActionExecuted = false;
    } else if (touch::event_type == touch::TOUCH_MOVE) {
        if (tick_usec - g_touchDownTime >= CONF_GUI_LONG_PRESS_TIMEOUT) {
            if (!g_touchActionExecuted) {
                g_touchActionExecuted = true;
                psu::changePowerState(true);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void init() {
    lcd::init();

#ifdef EEZ_PSU_SIMULATOR
    if (persist_conf::dev_conf.gui_opened) {
        simulator::front_panel::open();
    }
#endif

    touch::init();

    g_activePageId = -1;
	g_activePage = 0;

#if defined(EEZ_PSU_ARDUINO_MEGA)
    arduino_util::prog_read_buffer(document, (uint8_t *)&g_docBuffer, sizeof(Document));
    g_doc = &g_docBuffer;
#else
    g_doc = (Document *)document;
#endif
}

int getStartPageId() {
	return devices::anyFailedOrWarning() ? PAGE_ID_SELF_TEST_RESULT: PAGE_ID_MAIN;
}

void tick(unsigned long tick_usec) {
#ifdef EEZ_PSU_SIMULATOR
    if (!simulator::front_panel::isOpened()) {
        return;
    }
#endif

    touch::tick(tick_usec);

    if (g_activePageId == -1) {
        standbyTouchHandling(tick_usec);
        return;
    }

    // wait some time for transitional pages
    if (g_activePageId == PAGE_ID_STANDBY && tick_usec - g_showPageTime < CONF_GUI_STANDBY_PAGE_TIMEOUT) {
        standbyTouchHandling(tick_usec);
        return;
    } else if (g_activePageId == PAGE_ID_ENTERING_STANDBY && tick_usec - g_showPageTime < CONF_GUI_ENTERING_STANDBY_PAGE_TIMEOUT) {
        if (!psu::isPowerUp()) {
            unsigned long saved_showPageTime = g_showPageTime;
            showStandbyPage();
            g_showPageTime = saved_showPageTime - (CONF_GUI_STANDBY_PAGE_TIMEOUT - CONF_GUI_ENTERING_STANDBY_PAGE_TIMEOUT);
        }
        return;
    } else if (g_activePageId == PAGE_ID_WELCOME && tick_usec - g_showPageTime < CONF_GUI_WELCOME_PAGE_TIMEOUT) {
        return;
    }

    // turn the screen off if power is down
    if (!psu::isPowerUp()) {
        standbyTouchHandling(tick_usec);
        g_activePageId = -1;
		g_activePage = 0;
        turnOff();
        return;
    }

	if (touch::calibration::isCalibrating()) {
		touch::calibration::tick(tick_usec);
		return;
	}

    // select page to go after transitional page
    if (g_activePageId == PAGE_ID_WELCOME || g_activePageId == PAGE_ID_STANDBY || g_activePageId == PAGE_ID_ENTERING_STANDBY) {
		if (!touch::calibration::isCalibrated()) {
			// touch screen is not calibrated
			showPage(PAGE_ID_SCREEN_CALIBRATION_INTRO);
		} else {
			showPage(getStartPageId());
		}
        return;
    }

    // touch handling
	if (touch::event_type != touch::TOUCH_NONE) {
		g_timeOfLastActivity = millis();

		if (touch::event_type == touch::TOUCH_DOWN) {
			if (g_activePageId == PAGE_ID_SCREEN_CALIBRATION_INTRO) {
				return;
			}

			g_touchDownTime = tick_usec;
			g_touchActionExecuted = false;
			find_widget(touch::x, touch::y);
			DECL_WIDGET(widget, found_widget.widgetOffset);
			if (found_widget && widget->action) {
				g_foundWidgetAtDown = found_widget;
			} else {
				g_foundWidgetAtDown = 0;
			}
			if (g_foundWidgetAtDown) {
				select_widget(g_foundWidgetAtDown);
			} else {
				DECL_WIDGET(widget, found_widget.widgetOffset);
				if (found_widget && widget->type == WIDGET_TYPE_BUTTON_GROUP) {
					widget_button_group::onTouchDown(found_widget);
				} else if (g_activePageId == PAGE_ID_EDIT_MODE_SLIDER) {
					edit_mode_slider::onTouchDown();
				} else if (g_activePageId == PAGE_ID_EDIT_MODE_STEP) {
					edit_mode_step::onTouchDown();
				}
			}
		} else if (touch::event_type == touch::TOUCH_MOVE) {
			if (g_activePageId == PAGE_ID_SCREEN_CALIBRATION_INTRO) {
				return;
			}

			if (g_foundWidgetAtDown) {
				DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
				if (widget->action == ACTION_ID_TURN_OFF) {
					if (tick_usec - g_touchDownTime >= CONF_GUI_LONG_PRESS_TIMEOUT) {
						if (!g_touchActionExecuted) {
							deselect_widget();
							g_foundWidgetAtDown = 0;
							g_touchActionExecuted = true;
							psu::changePowerState(false);
						}
					}
				}
			} else {
				if (g_activePageId == PAGE_ID_EDIT_MODE_SLIDER) {
					edit_mode_slider::onTouchMove();
				} else if (g_activePageId == PAGE_ID_EDIT_MODE_STEP) {
					edit_mode_step::onTouchMove();
				} else if (g_activePageId == PAGE_ID_YES_NO || g_activePageId == PAGE_ID_YES_NO_CANCEL) {
	#ifdef CONF_DEBUG
					int x = touch::x;
					if (x < 1) x = 1;
					else if (x > lcd::lcd.getDisplayXSize() - 2) x = lcd::lcd.getDisplayXSize() - 2;

					int y = touch::y;
					if (y < 1) y = 1;
					else if (y > lcd::lcd.getDisplayYSize() - 2) y = lcd::lcd.getDisplayYSize() - 2;

					lcd::lcd.setColor(VGA_WHITE);
					lcd::lcd.fillRect(touch::x-1, touch::y-1, touch::x+1, touch::y+1);
	#endif
				}
			}
		} else if (touch::event_type == touch::TOUCH_UP) {
			if (g_activePageId == PAGE_ID_SCREEN_CALIBRATION_INTRO) {
				touch::calibration::enterCalibrationMode(PAGE_ID_YES_NO, getStartPageId());
				return;
			}

			if (g_foundWidgetAtDown) {
				deselect_widget();
				DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
				do_action(widget->action);
				g_foundWidgetAtDown = 0;
			} else {
				if (g_activePageId == PAGE_ID_EDIT_MODE_SLIDER) {
					edit_mode_slider::onTouchUp();
				} else if (g_activePageId == PAGE_ID_EDIT_MODE_STEP) {
					edit_mode_step::onTouchUp();
				}
			}
		}
	}

	//
	unsigned long inactivityPeriod = millis() - g_timeOfLastActivity;
	if (g_activePageId == PAGE_ID_EVENT_QUEUE) {
		if (inactivityPeriod >= 10 * 1000UL) {
			showPage(PAGE_ID_MAIN);
		}
	} else if (g_activePageId == PAGE_ID_SCREEN_CALIBRATION_INTRO) {
		if (inactivityPeriod >= 20 * 1000UL) {
			touch::calibration::enterCalibrationMode(PAGE_ID_YES_NO, getStartPageId());
			return;
		}
	}

    // update screen
    draw_tick();
}

}
}
} // namespace eez::psu::gui
