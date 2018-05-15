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

#include "stm32f4xx_hal.h"

#include "usbd_cdc_if.h"

////////////////////////////////////////////////////////////////////////////////

void pinMode(uint8_t pin, uint8_t mode) {
}

int digitalRead(uint8_t pin) {
    return 0;
}

void digitalWrite(uint8_t pin, uint8_t state) {
}

int analogRead(uint8_t pin) {
    return 0;
}

void analogWrite(uint8_t pin, int state) {
}

void attachInterrupt(uint8_t interrupt_no, InterruptCallback interrupt_callback, int mode) {
}

void detachInterrupt(uint8_t interrupt_no) {
}

////////////////////////////////////////////////////////////////////////////////

UARTClass Serial;
UARTClass SerialUSB;

void UARTClass::begin(unsigned long baud, UARTModes config) {
}

void UARTClass::end() {
}

int UARTClass::write(const char *buffer, int size) {
	while (true) {
		uint8_t result = CDC_Transmit_HS((uint8_t *) buffer, (uint16_t)size);
		if (result != USBD_BUSY) {
			return size;
		}
		delay(1);
	}
}

int UARTClass::print(const char *data) {
    return write(data, strlen(data));
}

int UARTClass::println(const char *data) {
	char buffer[1024];
    int size = sprintf(buffer, "%s\n", data);
    return write(buffer, size);
}

int UARTClass::print(int value) {
	char buffer[1024];
	int size = sprintf(buffer, "%d", value);
    return write(buffer, size);
}

int UARTClass::println(int value) {
	char buffer[1024];
	int size = sprintf(buffer, "%d\n", value);
    return write(buffer, size);
}

int UARTClass::print(float value, int numDigits) {
    // TODO numDigits
	char buffer[1024];
	int size = sprintf(buffer, "%f", value);
    return write(buffer, size);
}

int UARTClass::println(float value, int numDigits) {
    // TODO numDigits
	char buffer[1024];
	int size = sprintf(buffer, "%f\n", value);
    return write(buffer, size);
}

int UARTClass::println(IPAddress ipAddress) {
	char buffer[1024];
	int size = sprintf(buffer, "%d.%d.%d.%d\n",
        ipAddress._address.bytes[0],
        ipAddress._address.bytes[1],
        ipAddress._address.bytes[2],
        ipAddress._address.bytes[3]);
    return write(buffer, size);
}

UARTClass::operator bool() {
	return g_serialLineState ? true : false;
}

int UARTClass::available(void) {
	return queue_available(&g_serialQueue);
}

int UARTClass::read(void) {
	uint8_t data;
	if (queue_pop(&g_serialQueue, &data)) {
		return (int)data;
	}
	return -1;
}

void UARTClass::flush() {
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
    return 0;
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
}

////////////////////////////////////////////////////////////////////////////////

uint32_t millis() {
    return HAL_GetTick();
}

uint32_t micros() {
	return HAL_GetTick() * 1000;
}

void delay(uint32_t millis) {
	HAL_Delay(millis);
}

void delayMicroseconds(uint32_t microseconds) {
	HAL_Delay(microseconds);
}
