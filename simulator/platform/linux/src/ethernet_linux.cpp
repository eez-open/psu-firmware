/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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
#include "ethernet_platform.h"

#include <errno.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

namespace eez {
namespace psu {
namespace ethernet_platform {

static int listen_socket = -1;
static int client_socket = -1;

bool enable_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    flags = flags | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        return false;
    }
    return true;
}

bool bind(int port) {
    sockaddr_in serv_addr;
    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0) {
        DebugTraceF("EHTERNET: socket failed with error %d", errno);
        return false;
    }

    if (!enable_non_blocking(listen_socket)) {
        DebugTraceF("EHTERNET: ioctl on listen socket failed with error %d", errno);
        close(listen_socket);
        listen_socket = -1;
        return false;
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (::bind(listen_socket, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        DebugTraceF("EHTERNET: bind failed with error %d", errno);
        close(listen_socket);
        listen_socket = -1;
        return false;
    }

    if (listen(listen_socket, 5) < 0) {
        DebugTraceF("EHTERNET: listen failed with error %d", errno);
        close(listen_socket);
        listen_socket = -1;
        return false;
    }

    return true;
}

bool client_available() {
    if (connected()) return true;

    if (listen_socket == -1) {
        return 0;
    }

    sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    client_socket = accept(listen_socket, (sockaddr *)&cli_addr, &clilen);
    if (client_socket < 0) {
        if (errno == EWOULDBLOCK) {
            return false;
        }

        DebugTraceF("EHTERNET: accept failed with error %d", errno);
        close(listen_socket);
        listen_socket = -1;
        return false;
    }

    if (!enable_non_blocking(client_socket)) {
        DebugTraceF("EHTERNET: ioctl on client socket failed with error %d", errno);
        close(client_socket);
        listen_socket = -1;
        return false;
    }

    return true;
}

bool connected() {
    return client_socket != -1;
}

int available() {
    if (client_socket == -1) return 0;

    char x;
    int iResult = ::recv(client_socket, &x, SCPI_PARSER_INPUT_BUFFER_LENGTH / 2, MSG_PEEK);
    if (iResult > 0) {
        return iResult;
    }

    if (iResult < 0 && errno == EWOULDBLOCK) {
        return 0;
    }

    stop();

    return 0;
}

int read(char *buffer, int buffer_size) {
    int n = ::read(client_socket, buffer, buffer_size);
    if (n > 0) {
        return n;
    }

    if (n < 0 && errno == EWOULDBLOCK) {
        return 0;
    }

    stop();

    return 0;
}

int write(const char *buffer, int buffer_size) {
    if (client_socket != -1) {
        int n = ::write(client_socket, buffer, buffer_size);
        if (n < 0) {
            close(client_socket);
            client_socket = -1;
            return 0;
        }
        return n;
    }

    return 0;
}

void stop() {    
    int result = shutdown(client_socket, SHUT_WR);
    if (result < 0) {
        DebugTraceF("ETHERNET shutdown failed with error %d\n", errno);
    }
    close(client_socket);
    client_socket = -1;
}

}
}
} // namespace eez::psu::ethernet_platform
