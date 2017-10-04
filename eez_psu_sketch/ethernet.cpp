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

#include "persist_conf.h"
#include "serial_psu.h"
#include "event_queue.h"
#include "watchdog.h"

#if OPTION_ETHERNET

#if defined(EEZ_PSU_SIMULATOR) || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
#include <UIPEthernet.h>
#include <UIPServer.h>
#include <UIPClient.h>
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
#include <Ethernet2.h>
#include <EthernetServer.h>
#include <EthernetClient.h>
#endif

#include "ethernet.h"

#define CONF_CHECK_DHCP_LEASE_SEC 60

namespace eez {
namespace psu {

using namespace scpi;

namespace ethernet {

psu::TestResult g_testResult = psu::TEST_FAILED;

static EthernetServer *server;

static bool g_isConnected = false;
static EthernetClient g_activeClient;
//static uint32_t g_lastCheckDhcpLeaseTime;

////////////////////////////////////////////////////////////////////////////////

size_t ethernet_client_write(EthernetClient &client, const char *data, size_t len) {
    SPI_beginTransaction(ETHERNET_SPI);
    size_t size = client.write(data, len);
    SPI_endTransaction();

    return size;
}

size_t ethernet_client_write_str(EthernetClient &client, const char *str) {
    return ethernet_client_write(client, str, strlen(str));
}

////////////////////////////////////////////////////////////////////////////////

size_t SCPI_Write(scpi_t *context, const char * data, size_t len) {
    return ethernet_client_write(g_activeClient, data, len);
}

scpi_result_t SCPI_Flush(scpi_t * context) {
    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t *context, int_fast16_t err) {
    if (err != 0) {
        char errorOutputBuffer[256];
        sprintf_P(errorOutputBuffer, PSTR("**ERROR: %d,\"%s\"\r\n"), (int16_t)err, SCPI_ErrorTranslate(err));
        ethernet_client_write(g_activeClient, errorOutputBuffer, strlen(errorOutputBuffer));
    }

    return 0;
}

scpi_result_t SCPI_Control(scpi_t *context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    char outputBuffer[256];
    if (SCPI_CTRL_SRQ == ctrl) {
        sprintf_P(outputBuffer, PSTR("**SRQ: 0x%X (%d)\r\n"), val, val);
    } else {
        sprintf_P(outputBuffer, PSTR("**CTRL %02x: 0x%X (%d)\r\n"), ctrl, val, val);
    }

    ethernet_client_write(g_activeClient, outputBuffer, strlen(outputBuffer));

    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t *context) {
    char errorOutputBuffer[256];
    strcpy_P(errorOutputBuffer, PSTR("**Reset\r\n"));
    ethernet_client_write(g_activeClient, errorOutputBuffer, strlen(errorOutputBuffer));

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

void init() {
    if (!persist_conf::isEthernetEnabled()) {
        g_testResult = psu::TEST_SKIPPED;
        return;
    }

#ifdef EEZ_PSU_ARDUINO
    DebugTrace("Ethernet initialization started...");
#endif

#if defined(EEZ_PSU_SIMULATOR) || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    Enc28J60Network::setControlCS(ETH_SELECT);
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    Ethernet.init(ETH_SELECT);
    Ethernet.setDhcpTimeout(ETHERNET_DHCP_TIMEOUT * 1000UL);
#endif

    SPI.beginTransaction(ETHERNET_SPI);

    bool result;
    if (persist_conf::isEthernetDhcpEnabled()) {
        result = Ethernet.begin(persist_conf::devConf2.ethernetMacAddress);
    } else {
        uint8_t ipAddress[4];
        util::ipAddressToArray(persist_conf::devConf2.ethernetIpAddress, ipAddress);

        uint8_t dns[4];
        util::ipAddressToArray(persist_conf::devConf2.ethernetIpAddress, ipAddress);

        uint8_t gateway[4];
        util::ipAddressToArray(persist_conf::devConf2.ethernetIpAddress, ipAddress);

        uint8_t subnetMask[4];
        util::ipAddressToArray(persist_conf::devConf2.ethernetIpAddress, ipAddress);

        Ethernet.begin(persist_conf::devConf2.ethernetMacAddress, ipAddress, dns, gateway, subnetMask);

        result = 1;
    }

    if (!result) {
        SPI.endTransaction();

        g_testResult = psu::TEST_WARNING;
        DebugTrace("Ethernet not connected!");
        event_queue::pushEvent(event_queue::EVENT_WARNING_ETHERNET_NOT_CONNECTED);

        return;
    }

    server = new EthernetServer(persist_conf::devConf2.ethernetScpiPort);

    server->begin();

    SPI.endTransaction();

    g_testResult = psu::TEST_OK;

    DebugTraceF("Listening on port %d", (int)persist_conf::devConf2.ethernetScpiPort);

#ifdef EEZ_PSU_ARDUINO
#if CONF_DEBUG || CONF_DEBUG_LATEST
    if (persist_conf::isEthernetDhcpEnabled() && serial::g_testResult == TEST_OK) {
        SERIAL_PORT.print("My IP: "); SERIAL_PORT.println(Ethernet.localIP());
        SERIAL_PORT.print("Netmask: "); SERIAL_PORT.println(Ethernet.subnetMask());
        SERIAL_PORT.print("GW IP: "); SERIAL_PORT.println(Ethernet.gatewayIP());
        SERIAL_PORT.print("DNS IP: "); SERIAL_PORT.println(Ethernet.dnsServerIP());
    }
#endif
#endif

    scpi::init(g_scpiContext,
        scpi_psu_context,
        &scpi_interface,
        scpi_input_buffer, SCPI_PARSER_INPUT_BUFFER_LENGTH,
        error_queue_data, SCPI_PARSER_ERROR_QUEUE_SIZE + 1);

    //g_lastCheckDhcpLeaseTime = micros();
}

bool test() {
    if (g_testResult == psu::TEST_FAILED) {
        psu::generateError(SCPI_ERROR_ETHERNET_TEST_FAILED);
    }

    return g_testResult != psu::TEST_FAILED;
}

void tick(uint32_t tick_usec) {
    if (g_testResult != psu::TEST_OK) {
        return;
    }

    // This code is commented because DHCP lease renewal blocks main thread for 5 or more seconds.
    //if (persist_conf::devConf2.flags.ethernetDhcpEnabled) {
    //    int32_t diff = tick_usec - g_lastCheckDhcpLeaseTime;
    //    if (diff > CONF_CHECK_DHCP_LEASE_SEC * 1000000L) {
    //        // DebugTrace("DHCP lease check");
    //        g_lastCheckDhcpLeaseTime = tick_usec;
    //        watchdog::disable();
    //        Ethernet.maintain();
    //        watchdog::enable();
    //    }
    //}

    SPI_beginTransaction(ETHERNET_SPI);

    if (g_isConnected) {
        if (!g_activeClient.connected()) {
            g_isConnected = false;
            g_activeClient = EthernetClient();
            DebugTrace("Ethernet client lost!");
        }
    }

    EthernetClient client = server->available();

    if (client) {
        if (!g_isConnected) {
            client.flush();
            g_activeClient = client;
            g_isConnected = true;
            DebugTrace("A new ethernet client detected!");
        }

        size_t size;
        while ((size = client.available()) > 0) {
            uint8_t* msg = (uint8_t*)malloc(size);
            size = client.read(msg, size);
            if (client == g_activeClient) {
                SPI_endTransaction();
                input(g_scpiContext, (const char *)msg, size);
                SPI_beginTransaction(ETHERNET_SPI);
            }
            else {
                SPI_endTransaction();
                ethernet_client_write_str(client, "**ERROR: another client already connected\r\n");
                SPI_beginTransaction(ETHERNET_SPI);
                DebugTrace("Another client detected and ignored!");
            }
            free(msg);
        }
    }

    SPI_endTransaction();
}

uint32_t getIpAddress() {
    return Ethernet.localIP();
}

bool isConnected() {
    return g_isConnected;
}

void update() {
    if (g_isConnected) {
        if (g_activeClient.connected()) {
            g_activeClient.stop();
            g_activeClient = EthernetClient();
        }
        g_isConnected = false;
    }

    g_testResult = psu::TEST_WARNING;
}

}
}
} // namespace eez::psu::ethernet

#endif
