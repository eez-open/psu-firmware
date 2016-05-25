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

#pragma once

#ifdef _WIN32
#include <windows.h>
typedef HANDLE eez_dll_lib_t;
#define EEZ_DLL_EXPORT extern "C" __declspec(dllexport)
#else
#include <dlfcn.h>
typedef void* eez_dll_lib_t;
#define EEZ_DLL_EXPORT extern "C"
#endif

inline eez_dll_lib_t eez_dll_load(const char* lib_file_path) {
# ifdef _WIN32
	return ::LoadLibraryA(lib_file_path);
# else //_WIN32
	return ::dlopen(lib_file_path, RTLD_LAZY);
# endif //_WIN32
}

inline void eez_dll_unload(eez_dll_lib_t lib) {
# ifdef _WIN32
	::FreeLibrary((HMODULE)lib);
# else //_WIN32
	::dlclose(lib);
# endif //_WIN32
}

inline void* eez_dll_get_proc_address(eez_dll_lib_t lib, const char* proc_name) {
# ifdef _WIN32
	return ::GetProcAddress((HMODULE)lib, proc_name);
# else //_WIN32
	return ::dlsym(lib, proc_name);
# endif //_WIN32
}
