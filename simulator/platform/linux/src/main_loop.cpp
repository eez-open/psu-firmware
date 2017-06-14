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
#include "serial_psu.h"
#include "main_loop.h"

#include <errno.h>
#include "thread_queue.h"

using namespace eez::psu;

#define NEW_INPUT_MESSAGE 1
#define QUIT_MESSAGE      2

threadqueue queue;

void *input_thread(void *) {
    while (1) {
        int ch = getchar();
        if (ch == EOF) break;

        thread_queue_add(&queue, new char(ch), NEW_INPUT_MESSAGE);
    }

    thread_queue_add(&queue, 0, QUIT_MESSAGE);

    return 0;
}


int main_loop() {
    timespec timeout = { 0, TICK_TIMEOUT * 1000 * 1000 };

    if (thread_queue_init(&queue) != 0) return -1;

    pthread_t thread;
    pthread_create(&thread, 0, input_thread, 0);

    while (1) {
        threadmsg msg;
        char *p_ch;
        int ret;
        switch (ret = thread_queue_get(&queue, &timeout, &msg)) {
        case 0:
            switch (msg.msgtype) {
            case NEW_INPUT_MESSAGE:
                p_ch = (char *)msg.data;
                Serial.put(*p_ch);
                delete p_ch;
                break;

            case QUIT_MESSAGE:
                return 0;
            }
            break;

        case ETIMEDOUT:
            simulator::tick();
            break;

        default:
            return ret;
        }
    }
}

void main_loop_exit() {
    ::exit(0);
}