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
    return Serial.write(data, len);
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
    char errorOutputBuffer[256];
    if (SCPI_CTRL_SRQ == ctrl) {
        sprintf_P(errorOutputBuffer, PSTR("**SRQ: 0x%X (%d)\r\n"), val, val);
    }
    else {
        sprintf_P(errorOutputBuffer, PSTR("**CTRL %02x: 0x%X (%d)\r\n"), ctrl, val, val);
    }
    Serial.println(errorOutputBuffer);

    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t *context) {
    char errorOutputBuffer[256];
    strcpy_P(errorOutputBuffer, PSTR("**Reset\r\n"));
    Serial.println(errorOutputBuffer);

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

scpi_t scpi_context;

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
        Serial.end();
    }

    if (!persist_conf::isSerialEnabled()) {
        g_testResult = TEST_SKIPPED;
        return;
    }

    Serial.begin(persist_conf::getBaudFromIndex(persist_conf::getSerialBaudIndex()), getConfig());

#ifdef CONF_WAIT_SERIAL
    while (!Serial);
#endif

	while (Serial.available());

#ifdef EEZ_PSU_SIMULATOR
    Serial.print("EEZ PSU software simulator ver. ");
    Serial.println(FIRMWARE);
#else
    Serial.println("EEZ PSU serial com ready");
#endif

    scpi::init(scpi_context,
        scpi_psu_context,
        &scpi_interface,
        scpi_input_buffer, SCPI_PARSER_INPUT_BUFFER_LENGTH,
        error_queue_data, SCPI_PARSER_ERROR_QUEUE_SIZE + 1);

    g_isConnected = false;

    g_testResult = TEST_OK;
}

void tick(uint32_t tick_usec) {
    if (g_testResult == TEST_OK) {
        while (Serial.available()) {
            g_isConnected = true;
            char ch = (char)Serial.read();
            input(scpi_context, ch);
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
