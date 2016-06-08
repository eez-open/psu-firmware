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
#include "front_panel/render.h"
#include "imgui/window.h"

namespace eez {

using namespace imgui;

namespace psu {
namespace simulator {
namespace front_panel {

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
static WindowDefinition window_definition = {
	"EEZ Software Simulator",
	1475, 531,
	10,
	"eez.png"
};
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
static WindowDefinition window_definition = {
	"EEZ Software Simulator",
	1081, 366,
	10,
	"eez.png"
};
#endif

imgui::WindowDefinition *getWindowDefinition() {
	return &window_definition;
}

void render(Window *window, Data *data) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    window->addImage(0, 0, 1475, 531, "r1b9/front-panel.png");

    window->addOnOffImage(878, 34, 17, 16, data->standby, "r1b9/led-blue.png", "r1b9/led-off.png");

    window->addOnOffImage(878, 126, 17, 16, data->ch1.cv, "r1b9/led-yellow.png", "r1b9/led-off.png");
    window->addOnOffImage(878, 172, 17, 16, data->ch1.cc, "r1b9/led-red.png", "r1b9/led-off.png");
    window->addOnOffImage(983, 80, 17, 16, data->ch1.out_plus, "r1b9/led-green.png", "r1b9/led-off.png");
    window->addOnOffImage(1071, 80, 17, 16, data->ch1.sense_plus, "r1b9/led-yellow.png", "r1b9/led-off.png");
    window->addOnOffImage(1159, 80, 17, 16, data->ch1.sense_minus, "r1b9/led-yellow.png", "r1b9/led-off.png");
    window->addOnOffImage(1247, 80, 17, 16, data->ch1.out_minus, "r1b9/led-green.png", "r1b9/led-off.png");
    if (data->ch1.load_text) {
        window->addImage(992, 184, 266, 71, "r1b9/load.png");
        window->addText(1047, 217, 156, 32, data->ch1.load_text);
    }

    window->addOnOffImage(878, 366, 17, 16, data->ch2.cv, "r1b9/led-yellow.png", "r1b9/led-off.png");
    window->addOnOffImage(878, 412, 17, 16, data->ch2.cc, "r1b9/led-red.png", "r1b9/led-off.png");
    window->addOnOffImage(983, 324, 17, 16, data->ch2.out_plus, "r1b9/led-green.png", "r1b9/led-off.png");
    window->addOnOffImage(1071, 324, 17, 16, data->ch2.sense_plus, "r1b9/led-yellow.png", "r1b9/led-off.png");
    window->addOnOffImage(1159, 324, 17, 16, data->ch2.sense_minus, "r1b9/led-yellow.png", "r1b9/led-off.png");
    window->addOnOffImage(1247, 324, 17, 16, data->ch2.out_minus, "r1b9/led-green.png", "r1b9/led-off.png");
    if (data->ch2.load_text) {
        window->addImage(992, 428, 266, 71, "r1b9/load.png");
        window->addText(1047, 461, 156, 32, data->ch2.load_text);
    }

    data->reset = window->addButton(509, 398, 18, 18, "r1b9/reset-normal.png", "r1b9/reset-pressed.png");

    data->local_control_widget.x = 594;
    data->local_control_widget.y = 94;
    data->local_control_widget.w = 240;
    data->local_control_widget.h = 320;

    window->addUserWidget(&data->local_control_widget);
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
    window->addImage(0, 0, 1081, 366, "r2b6/small/front-panel.png");


    window->addOnOffImage(131, 243, 17, 17, data->standby, "r2b6/small/led-blue.png", "r2b6/small/led-off.png");

    window->addOnOffImage(689, 61, 17, 17, data->ch1.cv, "r2b6/small/led-yellow.png", "r2b6/small/led-off.png");
    window->addOnOffImage(689, 95, 17, 17, data->ch1.cc, "r2b6/small/led-red.png", "r2b6/small/led-off.png");
    window->addOnOffImage(713, 243, 17, 17, data->ch1.out, "r2b6/small/led-green.png", "r2b6/small/led-off.png");
    window->addOnOffImage(689, 128, 17, 17, data->ch1.sense, "r2b6/small/led-yellow.png", "r2b6/small/led-off.png");
    window->addOnOffImage(650, 128, 17, 17, data->ch1.prog, "r2b6/small/led-red.png", "r2b6/small/led-off.png");
    if (data->ch1.load_text) {
        window->addImage(652, 294, 138, 66, "r2b6/small/load.png");
        window->addText(677, 326, 88, 29, data->ch1.load_text);
    }

    window->addOnOffImage(918, 61, 17, 17, data->ch2.cv, "r2b6/small/led-yellow.png", "r2b6/small/led-off.png");
    window->addOnOffImage(918, 95, 17, 17, data->ch2.cc, "r2b6/small/led-red.png", "r2b6/small/led-off.png");
    window->addOnOffImage(894, 243, 17, 17, data->ch2.out, "r2b6/small/led-green.png", "r2b6/small/led-off.png");
    window->addOnOffImage(918, 128, 17, 17, data->ch2.sense, "r2b6/small/led-yellow.png", "r2b6/small/led-off.png");
    window->addOnOffImage(879, 128, 17, 17, data->ch2.prog, "r2b6/small/led-red.png", "r2b6/small/led-off.png");
    if (data->ch2.load_text) {
        window->addImage(834, 294, 138, 66, "r2b6/small/load.png");
        window->addText(859, 326, 88, 29, data->ch2.load_text);
    }

	data->reset = window->addButton(129, 165, 20, 20, "r2b6/small/reset-normal.png", "r2b6/small/reset-pressed.png");

    data->local_control_widget.x = 211;
    data->local_control_widget.y = 57;
    data->local_control_widget.w = 320;
    data->local_control_widget.h = 240;

    window->addUserWidget(&data->local_control_widget);
#endif
}

}
}
}
} // namespace eez::psu::simulator::front_panel;