'''
EEZ PSU Firmware
Copyright (C) 2015-present Envox d.o.o.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
'''

import os
import platform
import sys
import shutil

def rm_then_cp(src, dest, ignoreRootDirs=None):
    '''
    First remove dest folder (if exists) then copy src to desc.
    '''
    def copytreeIgnore(d, files):
        if ignoreRootDirs is not None:
            if d == src:
                return ignoreRootDirs
        return []

    if os.path.exists(dest):
        shutil.rmtree(dest)

    shutil.copytree(src, dest, ignore=copytreeIgnore)

def copy_lib(src_lib_dir, dst_name):
    #
    # find arduino libraries directory
    #
    ARDUINO_LIB_DIR_CANDIDATES = {
        "Linux": ["Arduino/libraries/", "Documents/Arduino/libraries/"],
        "Darwin": ["Documents/Arduino/libraries/"],
        "Windows": ["Documents\\Arduino\\libraries\\", "My Documents\\Arduino\\libraries\\"]
    }

    home_dir = os.path.expanduser("~")

    arduino_libs_dir = None

    candidates = ARDUINO_LIB_DIR_CANDIDATES.get(platform.system())
    if candidates:
        for candidate_dir in ARDUINO_LIB_DIR_CANDIDATES.get(platform.system()):
            arduino_libs_dir = os.path.join(home_dir, candidate_dir)
            if os.path.exists(arduino_libs_dir):
                break

    if arduino_libs_dir:
        # copy arduino scpi-parser library to the arduino libraries folder
        rm_then_cp(src_lib_dir, os.path.join(arduino_libs_dir, dst_name))
        return True
    else:
        print("Arduino libraries directory not found!")
        return False

SKETCH_SOURCE_DIRS = [
    { "path": "eez/app",                      "prefix": ""              },
    { "path": "eez/app/gui",                  "prefix": "gui_"          },
    { "path": "eez/app/platform/arduino_due", "prefix": "arduino_"      },
    { "path": "eez/app/scpi",                 "prefix": "scpi_"         },
    { "path": "eez/mw",                       "prefix": "mw_"           },
    { "path": "eez/mw/gui",                   "prefix": "mw_gui_"       },
    { "path": "eez/mw/gui/widget",            "prefix": "mw_gui_widget_" },
    { "path": "eez/mw/platform/arduino_due",  "prefix": "mw_arduino_"   }
]

def transform_source_line(line):
    if not line.startswith('#include "'):
        return line

    found_source_dir = None

    for source_dir in SKETCH_SOURCE_DIRS:
        if line.startswith('#include "' + source_dir["path"] + "/"):
            if not found_source_dir or len(source_dir["prefix"]) > len(found_source_dir["prefix"]):
                found_source_dir = source_dir

    if not found_source_dir:
        return line

    return '#include "' + found_source_dir["prefix"] + line[len('#include "' + found_source_dir["path"] + "/") :]


def build_arduino_sketch():
    arduino_sketch_dir = os.path.join(os.path.dirname(__file__), "eez_psu_sketch")

    # remove existing *.cpp and *.h files from arduino sketch dir
    for file in os.listdir(arduino_sketch_dir):
        if file.endswith(".cpp") or file.endswith(".h"):
            file_path = os.path.join(arduino_sketch_dir, file)
            try:
                os.unlink(file_path)
            except:
                print("delete falied", file_path)

    for sketch_source_dir in SKETCH_SOURCE_DIRS:
        sketch_source_dir_path = os.path.join(os.path.dirname(__file__), sketch_source_dir["path"])
        for source_file in os.listdir(sketch_source_dir_path):
            if source_file.endswith(".cpp") or source_file.endswith(".h"):
                sketch_file = sketch_source_dir["prefix"] + source_file

                source_file_path = os.path.join(sketch_source_dir_path, source_file)
                sketch_file_path = os.path.join(arduino_sketch_dir, sketch_file)
                if os.path.exists(sketch_file_path):
                    print("copy failed, sketch file exists", source_file_path, sketch_file_path)
                else:
                    try:
                        with open(source_file_path, "rb") as source_file_object:
                            source_file_content = source_file_object.read()

                            sketch_file_content = []
                            for line in source_file_content.split("\n"):
                                sketch_file_content.append(transform_source_line(line))

                            with open(sketch_file_path, "wb") as sketch_file_object:
                                sketch_file_object.write("\n".join(sketch_file_content))
                    except:
                        print("copy failed", source_file_path, sketch_file_path)

def build_scpi_parser_arduino_lib(scpi_parser_original_lib_dir, scpi_parser_dir):
    '''
    Build scpi-parser as arduino compatible library
    '''

    # copy *.h files
    rm_then_cp(os.path.join(scpi_parser_original_lib_dir, "inc/scpi"), os.path.join(scpi_parser_dir, "src/scpi"))

    # modify config.h
    config_h_file_path = os.path.join(scpi_parser_dir, "src/scpi/config.h")
    config_h_file = open(config_h_file_path)
    tmp_file_path = config_h_file_path + ".tmp"
    tmp_file = open(tmp_file_path, "w")
    for line in config_h_file:
        if line == "#ifdef SCPI_USER_CONFIG\n":
            tmp_file.write("// This is automatically added by the build-arduino-library.py\n")
            tmp_file.write("#define SCPI_USER_CONFIG 1\n")
            tmp_file.write("\n")
        tmp_file.write(line)
    config_h_file.close()
    tmp_file.close()
    os.unlink(config_h_file_path)
    os.rename(tmp_file_path, config_h_file_path)

    # copy scpi_user_config.h
    shutil.copyfile(os.path.join(os.path.dirname(__file__), "eez_psu_sketch/scpi_user_config.h"), os.path.join(scpi_parser_dir, "src/scpi/scpi_user_config.h"))

    # copy *.c files
    rm_then_cp(os.path.join(scpi_parser_original_lib_dir, "src"), os.path.join(scpi_parser_dir, "src/impl"))

def build_scpi_parser():
    scpi_parser_arduino_lib_dir = os.path.join(os.path.dirname(__file__), "libraries/scpi-parser")

    scpi_parser_original_lib_dir = os.path.join(os.path.dirname(__file__), "../libraries/scpi-parser/libscpi")
    if os.path.exists(scpi_parser_original_lib_dir):
        build_scpi_parser_arduino_lib(scpi_parser_original_lib_dir, scpi_parser_arduino_lib_dir)

    copy_lib(scpi_parser_arduino_lib_dir, "scpi-parser")

def build_eez_psu_lib():
    eez_psu_lib_dir = os.path.join(os.path.dirname(__file__), "libraries/eez_psu_lib")
    copy_lib(eez_psu_lib_dir, "eez_psu_lib")

def build_all():
    build_arduino_sketch()
    build_scpi_parser()
    build_eez_psu_lib()

if __name__ == "__main__":
    build_all()
