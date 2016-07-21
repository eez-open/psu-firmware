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
#include "dac.h"

namespace eez {
namespace psu {

////////////////////////////////////////////////////////////////////////////////

static const uint8_t DATA_BUFFER_A = 0B00010000;
static const uint8_t DATA_BUFFER_B = 0B00100100;

static const uint16_t DAC_MIN = 0;
static const uint16_t DAC_MAX = (1L << DAC_RES) - 1;

////////////////////////////////////////////////////////////////////////////////

#if CONF_DEBUG
extern uint16_t debug_u_dac[CH_MAX];
extern uint16_t debug_i_dac[CH_MAX];
#endif

////////////////////////////////////////////////////////////////////////////////

DigitalAnalogConverter::DigitalAnalogConverter(Channel &channel_) : channel(channel_) {
    test_result = psu::TEST_SKIPPED;
}

void DigitalAnalogConverter::set_value(uint8_t buffer, float value) {
    uint16_t DAC_value = (uint16_t)util::clamp(round(value), DAC_MIN, DAC_MAX);

#if CONF_DEBUG
    if (buffer == DATA_BUFFER_A) {
        debug::u_dac[channel.index - 1] = DAC_value;
    }
    else {
        debug::i_dac[channel.index - 1] = DAC_value;
    }
#endif

    SPI.beginTransaction(DAC8552_SPI);
    digitalWrite(channel.dac_pin, LOW);
    SPI.transfer(buffer);
    SPI.transfer(DAC_value >> 8); // send first byte
    SPI.transfer(DAC_value & 0xFF);  // send second byte

#if CONF_DEBUG
	if  (debug::g_set_voltage_or_current_time_start != 0) {
		unsigned long end = micros();
		DebugTraceF("Command duration[microseconds]: %ul", debug::g_set_voltage_or_current_time_start - end);
		debug::g_set_voltage_or_current_time_start = 0;
	}
#endif

    digitalWrite(channel.dac_pin, HIGH); // Deselect DAC
    SPI.endTransaction();
}

////////////////////////////////////////////////////////////////////////////////

bool DigitalAnalogConverter::init() {
    return test();
}

bool DigitalAnalogConverter::test() {
    test_result = psu::TEST_OK;

    if (channel.ioexp.test_result != psu::TEST_OK) {
        DebugTraceF("Ch%d DAC test skipped because of IO expander", channel.index);
        test_result = psu::TEST_SKIPPED;
        return true;
    }

    if (channel.adc.test_result != psu::TEST_OK) {
        DebugTraceF("Ch%d DAC test skipped because of ADC", channel.index);
        test_result = psu::TEST_SKIPPED;
        return true;
    }

    int save_cal_enabled = channel.flags.cal_enabled;
    channel.flags.cal_enabled = 0;

    int save_output_enabled = channel.flags.output_enabled;
    channel.flags.output_enabled = 0;
    channel.updateOutputEnable();

    test_result = psu::TEST_OK;

    // set U on DAC and check it on ADC
    float u_set = channel.U_MAX / 2;
    float i_set = channel.I_MAX / 2;

    float u_set_save = channel.u.set;
    channel.setVoltage(u_set);

    float i_set_save = channel.i.set;
    channel.setCurrent(i_set);

    delay(200);

    channel.adcReadMonDac();

    float u_mon = channel.u.mon_dac;
    float u_diff = u_mon - u_set;
    if (fabsf(u_diff) > u_set * DAC_TEST_TOLERANCE / 100) {
        test_result = psu::TEST_FAILED;

        DebugTraceF("Ch%d DAC test, U_set failure: expected=%d, got=%d, abs diff=%d",
            channel.index,
            (int)(u_set * 100),
            (int)(u_mon * 100),
            (int)(u_diff * 100));
    }

    float i_mon = channel.i.mon_dac;
    float i_diff = i_mon - i_set;
    if (fabsf(i_diff) > i_set * DAC_TEST_TOLERANCE / 100) {
        test_result = psu::TEST_FAILED;

        DebugTraceF("Ch%d DAC test, I_set failure: expected=%d, got=%d, abs diff=%d",
            channel.index,
            (int)(i_set * 100),
            (int)(i_mon * 100),
            (int)(i_diff * 100));
    }

    channel.flags.cal_enabled = save_cal_enabled;

    // Re-enable output just in case if it is out of sync.
    channel.flags.output_enabled = save_output_enabled;
    channel.updateOutputEnable();

    channel.setVoltage(u_set_save);
    channel.setCurrent(i_set_save);

    if (test_result == psu::TEST_FAILED) {
        if (channel.index == 1) {
            psu::generateError(SCPI_ERROR_CH1_DAC_TEST_FAILED);
        }
        else if (channel.index == 2) {
            psu::generateError(SCPI_ERROR_CH2_DAC_TEST_FAILED);
        }
        else {
            // TODO
        }
    }

    return test_result != psu::TEST_FAILED;
}

////////////////////////////////////////////////////////////////////////////////

void DigitalAnalogConverter::set_voltage(float value) {
    set_value(DATA_BUFFER_A, util::remap(value, channel.U_MIN, (float)DAC_MIN, channel.U_MAX, (float)DAC_MAX));
}

void DigitalAnalogConverter::set_current(float value) {
    set_value(DATA_BUFFER_B, util::remap(value, channel.I_MIN, (float)DAC_MIN, channel.I_MAX, (float)DAC_MAX));
}

}
} // namespace eez::psu
