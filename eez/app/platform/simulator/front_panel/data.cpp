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

#include "eez/app/psu.h"

#if OPTION_DISPLAY

#include "eez/app/gui/psu.h"
#include "eez/app/platform/simulator/front_panel/data.h"
#include "eez/app/bp.h"
#include "eez/app/platform/simulator/bp.h"
#include "eez/app/channel_dispatcher.h"

namespace eez {
namespace app {
namespace simulator {
namespace front_panel {

static Data g_data;
static char str_load[2][128];

void fillChannelData(ChannelData *data, int ch) {
    if (CH_NUM >= ch) {
        if (ch == 1) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
            data->cv = pins[LED_CV1] ? true : false;
            data->cc = pins[LED_CC1] ? true : false;
            data->out_plus = bp::g_lastConf & (1 << BP_LED_OUT1_PLUS) ? true : false;
            data->sense_plus = bp::g_lastConf & (1 << BP_LED_SENSE1_PLUS) ? true : false;
            data->sense_minus = bp::g_lastConf & (1 << BP_LED_SENSE1_MINUS) ? true : false;
            data->out_minus = bp::g_lastConf & (1 << BP_LED_OUT1_MINUS) ? true : false;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
            data->cv = bp::g_lastConf & (1 << BP_LED_CV1) ? true : false;
            data->cc = bp::g_lastConf & (1 << BP_LED_CC1) ? true : false;
            data->out = bp::g_lastConf & (1 << BP_LED_OUT1) ? true : false;
            data->sense = bp::g_lastConf & (1 << BP_LED_SENSE1) ? true : false;
            data->prog = bp::g_lastConf & (1 << BP_LED_PROG1) ? true : false;
#endif
        }
        else {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
            data->cv = pins[LED_CV2] ? true : false;
            data->cc = pins[LED_CC2] ? true : false;
            data->out_plus = bp::g_lastConf & (1 << BP_LED_OUT2_PLUS) ? true : false;
            data->sense_plus = bp::g_lastConf & (1 << BP_LED_SENSE2_PLUS) ? true : false;
            data->sense_minus = bp::g_lastConf & (1 << BP_LED_SENSE2_MINUS) ? true : false;
            data->out_minus = bp::g_lastConf & (1 << BP_LED_OUT2_MINUS) ? true : false;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
            data->cv = bp::g_lastConf & (1 << BP_LED_CV2) ? true : false;
            data->cc = bp::g_lastConf & (1 << BP_LED_CC2) ? true : false;
            data->out = bp::g_lastConf & (1 << BP_LED_OUT2) ? true : false;
            data->sense = bp::g_lastConf & (1 << BP_LED_SENSE2) ? true : false;
            data->prog = bp::g_lastConf & (1 << BP_LED_PROG2) ? true : false;
#endif
        }

        Channel &channel = Channel::get(ch - 1);
        if (channel.simulator.getLoadEnabled()) {
            data->load = channel.simulator.getLoad();

            char *str = &str_load[ch - 1][0];
            if (data->load == 0) {
                strcpy(str, "Shorted!");
            }
            else if (data->load == INFINITY) {
                // utf-8 infinity character
                // http://www.fileformat.info/info/unicode/char/221e/index.htm
                str[0] = (char)0xE2;
                str[1] = (char)0x88;
                str[2] = (char)0x9E;
                str[3] = 0;
            }
            else {
                *str = 0;
                strcatLoad(str, data->load);
            }
            data->load_text = str;
        }
        else {
            data->load = 0;
            data->load_text = 0;
        }
    }
    else {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
        data->cv = false;
        data->cc = false;
        data->out_plus = false;
        data->sense_plus = false;
        data->sense_minus = false;
        data->out_minus = false;
        data->load_text = 0;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
        data->cv = false;
        data->cc = false;
        data->out = false;
        data->sense = false;
        data->prog = false;
        data->load_text = 0;
#endif
    }
}

void Data::fill() {
	platform::simulator::front_panel::Data::fill();

    uint16_t bp_value = bp::g_lastConf;

    standby = bp_value & (1 << BP_STANDBY) ? true : false;

	coupled = channel_dispatcher::isCoupled();
	coupledOut = bp::g_lastConf & (1 << BP_LED_OUT1_RED) ? true : false;

    fillChannelData(&ch1, 1);
    fillChannelData(&ch2, 2);
}

void processChannelData(ChannelData *data, int ch) {
    if (CH_NUM >= ch) {
        Channel &channel = Channel::get(ch - 1);

        if (data->loadWidget.mouseData.button1IsDown) {
            if (data->loadWidget.mouseData.button1DownX >= 0 &&
                data->loadWidget.mouseData.button1DownX < data->loadWidget.w &&
                data->loadWidget.mouseData.button1DownY >= 0 &&
                data->loadWidget.mouseData.button1DownY < data->loadWidget.h)
            {
                data->loadOnMouseDown = data->load;
            } else {
                data->loadOnMouseDown = NAN;
            }
        }

        if (!isNaN(data->loadOnMouseDown)) {
            if (data->loadWidget.mouseData.button1IsPressed) {
                if (channel.simulator.getLoadEnabled()) {
                    float step = 1;
                    float load = data->loadOnMouseDown + step * (data->loadWidget.mouseData.x - data->loadWidget.mouseData.button1DownX);
                    if (load < 0 || isNaN(load)) {
                        load = 0;
                    } else if (load > 10000000) {
                        load = 10000000;
                    }
                    channel_dispatcher::setLoad(channel, load);
                }
            } else if (data->loadWidget.mouseData.button1IsUp) {
                if (data->loadWidget.mouseData.button1UpX == data->loadWidget.mouseData.button1DownX && data->loadWidget.mouseData.button1UpY == data->loadWidget.mouseData.button1DownY) {
                    channel_dispatcher::setLoadEnabled(channel, !channel.simulator.getLoadEnabled());
                }
            }
        }
    }
}

void Data::process() {
	platform::simulator::front_panel::Data::process();

    static bool reseting = false;

    if (reset) {
        if (!reseting) {
            reseting = true;
            app::reset();
            reseting = false;
        }
    }

    //
    processChannelData(&ch1, 1);
    processChannelData(&ch2, 2);
}

}
}
}
} // namespace eez::app::simulator::front_panel;

namespace eez {
namespace platform {
namespace simulator {
namespace front_panel {

Data *getData() {
    return &app::simulator::front_panel::g_data;
}

}
}
}
} // namespace eez::platform::simulator::front_panel;

#endif