/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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

#include "front_panel/data.h"

#include "psu.h"
#include "arduino_internal.h"
#include "chips.h"
#include "bp.h"
#include "lcd.h"

#include "UTouch.h"

namespace eez {
namespace psu {
namespace simulator {
namespace front_panel {

static char str_load[2][128];

void fillChannelData(ChannelData *data, int ch) {
    if (CH_NUM >= ch) {
        if (ch == 1) {
            data->cv = pins[LED_CV1] ? true : false;
            data->cc = pins[LED_CC1] ? true : false;
            data->out_plus = chips::bp_chip.getValue() & BP_LED_OUT1_PLUS ? true : false;
            data->sense_plus = chips::bp_chip.getValue() & BP_LED_SENSE1_PLUS ? true : false;
            data->sense_minus = chips::bp_chip.getValue() & BP_LED_SENSE1_MINUS ? true : false;
            data->out_minus = chips::bp_chip.getValue() & BP_LED_OUT1_MINUS ? true : false;
        }
        else {
            data->cv = pins[LED_CV2] ? true : false;
            data->cc = pins[LED_CC2] ? true : false;
            data->out_plus = chips::bp_chip.getValue() & BP_LED_OUT2_PLUS ? true : false;
            data->sense_plus = chips::bp_chip.getValue() & BP_LED_SENSE2_PLUS ? true : false;
            data->sense_minus = chips::bp_chip.getValue() & BP_LED_SENSE2_MINUS ? true : false;
            data->out_minus = chips::bp_chip.getValue() & BP_LED_OUT2_MINUS ? true : false;
        }

        Channel &channel = Channel::get(ch - 1);
        if (channel.simulator.getLoadEnabled()) {
            float load = channel.simulator.getLoad();
            char *str = &str_load[ch - 1][0];
            if (load == 0) {
                strcpy(str, "Shorted!");
            }
            else if (load == INFINITY) {
                // utf-8 infinity character
                // http://www.fileformat.info/info/unicode/char/221e/index.htm
                str[0] = (char)0xE2;
                str[1] = (char)0x88;
                str[2] = (char)0x9E;
                str[3] = 0;
            }
            else {
                *str = 0;
                util::strcatLoad(str, load);
            }
            data->load_text = str;
        }
        else {
            data->load_text = 0;
        }
    }
    else {
        data->cv = false;
        data->cc = false;
        data->out_plus = false;
        data->sense_plus = false;
        data->sense_minus = false;
        data->out_minus = false;
        data->load_text = 0;
    }
}

void fillLocalControlBuffer(Data *data) {
    if (!data->local_control_widget.pixels) {
        data->local_control_widget.pixels = new unsigned char[240 * 320 * 4];
    }

    word *src = gui::lcd::lcd.buffer;
    unsigned char *dst = data->local_control_widget.pixels;

    for (int x = 0; x < 240; ++x) {
        for (int y = 0; y < 320; ++y) {
            word color = *src++; // rrrrrggggggbbbbb

            *dst++ = (unsigned char)((color << 3) & 0xFF);        // blue
            *dst++ = (unsigned char)(((color >> 5) << 2) & 0xFF); // green
            *dst++ = (unsigned char)((color >> 11) << 3);         // red

            *dst++ = 255;
        }
    }
}

void fillData(Data *data) {
    uint16_t bp_value = chips::bp_chip.getValue();

    data->standby = bp_value & BP_STANDBY ? true : false;

    fillChannelData(&data->ch1, 1);
    fillChannelData(&data->ch2, 2);

    fillLocalControlBuffer(data);
}

void processData(Data *data) {
    if (data->reset) {
        DebugTrace("Reset");
        reset();
    }

    bool is_down = false;
    int x = -1;
    int y = -1;

    if (data->local_control_widget.mouse_data.is_pressed &&
        data->local_control_widget.mouse_data.down_x >= 0 &&
        data->local_control_widget.mouse_data.down_x < data->local_control_widget.w &&
        data->local_control_widget.mouse_data.down_y >= 0 &&
        data->local_control_widget.mouse_data.down_y < data->local_control_widget.h) 
    {
        is_down = true;
        x = data->local_control_widget.mouse_data.x;
        y = data->local_control_widget.mouse_data.y;
    }

    UTouch::setData(is_down, x, y);
}

}
}
}
} // namespace eez::psu::simulator::front_panel;