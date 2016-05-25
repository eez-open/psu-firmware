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

/// Analog to digital converter HW used by the channel.
class AnalogDigitalConverter {
public:
    static const uint16_t ADC_MIN = 0;
    static const uint16_t ADC_MAX = (1L << ADC_RES) - 1;

    static const uint8_t ADC_RESET = 0B00000110;
    static const uint8_t ADC_RDATA = 0B00010000;
    static const uint8_t ADC_START = 0B00001000;

    static const uint8_t ADC_WR3S1 = 0B01000110;
    static const uint8_t ADC_RD3S1 = 0B00100110;
    static const uint8_t ADC_WR1S0 = 0B01000000;
    static const uint8_t ADC_WR4S0 = 0B01000011;

    static const uint8_t ADC_REG0_READ_U_MON = 0x91; // B10010001: [7:4] AINP = AIN1, AINN = AVSS, [3:1] Gain = 1, [0] PGA disabled and bypassed
    static const uint8_t ADC_REG0_READ_I_MON = 0xA1; // B10100001: [7:4] AINP = AIN2, AINN = AVSS, [3:1] Gain = 1, [0] PGA disabled and bypassed

    static const uint8_t ADC_REG0_READ_U_SET = 0x81; // B10000001: [7:4] AINP = AIN0, AINN = AVSS, [3:1] Gain = 1, [0] PGA disabled and bypassed
    static const uint8_t ADC_REG0_READ_I_SET = 0xB1; // B10110001: [7:4] AINP = AIN3, AINN = AVSS, [3:1] Gain = 1, [0] PGA disabled and bypassed

    int test_result;
    uint8_t start_reg0;

    AnalogDigitalConverter(Channel &channel);

    bool init();
    bool test();

    void tick(unsigned long tick_usec);

    void start(uint8_t reg0);
    int16_t read();

private:
    Channel &channel;
    uint8_t current_sps;

    unsigned long start_time;

    uint8_t adc_timeout_recovery_attempts_counter;

    uint8_t getReg1Val();
};

}
} // namespace eez::psu
