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
 
#pragma once

/*
PSU 0-50V/3A I/O expander pinout:

Pin 0  In, ADC interrupt/DRDY
Pin 1  Out, DP enable (active low)
Pin 2  In, CV_ACTIVE
Pin 3  In, Temp sensor (V/F)
Pin 4  --, not in use
Pin 5  In, CC_ACTIVE
Pin 6  In, PWRGOOD
Pin 7  Out, OUTPUT_ENABLE
*/

namespace eez {
namespace psu {

class Channel;

/// IO Expander HW used by the channel.
class IOExpander {
public:
    static const uint8_t IO_BIT_IN_ADC_DRDY = 0;
    static const uint8_t IO_BIT_IN_CV_ACTIVE = 2;
    static const uint8_t IO_BIT_IN_TEMP_SENSOR = 3;
    static const uint8_t IO_BIT_IN_CC_ACTIVE = 5;
    static const uint8_t IO_BIT_IN_PWRGOOD = 6;

    static const uint8_t IO_BIT_OUT_DP_ENABLE = 1;
    static const uint8_t IO_BIT_OUT_OUTPUT_ENABLE = 7;

    static const uint8_t IOEXP_READ = 0B01000001;
    static const uint8_t IOEXP_WRITE = 0B01000000;

    static const uint8_t REG_IODIR = 0x00;
    static const uint8_t REG_IPOL = 0x01;
    static const uint8_t REG_GPINTEN = 0x02;
    static const uint8_t REG_DEVAL = 0x03;
    static const uint8_t REG_INTCON = 0x04;
    static const uint8_t REG_IOCON = 0x05;
    static const uint8_t REG_GPPU = 0x06;
    static const uint8_t REG_INTF = 0x07;
    static const uint8_t REG_INTCAP = 0x08;
    static const uint8_t REG_GPIO = 0x09;
    static const uint8_t REG_OLAT = 0x0A;

    static const size_t NUM_REGISTERS = REG_OLAT + 1;

    int test_result;

    IOExpander(Channel &channel);

    bool init();
    bool test();

    void tick(unsigned long tick_usec);

    bool test_bit(int io_bit);
    void change_bit(int io_bit, bool set);

    void on_interrupt();

private:
    Channel &channel;

    uint8_t reg_read_write(uint8_t opcode, uint8_t reg, uint8_t val);
    uint8_t reg_read(uint8_t reg);
    void reg_write(uint8_t reg, uint8_t val);
};

}
} // namespace eez::psu
