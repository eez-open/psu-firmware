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

#include "eez/mw/mw.h"

#if OPTION_DISPLAY

#include "eez/mw/platform/simulator/front_panel/data.h"
#include "eez/mw/gui/lcd.h"
#include "eez/mw/platform/lcd.h"
#include "eez/mw/platform/simulator/touch.h"

namespace eez {
namespace platform {
namespace simulator {
namespace front_panel {

void Data::fill() {
	if (!local_control_widget.pixels) {
		local_control_widget.pixels_w = mw::gui::lcd::getDisplayWidth();
		local_control_widget.pixels_h = mw::gui::lcd::getDisplayHeight();
		local_control_widget.pixels = new unsigned char[local_control_widget.pixels_w * local_control_widget.pixels_h * 4];
	}

	uint16_t *src = mw::gui::lcd::getBuffer();
	unsigned char *dst = local_control_widget.pixels;

	for (int x = 0; x < local_control_widget.pixels_w; ++x) {
		for (int y = 0; y < local_control_widget.pixels_h; ++y) {
			uint16_t color = *src++; // rrrrrggggggbbbbb

			*dst++ = (unsigned char)((color << 3) & 0xFF);        // blue
			*dst++ = (unsigned char)(((color >> 5) << 2) & 0xFF); // green
			*dst++ = (unsigned char)((color >> 11) << 3);         // red

			*dst++ = 255;
		}
	}
}

void Data::process() {
	//
	bool isDown = false;
	int x = -1;
	int y = -1;

	if (local_control_widget.mouseData.button1IsPressed &&
		local_control_widget.mouseData.button1DownX >= 0 &&
		local_control_widget.mouseData.button1DownX < local_control_widget.w &&
		local_control_widget.mouseData.button1DownY >= 0 &&
		local_control_widget.mouseData.button1DownY < local_control_widget.h) {
		isDown = true;
		x = (int)round(local_control_widget.mouseData.x / (1.0 * local_control_widget.w / local_control_widget.pixels_w));
		y = (int)round(local_control_widget.mouseData.y / (1.0 * local_control_widget.h / local_control_widget.pixels_h));
	}

	mw::gui::touch::write(isDown, x, y);
}

}
}
}
} // namespace eez::platform::simulator::front_panel;

#endif