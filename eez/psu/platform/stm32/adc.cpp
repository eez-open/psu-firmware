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
#include "eez/psu/adc.h"
#include "eez/psu/channel_dispatcher.h"

namespace eez {
namespace psu {

////////////////////////////////////////////////////////////////////////////////

AnalogDigitalConverter::AnalogDigitalConverter(Channel &channel_) : channel(channel_) {
    g_testResult = psu::TEST_SKIPPED;
}

void AnalogDigitalConverter::init() {
}

bool AnalogDigitalConverter::test() {
    g_testResult = psu::TEST_OK;
    return g_testResult != psu::TEST_FAILED;
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
    return 0;
}

}
} // namespace eez::psu
