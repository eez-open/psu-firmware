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
#include "ioexp.h"

namespace eez {
namespace psu {

////////////////////////////////////////////////////////////////////////////////

#define IPOL    0B00000000 // no pin is inverted
#define GPINTEN 0B00000000 // no interrupts
#define DEVAL   0B00000000 // 
#define INTCON  0B00000000 // 
#define IOCON   0B00100000 // sequential operation disabled, hw addressing disabled
#define GPPU    0B00100100 // pull up with 100K resistor pins 2 and 5

static const uint8_t REG_VALUES[] = {
    // reg                   value    test
    IOExpander::REG_IODIR,   0,       1,
    IOExpander::REG_IPOL,    IPOL,    1,
    IOExpander::REG_GPINTEN, GPINTEN, 1,
    IOExpander::REG_DEVAL,   DEVAL,   1,
    IOExpander::REG_INTCON,  INTCON,  1,
    IOExpander::REG_IOCON,   IOCON,   1,
    IOExpander::REG_GPPU,    GPPU,    1,
    IOExpander::REG_GPIO,    0,       0,
    0xFF
};

////////////////////////////////////////////////////////////////////////////////

IOExpander::IOExpander(
    Channel &channel_, 
    uint8_t IO_BIT_OUT_SET_100_PERCENT_,
    uint8_t IO_BIT_OUT_EXT_PROG_
)
    : channel(channel_)
    , IO_BIT_OUT_SET_100_PERCENT(IO_BIT_OUT_SET_100_PERCENT_)
    , IO_BIT_OUT_EXT_PROG(IO_BIT_OUT_EXT_PROG_)
{
    g_testResult = psu::TEST_SKIPPED;
	gpio = channel.ioexp_gpio_init;
    gpio_changed = false;
}

uint8_t IOExpander::getRegInitValue(int i) {
	if (REG_VALUES[i] == IOExpander::REG_IODIR) {
        return channel.ioexp_iodir;
    } else if (REG_VALUES[i] == IOExpander::REG_GPIO) {
        return gpio;
    } else {
        return REG_VALUES[i + 1];
    }
}

void IOExpander::init() {
    for (int i = 0; REG_VALUES[i] != 0xFF; i += 3) {
		reg_write(REG_VALUES[i], getRegInitValue(i));
    }
}

bool IOExpander::test() {
    g_testResult = psu::TEST_OK;

    for (int i = 0; REG_VALUES[i] != 0xFF; i += 3) {
        if (REG_VALUES[i] == IOExpander::REG_IODIR || REG_VALUES[i + 2]) {
            uint8_t value = reg_read(REG_VALUES[i]);
            uint8_t compare_with_value = getRegInitValue(i);
            if (value != compare_with_value) {
                DebugTraceF("Ch%d IO expander reg check failure: reg=%d, expected=%d, got=%d",
                    channel.index, (int)REG_VALUES[i], (int)compare_with_value, (int)value);

                g_testResult = psu::TEST_FAILED;
                break;
            }
        }
    }

    if (g_testResult == psu::TEST_OK) {
#if !CONF_SKIP_PWRGOOD_TEST
        channel.flags.powerOk = testBit(IO_BIT_IN_PWRGOOD);
        if (!channel.flags.powerOk) {
            DebugTraceF("Ch%d power fault", channel.index);
            psu::generateError(SCPI_ERROR_CH1_FAULT_DETECTED - (channel.index - 1));
        }
#else
		channel.flags.powerOk = 1;
#endif
    }
    else {
        channel.flags.powerOk = 0;
    }

    if (g_testResult == psu::TEST_FAILED) {
        if (channel.index == 1) {
            psu::generateError(SCPI_ERROR_CH1_IOEXP_TEST_FAILED);
        }
        else if (channel.index == 2) {
            psu::generateError(SCPI_ERROR_CH2_IOEXP_TEST_FAILED);
        }
        else {
            // TODO
        }
    }

    return g_testResult != psu::TEST_FAILED;
}

void IOExpander::tick(uint32_t tick_usec) {
    if (isPowerUp()) {
        uint8_t gpio = readGpio();

        if (gpio_changed) {
            if ((gpio & (1 << IO_BIT_OUT_DP_ENABLE)) != (this->gpio & (1 << IO_BIT_OUT_DP_ENABLE))) {
                DebugTrace("IOEXP write check failed for DP_ENABLE");
            }

            if ((gpio & (1 << IO_BIT_OUT_SET_100_PERCENT)) != (this->gpio & (1 << IO_BIT_OUT_SET_100_PERCENT))) {
                DebugTrace("IOEXP write check failed for SET_100_PERCENT");
            }

            if ((gpio & (1 << IO_BIT_OUT_EXT_PROG)) != (this->gpio & (1 << IO_BIT_OUT_EXT_PROG))) {
                DebugTrace("IOEXP write check failed for OUT_EXT_PROG");
            }

            if ((gpio & (1 << IO_BIT_OUT_OUTPUT_ENABLE)) != (this->gpio & (1 << IO_BIT_OUT_OUTPUT_ENABLE))) {
                DebugTrace("IOEXP write check failed for OUTPUT_ENABLE");
            }

            gpio_changed = false;
        }

        channel.eventGpio(gpio);
    }
}

uint8_t IOExpander::readGpio() {
	return reg_read(REG_GPIO);
}

bool IOExpander::testBit(int io_bit) {
    uint8_t value = readGpio();
    return value & (1 << io_bit) ? true : false;
}

void IOExpander::changeBit(int io_bit, bool set) {
    uint8_t newValue = set ? (gpio | (1 << io_bit)) : (gpio & ~(1 << io_bit));
	if (gpio != newValue) {
		gpio = newValue;
		if (!writeDisabled) {
			reg_write(REG_GPIO, gpio);
            gpio_changed = true;
		}
	}
}

void IOExpander::disableWrite() {
	writeDisabled = true;
}

void IOExpander::enableWriteAndFlush() {
	writeDisabled = false;
	
	reg_write(REG_GPIO, gpio);
    gpio_changed = true;
}

uint8_t IOExpander::reg_read(uint8_t reg) {
    SPI_beginTransaction(MCP23S08_SPI);
    digitalWrite(channel.isolator_pin, ISOLATOR_ENABLE);
    digitalWrite(channel.ioexp_pin, LOW);
    SPI.transfer(IOEXP_READ);
    SPI.transfer(reg);
    uint8_t result = SPI.transfer(0);
    digitalWrite(channel.ioexp_pin, HIGH);
    digitalWrite(channel.isolator_pin, ISOLATOR_DISABLE);
    SPI_endTransaction();
    return result;
}

void IOExpander::reg_write(uint8_t reg, uint8_t val) {
    SPI_beginTransaction(MCP23S08_SPI);
    digitalWrite(channel.ioexp_pin, LOW);
    SPI.transfer(IOEXP_WRITE);
    SPI.transfer(reg);
    SPI.transfer(val);
    digitalWrite(channel.ioexp_pin, HIGH);
    SPI_endTransaction();
}

}
} // namespace eez::psu
