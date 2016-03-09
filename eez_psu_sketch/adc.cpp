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
#include "adc.h"

namespace eez {
namespace psu {

////////////////////////////////////////////////////////////////////////////////

static const uint8_t ADC_REG2_VAL = 0B01100000; // Register 02h: External Vref, 50Hz rejection, PSW off, IDAC off
static const uint8_t ADC_REG3_VAL = 0B00000000; // Register 03h: IDAC1 disabled, IDAC2 disabled, dedicated DRDY

////////////////////////////////////////////////////////////////////////////////

AnalogDigitalConverter::AnalogDigitalConverter(Channel &channel_) : channel(channel_) {
    test_result = psu::TEST_SKIPPED;

    current_sps = ADC_SPS;
}

uint8_t AnalogDigitalConverter::getReg1Val() {
    return (current_sps << 5) | 0B00000000; // Register 01h: 256KHz modulator clock, Single shot, disable temp sensor, Current sources off
}

bool AnalogDigitalConverter::init() {
    SPI.beginTransaction(ADS1120_SPI);
    digitalWrite(channel.isolator_pin, LOW);
    digitalWrite(channel.adc_pin, LOW);

    // Send RESET command
    SPI.transfer(ADC_RESET);
    delayMicroseconds(100); // Guard time

    SPI.transfer(ADC_WR3S1);

    uint8_t reg1_val = (current_sps << 5) | 0B00000000; // Register 01h: 256KHz modulator clock, Single shot, disable temp sensor, Current sources off
    SPI.transfer(reg1_val);
    
    SPI.transfer(ADC_REG2_VAL);
    SPI.transfer(ADC_REG3_VAL);

    digitalWrite(channel.adc_pin, HIGH);
    digitalWrite(channel.isolator_pin, HIGH);
    SPI.endTransaction();

    return test();
}

bool AnalogDigitalConverter::test() {
    SPI.beginTransaction(ADS1120_SPI);
    digitalWrite(channel.isolator_pin, LOW);
    digitalWrite(channel.adc_pin, LOW);

    SPI.transfer(ADC_RD3S1);
    byte reg1 = SPI.transfer(0);
    byte reg2 = SPI.transfer(0);
    byte reg3 = SPI.transfer(0);

    digitalWrite(channel.adc_pin, HIGH);
    digitalWrite(channel.isolator_pin, HIGH);
    SPI.endTransaction();

    test_result = psu::TEST_OK;

    if (reg1 != getReg1Val()) {
        DebugTraceF("Ch%d ADC test failed reg1: expected=%d, got=%d", channel.index, getReg1Val(), reg1);
        test_result = psu::TEST_FAILED;
    }

    if (reg2 != ADC_REG2_VAL) {
        DebugTraceF("Ch%d ADC test failed reg2: expected=%d, got=%d", channel.index, ADC_REG2_VAL, reg2);
        test_result = psu::TEST_FAILED;
    }

    if (reg3 != ADC_REG3_VAL) {
        DebugTraceF("Ch%d ADC test failed reg3: expected=%d, got=%d", channel.index, ADC_REG3_VAL, reg3);
        test_result = psu::TEST_FAILED;
    }

    if (test_result == psu::TEST_FAILED) {
        if (channel.index == 1) {
            psu::generateError(SCPI_ERROR_CH1_ADC_TEST_FAILED);
        }
        else if (channel.index == 2) {
            psu::generateError(SCPI_ERROR_CH2_ADC_TEST_FAILED);
        }
        else {
            // TODO
        }
    }

    return test_result != psu::TEST_FAILED;
}

void AnalogDigitalConverter::tick(unsigned long tick_usec) {
    if (channel.isOutputEnabled()) {
        noInterrupts();
        unsigned long last_adc_start_time = start_time;
        interrupts();
        long diff = tick_usec - last_adc_start_time;
        if (diff > ADC_TIMEOUT_MS * 1000L) {
            if (adc_timeout_recovery_attempts_counter < MAX_ADC_TIMEOUT_RECOVERY_ATTEMPTS) {
                ++adc_timeout_recovery_attempts_counter;

                DebugTraceF("ADC timeout (%lu) detected on CH%d, recovery attempt no. %d", diff, channel.index, adc_timeout_recovery_attempts_counter);

                if (channel.init()) {
                    start(ADC_REG0_READ_U_MON);
                }
            }
            else {
                if (channel.index == 1) {
                    psu::generateError(SCPI_ERROR_CH1_ADC_TIMEOUT_DETECTED);
                }
                else if (channel.index == 2) {
                    psu::generateError(SCPI_ERROR_CH1_ADC_TIMEOUT_DETECTED);
                }
                else {
                    // TODO
                }

                channel.outputEnable(false);
                channel.remoteSensingEnable(false);
            }
        }
        else {
            adc_timeout_recovery_attempts_counter = 0;
        }
    }
}

void AnalogDigitalConverter::start(uint8_t reg0) {
    SPI.beginTransaction(ADS1120_SPI);
    digitalWrite(channel.isolator_pin, LOW);
    digitalWrite(channel.adc_pin, LOW);

    uint8_t sps = psu::isTimeCriticalMode() ? ADC_SPS_TIME_CRITICAL : ADC_SPS;
    if (sps != current_sps) {
        current_sps = sps;
        SPI.transfer(ADC_WR4S0);
        SPI.transfer(reg0);
        SPI.transfer(getReg1Val());
        SPI.transfer(ADC_REG2_VAL);
        SPI.transfer(ADC_REG3_VAL);
    } else {
        SPI.transfer(ADC_WR1S0);
        SPI.transfer(reg0);
    }

    start_reg0 = reg0;
    start_time = micros();

    // Start conversion (single shot)
    SPI.transfer(ADC_START);

    digitalWrite(channel.adc_pin, HIGH);
    digitalWrite(channel.isolator_pin, HIGH);
    SPI.endTransaction();
}


int16_t AnalogDigitalConverter::read() {
    SPI.beginTransaction(ADS1120_SPI);
    digitalWrite(channel.isolator_pin, LOW);
    digitalWrite(channel.adc_pin, LOW);

    // Read conversion data
    SPI.transfer(ADC_RDATA);
    uint16_t dmsb = SPI.transfer(0);
    uint16_t dlsb = SPI.transfer(0);

    digitalWrite(channel.adc_pin, HIGH);
    digitalWrite(channel.isolator_pin, HIGH);
    SPI.endTransaction();

    return (int16_t)((dmsb << 8) | dlsb);
}

}
} // namespace eez::psu
