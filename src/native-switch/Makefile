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
APP_VERSION := 1.4.1
TARGET := Bombermaaan
BUILD := build
ROMFS := romfs
NO_ICON := 1

SDL12_COMPAT := $(TOPDIR)/third-party/sdl12-compat-static
SDL12_INCLUDE := $(SDL12_COMPAT)/include
SDL12_SOURCE := $(SDL12_COMPAT)/src/SDL12_compat.c
UPDATER_SOURCE_DIR := $(TOPDIR)/../updater

GAME_CPPFILES := $(filter-out CDirectDraw.cpp CDirectInput.cpp,$(notdir $(wildcard $(TOPDIR)/../Bombermaaan/*.cpp)))
TINYXML_CPPFILES := tinystr.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp
CPPFILES := $(GAME_CPPFILES) $(TINYXML_CPPFILES) updater.cpp
CFILES := ResData.c

CPP_SOURCE_DIRS := $(TOPDIR)/../Bombermaaan $(TOPDIR)/../third-party/tinyxml $(UPDATER_SOURCE_DIR)
C_SOURCE_DIRS := $(TOPDIR)/../RESGEN
vpath %.cpp $(CPP_SOURCE_DIRS)
vpath %.c $(C_SOURCE_DIRS)

# Match the architecture flags used to build the current libnx runtime.  In
# particular, do not build application objects with a weaker AArch64 feature
# set than libnx itself; this matters on Switch releases which enforce the
# current homebrew ABI.
ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -mabi=lp64 -fPIE
COMMON_FLAGS := -g -O2 -ffunction-sections -fdata-sections $(ARCH) -DBOMBERMAAAN_SWITCH -D__SWITCH__
SDL12_FLAGS := -I$(SDL12_INCLUDE) -I$(TOPDIR)/include
PORT_FLAGS := -I$(PORTLIBS)/include -I$(PORTLIBS)/include/SDL2
LIBNX_FLAGS := -I$(LIBNX)/include
PROJECT_FLAGS := -I$(TOPDIR)/../Bombermaaan
UPDATER_FLAGS := -I$(UPDATER_SOURCE_DIR)

CFLAGS := $(COMMON_FLAGS) $(SDL12_FLAGS) $(PORT_FLAGS) $(LIBNX_FLAGS) $(PROJECT_FLAGS) $(UPDATER_FLAGS)
CXXFLAGS := $(COMMON_FLAGS) $(SDL12_FLAGS) $(PORT_FLAGS) $(LIBNX_FLAGS) $(PROJECT_FLAGS) $(UPDATER_FLAGS) -std=gnu++11 -fno-rtti -fno-exceptions
ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,--gc-sections

# The compatibility layer is static and forwards to the SDL2 port library.
LIBS := -lSDL2_mixer -lvorbisidec -logg -lmodplug -lmpg123 -lopusfile -lopus -lSDL2 -lEGL -lglapi -ldrm_nouveau -lnx -lpthread -lm
LIBDIRS += $(PORTLIBS) $(LIBNX)
CPPFLAGS += $(SDL12_FLAGS) $(PORT_FLAGS) $(LIBNX_FLAGS) $(PROJECT_FLAGS) $(UPDATER_FLAGS)

OFILES_SRC := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) sdl12_compat.o
OFILES := $(OFILES_SRC)

export OUTPUT := $(TOPDIR)/$(TARGET)
export TOPDIR := $(TOPDIR)
export DEPSDIR := $(TOPDIR)/$(BUILD)
export OFILES := $(OFILES)
export INCLUDE := $(SDL12_FLAGS) $(PORT_FLAGS) $(LIBNX_FLAGS) $(PROJECT_FLAGS) $(UPDATER_FLAGS) -I$(TOPDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export CFLAGS CXXFLAGS ASFLAGS LDFLAGS LIBS CPPFLAGS

# Embed the NACP as well as the RomFS.  Apart from making the title/version
# visible to hbmenu, this makes it possible to verify on the console that the
# freshly rebuilt NRO was copied instead of an older ABI-incompatible one.
NROFLAGS += --nacp=$(TOPDIR)/$(TARGET).nacp
NROFLAGS += --romfsdir=$(TOPDIR)/$(ROMFS)

ifneq ($(BUILD),$(notdir $(CURDIR)))

.PHONY: all clean $(BUILD)

all: $(BUILD)

$(BUILD):
	@mkdir -p $@
	@printf '%s\n' \
		"TOPDIR=$(TOPDIR)" \
		"DEVKITPRO=$(DEVKITPRO)" \
		"DEVKITA64=$(DEVKITA64)" \
		"PORTLIBS=$(PORTLIBS)" \
		"LIBNX=$(LIBNX)" \
		"CXX=$(CXX)" \
		"ARCH=$(ARCH)" \
		"CXXFLAGS=$(CXXFLAGS)" \
		"LDFLAGS=$(LDFLAGS)" \
		"NROFLAGS=$(NROFLAGS)" > $@/.build-context.new
	@if [ ! -f "$@/.build-context" ] || ! cmp -s "$@/.build-context.new" "$@/.build-context"; then \
		echo "build context changed; invalidating native-switch objects"; \
		find "$@" -mindepth 1 -maxdepth 1 ! -name '.build-context.new' -exec rm -rf {} +; \
	fi
	@mv -f $@/.build-context.new $@/.build-context
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(TOPDIR)/Makefile TOPDIR=$(TOPDIR)

clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).nro $(TARGET).nacp $(TARGET).lst

# The nested libnx make sees the same Makefile and gets its source lists from
# this outer invocation through exported variables.
else

DEPENDS := $(OFILES:.o=.d)

.PHONY: all
all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
	@elf2nro $< $@ $(NROFLAGS)

$(OUTPUT).elf: $(OFILES)
	@echo linking $(notdir $@)
	$(CXX) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

sdl12_compat.o: $(SDL12_SOURCE)
	@echo compiling $<
	$(CC) $(CFLAGS) -D__BUILDING_SDL12_COMPAT__ -MMD -MP -MF $(DEPSDIR)/$*.d -c $< -o $@

%.o: %.cpp
	@echo compiling $<
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(DEPSDIR)/$*.d -c $< -o $@

%.o: %.c
	@echo compiling $<
	$(CC) $(CFLAGS) -MMD -MP -MF $(DEPSDIR)/$*.d -c $< -o $@

-include $(DEPENDS)

endif
