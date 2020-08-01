BASEDIR := $(abspath ../../..)
BUILDDIR := $(abspath ./build)

ZOO_TYPE := frontend
ZOO_DEBUG_MENU := 1
ZOO_USE_DRIVER_IO_ROMFS := 1
ZOO_USE_LABEL_CACHE := 1
ZOO_USE_ROM_POINTERS := 1
ZOO_USE_UI := 1
ZOO_USE_UI_SIDEBAR_SLIM := 1

SOURCES := \
	src/main.c \
	src/sound_gba.c \
	src/video_gba.c
OBJECTS := $(BUILDDIR)/4x6_bin.o

OUTPUT := ZooAdvance
OUTEXT := .elf

all: $(OUTPUT).gba

# arch settings
include $(DEVKITARM)/gba_rules

ARCH :=	-mcpu=arm7tdmi -mtune=arm7tdmi -mthumb -mthumb-interwork
ARCH_CFLAGS := -fomit-frame-pointer $(ARCH) -DZOO_PLATFORM_GBA
ARCH_LDFLAGS := -specs=gba.specs $(ARCH)

INCLUDE_DIRS := $(DEVKITPRO)/libtonc/include
LIB_DIRS := $(DEVKITPRO)/libtonc/lib
LIB_FLAGS := -ltonc

include $(abspath ${BASEDIR})/src/Makefile

$(BUILDDIR)/4x6_bin.c $(BUILDDIR)/4x6_bin.h : $(BUILDDIR)/4x6.bin
	@echo $(notdir $@)
	@mkdir -p $(@D)
	@$(PYTHON3) $(TOOLSDIR)/bin2c.py $(BUILDDIR)/4x6_bin.c $(BUILDDIR)/4x6_bin.h $<

$(BUILDDIR)/4x6.bin : $(CURDIR)/tools/pack4x6.py $(BASEDIR)/fonts/4x6.png
	@echo packing $(notdir $@)
	@mkdir -p $(@D)
	@$(PYTHON3) $(CURDIR)/tools/pack4x6.py $(BASEDIR)/fonts/4x6.png $@
