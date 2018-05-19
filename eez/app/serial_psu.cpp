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
#include "eez/app/serial_psu.h"

#define CONF_CHUNK_SIZE CONF_SERIAL_BUFFER_SIZE

namespace eez {
namespace app {

using namespace scpi;

namespace serial {

TestResult g_testResult = TEST_FAILED;

long g_bauds[] = {4800, 9600, 19200, 38400, 57600, 115200};
size_t g_baudsSize = sizeof(g_bauds) / sizeof(long);

size_t SCPI_Write(scpi_t *context, const char * data, size_t len) {
	size_t written = 0;

	if (serial::g_testResult == TEST_OK) {
        for (size_t i = 0; i < len; i += CONF_SERIAL_BUFFER_SIZE) {
			int size = MIN(CONF_SERIAL_BUFFER_SIZE, len - i);
			SERIAL_PORT.write(data + i, size);
			written += size;
        }
    }

	return written;
}

scpi_result_t SCPI_Flush(scpi_t *context) {
    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t *context, int_fast16_t err) {
    if (err != 0) {
        scpi::printError(err);

		if (err == SCPI_ERROR_INPUT_BUFFER_OVERRUN) {
			scpi::onBufferOverrun(*context);
		}
	}

    return 0;
}

scpi_result_t SCPI_Control(scpi_t *context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    if (serial::g_testResult == TEST_OK) {
        char errorOutputBuffer[256];
        if (SCPI_CTRL_SRQ == ctrl) {
            sprintf(errorOutputBuffer, "**SRQ: 0x%X (%d)\r\n", val, val);
        }
        else {
            sprintf(errorOutputBuffer, "**CTRL %02x: 0x%X (%d)\r\n", ctrl, val, val);
        }
        SERIAL_PORT.println(errorOutputBuffer);
    }

    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t *context) {
    if (serial::g_testResult == TEST_OK) {
        char errorOutputBuffer[256];
        strcpy(errorOutputBuffer, "**Reset\r\n");
        SERIAL_PORT.println(errorOutputBuffer);
    }

    return reset() ? SCPI_RES_OK : SCPI_RES_ERR;
}

////////////////////////////////////////////////////////////////////////////////

static scpi_reg_val_t g_scpiPsuRegs[SCPI_PSU_REG_COUNT];
static scpi_psu_t g_scpiPsuContext = { g_scpiPsuRegs };

static scpi_interface_t g_scpiInterface = {
    SCPI_Error,
    SCPI_Write,
    SCPI_Control,
    SCPI_Flush,
    SCPI_Reset,
};

static char g_scpiInputBuffer[SCPI_PARSER_INPUT_BUFFER_LENGTH];
static int16_t g_errorQueueData[SCPI_PARSER_ERROR_QUEUE_SIZE + 1];

scpi_t g_scpiContext;

static bool g_isConnected;

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

    SERIAL_PORT.begin(persist_conf::getBaudFromIndex(persist_conf::getSerialBaudIndex()), getConfig());

#if CONF_WAIT_SERIAL && !CONF_SERIAL_USE_NATIVE_USB_PORT
    while (!SERIAL_PORT);
#endif

	while (SERIAL_PORT.available()) {
        SERIAL_PORT.read();
    }

#ifdef EEZ_PLATFORM_SIMULATOR
    SERIAL_PORT.print("EEZ PSU software simulator ver. ");
    SERIAL_PORT.println(FIRMWARE);
#else
    SERIAL_PORT.println("EEZ PSU serial com ready");
#endif

    scpi::init(g_scpiContext,
        g_scpiPsuContext,
        &g_scpiInterface,
        g_scpiInputBuffer, SCPI_PARSER_INPUT_BUFFER_LENGTH,
        g_errorQueueData, SCPI_PARSER_ERROR_QUEUE_SIZE + 1);

    g_testResult = TEST_OK;
}

void tick(uint32_t tick_usec) {
    if (g_testResult == TEST_OK) {
		bool isConnected = (bool)SERIAL_PORT;

		if (isConnected != g_isConnected) {
			g_isConnected = isConnected;
			if (g_isConnected) {
				scpi::emptyBuffer(g_scpiContext);
			}
		}

		if (g_isConnected) {
			size_t n = SERIAL_PORT.available();
			if (n > 0) {
				char buffer[CONF_CHUNK_SIZE];
				if (n > CONF_CHUNK_SIZE) {
					n = CONF_CHUNK_SIZE;
				}
				for (size_t i = 0; i < n; ++i) {
					buffer[i] = (char)SERIAL_PORT.read();
				}
				input(g_scpiContext, buffer, n);
			}
		}
    }
}

bool isConnected() {
    return g_testResult == TEST_OK && g_isConnected;
}

void update() {
    init();
}

}
}
} // namespace eez::app::serial
