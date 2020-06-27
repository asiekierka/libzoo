BASEDIR := $(abspath ../../..)
BUILDDIR := $(abspath ./build)
ZOO_TYPE := frontend
ZOO_USE_DRIVER_IO_POSIX := 1
ZOO_USE_UI := 1
ZOO_USE_UI_SIDEBAR_CLASSIC := 1
SOURCES := \
	src/main.c
OBJECTS := $(BUILDDIR)/6x10_bin.o

OUTPUT := ZooAdvance
OUTEXT := .elf

all: $(OUTPUT).elf

# arch settings
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

ARCH_CFLAGS := $(CFLAGS)
ARCH_LDFLAGS := $(LDFLAGS)
LIB_FLAGS := -lpsputility -lpsppower -lpspaudiolib -lpspaudio -lpspgum -lpspgu -lm $(PSPSDK_LIBS) -lc -lpspuser -lpspkernel

include $(abspath ${BASEDIR})/src/Makefile

$(BUILDDIR)/6x10_bin.c $(BUILDDIR)/6x10_bin.h : $(BUILDDIR)/6x10.bin
	@echo $(notdir $@)
	@mkdir -p $(@D)
	@$(PYTHON3) $(TOOLSDIR)/bin2c.py $(BUILDDIR)/6x10_bin.c $(BUILDDIR)/6x10_bin.h $<

$(BUILDDIR)/6x10.bin : $(CURDIR)/tools/fontpack.py $(BASEDIR)/fonts/6x10.png
	@echo packing $(notdir $@)
	@mkdir -p $(@D)
	@$(PYTHON3) $(CURDIR)/tools/fontpack.py $(BASEDIR)/fonts/6x10.png $@
