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

#include "eez/app/sound.h"
#include "eez/mw/gui/gui.h"
#include "eez/mw/gui/widget/list_graph.h"

namespace eez {
namespace mw {
namespace gui {

void ListGraphWidget_draw(int pageId, const WidgetCursor &widgetCursor) {
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
			lcd::setColor(style->background_color);
			lcd::fillRect(widgetCursor.x, widgetCursor.y, widgetCursor.x + (int)widget->w - 1, widgetCursor.y + (int)widget->h - 1);
		}

		int dwellListLength = data::getFloatListLength(listGraphWidget->dwellData);
		if (dwellListLength > 0) {
			float *dwellList = data::getFloatList(listGraphWidget->dwellData);

			const Style *styles[2] = {
				y1Style,
				y2Style
			};

			int listLength[2] = {
				data::getFloatListLength(listGraphWidget->y1Data),
				data::getFloatListLength(listGraphWidget->y2Data)
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

			int maxListLength = data::getFloatListLength(widget->data);

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
						lcd::setColor(style->background_color);
						lcd::fillRect(x1, widgetCursor.y, x2 - 1, widgetCursor.y + (int)widget->h - 1);
					}

					if (i == iRow) {
						lcd::setColor(cursorStyle->background_color);
						lcd::fillRect(x1, widgetCursor.y, x2 - 1, widgetCursor.y + (int)widget->h - 1);
					}
				}

				for (int k = 0; k < 2; ++k) {
					int j = iCursor % 3 == 2 ? k : 1 - k;

					if (listLength[j] > 0) {
						if (!skipDraw) {
							lcd::setColor(styles[j]->color);
						}

						float value = i < listLength[j] ? list[j][i] : list[j][listLength[j] - 1];
						int y = int((value - min[j]) * widget->h / (max[j] - min[j]));
						if (y < 0) y = 0;
						if (y >= (int)widget->h) y = (int)widget->h - 1;

						y = widgetCursor.y + ((int)widget->h - 1) - y;

						if (!skipDraw) {
							if (i > 0 && abs(yPrev[j] - y) > 1) {
								if (yPrev[j] < y) {
									lcd::drawVLine(x1, yPrev[j] + 1, y - yPrev[j] - 1);
								} else {
									lcd::drawVLine(x1, y, yPrev[j] - y - 1);
								}
							}
						}

						yPrev[j] = y;

						if (!skipDraw) {
							lcd::drawHLine(x1, y, x2 - x1);
						}
					}
				}

				xPrev = x2;
			}
		}
	}
}

void ListGraphWidget_onTouch(const WidgetCursor &widgetCursor, Event &touchEvent) {
	if (touchEvent.type == EVENT_TYPE_TOUCH_DOWN || touchEvent.type == EVENT_TYPE_TOUCH_MOVE) {
		DECL_WIDGET(widget, widgetCursor.widgetOffset);
		DECL_WIDGET_SPECIFIC(ListGraphWidget, listGraphWidget, widget);

		if (touchEvent.x < widgetCursor.x || touchEvent.x >= widgetCursor.x + (int)widget->w) {
			return;
		}
		if (touchEvent.y < widgetCursor.y || touchEvent.y >= widgetCursor.y + (int)widget->h) {
			return;
		}

		int dwellListLength = data::getFloatListLength(listGraphWidget->dwellData);
		if (dwellListLength > 0) {
			int iCursor = -1;

			float *dwellList = data::getFloatList(listGraphWidget->dwellData);

			int maxListLength = data::getFloatListLength(widget->data);

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

				if (touchEvent.x >= x1 && touchEvent.x < x2) {
					int iCurrentCursor = data::get(widgetCursor.cursor, listGraphWidget->cursorData).getInt();
					iCursor = i * 3 + iCurrentCursor % 3;
					break;
				}
			}

			if (iCursor >= 0) {
				data::set(widgetCursor.cursor, listGraphWidget->cursorData, data::Value(iCursor), 0);

				if (touchEvent.type == EVENT_TYPE_TOUCH_DOWN) {
					playClickSound();
				}
			}
		}
	}
}

}
}
} // namespace eez::mw::gui

#endif