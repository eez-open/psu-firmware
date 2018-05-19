/*
 * EEZ Middleware
 * Copyright (C) 2018-present, Envox d.o.o.
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

#include "eez/mw/mw.h"

#ifdef _WIN32
#undef INPUT
#undef OUTPUT
#include <Windows.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

uint32_t millis() {
#ifdef _WIN32
	return GetTickCount();
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t micros = tv.tv_sec*(uint64_t)1000000 + tv.tv_usec;
	return (uint32_t)(micros / 1000);
#endif
}

uint32_t micros() {
#ifdef _WIN32
	static bool firstTime = true;
	static unsigned __int64 frequency;
	static unsigned __int64 startTime;

	if (firstTime) {
		firstTime = false;
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
		QueryPerformanceCounter((LARGE_INTEGER *)&startTime);

		return 0;
	} else {
		unsigned __int64 time;
		QueryPerformanceCounter((LARGE_INTEGER *)&time);

		unsigned __int64 diff = (time - startTime) * 1000000L / frequency;

		return (uint32_t)(diff % 4294967296);
	}

#else
	timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t micros = tv.tv_sec*(uint64_t)1000000 + tv.tv_usec;
	return (uint32_t)(micros % 4294967296);
#endif
}

void delay(uint32_t millis) {
	delayMicroseconds(millis * 1000);
}

void delayMicroseconds(uint32_t microseconds) {
#ifdef _WIN32
	Sleep(microseconds / 1000);
#else
	timespec ts;
	ts.tv_sec = microseconds / 1000000;
	ts.tv_nsec = (microseconds % 1000000) * 1000;
	nanosleep(&ts, 0);
#endif
}
