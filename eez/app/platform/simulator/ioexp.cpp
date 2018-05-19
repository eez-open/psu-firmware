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
#include "eez/app/ioexp.h"

namespace eez {
namespace app {

////////////////////////////////////////////////////////////////////////////////

IOExpander::IOExpander(
	Channel &channel_,
	uint8_t IO_BIT_OUT_SET_100_PERCENT_,
	uint8_t IO_BIT_OUT_EXT_PROG_
)
	: IO_BIT_OUT_SET_100_PERCENT(IO_BIT_OUT_SET_100_PERCENT_)
	, IO_BIT_OUT_EXT_PROG(IO_BIT_OUT_EXT_PROG_)
	, channel(channel_) {
	g_testResult = TEST_SKIPPED;

	gpioa = channel.ioexp_gpio_init;
	gpiob = 0B00000001; // 5A
}

void IOExpander::init() {
}

bool IOExpander::test() {
	g_testResult = TEST_OK;
	channel.flags.powerOk = 1;
	return g_testResult != TEST_FAILED;
}

void IOExpander::tick(uint32_t tick_usec) {
	if (isPowerUp()) {
		uint8_t gpio0 = readGpio();
		channel.eventGpio(gpio0);
	}
}

uint8_t IOExpander::readGpio() {
	uint8_t result = gpioa;

	if (simulator::getPwrgood(channel.ioexp_pin)) {
		result |= 1 << IOExpander::IO_BIT_IN_PWRGOOD;
	} else {
		result &= ~(1 << IOExpander::IO_BIT_IN_PWRGOOD);
	}

	if (channel.getFeatures() & CH_FEATURE_RPOL) {
		if (!simulator::getRPol(channel.ioexp_pin)) {
			result |= 1 << IOExpander::IO_BIT_IN_RPOL;
		} else {
			result &= ~(1 << IOExpander::IO_BIT_IN_RPOL);
		}
	}

	if (simulator::getCV(channel.ioexp_pin)) {
		result |= 1 << IOExpander::IO_BIT_IN_CV_ACTIVE;
	} else {
		result &= ~(1 << IOExpander::IO_BIT_IN_CV_ACTIVE);
	}

	if (simulator::getCC(channel.ioexp_pin)) {
		result |= 1 << IOExpander::IO_BIT_IN_CC_ACTIVE;
	} else {
		result &= ~(1 << IOExpander::IO_BIT_IN_CC_ACTIVE);
	}

	return result;
}

bool IOExpander::testBit(int io_bit) {
	uint8_t value = readGpio();
	return value & (1 << io_bit) ? true : false;
}

void IOExpander::changeBit(int io_bit, bool set) {
	if (io_bit < 8) {
		uint8_t newValue = set ? (gpioa | (1 << io_bit)) : (gpioa & ~(1 << io_bit));
		if (gpioa != newValue) {
			gpioa = newValue;
		}
	} else {
		uint8_t newValue = set ? (gpiob | (1 << (io_bit - 8))) : (gpiob & ~(1 << (io_bit - 8)));
		if (gpiob != newValue) {
			gpiob = newValue;
		}
	}
}

}
} // namespace eez::app
