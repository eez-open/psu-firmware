/*
 * EEZ Middleware
 * Copyright (C) 2018-present, Envox d.o.o.
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

#include "eez/mw/mw.h"

#if OPTION_DISPLAY

#include "eez/mw/util.h"
#include "eez/mw/gui/gui.h"
#include "eez/mw/gui/draw.h"
#include "eez/mw/gui/widget/yt_graph.h"

#define CONF_GUI_YT_GRAPH_BLANK_PIXELS_AFTER_CURSOR 10

namespace eez {
namespace mw {
namespace gui {

int getYValue(
	const WidgetCursor &widgetCursor, const Widget *widget,
	uint8_t data, float min, float max,
	int position
) {
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
) {
	for (int position = startPosition; position < endPosition; ++position) {
		if (position < graphWidth) {
			int x = widgetCursor.x + xGraphOffset + position;

			lcd::setColor(color);
			lcd::drawVLine(x, widgetCursor.y, widget->h - 1);

			int y1 = getYValue(widgetCursor, widget, data1, min1, max1, position);
			int y1Prev = getYValue(widgetCursor, widget, data1, min1, max1, position == 0 ? position : position - 1);

			int y2 = getYValue(widgetCursor, widget, data2, min2, max2, position);
			int y2Prev = getYValue(widgetCursor, widget, data2, min2, max2, position == 0 ? position : position - 1);

			if (abs(y1Prev - y1) <= 1 && abs(y2Prev - y2) <= 1) {
				if (y1 == y2) {
					lcd::setColor(position % 2 ? data2Color : data1Color);
					lcd::drawPixel(x, widgetCursor.y + y1);
				} else {
					lcd::setColor(data1Color);
					lcd::drawPixel(x, widgetCursor.y + y1);

					lcd::setColor(data2Color);
					lcd::drawPixel(x, widgetCursor.y + y2);
				}
			} else {
				lcd::setColor(data1Color);
				if (abs(y1Prev - y1) <= 1) {
					lcd::drawPixel(x, widgetCursor.y + y1);
				} else {
					if (y1Prev < y1) {
						lcd::drawVLine(x, widgetCursor.y + y1Prev + 1, y1 - y1Prev - 1);
					} else {
						lcd::drawVLine(x, widgetCursor.y + y1, y1Prev - y1 - 1);
					}
				}

				lcd::setColor(data2Color);
				if (abs(y2Prev - y2) <= 1) {
					lcd::drawPixel(x, widgetCursor.y + y2);
				} else {
					if (y2Prev < y2) {
						lcd::drawVLine(x, widgetCursor.y + y2Prev + 1, y2 - y2Prev - 1);
					} else {
						lcd::drawVLine(x, widgetCursor.y + y2, y2Prev - y2 - 1);
					}
				}
			}
		}
	}
}

void YTGraphWidget_draw(int pageId, const WidgetCursor &widgetCursor) {
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
		lcd::setColor(color);
		lcd::fillRect(widgetCursor.x, widgetCursor.y, widgetCursor.x + (int)widget->w - 1, widgetCursor.y + (int)widget->h - 1);
	}

	int textWidth = 62; // TODO this is hardcoded value
	int textHeight = widget->h / 2;

	// draw first value text
	bool refreshText = !widgetCursor.previousState || widgetCursor.previousState->data != widgetCursor.currentState->data;

	if (refresh || refreshText) {
		char text[64];
		widgetCursor.currentState->data.toText(text, sizeof(text));

		drawText(pageId, text, -1, widgetCursor.x, widgetCursor.y, textWidth, textHeight, y1Style,
			widgetCursor.currentState->flags.pressed, false, false, NULL);
		if (!isActivePage(pageId)) {
			return;
		}
	}

	// draw second value text
	refreshText = !widgetCursor.previousState || ((YTGraphWidgetState *)widgetCursor.previousState)->y2Data != ((YTGraphWidgetState *)widgetCursor.currentState)->y2Data;

	if (refresh || refreshText) {
		char text[64];
		((YTGraphWidgetState *)widgetCursor.currentState)->y2Data.toText(text, sizeof(text));

		drawText(pageId, text, -1, widgetCursor.x, widgetCursor.y + textHeight, textWidth, textHeight, y2Style,
			widgetCursor.currentState->flags.pressed, false, false, NULL);
		if (!isActivePage(pageId)) {
			return;
		}
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
			widgetCursor.currentState->flags.pressed ? style->color : style->background_color,
			widgetCursor.currentState->flags.pressed ? style->background_color : style->color);
	} else {
		drawYTGraph(widgetCursor, widget,
			startPosition, numHistoryValues,
			currentHistoryValuePosition, numHistoryValues,
			textWidth, graphWidth,
			widget->data, min1, max1, y1Style->color,
			ytGraphWidget->y2Data, min2, max2, y2Style->color,
			widgetCursor.currentState->flags.pressed ? style->color : style->background_color,
			widgetCursor.currentState->flags.pressed ? style->background_color : style->color);

		drawYTGraph(widgetCursor, widget,
			0, endPosition,
			currentHistoryValuePosition, numHistoryValues,
			textWidth, graphWidth,
			widget->data, min1, max1, y1Style->color,
			ytGraphWidget->y2Data, min2, max2, y2Style->color,
			widgetCursor.currentState->flags.pressed ? style->color : style->background_color,
			widgetCursor.currentState->flags.pressed ? style->background_color : style->color);
	}

	int x = widgetCursor.x + textWidth;

	// draw cursor
	lcd::setColor(style->color);
	lcd::drawVLine(x + currentHistoryValuePosition, widgetCursor.y, (int)widget->h - 1);

	// draw blank lines
	int x1 = x + (currentHistoryValuePosition + 1) % numHistoryValues;
	int x2 = x + (currentHistoryValuePosition + CONF_GUI_YT_GRAPH_BLANK_PIXELS_AFTER_CURSOR) % numHistoryValues;

	lcd::setColor(style->background_color);
	if (x1 < x2) {
		lcd::fillRect(x1, widgetCursor.y, x2, widgetCursor.y + (int)widget->h - 1);
	} else {
		lcd::fillRect(x1, widgetCursor.y, x + graphWidth - 1, widgetCursor.y + (int)widget->h - 1);
		lcd::fillRect(x, widgetCursor.y, x2, widgetCursor.y + (int)widget->h - 1);
	}

	lastPosition[iChannel] = currentHistoryValuePosition;
}

}
}
} // namespace eez::mw::gui

#endif