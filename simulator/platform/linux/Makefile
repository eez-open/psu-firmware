CC = gcc
CXX = g++

# Simulator main program

SIM_PROGRAM_NAME = eez_psu_sim

SIM_CFLAGS = -c -Wextra -Wmissing-prototypes -Wimplicit \
    -DUSE_FULL_ERROR_LIST=0 \
	-I../../../libraries/scpi-parser/src \
	
SIM_CSOURCES = \
	-c ../../../libraries/scpi-parser/src/impl/*.c \
	-c third_party/thread_queue.c

SIM_CXXFLAGS = \
	-Wall -Wno-unused-variable -fpermissive \
	-DUSE_FULL_ERROR_LIST=0 \
	-DEEZ_PSU_SIMULATOR \
	-I../../../eez_psu_sketch \
	-I../../src \
	-I../../src/arduino \
	-I../../src/chips \
	-I../../src/ethernet \
	-I../../../libraries/eez_psu_lib/src \
	-I../../../libraries/scpi-parser/src \
	-Ithird_party \
	
SIM_CXXSOURCES = \
	src/*.cpp \
	../../src/*.cpp \
	../../src/arduino/*.cpp \
	../../src/chips/*.cpp \
	../../src/ethernet/*.cpp \
	../../src/front_panel/*.cpp \
	../../../eez_psu_sketch/*.cpp \
	../../../libraries/eez_psu_lib/src/eez_psu.cpp

SIM_LINKERFLAGS = -ldl -lpthread

# GUI dynamic library

GUI_DLIB_NAME = eez_imgui.so

GUI_CXXFLAGS = -Wall -fpermissive -fPIC `sdl2-config --cflags` -I../../src

GUI_SOURCES = ../../src/imgui/*.cpp

GUI_LINKERFLAGS = -shared `sdl2-config --libs` \
	-ldl -lpthread -lSDL2_image -lSDL2_ttf

# rules

all: clean simulator gui

clean:
	rm -f *.o $(SIM_PROGRAM_NAME) $(GUI_DLIB_NAME)

simulator:
	$(CC) $(SIM_CFLAGS) $(SIM_CSOURCES)
	$(CXX) *.o $(SIM_CXXFLAGS) $(SIM_CXXSOURCES) $(SIM_LINKERFLAGS) -o $(SIM_PROGRAM_NAME)

gui:
	$(CXX) $(GUI_CXXFLAGS) $(GUI_SOURCES) $(GUI_LINKERFLAGS) -o $(GUI_DLIB_NAME)

