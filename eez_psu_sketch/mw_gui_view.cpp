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

#include "mw_gui_widget/bar_graph.h"
#include "mw_gui_widget/bitmap.h"
#include "mw_gui_widget/button.h"
#include "mw_gui_widget/button_group.h"
#include "mw_gui_widget/container.h"
#include "mw_gui_widget/custom.h"
#include "mw_gui_widget/display_data.h"
#include "mw_gui_widget/list.h"
#include "mw_gui_widget/list_graph.h"
#include "mw_gui_widget/multiline_text.h"
#include "mw_gui_widget/rectangle.h"
#include "mw_gui_widget/scale.h"
#include "mw_gui_widget/select.h"
#include "mw_gui_widget/text.h"
#include "mw_gui_widget/toggle_button.h"
#include "mw_gui_widget/up_down.h"
#include "mw_gui_widget/yt_graph.h"

#define CONF_GUI_ENUM_WIDGETS_STACK_SIZE 5
#define CONF_GUI_BLINK_TIME 400000UL // 400ms
#define CONF_MAX_STATE_SIZE 4096

namespace eez {
namespace mw {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

static WidgetCursor g_selectedWidget;
static bool g_isBlinkTime;
static bool g_wasBlinkTime;

static uint8_t g_stateBuffer[2][CONF_MAX_STATE_SIZE];
static WidgetState *g_previousState;
static WidgetState *g_currentState;

static bool g_refreshPageOnNextTick;

static int g_findWidgetAtX;
static int g_findWidgetAtY;
static WidgetCursor g_foundWidget;

////////////////////////////////////////////////////////////////////////////////

typedef void (*DrawFunctionType)(int pageId, const WidgetCursor &widgetCursor);
static DrawFunctionType g_drawWidgetFunctions[] = {
	NULL, // WIDGET_TYPE_NONE
	NULL, // WIDGET_TYPE_CONTAINER
	NULL, // WIDGET_TYPE_LIST
	NULL, // WIDGET_TYPE_SELECT
	DisplayDataWidget_draw, // WIDGET_TYPE_DISPLAY_DATA
	TextWidget_draw, // WIDGET_TYPE_TEXT
	MultilineTextWidget_draw, // WIDGET_TYPE_MULTILINE_TEXT
	RectangleWidget_draw, // WIDGET_TYPE_RECTANGLE
	BitmapWidget_draw, // WIDGET_TYPE_BITMAP
	ButtonWidget_draw, // WIDGET_TYPE_BUTTON
	ToggleButtonWidget_draw, // WIDGET_TYPE_TOGGLE_BUTTON
	ButtonGroupWidget_draw, // WIDGET_TYPE_BUTTON_GROUP
	ScaleWidget_draw, // WIDGET_TYPE_SCALE
	BarGraphWidget_draw, // WIDGET_TYPE_BAR_GRAPH
	NULL, // WIDGET_TYPE_CUSTOM
	YTGraphWidget_draw, // WIDGET_TYPE_YT_GRAPH
	UpDownWidget_draw, // WIDGET_TYPE_UP_DOWN
	ListGraphWidget_draw, // WIDGET_TYPE_LIST_GRAPH
};

extern OnTouchDownFunctionType g_onTouchDownFunctions[] = {
	NULL, // WIDGET_TYPE_NONE
	NULL, // WIDGET_TYPE_CONTAINER
	NULL, // WIDGET_TYPE_LIST
	NULL, // WIDGET_TYPE_SELECT
	NULL, // WIDGET_TYPE_DISPLAY_DATA
	NULL, // WIDGET_TYPE_TEXT
	NULL, // WIDGET_TYPE_MULTILINE_TEXT
	NULL, // WIDGET_TYPE_RECTANGLE
	NULL, // WIDGET_TYPE_BITMAP
	NULL, // WIDGET_TYPE_BUTTON
	NULL, // WIDGET_TYPE_TOGGLE_BUTTON
	ButtonGroupWidget_onTouchDown, // WIDGET_TYPE_BUTTON_GROUP
	NULL, // WIDGET_TYPE_SCALE
	NULL, // WIDGET_TYPE_BAR_GRAPH
	NULL, // WIDGET_TYPE_CUSTOM
	NULL, // WIDGET_TYPE_YT_GRAPH
	NULL, // WIDGET_TYPE_UP_DOWN
	listGraphWidget_onTouchDown, // WIDGET_TYPE_LIST_GRAPH
};

extern OnTouchMoveFunctionType g_onTouchMoveFunctions[] = {
	NULL, // WIDGET_TYPE_NONE
	NULL, // WIDGET_TYPE_CONTAINER
	NULL, // WIDGET_TYPE_LIST
	NULL, // WIDGET_TYPE_SELECT
	NULL, // WIDGET_TYPE_DISPLAY_DATA
	NULL, // WIDGET_TYPE_TEXT
	NULL, // WIDGET_TYPE_MULTILINE_TEXT
	NULL, // WIDGET_TYPE_RECTANGLE
	NULL, // WIDGET_TYPE_BITMAP
	NULL, // WIDGET_TYPE_BUTTON
	NULL, // WIDGET_TYPE_TOGGLE_BUTTON
	NULL, // WIDGET_TYPE_BUTTON_GROUP
	NULL, // WIDGET_TYPE_SCALE
	NULL, // WIDGET_TYPE_BAR_GRAPH
	NULL, // WIDGET_TYPE_CUSTOM
	NULL, // WIDGET_TYPE_YT_GRAPH
	NULL, // WIDGET_TYPE_UP_DOWN
	listGraphWidget_onTouchMove, // WIDGET_TYPE_LIST_GRAPH
};

////////////////////////////////////////////////////////////////////////////////

int getCurrentStateBufferIndex() {
	return (uint8_t *)g_currentState == &g_stateBuffer[0][0] ? 0 : 1;
}

void drawWidget(int pageId, const WidgetCursor &widgetCursor_) {
	if (pageId != getActivePageId()) {
		return;
	}

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

	if (g_drawWidgetFunctions[widget->type]) {
		g_drawWidgetFunctions[widget->type](pageId, widgetCursor);
	}
}

void refreshWidget(WidgetCursor widgetCursor) {
	if (isActivePageInternal()) {
		((InternalPage *)getActivePage())->drawWidget(widgetCursor, widgetCursor == g_selectedWidget);
	} else {
		drawWidget(getActivePageId(), widgetCursor);
	}
}

WidgetCursor& getSelectedWidget() {
	return g_selectedWidget;
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

bool isBlinkTime() {
	return g_isBlinkTime;
}

////////////////////////////////////////////////////////////////////////////////

WidgetState *next(WidgetState *p) {
	return p ? (WidgetState *)(((uint8_t *)p) + p->size) : 0;
}

void enumWidget(int pageId, OBJ_OFFSET widgetOffset, int x, int y, data::Cursor &cursor, WidgetState *previousState, WidgetState *currentState, EnumWidgetsCallback callback);

void enumContainer(int pageId, List widgets, int x, int y, data::Cursor &cursor, WidgetState *previousState, WidgetState *currentState, EnumWidgetsCallback callback) {
	if (pageId != getActivePageId()) {
		return;
	}

	WidgetState *savedCurrentState = currentState;

	WidgetState *endOfContainerInPreviousState = 0;
	if (previousState) endOfContainerInPreviousState = next(previousState);

	// move to the first child widget state
	if (previousState) ++previousState;
	if (currentState) ++currentState;

	for (int index = 0; index < widgets.count; ++index) {
		OBJ_OFFSET childWidgetOffset = getListItemOffset(widgets, index, sizeof(Widget));
		enumWidget(pageId, childWidgetOffset, x, y, cursor, previousState, currentState, callback);

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

void enumWidget(int pageId, OBJ_OFFSET widgetOffset, int x, int y, data::Cursor &cursor, WidgetState *previousState, WidgetState *currentState, EnumWidgetsCallback callback) {
	if (!executeCriticalTasks(pageId)) {
		return;
	}

	DECL_WIDGET(widget, widgetOffset);

	x += widget->x;
	y += widget->y;

	if (widget->type == WIDGET_TYPE_CONTAINER) {
		DECL_WIDGET_SPECIFIC(ContainerWidget, container, widget);
		enumContainer(pageId, container->widgets, x, y, cursor, previousState, currentState, callback);
	} else if (widget->type == WIDGET_TYPE_CUSTOM) {
		DECL_WIDGET_SPECIFIC(CustomWidgetSpecific, customWidgetSpecific, widget);
		DECL_CUSTOM_WIDGET(customWidget, customWidgetSpecific->customWidget);
		enumContainer(pageId, customWidget->widgets, x, y, cursor, previousState, currentState, callback);
	} else if (widget->type == WIDGET_TYPE_LIST) {
		WidgetState *savedCurrentState = currentState;

		WidgetState *endOfContainerInPreviousState = 0;
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
					enumWidget(pageId, childWidgetOffset, x + xOffset, y + yOffset, cursor, previousState, currentState, callback);
					yOffset += childWidget->h;
				} else {
					// TODO: add vertical scroll
					break;
				}
			} else {
				DECL_WIDGET(childWidget, childWidgetOffset);
				if (xOffset < widget->w) {
					enumWidget(pageId, childWidgetOffset, x + xOffset, y + yOffset, cursor, previousState, currentState, callback);
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
	} else if (widget->type == WIDGET_TYPE_SELECT) {
		data::Value indexValue = data::get(cursor, widget->data);
		if (indexValue.getType() == VALUE_TYPE_NONE) {
			indexValue = data::Value(0);
		}

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

		enumWidget(pageId, selectedWidgetOffset, x, y, cursor, previousState, currentState, callback);

		if (currentState) {
			savedCurrentState->size = sizeof(WidgetState) + currentState->size;
		}
	} else {
		callback(pageId, WidgetCursor(widgetOffset, x, y, cursor, previousState, currentState));
	}
}

void enumWidgets(int pageId, WidgetState *previousState, WidgetState *currentState, EnumWidgetsCallback callback) {
	data::Cursor cursor;
	enumWidget(pageId, getPageOffset(pageId), 0, 0, cursor, previousState, currentState, callback);
}

void enumWidgets(int pageId, EnumWidgetsCallback callback) {
	enumWidgets(pageId, 0, 0, callback);
}

////////////////////////////////////////////////////////////////////////////////

void clearBackground() {
	// clear screen with background color
	DECL_WIDGET(page, getPageOffset(getActivePageId()));

	DECL_WIDGET_STYLE(style, page);
	lcd::setColor(style->background_color);

	DECL_WIDGET_SPECIFIC(PageWidget, pageSpecific, page);
	for (int i = 0; i < pageSpecific->transparentRectangles.count; ++i) {
		OBJ_OFFSET rectOffset = getListItemOffset(pageSpecific->transparentRectangles, i, sizeof(Rect));
		DECL_STRUCT_WITH_OFFSET(Rect, rect, rectOffset);
		lcd::fillRect(rect->x, rect->y, rect->x + rect->w - 1, rect->y + rect->h - 1);
	}
}

void drawActivePage(bool refresh) {
	if (refresh) {
		g_previousState = 0;
		g_currentState = (WidgetState *)(&g_stateBuffer[0][0]);
	} else {
		g_previousState = g_currentState;
		g_currentState = (WidgetState *)(&g_stateBuffer[getCurrentStateBufferIndex() == 0 ? 1 : 0][0]);
	}

	enumWidgets(getActivePageId(), g_previousState, g_currentState, drawWidget);
}

void drawTick() {
	g_wasBlinkTime = g_isBlinkTime;
	g_isBlinkTime = (micros() % (2 * CONF_GUI_BLINK_TIME)) > CONF_GUI_BLINK_TIME && touch::g_eventType == touch::TOUCH_NONE;

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

////////////////////////////////////////////////////////////////////////////////

void findWidgetStep(int pageId, const WidgetCursor &widgetCursor) {
	DECL_WIDGET(widget, widgetCursor.widgetOffset);

	bool inside =
		g_findWidgetAtX >= widgetCursor.x &&
		g_findWidgetAtX < widgetCursor.x + (int)widget->w &&
		g_findWidgetAtY >= widgetCursor.y &&
		g_findWidgetAtY < widgetCursor.y + (int)widget->h;

	if (inside) {
		g_foundWidget = widgetCursor;

		// @todo move this to up-down widget
		if (widget->type == WIDGET_TYPE_UP_DOWN) {
			if (g_findWidgetAtX < widgetCursor.x + widget->w / 2) {
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

		g_findWidgetAtX = x;
		g_findWidgetAtY = y;

		enumWidgets(getActivePageId(), findWidgetStep);

		return g_foundWidget;
	}
}

}
}
} // namespace eez::mw::gui

#endif
