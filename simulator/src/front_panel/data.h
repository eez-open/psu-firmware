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

#pragma once

#include "imgui/window.h"

namespace eez {
namespace psu {
namespace simulator {
namespace front_panel {

struct ChannelData {
    bool cv;
    bool cc;
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    bool out_plus;
    bool sense_plus;
    bool sense_minus;
    bool out_minus;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
    bool out;
	bool sense;
	bool prog;
#endif
    const char *load_text;
    float load;
    float loadOnMouseDown;
    imgui::UserWidget loadWidget;
};

/// Data presented in GUI front panel.
struct Data {
    bool standby;

    bool coupled;
    bool coupledOut;

    ChannelData ch1;
    ChannelData ch2;

    bool reset;

    imgui::UserWidget local_control_widget;
};

void fillData(Data *data);
void processData(Data *data);

}
}
}
} // namespace eez::psu::simulator::front_panel;
