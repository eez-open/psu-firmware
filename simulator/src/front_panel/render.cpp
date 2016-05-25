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

#include "front_panel/render.h"
#include "imgui/window.h"

#include <string.h>
#include <stdlib.h>

namespace eez {

using namespace imgui;

namespace psu {
namespace simulator {
namespace front_panel {

static WindowDefinition window_definition = {
	"EEZ Software Simulator",
	1475, 531,
	10,
	"eez.png"
};

imgui::WindowDefinition *getWindowDefinition() {
	return &window_definition;
}

void render(Window *window, Data *data) {
    window->addImage(0, 0, 1475, 531, "front-panel.png");

    window->addOnOffImage(878, 34, 17, 16, data->standby, "led-blue.png", "led-off.png");

    window->addOnOffImage(878, 126, 17, 16, data->ch1.cv, "led-yellow.png", "led-off.png");
    window->addOnOffImage(878, 172, 17, 16, data->ch1.cc, "led-red.png", "led-off.png");
    window->addOnOffImage(983, 80, 17, 16, data->ch1.out_plus, "led-green.png", "led-off.png");
    window->addOnOffImage(1071, 80, 17, 16, data->ch1.sense_plus, "led-yellow.png", "led-off.png");
    window->addOnOffImage(1159, 80, 17, 16, data->ch1.sense_minus, "led-yellow.png", "led-off.png");
    window->addOnOffImage(1247, 80, 17, 16, data->ch1.out_minus, "led-green.png", "led-off.png");
    if (data->ch1.load_text) {
        window->addImage(992, 184, 266, 71, "load.png");
        window->addText(1047, 217, 156, 32, data->ch1.load_text);
    }

    window->addOnOffImage(878, 366, 17, 16, data->ch2.cv, "led-yellow.png", "led-off.png");
    window->addOnOffImage(878, 412, 17, 16, data->ch2.cc, "led-red.png", "led-off.png");
    window->addOnOffImage(983, 324, 17, 16, data->ch2.out_plus, "led-green.png", "led-off.png");
    window->addOnOffImage(1071, 324, 17, 16, data->ch2.sense_plus, "led-yellow.png", "led-off.png");
    window->addOnOffImage(1159, 324, 17, 16, data->ch2.sense_minus, "led-yellow.png", "led-off.png");
    window->addOnOffImage(1247, 324, 17, 16, data->ch2.out_minus, "led-green.png", "led-off.png");
    if (data->ch2.load_text) {
        window->addImage(992, 428, 266, 71, "load.png");
        window->addText(1047, 461, 156, 32, data->ch2.load_text);
    }

    data->reset = window->addButton(509, 398, 18, 18, "reset-normal.png", "reset-pressed.png");

    data->local_control_widget.x = 594;
    data->local_control_widget.y = 94;
    data->local_control_widget.w = 240;
    data->local_control_widget.h = 320;

    window->addUserWidget(&data->local_control_widget);
}

}
}
}
} // namespace eez::psu::simulator::front_panel;