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

namespace eez {
namespace psu {

/// Digital to analog converter HW used by the channel.
class DigitalAnalogConverter {
public:
    static const uint8_t DATA_BUFFER_A = 0B00010000;
    static const uint8_t DATA_BUFFER_B = 0B00100100;

    static const uint16_t DAC_MIN = 0;
    static const uint16_t DAC_MAX = (1L << DAC_RES) - 1;

    psu::TestResult g_testResult;

    DigitalAnalogConverter(Channel &channel);

    void init();
    bool test();

    void set_voltage(float voltage);
    void set_current(float voltage);

private:
    Channel &channel;

    void set_value(uint8_t buffer, float value);
};

}
} // namespace eez::psu
