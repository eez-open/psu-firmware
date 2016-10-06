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
#include "main_loop.h"

#undef INPUT
#undef OUTPUT

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace eez::psu;

#define NEW_INPUT_MESSAGE WM_USER

static DWORD main_thread_id;

DWORD WINAPI input_thread_proc(_In_ LPVOID lpParameter) {
    while (1) {
		int ch = getchar();
		if (ch == EOF) break;

		PostThreadMessage(main_thread_id, NEW_INPUT_MESSAGE, ch, 0);
	}

	PostThreadMessage(main_thread_id, WM_QUIT, 0, 0);

	return 0;
}

int main_loop() {
	MSG msg;

	main_thread_id = GetCurrentThreadId();
	CreateThread(0, 0, input_thread_proc, 0, 0, 0);

	while (1) {
		switch (MsgWaitForMultipleObjects(0, 0, FALSE, TICK_TIMEOUT, QS_POSTMESSAGE)) {
		case WAIT_OBJECT_0:
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
                if (msg.message == WM_PAINT) {
                    break;
                }

				switch (msg.message) {
				case NEW_INPUT_MESSAGE:
                    Serial.put(msg.wParam);
					break;
				case WM_QUIT:
					return 0;
				}
			}
			break;

		case WAIT_TIMEOUT:
            simulator::tick();
			break;

		default:
			return -1;
		}
	}
}

void main_loop_exit() {
    GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);
}