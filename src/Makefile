# base defines

INCLUDEDIR := $(BASEDIR)/include
SRCDIR := $(BASEDIR)/src
TOOLSDIR := $(BASEDIR)/tools

SOURCES := $(foreach srcf,$(SOURCES),$(abspath $(srcf)))
SOURCES += \
  $(SRCDIR)/libzoo/zoo_callstack.c \
  $(SRCDIR)/libzoo/zoo_elements.c \
  $(SRCDIR)/libzoo/zoo_game_io.c \
  $(SRCDIR)/libzoo/zoo_game.c \
  $(SRCDIR)/libzoo/zoo_input.c \
  $(SRCDIR)/libzoo/zoo_io.c \
  $(SRCDIR)/libzoo/zoo_oop.c \
  $(SRCDIR)/libzoo/zoo_sound.c \
  $(SRCDIR)/libzoo/zoo_window.c \
  $(SRCDIR)/libzoo/zoo_window_classic.c \
  $(SRCDIR)/libzoo/zoo.c

$(SRCDIR)/libzoo/zoo_oop.c : $(BUILDDIR)/libzoo/zoo_oop_token.c

INCLUDE_DIRS += $(INCLUDEDIR)

# automatically generated headers
INCLUDE_DIRS += $(BUILDDIR)

# shared configuration

ifeq (${ZOO_TYPE},library)
ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS = 1
else ifeq (${ZOO_TYPE},frontend)
ifneq ($(or ${ZOO_USE_EDITOR}),)
ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS = 1
endif
endif

# dependencies

ifneq ($(or ${ZOO_USE_UI}),)
ZOO_USE_DRIVER_IO_PATH = 1
endif

ifneq ($(or ${ZOO_USE_DRIVER_IO_POSIX},${ZOO_USE_DRIVER_IO_ROMFS}),)
ZOO_USE_DRIVER_IO_PATH = 1
endif

ifneq ($(or ${ZOO_USE_ROM_POINTERS}),)
# ROM pointer functionality necessiaties that object data be read-only.
ZOO_USE_LABEL_CACHE = 1
endif

# compiler flags

OPTIMIZE_CFLAGS ?= -O2
OPTIMIZE_LDFLAGS ?= 

OPTIMIZE_CFLAGS += -DZOO_OPTIMIZE

CFLAGS := -g -Wall -Wno-unused $(OPTIMIZE_CFLAGS) $(ARCH_CFLAGS)
LDFLAGS	:= -g $(OPTIMIZE_LDFLAGS) $(ARCH_LDFLAGS)

# auxillary inclusions

ifdef ZOO_USE_DRIVER_IO_PATH
SOURCES += $(SRCDIR)/drivers/zoo_io_path.c
endif

ifdef ZOO_USE_DRIVER_IO_POSIX
SOURCES += $(SRCDIR)/drivers/zoo_io_posix.c
endif

ifdef ZOO_USE_DRIVER_IO_ROMFS
SOURCES += $(SRCDIR)/drivers/zoo_io_romfs.c
endif

ifdef ZOO_USE_DRIVER_SOUND_PCM
SOURCES += $(SRCDIR)/drivers/zoo_sound_pcm.c
endif

ifdef ZOO_USE_UI_SIDEBAR_CLASSIC
SOURCES += $(SRCDIR)/ui/zoo_sidebar_classic.c
endif

ifdef ZOO_USE_UI_SIDEBAR_SLIM
SOURCES += $(SRCDIR)/ui/zoo_sidebar_slim.c
endif

ifdef ZOO_USE_UI_OSK
SOURCES += $(SRCDIR)/ui/zoo_ui_osk.c
CFLAGS += -DZOO_UI_OSK
endif

ifdef ZOO_USE_UI
SOURCES += $(SRCDIR)/ui/zoo_ui.c \
	$(SRCDIR)/ui/zoo_ui_file_select.c \
	$(SRCDIR)/ui/zoo_ui_input.c \
	$(SRCDIR)/ui/zoo_ui_prompt.c \
	$(SRCDIR)/ui/zoo_ui_util.c

ifdef ZOO_DEBUG_MENU
CFLAGS += -DZOO_DEBUG_MENU
SOURCES += $(SRCDIR)/ui/zoo_ui_debug.c
endif
endif # ZOO_USE_UI

ifdef ZOO_USE_LABEL_CACHE
CFLAGS += -DZOO_USE_LABEL_CACHE
SOURCES += $(SRCDIR)/libzoo/zoo_oop_label_cache.c
endif

ifdef ZOO_USE_ROM_POINTERS
CFLAGS += -DZOO_USE_ROM_POINTERS
endif

# tools

LD := $(CC)
PYTHON3 := python3

# add include/library directories

CFLAGS += $(foreach dir,$(INCLUDE_DIRS),-I$(abspath $(dir)))
LDFLAGS_LIBS := $(foreach dir,$(LIB_DIRS),-L$(abspath $(dir))) $(LIB_FLAGS)

# final definitions

CFLAGS_DEPS += -MT $@ -MMD -MP -MF $(BUILDDIR)/$*.d

EARLY_OBJECTS := $(OBJECTS)
OBJECTS += $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))

$(OUTPUT)$(OUTEXT) : $(OBJECTS)
	@echo linking $(notdir $@)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LDFLAGS_LIBS) -o $@

$(BUILDDIR)/%.o : $(SRCDIR)/%.c $(BUILDDIR)/%.d $(EARLY_OBJECTS)
	@echo $(notdir $<)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS_DEPS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o : $(BUILDDIR)/%.c $(BUILDDIR)/%.d
	@echo $(notdir $<)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS_DEPS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.c : $(SRCDIR)/%.tok
	@echo $(notdir $<)
	@mkdir -p $(@D)
	$(PYTHON3) $(TOOLSDIR)/tok2c.py $< $@

.PHONY: clean

clean:
	rm -rf $(BUILDDIR) $(OUTPUT).*

DEPFILES := $(OBJECTS:.o=.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))
