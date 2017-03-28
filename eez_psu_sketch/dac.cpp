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

DigitalAnalogConverter::DigitalAnalogConverter(Channel &channel_) : channel(channel_) {
    g_testResult = psu::TEST_SKIPPED;
}

void DigitalAnalogConverter::set_value(uint8_t buffer, uint16_t value) {
#if CONF_DEBUG
    if (buffer == DATA_BUFFER_A) {
        debug::g_uDac[channel.index - 1].set(value);
    }
    else {
        debug::g_iDac[channel.index - 1].set(value);
    }
#endif

    SPI_beginTransaction(DAC8552_SPI);
    digitalWrite(channel.dac_pin, LOW);

    SPI.transfer(buffer);
    SPI.transfer(value >> 8); // send first byte
    SPI.transfer(value & 0xFF);  // send second byte

    digitalWrite(channel.dac_pin, HIGH); // Deselect DAC
    SPI_endTransaction();
}

void DigitalAnalogConverter::set_value(uint8_t buffer, float value) {
    set_value(buffer, (uint16_t)util::clamp(round(value), DAC_MIN, DAC_MAX));
}

////////////////////////////////////////////////////////////////////////////////

void DigitalAnalogConverter::init() {
}

bool DigitalAnalogConverter::test() {
    g_testResult = psu::TEST_OK;

    if (channel.ioexp.g_testResult != psu::TEST_OK) {
        DebugTraceF("Ch%d DAC test skipped because of IO expander", channel.index);
        g_testResult = psu::TEST_SKIPPED;
        return true;
    }

    if (channel.adc.g_testResult != psu::TEST_OK) {
        DebugTraceF("Ch%d DAC test skipped because of ADC", channel.index);
        g_testResult = psu::TEST_SKIPPED;
        return true;
    }

    m_testing = true;

    bool saveCalibrationEnabled = channel.isCalibrationEnabled();
    channel.calibrationEnableNoEvent(false);

    // disable OE on channel
    int save_output_enabled = channel.flags.outputEnabled;
    channel.flags.outputEnabled = 0;
    channel.ioexp.changeBit(IOExpander::IO_BIT_OUT_OUTPUT_ENABLE, false);

    g_testResult = psu::TEST_OK;

    // set U on DAC and check it on ADC
    float u_set = channel.u.max / 2;
    float i_set = channel.i.max / 2;

    float u_set_save = channel.u.set;
    channel.setVoltage(u_set);

    float i_set_save = channel.i.set;
    channel.setCurrent(i_set);

    delay(200);

    channel.adcReadMonDac();

    float u_mon = channel.u.mon_dac;
    float u_diff = u_mon - u_set;
    if (fabsf(u_diff) > u_set * DAC_TEST_TOLERANCE / 100) {
        g_testResult = psu::TEST_FAILED;

        DebugTraceF("Ch%d DAC test, U_set failure: expected=%d, got=%d, abs diff=%d",
            channel.index,
            (int)(u_set * 100),
            (int)(u_mon * 100),
            (int)(u_diff * 100));
    }

    float i_mon = channel.i.mon_dac;
    float i_diff = i_mon - i_set;
    if (fabsf(i_diff) > i_set * DAC_TEST_TOLERANCE / 100) {
        g_testResult = psu::TEST_FAILED;

        DebugTraceF("Ch%d DAC test, I_set failure: expected=%d, got=%d, abs diff=%d",
            channel.index,
            (int)(i_set * 100),
            (int)(i_mon * 100),
            (int)(i_diff * 100));
    }

    channel.calibrationEnableNoEvent(saveCalibrationEnabled);

    // Re-enable output
    if (save_output_enabled) {
        channel.flags.outputEnabled = true;
        channel.ioexp.changeBit(IOExpander::IO_BIT_OUT_OUTPUT_ENABLE, true);
    }

    channel.setVoltage(u_set_save);
    channel.setCurrent(i_set_save);

    if (g_testResult == psu::TEST_FAILED) {
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

    m_testing = false;

    return g_testResult != psu::TEST_FAILED;
}

////////////////////////////////////////////////////////////////////////////////

void DigitalAnalogConverter::set_voltage(float value) {
    set_value(DATA_BUFFER_A, util::remap(value, channel.U_MIN, (float)DAC_MIN, channel.U_MAX, (float)DAC_MAX));
}

void DigitalAnalogConverter::set_current(float value) {
    set_value(DATA_BUFFER_B, util::remap(value, channel.I_MIN, (float)DAC_MIN, channel.getDualRangeMax(), (float)DAC_MAX));
}

void DigitalAnalogConverter::set_voltage(uint16_t voltage) {
    set_value(DATA_BUFFER_A, voltage);
}

void DigitalAnalogConverter::set_current(uint16_t current) {
    set_value(DATA_BUFFER_B, current);
}

}
} // namespace eez::psu
