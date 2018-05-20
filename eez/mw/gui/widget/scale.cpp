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
#include "eez/mw/gui/widget/scale.h"

namespace eez {
namespace mw {
namespace gui {

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
					lcd::setColor(style->border_color);
					if (vertical) {
						lcd::drawHLine(x1, y, l1 - 1);
					} else {
						lcd::drawVLine(y, x1, l1 - 1);
					}
				} else if (y_i % (s / 2) == 0) {
					lcd::setColor(style->border_color);
					if (vertical) {
						if (flip) {
							lcd::drawHLine(x1 + l1 / 2, y, l1 / 2);
						} else {
							lcd::drawHLine(x1, y, l1 / 2);
						}
					} else {
						if (flip) {
							lcd::drawVLine(y, x1 + l1 / 2, l1 / 2);
						} else {
							lcd::drawVLine(y, x1, l1 / 2);
						}
					}
				} else if (y_i % (s / 10) == 0) {
					lcd::setColor(style->border_color);
					if (vertical) {
						if (flip) {
							lcd::drawHLine(x1 + l1 - l1 / 4, y, l1 / 4);
						} else {
							lcd::drawHLine(x1, y, l1 / 4);
						}
					} else {
						if (flip) {
							lcd::drawVLine(y, x1 + l1 - l1 / 4, l1 / 4);
						} else {
							lcd::drawVLine(y, x1, l1 / 4);
						}
					}
				} else {
					lcd::setColor(style->background_color);
					if (vertical) {
						lcd::drawHLine(x1, y, l1 - 1);
					} else {
						lcd::drawVLine(y, x1, l1 - 1);
					}
				}
			}
		}

		int d = abs(y_i - y_value);
		if (d <= int(needleSize / 2)) {
			// draw thumb
			lcd::setColor(style->color);
			if (vertical) {
				if (flip) {
					lcd::drawHLine(x2, y, l2 - d - 1);
				} else {
					lcd::drawHLine(x2 + d, y, l2 - d - 1);
				}
			} else {
				if (flip) {
					lcd::drawVLine(y, x2, l2 - d - 1);
				} else {
					lcd::drawVLine(y, x2 + d, l2 - d - 1);
				}
			}

			if (y_i != y_value) {
				lcd::setColor(style->background_color);
				if (vertical) {
					if (flip) {
						lcd::drawHLine(x2 + l2 - d, y, d - 1);
					} else {
						lcd::drawHLine(x2, y, d - 1);
					}
				} else {
					if (flip) {
						lcd::drawVLine(y, x2 + l2 - d, d - 1);
					} else {
						lcd::drawVLine(y, x2, d - 1);
					}
				}
			}
		} else {
			// erase
			lcd::setColor(style->background_color);
			if (vertical) {
				lcd::drawHLine(x2, y, l2 - 1);
			} else {
				lcd::drawVLine(y, x2, l2 - 1);
			}
		}
	}
}

void ScaleWidget_draw(int pageId, const WidgetCursor &widgetCursor) {
	DECL_WIDGET(widget, widgetCursor.widgetOffset);

	widgetCursor.currentState->size = sizeof(WidgetState);
	widgetCursor.currentState->data = data::get(widgetCursor.cursor, widget->data);

	bool refresh = !widgetCursor.previousState ||
		widgetCursor.previousState->data != widgetCursor.currentState->data;

	if (refresh) {
		float min = data::getMin(widgetCursor.cursor, widget->data).getFloat();
		float max = data::getMax(widgetCursor.cursor, widget->data).getFloat();

		DECL_WIDGET_STYLE(style, widget);

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
		} else {
			f = 10 * (f / 10);
			d = 10;
		}

		int y_min = (int)round(min * f);
		int y_max = (int)round(max * f);
		int y_value = (int)round(widgetCursor.currentState->data.getFloat() * f);

		int y_from_min = y_min - needleSize / 2;
		int y_from_max = y_max + needleSize / 2;

		drawScale(widget, scale_widget, style, y_from_min, y_from_max, y_min, y_max, y_value, f, d, true);

		onScaleUpdatedHook(widget->data, vertical, vertical ? widget->w : widget->h, (max - min) * f);
	}
}

}
}
} // namespace eez::mw::gui

#endif