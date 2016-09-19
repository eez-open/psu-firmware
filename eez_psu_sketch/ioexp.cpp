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
#include "adc.h"

namespace eez {
namespace psu {

////////////////////////////////////////////////////////////////////////////////

#define IPOL    0B00000000 // no pin is inverted
#define GPINTEN 0B00000001 // enable interrupt for pin 0
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

static void ioexp_interrupt_ch1() {
    Channel::get(0).ioexp.on_interrupt();
}

static void ioexp_interrupt_ch2() {
    Channel::get(1).ioexp.on_interrupt();
}

////////////////////////////////////////////////////////////////////////////////

IOExpander::IOExpander(
    Channel &channel_, 
    uint8_t IO_BIT_OUT_SET_100_PERCENT_,
    uint8_t IO_BIT_OUT_EXT_PROG_
)
    : channel(channel_)
    , IO_BIT_OUT_SET_100_PERCENT(IO_BIT_OUT_SET_100_PERCENT_)
    , IO_BIT_OUT_EXT_PROG(IO_BIT_OUT_EXT_PROG_)
	, writeDisabled(false)
{
    test_result = psu::TEST_SKIPPED;
	gpio = channel.ioexp_gpio_init;
}

uint8_t IOExpander::getRegInitValue(int i) {
	if (REG_VALUES[i] == IOExpander::REG_IODIR) {
        return channel.ioexp_iodir;
    } else if (REG_VALUES[i] == IOExpander::REG_GPIO) {
        return channel.ioexp_gpio_init;
    } else {
        return REG_VALUES[i + 1];
    }
}

bool IOExpander::init() {
    for (int i = 0; REG_VALUES[i] != 0xFF; i += 3) {
		reg_write(REG_VALUES[i], getRegInitValue(i));
    }

    int intNum = digitalPinToInterrupt(channel.convend_pin);
    SPI.usingInterrupt(intNum);
    attachInterrupt(
        intNum,
        channel.index == 1 ? ioexp_interrupt_ch1 : ioexp_interrupt_ch2,
        FALLING
        );

    return test();
}

bool IOExpander::test() {
    test_result = psu::TEST_OK;

    for (int i = 0; REG_VALUES[i] != 0xFF; i += 3) {
        if (REG_VALUES[i] == IOExpander::REG_IODIR || REG_VALUES[i + 2]) {
            uint8_t value = reg_read(REG_VALUES[i]);
            uint8_t compare_with_value = getRegInitValue(i);
            if (value != compare_with_value) {
                DebugTraceF("Ch%d IO expander reg check failure: reg=%d, expected=%d, got=%d",
                    channel.index, (int)REG_VALUES[i], (int)compare_with_value, (int)value);

                test_result = psu::TEST_FAILED;
                break;
            }
        }
    }

    if (test_result == psu::TEST_OK) {
#if !CONF_SKIP_PWRGOOD_TEST
        channel.flags.power_ok = test_bit(IO_BIT_IN_PWRGOOD);
        if (!channel.flags.power_ok) {
            DebugTraceF("Ch%d power fault", channel.index);
            psu::generateError(SCPI_ERROR_CH1_FAULT_DETECTED - (channel.index - 1));
        }
#else
		channel.flags.power_ok = 1;
#endif
    }
    else {
        channel.flags.power_ok = 0;
    }

    if (test_result == psu::TEST_FAILED) {
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

    return test_result != psu::TEST_FAILED;
}

void IOExpander::tick(unsigned long tick_usec) {
}

uint8_t IOExpander::read_gpio() {
	return reg_read(REG_GPIO);
}

bool IOExpander::test_bit(int io_bit) {
    uint8_t value = read_gpio();
    return value & (1 << io_bit) ? true : false;
}

void IOExpander::change_bit(int io_bit, bool set) {
	noInterrupts();
    
	uint8_t newValue = set ? (gpio | (1 << io_bit)) : (gpio & ~(1 << io_bit));
	if (gpio != newValue) {
		gpio = newValue;
		if (!writeDisabled) {
			reg_write(REG_GPIO, gpio);
			gpioWritten = gpio;
		}
	}
	
	interrupts();
}

void IOExpander::writeDisable() {
	writeDisabled = true;
}

void IOExpander::writeEnable() {
	noInterrupts();
	
	writeDisabled = false;
	if (gpioWritten != gpio) {
		reg_write(REG_GPIO, gpioWritten);
		gpioWritten = gpio;
	}

	interrupts();
}


void IOExpander::on_interrupt() {
	g_insideInterruptHandler = true;

    // IMPORTANT!
    // Read ADC first, then INTF and GPIO.
    // Otherwise, it will generate 2 interrupts for single ADC start shot!
    int16_t adc_data = channel.adc.read();
    uint8_t gpio = reg_read(REG_GPIO);

    channel.event(gpio, adc_data);

#if CONF_DEBUG
    debug::ioexpIntTick(micros());
#endif

	g_insideInterruptHandler = false;
}

uint8_t IOExpander::reg_read_write(uint8_t opcode, uint8_t reg, uint8_t val) {
    SPI.beginTransaction(MCP23S08_SPI);
    digitalWrite(channel.isolator_pin, ISOLATOR_ENABLE);
    digitalWrite(channel.ioexp_pin, LOW);
    SPI.transfer(opcode);
    SPI.transfer(reg);
    uint8_t result = SPI.transfer(val);
    digitalWrite(channel.ioexp_pin, HIGH);
    digitalWrite(channel.isolator_pin, ISOLATOR_DISABLE);
    SPI.endTransaction();
    return result;
}

uint8_t IOExpander::reg_read(uint8_t reg) {
    return reg_read_write(IOEXP_READ, reg, 0);
}

void IOExpander::reg_write(uint8_t reg, uint8_t val) {
    reg_read_write(IOEXP_WRITE, reg, val);
	if (reg == REG_GPIO) {
		DebugTraceF("Ch%d GPIO 0x%02x", (int)channel.index, (int)val);
	}
}

}
} // namespace eez::psu
