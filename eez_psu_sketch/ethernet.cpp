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

#if OPTION_ETHERNET

#if defined(EEZ_PSU_SIMULATOR) || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
#include <UIPEthernet.h>
#include <UIPServer.h>
#include <UIPClient.h>
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
#include <Ethernet2.h>
#include <EthernetServer.h>
#include <EthernetClient.h>
#endif

#include "ethernet.h"

namespace eez {
namespace psu {

using namespace scpi;

namespace ethernet {

psu::TestResult test_result = psu::TEST_FAILED;

uint8_t mac[6] = ETHERNET_MAC_ADDRESS;

EthernetServer server(TCP_PORT);

bool firstClientDetected = false;
EthernetClient firstClient;

////////////////////////////////////////////////////////////////////////////////

size_t ethernet_client_write(EthernetClient &client, const char *data, size_t len) {
    SPI.beginTransaction(ENC28J60_SPI);
    size_t size = client.write(data, len);
    SPI.endTransaction();

    return size;
}

size_t ethernet_client_write_str(EthernetClient &client, const char *str) {
    return ethernet_client_write(client, str, strlen(str));
}

////////////////////////////////////////////////////////////////////////////////

size_t SCPI_Write(scpi_t *context, const char * data, size_t len) {
    return ethernet_client_write(firstClient, data, len);
}

scpi_result_t SCPI_Flush(scpi_t * context) {
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

bool init() {
#ifdef EEZ_PSU_ARDUINO
    DebugTrace("Ethernet initialization started...");
#endif

#if defined(EEZ_PSU_SIMULATOR) || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    Enc28J60Network::setControlCS(ETH_SELECT);
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
    Ethernet.init(ETH_SELECT);
    Ethernet.setDhcpTimeout(ETHERNET_DHCP_TIMEOUT * 1000UL);
#endif

    SPI.beginTransaction(ENC28J60_SPI);
    test_result = Ethernet.begin(mac) ? psu::TEST_OK : psu::TEST_FAILED;
    if (!test_result) {
        SPI.endTransaction();
        DebugTrace("Ethernet initialization failed!");
        return false;
    }
    server.begin();
    SPI.endTransaction();

    DebugTraceF("Listening on port %d", (int)TCP_PORT);

#ifdef EEZ_PSU_ARDUINO
#if CONF_DEBUG || CONF_DEBUG_LATEST
    Serial.print("My IP: "); Serial.println(Ethernet.localIP());
    Serial.print("Netmask: "); Serial.println(Ethernet.subnetMask());
    Serial.print("GW IP: "); Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS IP: "); Serial.println(Ethernet.dnsServerIP());
#endif
#endif

    scpi::init(scpi_context,
        scpi_psu_context,
        &scpi_interface,
        scpi_input_buffer, SCPI_PARSER_INPUT_BUFFER_LENGTH,
        error_queue_data, SCPI_PARSER_ERROR_QUEUE_SIZE + 1);

    return test();
}

bool test() {
    if (test_result == psu::TEST_FAILED) {
        psu::generateError(SCPI_ERROR_ETHERNET_TEST_FAILED);
    }

    return test_result != psu::TEST_FAILED;
}

void tick(unsigned long tick_usec) {
    if (test_result != psu::TEST_OK) {
        return;
    }

    SPI.beginTransaction(ENC28J60_SPI);

    if (firstClientDetected) {
        if (!firstClient.connected()) {
            firstClientDetected = false;
            firstClient = EthernetClient();
            DebugTrace("Ethernet client lost!");
        }
    }

    EthernetClient client = server.available();

    if (client) {
        if (!firstClient) {
            firstClient = client;
            firstClientDetected = true;
            DebugTrace("A new ethernet client detected!");
        }

        size_t size;
        while ((size = client.available()) > 0) {
            uint8_t* msg = (uint8_t*)malloc(size);
            size = client.read(msg, size);
            if (client == firstClient) {
                SPI.endTransaction();
                for (size_t i = 0; i < size; ++i) {
                    input(scpi_context, msg[i]);
                }
                SPI.beginTransaction(ENC28J60_SPI);
            }
            else {
                SPI.endTransaction();
                ethernet_client_write_str(client, "Already connected!\r\n");
                SPI.beginTransaction(ENC28J60_SPI);

                client.stop();

                DebugTrace("Another client detected and disconnected!");
            }
            free(msg);
        }
    }

    SPI.endTransaction();
}

}
}
} // namespace eez::psu::ethernet

#endif
