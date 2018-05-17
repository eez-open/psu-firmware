CC = gcc
CXX = g++

# Simulator main program

PROGRAM_NAME = eez_psu_sim

CFLAGS = -g -c -Wextra -Wmissing-prototypes -Wimplicit \
    -DUSE_FULL_ERROR_LIST=0 \
	-I../../../../../libraries/scpi-parser/src \

CSOURCES = \
	-c ../../../../../libraries/scpi-parser/src/impl/*.c \
	-c ../../../../platform/simulator/linux/third_party/*.c

CXXFLAGS = -g \
	-Wall -Wno-unused-variable -fpermissive -Wno-reorder -Wno-parentheses \
	-DUSE_FULL_ERROR_LIST=0 \
	-DEEZ_PLATFORM_SIMULATOR \
	-I../../../../.. \
	-I../../../../../libraries/eez_psu_lib/src \
	-I../../../../../libraries/scpi-parser/src \
	-I../../../../platform/simulator/linux/third_party \
	-fPIC `sdl2-config --cflags`

CXXSOURCES = \
	../../../../app/gui/*.cpp \
	../../../../mw/*.cpp \
	../../../../mw/gui/*.cpp \
	../../../../platform/simulator/*.cpp \
	../../../../platform/simulator/imgui/*.cpp \
	../../../../platform/simulator/linux/*.cpp \
	../../../../psu/*.cpp \
	../../../../psu/gui/*.cpp \
	../../../../psu/platform/simulator/*.cpp \
	../../../../psu/platform/simulator/ethernet/*.cpp \
	../../../../psu/platform/simulator/front_panel/*.cpp \
	../../../../psu/scpi/*.cpp

LINKERFLAGS = -ldl -lpthread `sdl2-config --libs` -lSDL2_image -lSDL2_ttf

# rules

all: clean simulator

clean:
	rm -f *.o $(PROGRAM_NAME)

simulator:
	$(CC) $(CFLAGS) $(CSOURCES)
	$(CXX) *.o $(CXXFLAGS) $(CXXSOURCES) $(LINKERFLAGS) -o $(PROGRAM_NAME)

