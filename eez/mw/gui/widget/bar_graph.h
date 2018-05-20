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
 
#pragma once

#include "eez/mw/gui/view.h"

#define BAR_GRAPH_ORIENTATION_LEFT_RIGHT 1
#define BAR_GRAPH_ORIENTATION_RIGHT_LEFT 2
#define BAR_GRAPH_ORIENTATION_TOP_BOTTOM 3
#define BAR_GRAPH_ORIENTATION_BOTTOM_TOP 4

namespace eez {
namespace mw {
namespace gui {

#pragma pack(push, 1)

struct BarGraphWidget {
	uint8_t orientation; // BAR_GRAPH_ORIENTATION_...
	uint8_t textStyle;
	uint8_t line1Data;
	uint8_t line1Style;
	uint8_t line2Data;
	uint8_t line2Style;
};

#pragma pack(pop)

#ifdef EEZ_PLATFORM_ARDUINO_DUE
#pragma pack(push, 1)
#endif

struct BarGraphWidgetState {
    WidgetState genericState;
    data::Value line1Data;
    data::Value line2Data;
};

#ifdef EEZ_PLATFORM_ARDUINO_DUE
#pragma pack(pop)
#endif

void BarGraphWidget_draw(int pageId, const WidgetCursor &widgetCursor);

}
}
} // namespace eez::mw::gui