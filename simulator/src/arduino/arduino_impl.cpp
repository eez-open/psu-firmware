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
#include "arduino_internal.h"
#include "chips.h"
#include "temp_sensor.h"
#include "front_panel/control.h"

namespace eez {
namespace psu {
namespace simulator {
namespace arduino {

////////////////////////////////////////////////////////////////////////////////

int pins[NUM_PINS];
InterruptCallback interrupt_callbacks[NUM_INTERRUPTS];

////////////////////////////////////////////////////////////////////////////////

void pinMode(uint8_t pin, uint8_t mode) {
}

int digitalRead(uint8_t pin) {
    return pins[pin];
}

void digitalWrite(uint8_t pin, uint8_t state) {
    pins[pin] = state;
    chips::select(pin, state);
    if (interrupt_callbacks[pin]) {
        interrupt_callbacks[pin]();
    }
}

float temp_sensor_cels_to_volt(float cels, float p1_volt, float p1_cels, float p2_volt, float p2_cels) {
    return util::remap(cels, p1_cels, p1_volt, p2_cels, p2_volt);
}

int analogRead(uint8_t pin) {
    for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
        temp_sensor::TempSensor &tempSensor = temp_sensor::sensors[i];
        if (tempSensor.installed && tempSensor.pin == pin) {
            float cels = simulator::getTemperature(i);
            float volt = util::remap(cels, tempSensor.p1_cels, tempSensor.p1_volt, tempSensor.p2_cels, tempSensor.p2_volt);
            float adc = util::remap(volt, (float)temp_sensor::MIN_U, (float)temp_sensor::MIN_ADC, (float)temp_sensor::MAX_U, (float)temp_sensor::MAX_ADC); \
            return (int)util::clamp(adc, (float)temp_sensor::MIN_ADC, (float)temp_sensor::MAX_ADC); \
        }
    }

    return pins[pin];
}

void analogWrite(uint8_t pin, int state) {
    pins[pin] = state;
}

void attachInterrupt(uint8_t interrupt_no, InterruptCallback interrupt_callback, int mode) {
    interrupt_callbacks[interrupt_no] = interrupt_callback;
}

void detachInterrupt(uint8_t interrupt_no) {
    interrupt_callbacks[interrupt_no] = 0;
}

////////////////////////////////////////////////////////////////////////////////

SimulatorSerial Serial;

void SimulatorSerial::begin(unsigned long baud) {
}

int SimulatorSerial::write(const char *buffer, int size) {
    return fwrite(buffer, 1, size, stdout);
}

int SimulatorSerial::print(const char *data) {
    return write(data, strlen(data));
}

int SimulatorSerial::println(const char *data) {
    return printf("%s\n", data);
}

int SimulatorSerial::print(int value) {
    return printf("%d", value);
}

int SimulatorSerial::println(int value) {
    return printf("%d\n", value);
}

int SimulatorSerial::print(float value, int numDigits) {
    // TODO numDigits
    return printf("%f", value);
}

int SimulatorSerial::println(float value, int numDigits) {
    // TODO numDigits
    return printf("%f\n", value);
}

int SimulatorSerial::println(IPAddress ipAddress) {
    return printf("%d.%d.%d.%d\n",
        ipAddress._address.bytes[0],
        ipAddress._address.bytes[1],
        ipAddress._address.bytes[2],
        ipAddress._address.bytes[3]);
}

int SimulatorSerial::available(void) {
    return input.size();
}

int SimulatorSerial::read(void) {
    int ch = input.front();
    input.pop();
    return ch;
}

void SimulatorSerial::put(int ch) {
    input.push(ch);
}

void SimulatorSerial::flush() {
}

////////////////////////////////////////////////////////////////////////////////

SPISettings::SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
}

////////////////////////////////////////////////////////////////////////////////

SimulatorSPI SPI;

void SimulatorSPI::begin() {
}

void SimulatorSPI::usingInterrupt(uint8_t interruptNumber) {
}

void SimulatorSPI::beginTransaction(SPISettings settings) {
}

uint8_t SimulatorSPI::transfer(uint8_t data) {
    return chips::transfer(data);
}

void SimulatorSPI::endTransaction(void) {
}

void SimulatorSPI::attachInterrupt() {
}

void SimulatorSPI::setBitOrder(int _order) {
}

void SimulatorSPI::setDataMode(uint8_t _mode) {
}

void SimulatorSPI::setClockDivider(uint8_t _div) {
}

////////////////////////////////////////////////////////////////////////////////

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
    front_panel::beep(frequency, duration);
}

////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#undef INPUT
#undef OUTPUT
#include <Windows.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

uint32_t millis() {
#ifdef _WIN32
    return GetTickCount();
#else
    timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t micros = tv.tv_sec*(uint64_t)1000000 + tv.tv_usec;
    return (uint32_t)(micros / 1000);
#endif
}

uint32_t micros() {
#ifdef _WIN32
    static bool firstTime = true;
    static unsigned __int64 frequency;
    static unsigned __int64 startTime;

    if (firstTime) {
        firstTime = false;
        QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
        QueryPerformanceCounter((LARGE_INTEGER *)&startTime);

        return 0;
    } else {
        unsigned __int64 time;
        QueryPerformanceCounter((LARGE_INTEGER *)&time);

        unsigned __int64 diff = (time - startTime) * 1000000L / frequency;

        return (uint32_t)(diff % 4294967296);
    }

#else
    timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t micros = tv.tv_sec*(uint64_t)1000000 + tv.tv_usec;
    return (uint32_t)(micros % 4294967296);
#endif
}

void delay(uint32_t millis) {
    delayMicroseconds(millis * 1000);
} 

void delayMicroseconds(uint32_t microseconds) {
#ifdef _WIN32
    Sleep(microseconds / 1000);
#else
    timespec ts;
    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;
    nanosleep(&ts, 0);
#endif
}

}
}
}
} // namespace eez::psu::simulator::arduino;
