BASEDIR := $(abspath ../../..)
BUILDDIR := $(abspath ./build)
CURDIR := $(abspath .)

ZOO_TYPE := frontend
ZOO_USE_DRIVER_IO_POSIX := 1
ZOO_USE_LABEL_CACHE := 1
ZOO_USE_UI := 1
ZOO_USE_UI_SIDEBAR_CLASSIC := 1
ZOO_USE_UI_SIDEBAR_SLIM := 1
SOURCES := \
	src/main.c
OBJECTS := $(BUILDDIR)/4x8_bin.o

OPTIMIZE_CFLAGS := -s -Os
OPTIMIZE_LDFLAGS := -s -Os

OUTPUT := zoo
OUTEXT := .tos

all: $(OUTPUT)$(OUTEXT)

# arch settings - libcmini
#ARCH_CFLAGS := -m68000 -isystem "$(abspath src/)" -DZOO_PLATFORM_ATARI_ST
#ARCH_LDFLAGS := -m68000 -nostdlib
#LIB_FLAGS := -lgem -lcmini -lgcc
# arch settings - mintlib
ARCH_CFLAGS := -m68000 -DZOO_PLATFORM_ATARI_ST
ARCH_LDFLAGS := -m68000
LIB_FLAGS := -lgem

CC := m68k-atari-mint-gcc

include $(abspath ${BASEDIR})/src/Makefile

$(BUILDDIR)/4x8_bin.c $(BUILDDIR)/4x8_bin.h : $(BUILDDIR)/4x8.bin
	@echo $(notdir $@)
	@mkdir -p $(@D)
	@$(PYTHON3) $(TOOLSDIR)/bin2c.py $(BUILDDIR)/4x8_bin.c $(BUILDDIR)/4x8_bin.h $<

$(BUILDDIR)/4x8.bin : $(CURDIR)/tools/pack1bit.py $(BASEDIR)/fonts/4x8.png
	@echo packing $(notdir $@)
	@mkdir -p $(@D)
	@$(PYTHON3) $(CURDIR)/tools/pack1bit.py $(BASEDIR)/fonts/4x8.png $@
