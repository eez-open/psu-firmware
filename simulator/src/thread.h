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

#ifndef EEZ_PSU_THREAD_H
#define EEZ_PSU_THREAD_H

#ifdef _WIN32

#include <windows.h>
typedef HANDLE eez_thread_handle_t;
typedef DWORD(WINAPI *eez_thread_proc_t)(_In_ LPVOID);
#define THREAD_PROC(NAME) DWORD WINAPI NAME(_In_ LPVOID param)

#else

#include <pthread.h>
typedef pthread_t eez_thread_handle_t;
typedef void *(*eez_thread_proc_t)(void *);
#define THREAD_PROC(NAME) void *NAME(void *param)

#endif

eez_thread_handle_t eez_thread_create(eez_thread_proc_t thread_proc, void *param) {
# ifdef _WIN32
	return ::CreateThread(0, 0, thread_proc, param, 0, 0);
# else //_WIN32
	eez_thread_handle_t thread_handle;
	int rc = pthread_create(&thread_handle, NULL, thread_proc, param);
	if (rc) {
		return 0;
	}
	return thread_handle;
# endif //_WIN32
}

void eez_thread_join(eez_thread_handle_t handle) {
# ifdef _WIN32
	::WaitForSingleObject(handle, INFINITE);
	::CloseHandle(handle);
# else //_WIN32
	pthread_join(handle, NULL);
# endif //_WIN32
}

#endif // EEZ_PSU_DLL_H