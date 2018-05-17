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
#include "eeprom.h"

namespace eez {
namespace psu {
namespace eeprom {

psu::TestResult g_testResult = psu::TEST_FAILED;

////////////////////////////////////////////////////////////////////////////////

static const uint16_t EEPROM_TEST_ADDRESS = 0;
static const uint16_t EEPROM_TEST_BUFFER_SIZE = 64;

////////////////////////////////////////////////////////////////////////////////

void send_address(uint16_t address) {
    SPI.transfer((uint8_t)(address >> 8)); // MSByte
    SPI.transfer((uint8_t)(address));      // LSByte
}

void read_chunk(uint8_t *buffer, uint16_t buffer_size, uint16_t address) {
    SPI_beginTransaction(AT25256B_SPI);

    digitalWrite(EEPROM_SELECT, LOW);  // select chip
    SPI.transfer(READ);                // transmit read opcode

    send_address(address);

    while (buffer_size--) {
        *buffer++ = SPI.transfer(0xFF);    // get data byte
    }

    digitalWrite(EEPROM_SELECT, HIGH); // release chip, signal end transfer
    SPI_endTransaction();
}

void read(uint8_t *buffer, uint16_t buffer_size, uint16_t address) {
    for (uint16_t i = 0; i < buffer_size; i += 64) {
        read_chunk(buffer + i, MIN(buffer_size - i, 64), address + i);
    }

    delay(1);

    uint8_t verifyBuffer[256];
    uint16_t verifySize = MIN(buffer_size, sizeof(verifyBuffer));
    for (uint16_t i = 0; i < verifySize; i += 64) {
        read_chunk(verifyBuffer + i, MIN(buffer_size - i, 64), address + i);
    }

    if (memcmp(buffer, verifyBuffer, verifySize)) {
        DebugTrace("EEPROM read verify error");
    }
}

bool is_write_in_progress() {
    SPI_beginTransaction(AT25256B_SPI);
    digitalWrite(EEPROM_SELECT, LOW);
    SPI.transfer(RDSR); // send RDSR command
    uint8_t data = SPI.transfer(0xFF); // get data byte
    digitalWrite(EEPROM_SELECT, HIGH);
    SPI_endTransaction();
    return (data & (1 << 0));
}

void write_chunk(const uint8_t *buffer, uint16_t buffer_size, uint16_t address) {
    SPI_beginTransaction(AT25256B_SPI);

    // enable writing
    digitalWrite(EEPROM_SELECT, LOW);  // select chip
    SPI.transfer(WREN);                // send write enable command
    digitalWrite(EEPROM_SELECT, HIGH); // deselect chip

    digitalWrite(EEPROM_SELECT, LOW);  // select chip
    SPI.transfer(WRITE);               // send write command

    send_address(address);

    // write buffer
    for (uint16_t i = 0; i < buffer_size; ++i) {
        SPI.transfer(buffer[i]);
    }

    digitalWrite(EEPROM_SELECT, HIGH); // release chip
    SPI_endTransaction();

    uint32_t s = micros();
    while (is_write_in_progress()) {
        uint32_t e = micros();
        if (e - s > 3000) {
            DebugTrace("EEPROM write failure!");
            break;
        }
    }

    // disable writing
    SPI_beginTransaction(AT25256B_SPI);
    digitalWrite(EEPROM_SELECT, LOW);  // select chip
    SPI.transfer(WRDI);                // send write disable command
    digitalWrite(EEPROM_SELECT, HIGH); // deselect chip
    SPI_endTransaction();
}

static uint8_t *buffer_verify;
static uint16_t buffer_verify_size;

bool write(const uint8_t *buffer, uint16_t buffer_size, uint16_t address) {
    for (uint16_t i = 0; i < buffer_size; i += 64) {
        write_chunk(buffer + i, MIN(buffer_size - i, 64), address + i);
    }

    if (!buffer_verify || buffer_size > buffer_verify_size) {
        if (buffer_verify) {
            free(buffer_verify);
        }
        buffer_verify = (uint8_t *)malloc(buffer_size);
        buffer_verify_size = buffer_size;
        //DebugTraceF("buffer_verify_size = %u", buffer_verify_size);
    }

    read(buffer_verify, buffer_size, address);

    if (memcmp(buffer, buffer_verify, buffer_size) == 0) {
        return true;
    }

    DebugTrace("EEPROM write verify failed!");
    return false;
}

void init() {
    if (OPTION_EXT_EEPROM) {
        // write 0 (no protection) to status register
        SPI_beginTransaction(AT25256B_SPI);
        digitalWrite(EEPROM_SELECT, LOW);
        SPI.transfer(WRSR);
        SPI.transfer(0);
        digitalWrite(EEPROM_SELECT, HIGH);
        SPI_endTransaction();
    }
}

bool test() {
    if (OPTION_EXT_EEPROM) {
        // write buffer to eeprom
        uint8_t test_buffer[EEPROM_TEST_BUFFER_SIZE];

        for (uint16_t i = 0; i < EEPROM_TEST_BUFFER_SIZE; ++i) {
            test_buffer[i] = i % 32;
        }

        write(test_buffer, EEPROM_TEST_BUFFER_SIZE, EEPROM_TEST_ADDRESS);

        // read buffer from eeprom
        for (uint16_t i = 0; i < EEPROM_TEST_BUFFER_SIZE; ++i) {
            test_buffer[i] = 0;
        }

        read(test_buffer, EEPROM_TEST_BUFFER_SIZE, EEPROM_TEST_ADDRESS);

        // compare it
        g_testResult = psu::TEST_OK;
        for (uint16_t i = 0; i < EEPROM_TEST_BUFFER_SIZE; ++i) {
            if (test_buffer[i] != i % 32) {
                DebugTraceF("EEPROM test failed at index: %d", i);
                g_testResult = psu::TEST_FAILED;
                break;
            }
        }
    }
    else {
        g_testResult = psu::TEST_SKIPPED;
    }

    if (g_testResult == psu::TEST_FAILED) {
        psu::generateError(SCPI_ERROR_EXT_EEPROM_TEST_FAILED);
    }

    return g_testResult != psu::TEST_FAILED;
}

}
}
} // namespace eez::psu::eeprom