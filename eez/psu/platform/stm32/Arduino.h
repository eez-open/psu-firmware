/*
 * EEZ PSU Firmware
 * Copyright (C) 2018-present, Envox d.o.o.
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

#define F_CPU 80
typedef uint8_t byte;

#define interrupts() (void)0
#define noInterrupts() (void)0

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
    friend class UARTClass;

private:
    union {
        uint8_t bytes[4];  // IPv4 address
        uint32_t dword;
    } _address;

public:
    operator uint32_t() const { return _address.dword; };
};

#define   US_MR_CHRL_8_BIT (0x3u << 6) // 8 bits

#define   US_MR_NBSTOP_1_BIT (0x0u << 12) // 1 stop bit

#define   UART_MR_PAR_EVEN  (0x0u << 9) // Even parity
#define   UART_MR_PAR_ODD   (0x1u << 9) // Odd parity
#define   UART_MR_PAR_SPACE (0x2u << 9) // Space: parity forced to 0
#define   UART_MR_PAR_MARK  (0x3u << 9) // Mark: parity forced to 1
#define   UART_MR_PAR_NO    (0x4u << 9) // No parity

/// Bare minimum implementation of the Arduino Serial object
class UARTClass {
public:
    enum UARTModes {
      Mode_8N1 = US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_NO,
      Mode_8E1 = US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_EVEN,
      Mode_8O1 = US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_ODD,
      Mode_8M1 = US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_MARK,
      Mode_8S1 = US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT | UART_MR_PAR_SPACE
    };

    void begin(unsigned long baud, UARTModes config = Mode_8N1);
    void end();
    int write(const char *buffer, int size);
    int print(const char *data);
    int println(const char *data);
    int print(int value);
    int println(int value);
    int print(float value, int numDigits);
    int println(float value, int numDigits);
    int println(IPAddress ipAddress);
    operator bool();
    int available(void);
    int read(void);
    void flush(void);
};

extern UARTClass Serial;
extern UARTClass SerialUSB;
