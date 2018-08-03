/*
 * EEZ Middleware
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

#include "eez/app/psu.h"
#include "eez/mw/platform/simulator/main_loop.h"

#include <emscripten.h>

#include <stdlib.h>
#include <stdio.h>

static int g_initialized = false;

void eez_app_boot() {
	EM_ASM(
		FS.mkdir("/persistent_data"); 
		FS.mount(IDBFS, {}, "/persistent_data");

		//Module.print("start file sync..");

		//flag to check when data are synchronized
		Module.syncdone = 0;

		FS.syncfs(true, function(err) {
			assert(!err);
			//Module.print("end file sync..");
			Module.syncdone = 1;
		});
	);
}

uint32_t g_mainLoopCounter = 0;

void main_loop() {
    ++g_mainLoopCounter;

    if (emscripten_run_script_int("Module.syncdone") == 1) {
        if (!g_initialized) {
            g_initialized = true;
            eez::app::simulator::init();
            eez::app::boot();
        } else {
            eez_app_tick();

            while (1) {
                int ch = getchar();
                if (ch == 0 || ch == EOF) {
                    break;
                }
                eez_app_serial_put(ch);
            }

            EM_ASM(
                if (Module.syncdone) {
                    //Module.print("Start File sync..");
                    Module.syncdone = 0;
                    
                    FS.syncfs(false, function (err) {
                        assert(!err);
                        //Module.print("End File sync..");
                        Module.syncdone = 1;
                    });
                }
            );
        }
    }
}

int eez_app_main_loop() {
    emscripten_set_main_loop(main_loop, 0, 1);
}

void eez_app_main_loop_exit() {
}