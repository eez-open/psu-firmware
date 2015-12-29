'''
EEZ PSU Firmware
Copyright (C) 2015 Envox d.o.o.

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

build_arduino_library = __import__("build-arduino-library")

# scpi-parser
libscpi_dir = os.path.join(os.path.dirname(__file__), '../libraries/scpi-parser/libscpi')
scpi_parser_dir = os.path.join(os.path.dirname(__file__), 'libraries/scpi-parser')
build_arduino_library.build_scpi_parser_lib(libscpi_dir, scpi_parser_dir)
build_arduino_library.copy_lib(scpi_parser_dir, 'scpi-parser')

# UIPEthernet
arduino_uip_dir = os.path.join(os.path.dirname(__file__), '../libraries/arduino_uip')
UIPEthernet_dir = os.path.join(os.path.dirname(__file__), 'libraries/UIPEthernet')
build_arduino_library.build_UIPEthernet_lib(arduino_uip_dir, UIPEthernet_dir)
build_arduino_library.copy_lib(UIPEthernet_dir, 'UIPEthernet')

# eez_psu_lib
eez_psu_lib_dir = os.path.join(os.path.dirname(__file__), 'libraries/eez_psu_lib')
build_arduino_library.copy_lib(eez_psu_lib_dir, 'eez_psu_lib')
