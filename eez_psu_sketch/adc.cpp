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
#include "adc.h"
#include "channel_dispatcher.h"

namespace eez {
namespace psu {

////////////////////////////////////////////////////////////////////////////////

static const uint8_t ADC_REG2_VAL = 0B01100000; // Register 02h: External Vref, 50Hz rejection, PSW off, IDAC off
static const uint8_t ADC_REG3_VAL = 0B00000000; // Register 03h: IDAC1 disabled, IDAC2 disabled, dedicated DRDY

////////////////////////////////////////////////////////////////////////////////

#if ADC_USE_INTERRUPTS
static void adc_interrupt_ch1() {
    Channel::get(0).adc.onInterrupt();
}

static void adc_interrupt_ch2() {
    Channel::get(1).adc.onInterrupt();
}
#endif

////////////////////////////////////////////////////////////////////////////////

AnalogDigitalConverter::AnalogDigitalConverter(Channel &channel_) : channel(channel_) {
    g_testResult = psu::TEST_SKIPPED;

#if ADC_USE_INTERRUPTS
    current_sps = ADC_SPS;
#endif
}

uint8_t AnalogDigitalConverter::getReg1Val() {
#if ADC_USE_INTERRUPTS
    return (current_sps << 5) | 0B00000000;
#else
    return (ADC_SPS << 5) | 0B00000000;
#endif
}

void AnalogDigitalConverter::init() {
#if ADC_USE_INTERRUPTS
    int intNum = digitalPinToInterrupt(channel.convend_pin);
    SPI_usingInterrupt(intNum);
    attachInterrupt(
        intNum,
        channel.index == 1 ? adc_interrupt_ch1 : adc_interrupt_ch2,
        FALLING
        );
#endif

    SPI_beginTransaction(ADS1120_SPI);
    digitalWrite(channel.isolator_pin, ISOLATOR_ENABLE);
    digitalWrite(channel.adc_pin, LOW);

    // Send RESET command
    SPI.transfer(ADC_RESET);
    delayMicroseconds(100); // Guard time

    SPI.transfer(ADC_WR3S1);

    uint8_t reg1_val = getReg1Val();
    SPI.transfer(reg1_val);
    
    SPI.transfer(ADC_REG2_VAL);
    SPI.transfer(ADC_REG3_VAL);

    digitalWrite(channel.adc_pin, HIGH);
    digitalWrite(channel.isolator_pin, ISOLATOR_DISABLE);
    SPI_endTransaction();
}

bool AnalogDigitalConverter::test() {
    SPI_beginTransaction(ADS1120_SPI);
    digitalWrite(channel.isolator_pin, ISOLATOR_ENABLE);
    digitalWrite(channel.adc_pin, LOW);

    SPI.transfer(ADC_RD3S1);
    byte reg1 = SPI.transfer(0);
    byte reg2 = SPI.transfer(0);
    byte reg3 = SPI.transfer(0);

    digitalWrite(channel.adc_pin, HIGH);
    digitalWrite(channel.isolator_pin, ISOLATOR_DISABLE);
    SPI_endTransaction();

    g_testResult = psu::TEST_OK;

    if (reg1 != getReg1Val()) {
        DebugTraceF("Ch%d ADC test failed reg1: expected=%d, got=%d", channel.index, getReg1Val(), reg1);
        g_testResult = psu::TEST_FAILED;
    }

    if (reg2 != ADC_REG2_VAL) {
        DebugTraceF("Ch%d ADC test failed reg2: expected=%d, got=%d", channel.index, ADC_REG2_VAL, reg2);
        g_testResult = psu::TEST_FAILED;
    }

    if (reg3 != ADC_REG3_VAL) {
        DebugTraceF("Ch%d ADC test failed reg3: expected=%d, got=%d", channel.index, ADC_REG3_VAL, reg3);
        g_testResult = psu::TEST_FAILED;
    }

    if (g_testResult == psu::TEST_FAILED) {
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

    return g_testResult != psu::TEST_FAILED;
}

void AnalogDigitalConverter::tick(uint32_t tick_usec) {
#if ADC_USE_INTERRUPTS
    if (channel.isOk()) {
        if (channel.isOutputEnabled()) {
            noInterrupts();
            uint32_t last_adc_start_time = start_time;
            interrupts();
            int32_t diff = tick_usec - last_adc_start_time;
            if (diff > ADC_TIMEOUT_MS * 1000L) {
                if (adc_timeout_recovery_attempts_counter < MAX_ADC_TIMEOUT_RECOVERY_ATTEMPTS) {
                    ++adc_timeout_recovery_attempts_counter;

                    DebugTraceF("ADC timeout (%lu) detected on CH%d, recovery attempt no. %d", diff, channel.index, adc_timeout_recovery_attempts_counter);

                    uint32_t saved_start_time = start_time;

                    channel.init();
                    start(ADC_REG0_READ_U_MON);

                    start_time = saved_start_time;
                }
                else {
                    if (channel.index == 1) {
                        psu::generateError(SCPI_ERROR_CH1_ADC_TIMEOUT_DETECTED);
                    }
                    else if (channel.index == 2) {
                        psu::generateError(SCPI_ERROR_CH2_ADC_TIMEOUT_DETECTED);
                    }
                    else {
                        // TODO
                    }

                    channel_dispatcher::outputEnable(channel, false);
                    channel_dispatcher::remoteSensingEnable(channel, false);
                    if (channel.getFeatures() & CH_FEATURE_RPROG) {
                        channel_dispatcher::remoteProgrammingEnable(channel, false);
                    }
                    if (channel.getFeatures() & CH_FEATURE_LRIPPLE) {
                        channel_dispatcher::lowRippleEnable(channel, false);
                    }
                }
            }
            else {
                adc_timeout_recovery_attempts_counter = 0;
            }
        }
    } else {
        start_time = micros();
    }
#else
    if (start_reg0 && tick_usec - start_time > ADC_READ_TIME_US) {
        int16_t adc_data = read();
        channel.eventAdcData(adc_data);

#if CONF_DEBUG
        debug::g_adcCounter.inc();
#endif
}
#endif
}

void AnalogDigitalConverter::start(uint8_t reg0) {
    start_reg0 = reg0;

    if (start_reg0) {
        SPI_beginTransaction(ADS1120_SPI);
        digitalWrite(channel.isolator_pin, ISOLATOR_ENABLE);
        digitalWrite(channel.adc_pin, LOW);

#if ADC_USE_INTERRUPTS
        uint8_t sps = psu::isTimeCriticalMode() ? ADC_SPS_TIME_CRITICAL : ADC_SPS;
        if (sps != current_sps) {
            current_sps = sps;
            SPI.transfer(ADC_WR4S0);
            SPI.transfer(start_reg0);
            SPI.transfer(getReg1Val());
            SPI.transfer(ADC_REG2_VAL);
            SPI.transfer(ADC_REG3_VAL);
        } else {
            SPI.transfer(ADC_WR1S0);
            SPI.transfer(start_reg0);
        }
#else
        SPI.transfer(ADC_WR1S0);
        SPI.transfer(start_reg0);
#endif

        // Start conversion (single shot)
        SPI.transfer(ADC_START);

        digitalWrite(channel.adc_pin, HIGH);
        digitalWrite(channel.isolator_pin, ISOLATOR_DISABLE);
        SPI_endTransaction();

        start_time = micros();
    }
}

int16_t AnalogDigitalConverter::read() {
    SPI_beginTransaction(ADS1120_SPI);
    digitalWrite(channel.isolator_pin, ISOLATOR_ENABLE);
    digitalWrite(channel.adc_pin, LOW);

    // Read conversion data
    SPI.transfer(ADC_RDATA);
    uint16_t dmsb = SPI.transfer(0);
    uint16_t dlsb = SPI.transfer(0);

    digitalWrite(channel.adc_pin, HIGH);
    digitalWrite(channel.isolator_pin, ISOLATOR_DISABLE);
    SPI_endTransaction();

    return (int16_t)((dmsb << 8) | dlsb);
}

#if ADC_USE_INTERRUPTS
void AnalogDigitalConverter::onInterrupt() {
    g_insideInterruptHandler = true;

    int16_t adc_data = read();
    channel.eventAdcData(adc_data);

#if CONF_DEBUG
    debug::adcReadTick(micros());
#endif

    g_insideInterruptHandler = false;
}
#endif

}
} // namespace eez::psu
