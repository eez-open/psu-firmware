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
#if OPTION_DISPLAY
#include "eez/mw/platform/simulator/front_panel/control.h"
#endif

#include "eez/mw/platform/simulator/main_loop.h"

// for home directory (see getConfFilePath)
#ifdef _WIN32
#undef INPUT
#undef OUTPUT
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable: 4091)
#include <Shlobj.h>
#pragma warning(pop)
#include <direct.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

////////////////////////////////////////////////////////////////////////////////

char *getConfFilePath(const char *file_name) {
	static char file_path[1024];

	*file_path = 0;

#ifdef _WIN32
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, file_path))) {
		strcat(file_path, "\\.eez_psu_sim");
		_mkdir(file_path);
		strcat(file_path, "\\");
	}
#elif defined(__EMSCRIPTEN__)
	strcat(file_path, "/persistent_data/");
#else
	const char *home_dir = 0;
	if ((home_dir = getenv("HOME")) == NULL) {
		home_dir = getpwuid(getuid())->pw_dir;
	}
	if (home_dir) {
		strcat(file_path, home_dir);
		strcat(file_path, "/.eez_psu_sim");
		mkdir(file_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		strcat(file_path, "/");
	}
#endif

	char *q = file_path + strlen(file_path);
	const char *p = file_name;
	while (*p) {
		char ch = *p++;
#ifdef _WIN32
		if (ch == '/') *q++ = '\\';
#else
		if (ch == '\\') *q++ = '/';
#endif
		else *q++ = ch;
	}
	*q = 0;

	return file_path;
}

////////////////////////////////////////////////////////////////////////////////

namespace eez {
namespace app {
namespace simulator {

static float g_temperature[temp_sensor::NUM_TEMP_SENSORS];
static bool g_pwrgood[CH_MAX];
static bool g_rpol[CH_MAX];
static bool g_cv[CH_MAX];
static bool g_cc[CH_MAX];
float g_uSet[CH_MAX];
float g_iSet[CH_MAX];

void init() {
    for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
        g_temperature[i] = 25.0f;
    }

	for (int i = 0; i < CH_MAX; ++i) {
		g_pwrgood[i] = true;
		g_rpol[i] = false;
	}

#if OPTION_DISPLAY
	platform::simulator::front_panel::tick();
#endif
}

void tick() {
    app::tick();
#if OPTION_DISPLAY
	platform::simulator::front_panel::tick();
#endif
}

void setTemperature(int sensor, float value) {
    g_temperature[sensor] = value;
}

float getTemperature(int sensor) {
    return g_temperature[sensor];
}

bool getPwrgood(int pin) {
	return g_pwrgood[pin];
}

void setPwrgood(int pin, bool on) {
	g_pwrgood[pin] = on;
}

bool getRPol(int pin) {
	return g_rpol[pin];
}

void setRPol(int pin, bool on) {
	g_rpol[pin] = on;
}

bool getCV(int pin) {
	return g_cv[pin];
}

void setCV(int pin, bool on) {
	g_cv[pin] = on;
}

bool getCC(int pin) {
	return g_cc[pin];
}

void setCC(int pin, bool on) {
	g_cc[pin] = on;
}

////////////////////////////////////////////////////////////////////////////////

void exit() {
    eez_app_main_loop_exit();
}

} // namespace simulator

}
} // namespace eez::app

#if !defined(__EMSCRIPTEN__)
void eez_app_boot() {
	eez::app::simulator::init();
	eez::app::boot();
}
#endif

void eez_app_tick() {
	eez::app::simulator::tick();
}

void eez_app_serial_put(int ch) {
	SERIAL_PORT.put(ch);
}

void eez_app_exit() {
#if OPTION_DISPLAY
    eez::platform::simulator::front_panel::close();
#endif
}