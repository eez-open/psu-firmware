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
}

int analogRead(uint8_t pin) {

#define TEMP_SENSOR(NAME, PIN, CAL_POINTS, CH_NUM, QUES_REG_BIT) \
    if (pin == PIN) { \
        float cels = simulator::getTemperature(temp_sensor::NAME); \
        float volt = util::remap(cels, CAL_POINTS); \
        float adc = util::remap(volt, (float)temp_sensor::MIN_U, (float)temp_sensor::MIN_ADC, (float)temp_sensor::MAX_U, (float)temp_sensor::MAX_ADC); \
        return (int)util::clamp(adc, (float)temp_sensor::MIN_ADC, (float)temp_sensor::MAX_ADC); \
    }

	TEMP_SENSORS

#undef TEMP_SENSOR

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

int SimulatorSerial::println(int value) {
    return printf("%d\n", value);
}

int SimulatorSerial::println(const char *data) {
    return printf("%s\n", data);
}

int SimulatorSerial::println(IPAddress ipAddress) {
    return printf("%d.%d.%d.%d\n", ipAddress.bytes[0], ipAddress.bytes[1], ipAddress.bytes[2], ipAddress.bytes[3]);
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
    return GetTickCount() * 1000;
#else
    timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t micros = tv.tv_sec*(uint64_t)1000000 + tv.tv_usec;
    return (uint32_t)(micros);
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
    ts.tv_nsec = (microseconds % 1000000) * 1000000;
    nanosleep(&ts, 0);
#endif
}

}
}
}
} // namespace eez::psu::simulator::arduino;
