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

#include "eez/psu/psu.h"
#include "eez/psu/dac.h"

namespace eez {
namespace psu {

////////////////////////////////////////////////////////////////////////////////

static const uint8_t DATA_BUFFER_A = 0B00010000;
static const uint8_t DATA_BUFFER_B = 0B00100100;

static const uint16_t DAC_MIN = 0;
static const uint16_t DAC_MAX = (1L << DAC_RES) - 1;

extern uint16_t g_uSet[CH_MAX];
extern uint16_t g_iSet[CH_MAX];

////////////////////////////////////////////////////////////////////////////////

DigitalAnalogConverter::DigitalAnalogConverter(Channel &channel_) : channel(channel_) {
	g_testResult = psu::TEST_SKIPPED;
}

void DigitalAnalogConverter::set_value(uint8_t buffer, uint16_t value) {
	if (buffer == DATA_BUFFER_A) {
		float adcValue = remap(value,
			DigitalAnalogConverter::DAC_MIN, AnalogDigitalConverter::ADC_MIN,
			DigitalAnalogConverter::DAC_MAX, AnalogDigitalConverter::ADC_MAX);
		g_uSet[channel.index - 1] = (uint16_t)clamp(adcValue,
			AnalogDigitalConverter::ADC_MIN, AnalogDigitalConverter::ADC_MAX);

	} else {
		float adcValue = remap(value,
			DigitalAnalogConverter::DAC_MIN, AnalogDigitalConverter::ADC_MIN,
			DigitalAnalogConverter::DAC_MAX, AnalogDigitalConverter::ADC_MAX);
		g_iSet[channel.index - 1] = (uint16_t)clamp(adcValue,
			AnalogDigitalConverter::ADC_MIN, AnalogDigitalConverter::ADC_MAX);
	}

#if CONF_DEBUG
	if (buffer == DATA_BUFFER_A) {
		debug::g_uDac[channel.index - 1].set(value);
	} else {
		debug::g_iDac[channel.index - 1].set(value);
	}
#endif
}

void DigitalAnalogConverter::set_value(uint8_t buffer, float value) {
	set_value(buffer, (uint16_t)clamp(round(value), DAC_MIN, DAC_MAX));
}

////////////////////////////////////////////////////////////////////////////////

void DigitalAnalogConverter::init() {
}

bool DigitalAnalogConverter::test() {
	g_testResult = psu::TEST_OK;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

void DigitalAnalogConverter::set_voltage(float value) {
	set_value(DATA_BUFFER_A, remap(value, channel.U_MIN, (float)DAC_MIN, channel.U_MAX, (float)DAC_MAX));
}

void DigitalAnalogConverter::set_current(float value) {
	set_value(DATA_BUFFER_B, remap(value, channel.I_MIN, (float)DAC_MIN, channel.getDualRangeMax(), (float)DAC_MAX));
}

void DigitalAnalogConverter::set_voltage(uint16_t voltage) {
	set_value(DATA_BUFFER_A, voltage);
}

void DigitalAnalogConverter::set_current(uint16_t current) {
	set_value(DATA_BUFFER_B, current);
}

}
} // namespace eez::psu
