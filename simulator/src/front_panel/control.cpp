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
#include "front_panel/control.h"
#include "front_panel/render.h"
#include "imgui/window.h"
#include "imgui/beeper.h"
#include "persist_conf.h"

#ifdef _WIN32
#undef INPUT
#undef OUTPUT
#endif

#include "dll.h"

#include <stdio.h>

#ifdef _WIN32
#define LIB_FILE_PATH "eez_imgui.dll"
#else
#define LIB_FILE_PATH "./eez_imgui.so"
#endif

namespace eez {

using namespace imgui;

namespace psu {
namespace simulator {
namespace front_panel {

static bool g_lib_loaded = false;
static eez_dll_lib_t g_lib = 0;
static create_window_ptr_t g_create_window_ptr = 0;
static get_desktop_resolution_ptr_t g_get_desktop_resolution_ptr = 0;
static Window* g_window;
static Data g_data;

static beep_ptr_t g_beep_ptr = 0;

void load_lib() {
    if (!g_lib_loaded) {
        g_lib = eez_dll_load(LIB_FILE_PATH);
        if (g_lib) {
            g_create_window_ptr = (create_window_ptr_t)eez_dll_get_proc_address(g_lib, "eez_imgui_create_window");
            if (!g_create_window_ptr) {
                printf("Incompatible GUI library!\n");
            }
            g_get_desktop_resolution_ptr = (get_desktop_resolution_ptr_t)eez_dll_get_proc_address(g_lib, "eez_imgui_get_desktop_resolution");
            g_beep_ptr = (beep_ptr_t)eez_dll_get_proc_address(g_lib, "eez_imgui_beep");
        }
        else {
            printf("GUI library could not be loaded!\n");
        }
        g_lib_loaded = true;
    }
}

bool isOpened() {
    return g_window ? true : false;
}

bool open() {
    if (g_window) {
        return true;
    }

    load_lib();
    
    if (!g_create_window_ptr) {
        return false;
    }

    int w, h;
    g_get_desktop_resolution_ptr(&w, &h);
    
    g_window = g_create_window_ptr(getWindowDefinition(w, h));

    if (!persist_conf::dev_conf.gui_opened) {
        persist_conf::dev_conf.gui_opened = true;
        persist_conf::saveDevice();
    }

    return g_window != 0;
}

void close() {
    if (g_window) {
        delete g_window;
        g_window = 0;

        if (persist_conf::dev_conf.gui_opened) {
            persist_conf::dev_conf.gui_opened = false;
            persist_conf::saveDevice();
        }
    }
}

void tick() {
    if (g_window) {
        if (g_window->pollEvent()) {
            g_window->beginUpdate();

            fillData(&g_data);

            render(g_window, &g_data);

            processData(&g_data);

            g_window->endUpdate();
        }
        else {
            close();
        }
    }
}

void beep(double freq, int duration) {
    load_lib();
    if (g_beep_ptr) {
        g_beep_ptr(freq, duration);
    }
}


}
}
}
} // namespace eez::psu::simulator::front_panel;