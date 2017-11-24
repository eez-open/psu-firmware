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
#include "serial_psu.h"

namespace eez {
namespace psu {

using namespace scpi;

namespace serial {

psu::TestResult g_testResult = psu::TEST_FAILED;
static bool g_isConnected = false;

long g_bauds[] = {4800, 9600, 19200, 38400, 57600, 115200};
size_t g_baudsSize = sizeof(g_bauds) / sizeof(long);

size_t SCPI_Write(scpi_t *context, const char * data, size_t len) {
    if (serial::g_testResult == TEST_OK) {
        for (size_t i = 0; i < len; i += 64) {
            SERIAL_PORT.write(data + i, MIN(64, len - i));
        }
    }
    return 0;
}

scpi_result_t SCPI_Flush(scpi_t *context) {
    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t *context, int_fast16_t err) {
    if (err != 0) {
        scpi::printError(err);
    }

    return 0;
}

scpi_result_t SCPI_Control(scpi_t *context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    if (serial::g_testResult == TEST_OK) {
        char errorOutputBuffer[256];
        if (SCPI_CTRL_SRQ == ctrl) {
            sprintf_P(errorOutputBuffer, PSTR("**SRQ: 0x%X (%d)\r\n"), val, val);
        }
        else {
            sprintf_P(errorOutputBuffer, PSTR("**CTRL %02x: 0x%X (%d)\r\n"), ctrl, val, val);
        }
        SERIAL_PORT.println(errorOutputBuffer);
    }

    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t *context) {
    if (serial::g_testResult == TEST_OK) {
        char errorOutputBuffer[256];
        strcpy_P(errorOutputBuffer, PSTR("**Reset\r\n"));
        SERIAL_PORT.println(errorOutputBuffer);
    }

    return psu::reset() ? SCPI_RES_OK : SCPI_RES_ERR;
}

////////////////////////////////////////////////////////////////////////////////

scpi_reg_val_t scpi_psu_regs[SCPI_PSU_REG_COUNT];
scpi_psu_t scpi_psu_context = { scpi_psu_regs, 1 };

scpi_interface_t scpi_interface = {
    SCPI_Error,
    SCPI_Write,
    SCPI_Control,
    SCPI_Flush,
    SCPI_Reset,
};

char scpi_input_buffer[SCPI_PARSER_INPUT_BUFFER_LENGTH];
int16_t error_queue_data[SCPI_PARSER_ERROR_QUEUE_SIZE + 1];

scpi_t g_scpiContext;

////////////////////////////////////////////////////////////////////////////////

UARTClass::UARTModes getConfig() {
    switch(persist_conf::getSerialParity()) {
    case PARITY_NONE:  return UARTClass::Mode_8N1;
    case PARITY_EVEN:  return UARTClass::Mode_8E1;
    case PARITY_ODD:   return UARTClass::Mode_8O1;
    case PARITY_MARK:  return UARTClass::Mode_8M1;
    case PARITY_SPACE: return UARTClass::Mode_8S1;
    }

    return UARTClass::Mode_8N1;
}

void init() {
    if (g_testResult == TEST_OK) {
        SERIAL_PORT.end();
    }

    if (!persist_conf::isSerialEnabled()) {
        g_testResult = TEST_SKIPPED;
        return;
    }

    if (scpi::g_busy) {
        return;
    }

    SERIAL_PORT.begin(persist_conf::getBaudFromIndex(persist_conf::getSerialBaudIndex()), getConfig());

#if CONF_WAIT_SERIAL && !CONF_SERIAL_USE_NATIVE_USB_PORT
    while (!SERIAL_PORT);
#endif

	while (SERIAL_PORT.available()) {
        SERIAL_PORT.read();
    }

#ifdef EEZ_PSU_SIMULATOR
    SERIAL_PORT.print("EEZ PSU software simulator ver. ");
    SERIAL_PORT.println(FIRMWARE);
#else
    SERIAL_PORT.println("EEZ PSU serial com ready");
#endif

    scpi::init(g_scpiContext,
        scpi_psu_context,
        &scpi_interface,
        scpi_input_buffer, SCPI_PARSER_INPUT_BUFFER_LENGTH,
        error_queue_data, SCPI_PARSER_ERROR_QUEUE_SIZE + 1);

    g_isConnected = false;

    g_testResult = TEST_OK;
}

void tick(uint32_t tick_usec) {
    if (g_testResult == TEST_OK) {
        for (unsigned i = 0; i < 64 && SERIAL_PORT.available(); ++i) {
            g_isConnected = true;
            char ch = (char)SERIAL_PORT.read();
            input(g_scpiContext, ch);
        }
    }
}

bool isConnected() {
    return g_isConnected;
}

void update() {
    init();
}

}
}
} // namespace eez::psu::serial
