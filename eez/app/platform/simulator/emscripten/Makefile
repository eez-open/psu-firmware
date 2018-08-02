CC = emcc
CXX = em++

# Simulator main program

PROGRAM_NAME = eez_psu_sim.html

CFLAGS = -g -c -Wextra -Wmissing-prototypes -Wimplicit \
    -DUSE_FULL_ERROR_LIST=0 \
	-I../../../../../libraries/scpi-parser/src \

CSOURCES = \
	-c ../../../../../libraries/scpi-parser/src/impl/*.c \
	-c ../../../../mw/platform/simulator/linux/third_party/*.c

CXXFLAGS = -g \
	-Wall -Wno-unused-variable -fpermissive -Wno-reorder -Wno-parentheses \
	-DUSE_FULL_ERROR_LIST=0 \
	-DEEZ_PLATFORM_SIMULATOR \
	-I../../../../.. \
	-I../../../../../libraries/scpi-parser/src \
	-I../../../../mw/platform/simulator/linux/third_party \
	-fPIC \
	-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s SDL2_IMAGE_FORMATS='[png]' \
	-s DEMANGLE_SUPPORT=1

CXXSOURCES = \
	../../../../mw/*.cpp \
	../../../../mw/gui/*.cpp \
	../../../../mw/gui/widget/*.cpp \
	../../../../mw/platform/*.cpp \
	../../../../mw/platform/simulator/*.cpp \
	../../../../mw/platform/simulator/front_panel/*.cpp \
	../../../../mw/platform/simulator/imgui/*.cpp \
	../../../../mw/platform/simulator/emscripten/*.cpp \
	../../../../app/*.cpp \
	../../../../app/gui/*.cpp \
	../../../../app/platform/simulator/*.cpp \
	../../../../app/platform/simulator/ethernet/*.cpp \
	../../../../app/platform/simulator/front_panel/*.cpp \
	../../../../app/scpi/*.cpp

LINKERFLAGS = -lpthread --preload-file ../../simulator/images@images --preload-file ../../simulator/fonts@fonts

# rules

all: clean simulator

clean:
	rm -f *.o $(PROGRAM_NAME)

simulator:
	$(CC) $(CFLAGS) $(CSOURCES)
	$(CXX) *.o $(CXXFLAGS) $(CXXSOURCES) $(LINKERFLAGS) -o $(PROGRAM_NAME)
