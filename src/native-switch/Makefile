#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

APP_TITLE  := Bombermaaan
APP_AUTHOR := Bombermaaan contributors
TARGET := Bombermaaan
BUILD := build
export OUTPUT := $(TOPDIR)/$(TARGET)
APP_VERSION ?= $(shell sed -n \
	's/^#define BOMBERMAAAN_VERSION_STRING[[:space:]]*"\(.*\)"/\1/p' \
	"$(TOPDIR)/../Bombermaaan/version.hpp")

BUILD_ROOT := $(TOPDIR)/build
GAME_BUILD := $(BUILD_ROOT)/game
LAUNCHER_BUILD := $(BUILD_ROOT)/launcher
ROMFS := $(TOPDIR)/romfs
LEVELS_SOURCE := $(TOPDIR)/../../levels
LEVEL_FILES := $(wildcard $(LEVELS_SOURCE)/*.TXT)
LEVELS_STAMP := $(ROMFS)/.levels.stamp
APP_ICON := $(TOPDIR)/Bombermaaan-icon.jpg
VERSION_HEADER := $(TOPDIR)/../Bombermaaan/version.hpp

SDL12_COMPAT := $(TOPDIR)/third-party/sdl12-compat-static
SDL12_INCLUDE := $(SDL12_COMPAT)/include
SDL12_SOURCE := $(SDL12_COMPAT)/src/SDL12_compat.c
UPDATER_SOURCE_DIR := $(TOPDIR)/../updater

# The launcher gets its identity from compile definitions because launcher.cpp
# is supplied by the updater submodule. Keep the default URL in one place by
# reading it from the game's version header.
BOMBERMAAAN_MANIFEST_URL ?= $(shell sed -n \
	's/.*"\(http[^"]*\)".*/\1/p' "$(VERSION_HEADER)" | head -n 1)
ifeq ($(strip $(BOMBERMAAAN_MANIFEST_URL)),)
BOMBERMAAAN_MANIFEST_URL := http://192.168.22.7/switch/Bombermaaan/Bombermaaan.json
endif

LAUNCHER_DEFINES := \
	'-DSWITCH_UPDATER_APPLICATION_NAME="$(APP_TITLE)"' \
	'-DSWITCH_UPDATER_CURRENT_VERSION="$(APP_VERSION)"' \
	'-DSWITCH_UPDATER_MANIFEST_URL="$(BOMBERMAAAN_MANIFEST_URL)"'

GAME_CPPFILES := $(filter-out CDirectDraw.cpp CDirectInput.cpp,\
	$(notdir $(wildcard $(TOPDIR)/../Bombermaaan/*.cpp)))
GAME_CPPFILES += SDL2Controller.cpp
GAME_CPPFILES += tinystr.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp
GAME_CFILES := ResData.c

LAUNCHER_CPPFILES := launcher.cpp updater.cpp manifest.cpp
LAUNCHER_CFILES := json.c

GAME_SOURCE_DIRS := \
	$(TOPDIR)/../Bombermaaan \
	$(TOPDIR)/../third-party/tinyxml \
	$(TOPDIR)
LAUNCHER_SOURCE_DIRS := $(UPDATER_SOURCE_DIR)
GAME_C_SOURCE_DIRS := $(TOPDIR)/../RESGEN

vpath %.cpp $(GAME_SOURCE_DIRS)
vpath %.cpp $(LAUNCHER_SOURCE_DIRS)
vpath %.c $(GAME_C_SOURCE_DIRS)

ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -mabi=lp64 -fPIE
COMMON_FLAGS := -g -O2 -ffunction-sections -fdata-sections $(ARCH) \
	-DBOMBERMAAAN_SWITCH -D__SWITCH__
SDL12_FLAGS := -I$(SDL12_INCLUDE) -I$(TOPDIR)/include
PORT_FLAGS := -I$(PORTLIBS)/include -I$(PORTLIBS)/include/SDL2
LIBNX_FLAGS := -I$(LIBNX)/include
PROJECT_FLAGS := -I$(TOPDIR)/../Bombermaaan
UPDATER_FLAGS := -I$(UPDATER_SOURCE_DIR)

GAME_CFLAGS := $(COMMON_FLAGS) $(SDL12_FLAGS) $(PORT_FLAGS) $(LIBNX_FLAGS) \
	$(PROJECT_FLAGS)
GAME_CXXFLAGS := $(GAME_CFLAGS) -std=gnu++11 -fno-rtti -fno-exceptions
LAUNCHER_CFLAGS := $(COMMON_FLAGS) $(LIBNX_FLAGS) $(UPDATER_FLAGS)
LAUNCHER_CXXFLAGS := $(LAUNCHER_CFLAGS) -std=gnu++11 -fno-rtti -fno-exceptions \
	$(LAUNCHER_DEFINES)

LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) \
	-Wl,--gc-sections
GAME_LIBS := -lSDL2_mixer -lvorbisidec -logg -lmodplug -lmpg123 \
	-lopusfile -lopus -lSDL2 -lEGL -lglapi -ldrm_nouveau -lnx -lpthread -lm
LAUNCHER_LIBS := -lnx -lpthread -lm
LIBDIRS := $(PORTLIBS) $(LIBNX)
LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

GAME_OBJECTS := $(addprefix $(GAME_BUILD)/,$(GAME_CPPFILES:.cpp=.o))
GAME_OBJECTS += $(addprefix $(GAME_BUILD)/,$(GAME_CFILES:.c=.o))
GAME_OBJECTS += $(GAME_BUILD)/sdl12_compat.o
LAUNCHER_OBJECTS := $(addprefix $(LAUNCHER_BUILD)/,$(LAUNCHER_CPPFILES:.cpp=.o))
LAUNCHER_OBJECTS += $(addprefix $(LAUNCHER_BUILD)/,$(LAUNCHER_CFILES:.c=.o))

GAME_ELF := $(TOPDIR)/game.elf
GAME_NRO := $(TOPDIR)/game.nro
LAUNCHER_ELF := $(TOPDIR)/Bombermaaan.elf
LAUNCHER_NRO := $(TOPDIR)/Bombermaaan.nro
NACP := $(TOPDIR)/Bombermaaan.nacp

.PHONY: all clean

all: $(LAUNCHER_NRO) $(GAME_NRO)

# The launcher compile definitions and the NACP both follow version.hpp.
$(LAUNCHER_OBJECTS) $(NACP): $(VERSION_HEADER)

$(LAUNCHER_NRO): $(LAUNCHER_ELF) $(NACP) $(APP_ICON)
	@echo creating $(notdir $@)
	@elf2nro "$(LAUNCHER_ELF)" "$@" \
		--icon="$(APP_ICON)" \
		--nacp="$(NACP)"

$(GAME_NRO): $(GAME_ELF) $(NACP) $(APP_ICON) $(ROMFS) $(LEVELS_STAMP)
	@echo creating $(notdir $@)
	@elf2nro "$(GAME_ELF)" "$@" \
		--icon="$(APP_ICON)" \
		--nacp="$(NACP)" \
		--romfsdir="$(ROMFS)"

$(LEVELS_STAMP): $(LEVEL_FILES)
	@mkdir -p "$(ROMFS)/Levels"
	@cp $(LEVEL_FILES) "$(ROMFS)/Levels/"
	@touch "$@"

$(LAUNCHER_ELF): $(LAUNCHER_OBJECTS)
	@echo linking $(notdir $@)
	$(CXX) $(LDFLAGS) $^ $(LIBPATHS) $(LAUNCHER_LIBS) -o "$@"

$(GAME_ELF): $(GAME_OBJECTS)
	@echo linking $(notdir $@)
	$(CXX) $(LDFLAGS) $^ $(LIBPATHS) $(GAME_LIBS) -o "$@"

$(GAME_BUILD)/sdl12_compat.o: $(SDL12_SOURCE)
	@mkdir -p "$(@D)"
	@echo compiling $<
	$(CC) $(GAME_CFLAGS) -D__BUILDING_SDL12_COMPAT__ \
		-MMD -MP -MF "$(@:.o=.d)" -c "$<" -o "$@"

$(LAUNCHER_BUILD)/json.o: \
		$(UPDATER_SOURCE_DIR)/third_party/json-parser/json.c
	@mkdir -p "$(@D)"
	@echo compiling $<
	$(CC) $(LAUNCHER_CFLAGS) -MMD -MP -MF "$(@:.o=.d)" -c "$<" -o "$@"

$(GAME_BUILD)/%.o: %.cpp
	@mkdir -p "$(@D)"
	@echo compiling $<
	$(CXX) $(GAME_CXXFLAGS) -MMD -MP -MF "$(@:.o=.d)" -c "$<" -o "$@"

$(GAME_BUILD)/%.o: %.c
	@mkdir -p "$(@D)"
	@echo compiling $<
	$(CC) $(GAME_CFLAGS) -MMD -MP -MF "$(@:.o=.d)" -c "$<" -o "$@"

$(LAUNCHER_BUILD)/%.o: %.cpp
	@mkdir -p "$(@D)"
	@echo compiling $<
	$(CXX) $(LAUNCHER_CXXFLAGS) -MMD -MP -MF "$(@:.o=.d)" -c "$<" -o "$@"

clean:
	@rm -rf "$(BUILD_ROOT)" \
		"$(GAME_ELF)" "$(GAME_NRO)" \
		"$(LAUNCHER_ELF)" "$(LAUNCHER_NRO)" \
		"$(NACP)" "$(LEVELS_STAMP)" "$(ROMFS)/Levels/"*.TXT \
		"$(TOPDIR)/game.lst" "$(TOPDIR)/Bombermaaan.lst"

-include $(GAME_OBJECTS:.o=.d) $(LAUNCHER_OBJECTS:.o=.d)
