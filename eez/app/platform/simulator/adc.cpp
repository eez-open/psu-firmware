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
#include "eez/app/adc.h"
#include "eez/app/channel_dispatcher.h"

namespace eez {
namespace app {

static uint16_t g_uMon[CH_MAX];
static uint16_t g_iMon[CH_MAX];

uint16_t g_uSet[CH_MAX];
uint16_t g_iSet[CH_MAX];

void updateValues(Channel& channel) {
	int i = channel.index - 1;

	if (channel.simulator.getLoadEnabled()) {
		float u_set_v = channel.isRemoteProgrammingEnabled() ? remap(channel.simulator.voltProgExt, 0, 0, 2.5, channel.u.max) : channel.remapAdcDataToVoltage(g_uSet[i]);
		float i_set_a = channel.remapAdcDataToCurrent(g_iSet[i]);

		float u_mon_v = i_set_a * channel.simulator.load;
		float i_mon_a = i_set_a;
		if (u_mon_v > u_set_v) {
			u_mon_v = u_set_v;
			i_mon_a = u_set_v / channel.simulator.load;

			simulator::setCV(channel.ioexp_pin, true);
			simulator::setCC(channel.ioexp_pin, false);
		} else {
			simulator::setCV(channel.ioexp_pin, false);
			simulator::setCC(channel.ioexp_pin, true);
		}

		g_uMon[i] = channel.remapVoltageToAdcData(u_mon_v);
		g_iMon[i] = channel.remapCurrentToAdcData(i_mon_a);

		return;
	} else {
		if (channel.isOutputEnabled()) {
			g_uMon[i] = g_uSet[i];
			g_iMon[i] = 0;
			if (g_uSet[i] > 0 && g_iSet[i] > 0) {
				simulator::setCV(channel.ioexp_pin, true);
				simulator::setCC(channel.ioexp_pin, false);
			} else {
				simulator::setCV(channel.ioexp_pin, false);
				simulator::setCC(channel.ioexp_pin, true);
			}
			return;
		}
	}

	g_uMon[i] = 0;
	g_iMon[i] = 0;
	simulator::setCV(channel.ioexp_pin, false);
	simulator::setCC(channel.ioexp_pin, false);
}

AnalogDigitalConverter::AnalogDigitalConverter(Channel &channel_) : channel(channel_) {
	g_testResult = TEST_SKIPPED;
}

void AnalogDigitalConverter::init() {
}

bool AnalogDigitalConverter::test() {
	g_testResult = TEST_OK;
	return true;
}

void AnalogDigitalConverter::tick(uint32_t tick_usec) {
	if (start_reg0) {
		int16_t adc_data = read();
		channel.eventAdcData(adc_data);
#if CONF_DEBUG
		debug::g_adcCounter.inc();
#endif
	}
}

void AnalogDigitalConverter::start(uint8_t reg0) {
	start_reg0 = reg0;
}

int16_t AnalogDigitalConverter::read() {
	updateValues(channel);

	if (start_reg0 == AnalogDigitalConverter::ADC_REG0_READ_U_MON) {
		return g_uMon[channel.index - 1];
	}
	if (start_reg0 == AnalogDigitalConverter::ADC_REG0_READ_I_MON) {
		return g_iMon[channel.index - 1];
	}
	if (start_reg0 == AnalogDigitalConverter::ADC_REG0_READ_U_SET) {
		return g_uSet[channel.index - 1];
	} else {
		return g_iSet[channel.index - 1];
	}
}

}
} // namespace eez::app
