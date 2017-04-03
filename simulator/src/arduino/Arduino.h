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

#pragma once

#include <stdint.h>
#include <queue>

typedef uint8_t byte;

namespace eez {
namespace psu {
namespace simulator {
/// Arduino API simulation.
namespace arduino {

#define LSBFIRST 0
#define MSBFIRST 1

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define HIGH 0x1
#define LOW  0x0

#define CHANGE 1
#define FALLING 2
#define RISING 3

#define digitalPinToInterrupt(p) (p)

static const uint8_t SS = 53;
static const uint8_t MOSI = 51;
static const uint8_t MISO = 50;
static const uint8_t SCK = 52;

void pinMode(uint8_t pin, uint8_t mode);
int digitalRead(uint8_t pin);
void digitalWrite(uint8_t pin, uint8_t state);
int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int state);

typedef void(*InterruptCallback)(void);

void attachInterrupt(uint8_t, InterruptCallback, int mode);
void detachInterrupt(uint8_t);

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0);

uint32_t millis();
uint32_t micros();
void delay(uint32_t millis);
void delayMicroseconds(uint32_t microseconds);

/// Bare minimum implementation of the Arduino IPAddress class
class IPAddress {
    friend class SimulatorSerial;

private:
    union {
        uint8_t bytes[4];  // IPv4 address
        uint32_t dword;
    } _address;

public:
    operator uint32_t() const { return _address.dword; };
};

/// Bare minimum implementation of the Arduino Serial object
class SimulatorSerial {
public:
    void begin(unsigned long baud);
    int write(const char *buffer, int size);
    int print(const char *data);
    int println(int value);
    int println(float value);
    int println(const char *data);
    int println(IPAddress ipAddress);
    operator bool() { return true; }
    int available(void);
    int read(void);
    void flush(void);

    void put(int ch);

private:
    std::queue<int> input;
};

extern SimulatorSerial Serial;

#define PROGMEM
#define pgm_read_byte_near(address_short) (*(address_short))

}
}
}
} // namespace eez::psu::simulator::arduino;

using namespace eez::psu::simulator::arduino;
