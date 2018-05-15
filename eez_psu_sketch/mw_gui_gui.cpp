/*
 * EEZ Middleware
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

#include "mw_mw.h"

#if OPTION_DISPLAY

#include "mw_gui_gui.h"
#include "mw_gui_widget_button_group.h"
#include "mw_util.h"

#define CONF_GUI_PAGE_NAVIGATION_STACK_SIZE 5
#define CONF_GUI_LONG_TOUCH_TIMEOUT 1000000L // 1s
#define CONF_GUI_KEYPAD_AUTO_REPEAT_DELAY 250000L // 250ms
#define CONF_GUI_EXTRA_LONG_TOUCH_TIMEOUT 30000000L // 30s
#define MAX_EVENTS 16

namespace eez {
namespace mw {
namespace gui {

using namespace lcd;

////////////////////////////////////////////////////////////////////////////////

Styles *g_styles;
Document *g_document;

static int g_activePageId;
static Page *g_activePage;

static int g_previousPageId;

static struct Event {
    int activePageId;
    Page *activePage;
} g_pageNavigationStack[CONF_GUI_PAGE_NAVIGATION_STACK_SIZE];
static int g_pageNavigationStackPointer = 0;

WidgetCursor g_foundWidgetAtDown;

uint32_t g_showPageTime;
static bool g_touchActionExecuted;
static bool g_touchActionExecutedAtDown;

////////////////////////////////////////////////////////////////////////////////

static uint32_t g_touchDownTime;
static uint32_t g_lastAutoRepeatEventTime;
static bool g_longTouchGenerated;
static bool g_extraLongTouchGenerated;

enum EventType {
    EVENT_TYPE_TOUCH_DOWN,
    EVENT_TYPE_TOUCH_MOVE,
    EVENT_TYPE_LONG_TOUCH,
	EVENT_TYPE_EXTRA_LONG_TOUCH,
	EVENT_TYPE_FAST_AUTO_REPEAT,
    EVENT_TYPE_AUTO_REPEAT,
	EVENT_TYPE_TOUCH_UP
};

static int g_numEvents;
static struct {
    EventType type;
    int x;
    int y;
} g_events[MAX_EVENTS];

////////////////////////////////////////////////////////////////////////////////

bool isActivePageInternal() {
    return g_activePageId < INTERNAL_PAGE_ID_NONE;
}

////////////////////////////////////////////////////////////////////////////////

void action_internal_select_enum_item() {
	((SelectFromEnumPage *)getActivePage())->selectEnumItem();
}

void action_internal_show_previous_page() {
	popPage();
}

static ActionExecFunc g_internalActionExecFunctions[] = {
	0,
	action_internal_select_enum_item,
	action_internal_show_previous_page
};

bool isInternalAction(int actionId) {
	return actionId < 0;
}

int getAction(WidgetCursor &widgetCursor) {
	if (isActivePageInternal()) {
		return ((InternalPage *)g_activePage)->getAction(widgetCursor);
	} else {
		DECL_WIDGET(widget, widgetCursor.widgetOffset);
		return widget->action;
	}
}

void executeInternalAction(int actionId) {
	g_internalActionExecFunctions[-actionId]();
}

void executeAction(int actionId) {
	if (isInternalAction(actionId)) {
		executeInternalAction(actionId);
	} else {
		executeUserActionHook(actionId);
	}
	playClickSound();
}

////////////////////////////////////////////////////////////////////////////////

int getActivePageId() {
    return g_activePageId;
}

bool isActivePage(int pageId) {
    return pageId == g_activePageId;
}

Page *getActivePage() {
    return g_activePage;
}

int getPreviousPageId() {
    return g_previousPageId;
}

Page *getPreviousPage() {
    if (g_pageNavigationStackPointer > 0) {
        return g_pageNavigationStack[g_pageNavigationStackPointer - 1].activePage;
    } else {
        return 0;
    }
}

void doShowPage(int index, Page *page = 0) {
    // delete current page
    if (g_activePage) {
        delete g_activePage;
    }

    g_previousPageId = g_activePageId;
    g_activePageId = index;

    if (page) {
        g_activePage = page;
    } else {
        g_activePage = createPageFromId(g_activePageId);
    }

    if (g_activePage) {
        g_activePage->pageWillAppear();
    }

    g_showPageTime = micros();

	onPageChanged();

    refreshPage();
}

void setPage(int pageId) {
    // delete stack
    for (int i = 0; i < g_pageNavigationStackPointer; ++i) {
        if (g_pageNavigationStack[i].activePage) {
            delete g_pageNavigationStack[i].activePage;
        }
    }
    g_pageNavigationStackPointer = 0;

    //
    doShowPage(pageId);
}

void replacePage(int pageId, Page *page) {
    doShowPage(pageId, page);
}

void pushPage(int pageId, Page *page) {
    // push current page on stack
    if (g_activePageId != INTERNAL_PAGE_ID_NONE) {
        if (g_pageNavigationStackPointer == CONF_GUI_PAGE_NAVIGATION_STACK_SIZE) {
            // no more space on the stack

            // delete page on the bottom
            if (g_pageNavigationStack[0].activePage) {
                delete g_pageNavigationStack[0].activePage;
            }

            // move stack one down
            for (int i = 1; i < g_pageNavigationStackPointer; ++i) {
                g_pageNavigationStack[i-1].activePageId = g_pageNavigationStack[i].activePageId;
                g_pageNavigationStack[i-1].activePage = g_pageNavigationStack[i].activePage;
            }

            --g_pageNavigationStackPointer;
        }

        g_pageNavigationStack[g_pageNavigationStackPointer].activePageId = g_activePageId;
        g_pageNavigationStack[g_pageNavigationStackPointer].activePage = g_activePage;
        g_activePage = 0;
        ++g_pageNavigationStackPointer;
    }

    doShowPage(pageId, page);
}

void popPage() {
    if (g_pageNavigationStackPointer > 0) {
        --g_pageNavigationStackPointer;

        doShowPage(g_pageNavigationStack[g_pageNavigationStackPointer].activePageId,
            g_pageNavigationStack[g_pageNavigationStackPointer].activePage);
    } else {
        doShowPage(getMainPageId());
    }
}

bool isPageActiveOrOnStack(int pageId) {
    if (g_activePageId == pageId) {
        return true;
    }

    for (int i = 0; i < g_pageNavigationStackPointer; ++i) {
        if (g_pageNavigationStack[i].activePageId == pageId) {
            return true;
        }
    }
    return false;
}

void showPage(int pageId) {
	if (g_activePageId != pageId) {
		setPage(pageId);
		flush();
	}
}

void pushSelectFromEnumPage(const data::EnumItem *enumDefinition, uint8_t currentValue, bool (*disabledCallback)(uint8_t value), void (*onSet)(uint8_t)) {
    pushPage(INTERNAL_PAGE_ID_SELECT_FROM_ENUM, new SelectFromEnumPage(enumDefinition, currentValue, disabledCallback, onSet));
}

bool isWidgetActionEnabled(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    if (widget->action) {

		bool result;
		if (isWidgetActionEnabledHook(widget, result)) {
			return result;
		}

        if (widget->type == WIDGET_TYPE_BUTTON) {
            DECL_WIDGET_SPECIFIC(ButtonWidget, buttonWidget, widget);
            if (!data::get(widgetCursor.cursor, buttonWidget->enabled).getInt()) {
                return false;
            }
        }

        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////

void pushEvent(EventType type) {
    // allow only one EVENT_TYPE_FAST_AUTO_REPEAT in the stack
    if (type == EVENT_TYPE_FAST_AUTO_REPEAT) {
        for (int i = 0; i < g_numEvents; ++i) {
            if (g_events[i].type == EVENT_TYPE_FAST_AUTO_REPEAT) {
                return;
            }
        }
    }

    // allow only one EVENT_TYPE_AUTO_REPEAT in the stack
    if (type == EVENT_TYPE_AUTO_REPEAT) {
        for (int i = 0; i < g_numEvents; ++i) {
            if (g_events[i].type == EVENT_TYPE_AUTO_REPEAT) {
                return;
            }
        }
    }

    // ignore EVENT_TYPE_TOUCH_MOVE if it is the same as the last event
    if (type == EVENT_TYPE_TOUCH_MOVE) {
        if (g_numEvents > 0 && g_events[g_numEvents-1].type == EVENT_TYPE_TOUCH_MOVE &&
            g_events[g_numEvents-1].x == touch::g_x && g_events[g_numEvents-1].y == touch::g_y) {
            return;
        }
    }

    // if events stack is full then remove oldest EVENT_TYPE_TOUCH_MOVE
    if (g_numEvents == MAX_EVENTS) {
        for (int i = 0; i < g_numEvents; ++i) {
            if (g_events[i].type == EVENT_TYPE_TOUCH_MOVE) {
                for (int j = i + 1; j < g_numEvents; ++j) {
                    g_events[j - 1] = g_events[j];
                }
                --g_numEvents;
                break;
            }
        }
    }

    if (g_numEvents < MAX_EVENTS) {
        // push new event on the stack
        g_events[g_numEvents].type = type;
        g_events[g_numEvents].x = touch::g_x;
        g_events[g_numEvents].y = touch::g_y;
        ++g_numEvents;
    }
}

void processEvents();

void gui_touchHandling(uint32_t tick_usec) {
    if (touch::g_eventType == touch::TOUCH_DOWN) {
        g_touchDownTime = tick_usec;
        g_lastAutoRepeatEventTime = tick_usec;
        g_longTouchGenerated = false;
		g_extraLongTouchGenerated = false;
        pushEvent(EVENT_TYPE_TOUCH_DOWN);
    } else if (touch::g_eventType == touch::TOUCH_MOVE) {
        pushEvent(EVENT_TYPE_TOUCH_MOVE);

        if (!g_longTouchGenerated && int32_t(tick_usec - g_touchDownTime) >= CONF_GUI_LONG_TOUCH_TIMEOUT) {
            g_longTouchGenerated = true;
            pushEvent(EVENT_TYPE_LONG_TOUCH);
        }

		if (g_longTouchGenerated && !g_extraLongTouchGenerated && int32_t(tick_usec - g_touchDownTime) >= CONF_GUI_EXTRA_LONG_TOUCH_TIMEOUT) {
			g_extraLongTouchGenerated = true;
			pushEvent(EVENT_TYPE_EXTRA_LONG_TOUCH);
		}

        if (int32_t(tick_usec - g_lastAutoRepeatEventTime) >= CONF_GUI_KEYPAD_AUTO_REPEAT_DELAY) {
            pushEvent(EVENT_TYPE_AUTO_REPEAT);
            g_lastAutoRepeatEventTime = tick_usec;
        }

    } else if (touch::g_eventType == touch::TOUCH_UP) {
        pushEvent(EVENT_TYPE_TOUCH_UP);
    }
}

void processEvents() {
    for (int i = 0; i < g_numEvents; ++i) {
        if (g_events[i].type == EVENT_TYPE_TOUCH_DOWN) {
            g_touchActionExecuted = false;
            g_touchActionExecutedAtDown = false;
            WidgetCursor foundWidget = findWidget(g_events[i].x, g_events[i].y);
            g_foundWidgetAtDown = 0;
            if (foundWidget) {
                if (isActivePageInternal()) {
                    g_foundWidgetAtDown = foundWidget;
                } else {
                    if (isWidgetActionEnabled(foundWidget)) {
                        g_foundWidgetAtDown = foundWidget;
                    }
                }

                if (g_foundWidgetAtDown) {
					int action = getAction(g_foundWidgetAtDown);
                    if (testExecuteActionOnTouchDownHook(action)) {
                        executeAction(action);
                        g_touchActionExecutedAtDown = true;
                    } else {
                        selectWidget(g_foundWidgetAtDown);
                    }
                } else {
                    if (!isActivePageInternal()) {
                        DECL_WIDGET(widget, foundWidget.widgetOffset);
						if (foundWidget && widget->type == WIDGET_TYPE_BUTTON_GROUP) {
                            widgetButtonGroup::onTouchDown(foundWidget);
                        } else {
							onTouchDownHook(foundWidget, g_events[i].x, g_events[i].y);
						}
                    }
                }
            }
        } else if (g_events[i].type == EVENT_TYPE_TOUCH_MOVE) {
            if (!g_foundWidgetAtDown) {
				onTouchMoveHook(g_events[i].x, g_events[i].y);
            }
        } else if (g_events[i].type == EVENT_TYPE_LONG_TOUCH) {
            if (!g_touchActionExecuted) {
				if (!g_foundWidgetAtDown || !isAutoRepeatActionHook(getAction(g_foundWidgetAtDown))) {
					if (onLongTouchHook()) {
						g_touchActionExecuted = true;
					}
				}
            }
		} else if (g_events[i].type == EVENT_TYPE_EXTRA_LONG_TOUCH) {
			if (!g_touchActionExecuted) {
				if (!g_foundWidgetAtDown || !isAutoRepeatActionHook(getAction(g_foundWidgetAtDown))) {
					if (onExtraLongTouchHook()) {
						g_touchActionExecuted = true;
					}
				}
			}
		} else if (g_events[i].type == EVENT_TYPE_AUTO_REPEAT) {
            if (g_foundWidgetAtDown) {
                int action = getAction(g_foundWidgetAtDown);
                if (isAutoRepeatActionHook(action)) {
                    g_touchActionExecuted = true;
                    executeAction(action);
                }
            }
        } else if (g_events[i].type == EVENT_TYPE_TOUCH_UP) {
            if (g_foundWidgetAtDown) {
                if (!g_touchActionExecutedAtDown) {
                    deselectWidget();
                    if (!g_touchActionExecuted) {
                        int action = getAction(g_foundWidgetAtDown);
                        executeAction(action);
                    }
                }
                g_foundWidgetAtDown = 0;
            } else {
                if (!onTouchUpHook()) {
                    DECL_WIDGET(page, getPageOffset(getActivePageId()));
                    DECL_WIDGET_SPECIFIC(PageWidget, pageSpecific, page);
                    if (pageSpecific->closePageIfTouchedOutside) {
                        if (!pointInsideRect(g_events[i].x, g_events[i].y, page->x, page->y, page->w, page->h)) {
                            popPage();
                        }
                    }
                }
            }
        }
    }

    g_numEvents = 0;
}

void gui_init() {
	g_activePageId = INTERNAL_PAGE_ID_NONE;
	g_activePage = 0;

	g_styles = (Styles *)styles;
	g_document = (Document *)document;
}

void gui_tick(uint32_t tick_usec) {
    processEvents();
    drawTick();
}


}
}
} // namespace eez::mw::gui

#endif
