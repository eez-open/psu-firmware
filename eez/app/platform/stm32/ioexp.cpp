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

#define IPOL    0B00000000 // no pin is inverted
#define GPINTEN 0B00000000 // no interrupts
#define DEFVAL  0B00000000 //
#define INTCON  0B00000000 //
#define IOCON   0B00100000 // sequential operation disabled, hw addressing disabled
#define GPPU    0B00100100 // pull up with 100K resistor pins 2 and 5

static const uint8_t REG_VALUES_8[] = {
    // reg                   value    test
    IOExpander::REG_IODIR,   0,       1,
    IOExpander::REG_IPOL,    IPOL,    1,
    IOExpander::REG_GPINTEN, GPINTEN, 1,
    IOExpander::REG_DEFVAL,  DEFVAL,   1,
    IOExpander::REG_INTCON,  INTCON,  1,
    IOExpander::REG_IOCON,   IOCON,   1,
    IOExpander::REG_GPPU,    GPPU,    1,
    IOExpander::REG_GPIO,    0,       0,
    0xFF
};

static const uint8_t REG_VALUES_16[] = {
    // reg                   value    test
    IOExpander::REG_IODIRA,   0,       1,
    IOExpander::REG_IODIRB,   0,       1,
    IOExpander::REG_IPOLA,    IPOL,    1,
    IOExpander::REG_GPINTENA, GPINTEN, 1,
    IOExpander::REG_DEFVALA,  DEFVAL,  1,
    IOExpander::REG_INTCONA,  INTCON,  1,
    IOExpander::REG_IOCONA,   IOCON,   1,
    IOExpander::REG_GPPUA,    GPPU,    1,
    IOExpander::REG_GPIOA,    0,       0,
    IOExpander::REG_GPIOB,    0,       0,
    0xFF
};

////////////////////////////////////////////////////////////////////////////////

IOExpander::IOExpander(
    Channel &channel_,
    uint8_t IO_BIT_OUT_SET_100_PERCENT_,
    uint8_t IO_BIT_OUT_EXT_PROG_
)
    : IO_BIT_OUT_SET_100_PERCENT(IO_BIT_OUT_SET_100_PERCENT_)
    , IO_BIT_OUT_EXT_PROG(IO_BIT_OUT_EXT_PROG_)
	, channel(channel_)
{
    g_testResult = TEST_SKIPPED;

    gpioa = channel.ioexp_gpio_init;
    gpiob = 0B00000001; // 5A
}

uint8_t IOExpander::getRegInitValue(int i) {
    if (channel.boardRevision == CH_BOARD_REVISION_R5B12) {
	    if (REG_VALUES_16[i] == IOExpander::REG_IODIRA) {
            return channel.ioexp_iodir;
        } else if (REG_VALUES_16[i] == IOExpander::REG_IODIRB) {
            return 0; // // bits 8-15 are all output
        } else if (REG_VALUES_16[i] == IOExpander::REG_GPIOA) {
            return gpioa;
        } else if (REG_VALUES_16[i] == IOExpander::REG_GPIOB) {
            return gpiob;
        } else {
            return REG_VALUES_16[i + 1];
        }
    } else {
	    if (REG_VALUES_8[i] == IOExpander::REG_IODIR) {
            return channel.ioexp_iodir;
        } else if (REG_VALUES_8[i] == IOExpander::REG_GPIO) {
            return gpioa;
        } else {
            return REG_VALUES_8[i + 1];
        }
    }
}

void IOExpander::init() {
    const uint8_t *regValues = channel.boardRevision == CH_BOARD_REVISION_R5B12 ? REG_VALUES_16 : REG_VALUES_8;

    for (int i = 0; regValues[i] != 0xFF; i += 3) {
		reg_write(regValues[i], getRegInitValue(i));
    }
}

bool IOExpander::test() {
    g_testResult = TEST_OK;

    const uint8_t *regValues = channel.boardRevision == CH_BOARD_REVISION_R5B12 ? REG_VALUES_16 : REG_VALUES_8;

    for (int i = 0; regValues[i] != 0xFF; i += 3) {
        if (regValues[i + 2]) {
            uint8_t value = reg_read(regValues[i]);
            uint8_t compare_with_value = getRegInitValue(i);
            if (value != compare_with_value) {
                DebugTraceF("Ch%d IO expander reg check failure: reg=%d, expected=%d, got=%d",
                    channel.index, (int)regValues[i], (int)compare_with_value, (int)value);

                g_testResult = TEST_FAILED;
                break;
            }
        }
    }

    if (g_testResult == TEST_OK) {
#if !CONF_SKIP_PWRGOOD_TEST
        channel.flags.powerOk = testBit(IO_BIT_IN_PWRGOOD);
        if (!channel.flags.powerOk) {
            DebugTraceF("Ch%d power fault", channel.index);
            generateError(SCPI_ERROR_CH1_FAULT_DETECTED - (channel.index - 1));
        }
#else
		channel.flags.powerOk = 1;
#endif
    }
    else {
        channel.flags.powerOk = 0;
    }

    if (g_testResult == TEST_FAILED) {
        if (channel.index == 1) {
            generateError(SCPI_ERROR_CH1_IOEXP_TEST_FAILED);
        }
        else if (channel.index == 2) {
            generateError(SCPI_ERROR_CH2_IOEXP_TEST_FAILED);
        }
        else {
            // TODO
        }
    }

    return g_testResult != TEST_FAILED;
}

void IOExpander::tick(uint32_t tick_usec) {
    if (isPowerUp()) {
        uint8_t gpio0 = readGpio();
        channel.eventGpio(gpio0);
    }
}

uint8_t IOExpander::readGpio() {
    if (channel.boardRevision == CH_BOARD_REVISION_R5B12) {
    	return reg_read(REG_GPIOA);
    } else {
        return reg_read(REG_GPIO);
    }
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
			reg_write(channel.boardRevision == CH_BOARD_REVISION_R5B12 ? REG_GPIOA : REG_GPIO, gpioa);
	    }
    } else {
        uint8_t newValue = set ? (gpiob | (1 << (io_bit - 8))) : (gpiob & ~(1 << (io_bit - 8)));
	    if (gpiob != newValue) {
		    gpiob = newValue;
			reg_write(REG_GPIOB, gpiob);
	    }
    }
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
} // namespace eez::app
