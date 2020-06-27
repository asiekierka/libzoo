BASEDIR := $(abspath ../../..)
BUILDDIR := $(abspath ./build)
ZOO_TYPE := frontend
ZOO_USE_DRIVER_IO_POSIX := 1
ZOO_USE_DRIVER_SOUND_PCM := 1
ZOO_USE_UI := 1
ZOO_USE_UI_SIDEBAR_CLASSIC := 1
ZOO_USE_UI_SIDEBAR_SLIM := 1
SOURCES := \
	src/8x14.c \
	src/main.c \
	src/render_software.c

OUTPUT := zoo
OUTEXT := 

all: $(OUTPUT)

# arch settings
ARCH_CFLAGS := $(shell sdl2-config --cflags)
ARCH_LDFLAGS := $(shell sdl2-config --libs)

include $(abspath ${BASEDIR})/src/Makefile