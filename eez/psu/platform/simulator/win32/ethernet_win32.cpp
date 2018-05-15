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
#include "eez/psu/platform/simulator/ethernet/ethernet_platform.h"

#undef INPUT
#undef OUTPUT

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

namespace eez {
namespace psu {
namespace ethernet_platform {

static SOCKET listen_socket = INVALID_SOCKET;
static SOCKET client_socket = INVALID_SOCKET;

bool bind(int port) {
    WSADATA wsaData;
    int iResult;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        DebugTraceF("EHTERNET: WSAStartup failed with error %d\n", iResult);
        return false;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    char port_str[16];
    _itoa(port, port_str, 10);
    iResult = getaddrinfo(NULL, port_str, &hints, &result);
    if (iResult != 0) {
        DebugTraceF("EHTERNET: getaddrinfo failed with error %d\n", iResult);
        return false;
    }

    // Create a SOCKET for connecting to server
    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        DebugTraceF("EHTERNET: socket failed with error %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        return false;
    }

    u_long iMode = 1;
    iResult = ioctlsocket(listen_socket, FIONBIO, &iMode);
    if (iResult != NO_ERROR) {
        DebugTraceF("EHTERNET: ioctlsocket failed with error %ld\n", iResult);
        freeaddrinfo(result);
        closesocket(listen_socket);
        listen_socket = INVALID_SOCKET;
        return false;
    }

    // Setup the TCP listening socket
    iResult = ::bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        DebugTraceF("EHTERNET: bind failed with error %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listen_socket);
        listen_socket = INVALID_SOCKET;
        return false;
    }

    freeaddrinfo(result);

    iResult = listen(listen_socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        DebugTraceF("EHTERNET listen failed with error %d\n", WSAGetLastError());
        closesocket(listen_socket);
        listen_socket = INVALID_SOCKET;
        return false;
    }

    return true;
}

bool client_available() {
    if (connected()) return true;

    if (listen_socket == INVALID_SOCKET) {
        return false;
    }

    // Accept a client socket
    client_socket = accept(listen_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            return false;
        }

        DebugTraceF("EHTERNET accept failed with error %d\n", WSAGetLastError());
        closesocket(listen_socket);
        listen_socket = INVALID_SOCKET;
        return false;
    }

    return true;
}

bool connected() {
    return client_socket != INVALID_SOCKET;
}

int available() {
    if (client_socket == INVALID_SOCKET) return 0;

    char buffer[SCPI_PARSER_INPUT_BUFFER_LENGTH / 2];
    int iResult = ::recv(client_socket, buffer, SCPI_PARSER_INPUT_BUFFER_LENGTH / 2, MSG_PEEK);
    if (iResult > 0) {
        return iResult;
    }

    if (iResult < 0 && WSAGetLastError() == WSAEWOULDBLOCK) {
        return 0;
    }

    stop();

    return 0;
}

int read(char *buffer, int buffer_size) {
    int iResult = ::recv(client_socket, buffer, buffer_size, 0);
    if (iResult > 0) {
        return iResult;
    }

    if (iResult < 0 && WSAGetLastError() == WSAEWOULDBLOCK) {
        return 0;
    }

    stop();

    return 0;
}

int write(const char *buffer, int buffer_size) {
    int iSendResult;

    if (client_socket != INVALID_SOCKET) {
        // Echo the buffer back to the sender
        iSendResult = ::send(client_socket, buffer, buffer_size, 0);
        if (iSendResult == SOCKET_ERROR) {
            DebugTraceF("send failed with error: %d\n", WSAGetLastError());
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            return 0;
        }
        return iSendResult;
    }

    return 0;
}

void stop() {
    if (client_socket != INVALID_SOCKET) {
        int iResult = shutdown(client_socket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            DebugTraceF("EHTERNET shutdown failed with error %d\n", WSAGetLastError());
        }
        closesocket(client_socket);
        client_socket = INVALID_SOCKET;
    }
}

}
}
} // namespace eez::psu::ethernet_platform
