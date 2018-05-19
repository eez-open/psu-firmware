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
#include "eez/app/platform/stm32/serial.h"

#include "usbd_cdc_if.h"

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
