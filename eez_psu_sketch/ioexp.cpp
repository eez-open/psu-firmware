/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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

#define IODIR   0B01111101 // pins 1 and 7 set as output
#define IPOL    0B00100100 // pins 2 and 5 are set as inverted
#define GPINTEN 0B00000001 // enable interrupt for pin 0
#define DEVAL   0B00000000 // 
#define INTCON  0B00000000 // 
#define IOCON   0B00100000 // sequential operation disabled, hw addressing disabled
#define GPPU    0B00000000 // do not pull up with 100K resistor
#define GPIO    0B00000010 // 

static const uint8_t REG_VALUES[] = {
    // reg                   value    test
    IOExpander::REG_IODIR,   IODIR,   1,
    IOExpander::REG_IPOL,    IPOL,    1,
    IOExpander::REG_GPINTEN, GPINTEN, 1,
    IOExpander::REG_DEVAL,   DEVAL,   1,
    IOExpander::REG_INTCON,  INTCON,  1,
    IOExpander::REG_IOCON,   IOCON,   1,
    IOExpander::REG_GPPU,    GPPU,    1,
    IOExpander::REG_GPIO,    GPIO,    0,
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

IOExpander::IOExpander(Channel &channel_) : channel(channel_) {
    test_result = psu::TEST_SKIPPED;
}

bool IOExpander::init() {
    for (int i = 0; REG_VALUES[i] != 0xFF; i += 3) {
        reg_write(REG_VALUES[i], REG_VALUES[i + 1]);
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
        if (REG_VALUES[i + 2]) {
            uint8_t value = reg_read(REG_VALUES[i]);

            if (value != REG_VALUES[i + 1]) {
                DebugTrace("Ch%d IO expander reg check failure: reg=%d, expected=%d, got=%d",
                    channel.index, (int)REG_VALUES[i], (int)REG_VALUES[i + 1], (int)value);

                test_result = psu::TEST_FAILED;
                break;
            }
        }
    }

    if (test_result == psu::TEST_OK) {
        channel.flags.power_ok = test_bit(IO_BIT_IN_PWRGOOD);
        if (!channel.flags.power_ok) {
            DebugTrace("Ch%d power fault", channel.index);
            psu::generateError(SCPI_ERROR_CHANNEL_FAULT_DETECTED);
        }
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

bool IOExpander::test_bit(int io_bit) {
    uint8_t value = reg_read(REG_GPIO);
    return value & (1 << io_bit) ? true : false;
}

void IOExpander::change_bit(int io_bit, bool set) {
    uint8_t value = reg_read(REG_GPIO);
    uint8_t newValue = set ? (value | (1 << io_bit)) : (value & ~(1 << io_bit));
    reg_write(REG_GPIO, newValue);
}

void IOExpander::on_interrupt() {
    // IMPORTANT!
    // Read ADC first, then INTF and GPIO.
    // Otherwise, it will generate 2 interrupts for single ADC start shot!
    int16_t adc_data = channel.adc.read();
    uint8_t gpio = reg_read(REG_GPIO);

    channel.event(gpio, adc_data);

#if CONF_DEBUG
    debug::ioexpIntTick(micros());
#endif
}

uint8_t IOExpander::reg_read_write(uint8_t opcode, uint8_t reg, uint8_t val) {
    SPI.beginTransaction(MCP23S08_SPI);
    digitalWrite(channel.isolator_pin, LOW);
    digitalWrite(channel.ioexp_pin, LOW);
    SPI.transfer(opcode);
    SPI.transfer(reg);
    uint8_t result = SPI.transfer(val);
    digitalWrite(channel.ioexp_pin, HIGH);
    digitalWrite(channel.isolator_pin, HIGH);
    SPI.endTransaction();
    return result;
}

uint8_t IOExpander::reg_read(uint8_t reg) {
    return reg_read_write(IOEXP_READ, reg, 0);
}

void IOExpander::reg_write(uint8_t reg, uint8_t val) {
    reg_read_write(IOEXP_WRITE, reg, val);
}

}
} // namespace eez::psu
