## -*- Makefile -*-
## mingw Makefile for AziAudio Project64 Audio Plugin
##
## Build can be configured to a small amount with environment variables.
## Set BUILD_ARCH to x86 for a 32 bit build or x86_64 for a 64 bit build.
## If BUILD_ARCH is not set, we will attempt to detect the architecture.
## Set BUILD_DEBUG to 1 for a debug build. Anything else for a release build.

#### Debug location ####
PJ64LDIR=$(HOME)/emu/Project64
PJ64LEXE=run.sh
TEST_CASE_EXE=tests.exe

#### Compiler and tool definitions shared by all build targets #####
# Cfg
ifeq ($(BUILD_ARCH),)
GCC_ARCH = $(shell gcc -dumpmachine | sed s/-.*//)
ifeq ($(GCC_ARCH),x86_64)
BUILD_ARCH = x86_64
else
BUILD_ARCH = x86
endif
endif

ifneq ($(BUILD_DEBUG),1)
PLUGIN_FILE = AziAudio_m.dll
RESFLAGS =
BASICOPTS = -O3
BUILD_TYPE = Release
else
PLUGIN_FILE = AziAudio_md.dll
RESFLAGS = -D_DEBUG
BASICOPTS = -g -D_DEBUG
BUILD_TYPE = Debug
endif

BUILD_PREFIX = i686-w64-mingw32-
CC = $(BUILD_PREFIX)gcc
CXX = $(BUILD_PREFIX)g++
WINDRES = $(BUILD_PREFIX)windres
COMMON_FLAGS = -msse2 -DSSE2_SUPPORT -mstackrealign -I"3rd Party/directx/include" -I"3rd Party" -Wall -Wno-attributes -Wno-unknown-pragmas # -D_WIN32_WINNT=0x0501 -DWINVER=0x0501
CFLAGS = $(BASICOPTS) $(COMMON_FLAGS)
CXXFLAGS = $(BASICOPTS) $(COMMON_FLAGS) $(CPPFLAGS)
LDFLAGS = -static-libstdc++ -static-libgcc -static -lole32 -lcomctl32 -lwinmm -ldsound -lksuser

# Define the target directories.
BINDIR=bin
SRCDIR=AziAudio
OBJDIR=$(BINDIR)/$(BUILD_TYPE)_$(BUILD_ARCH)

all: $(BINDIR)/$(PLUGIN_FILE)
	cp "$(BINDIR)/$(PLUGIN_FILE)" "$(PJ64LDIR)/Plugin/$(PLUGIN_FILE)"

test: $(BINDIR)/$(TEST_CASE_EXE)
	cd $(BINDIR) && wine $(TEST_CASE_EXE)

run: all
	cd $(PJ64LDIR) && $(PJ64LDIR)/$(PJ64LEXE)

TEST_OBJS = \
	$(OBJDIR)/tests/testmain.o \
	$(OBJDIR)/tests/configtests.o


COMMON_OBJS =  \
	$(OBJDIR)/WaveOut.o \
	$(OBJDIR)/ABI_Resample.o \
	$(OBJDIR)/ABI_MixerInterleave.o \
	$(OBJDIR)/ABI_Filters.o \
	$(OBJDIR)/ABI_Envmixer.o \
	$(OBJDIR)/ABI_Buffers.o \
	$(OBJDIR)/ABI_Adpcm.o \
	$(OBJDIR)/ABI3mp3.o \
	$(OBJDIR)/ABI3.o \
	$(OBJDIR)/ABI2.o \
	$(OBJDIR)/ABI1.o \
	$(OBJDIR)/Configuration.o \
	$(OBJDIR)/Mupen64plusHLE/musyx.o \
	$(OBJDIR)/Mupen64plusHLE/Mupen64Support.o \
	$(OBJDIR)/Mupen64plusHLE/memory.o \
	$(OBJDIR)/Mupen64plusHLE/audio.o \
	$(OBJDIR)/SoundDriverFactory.o \
	$(OBJDIR)/SoundDriverInterface.o \
	$(OBJDIR)/SoundDriver.o \
	$(OBJDIR)/SoundDriverLegacy.o \
	$(OBJDIR)/WaveOutSoundDriver.o \
	$(OBJDIR)/XAudio2SoundDriver.o \
	$(OBJDIR)/XAudio2SoundDriverLegacy.o \
	$(OBJDIR)/DirectSoundDriver.o \
	$(OBJDIR)/DirectSoundDriverLegacy.o \
	$(OBJDIR)/WASAPISoundDriver.o \
	$(OBJDIR)/NoSoundDriver.o \
	$(OBJDIR)/HLEMain.o \
	$(OBJDIR)/main.o \
	$(OBJDIR)/resource.o

# Link or archive
$(BINDIR)/$(TEST_CASE_EXE): ALL_DIRS $(XA_OBJS) $(COMMON_OBJS) $(TEST_OBJS)
	$(CXX) -mconsole $(CXXFLAGS) $(CPPFLAGS) -o $@ $(COMMON_OBJS) $(TEST_OBJS) $(LDFLAGS)

# Link or archive
$(BINDIR)/$(PLUGIN_FILE): ALL_DIRS $(XA_OBJS) $(COMMON_OBJS)
	$(CXX) -shared $(CXXFLAGS) $(CPPFLAGS) -o $@ $(COMMON_OBJS) $(LDFLAGS)


# Compile source files into .o files

$(OBJDIR)/Mupen64plusHLE/%.o: AziAudio/Mupen64plusHLE/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: AziAudio/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: AziAudio/%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o: AziAudio/%.rc
	$(WINDRES) $(RESFLAGS) $< $@

.PHONY: ALL_DIRS

ALL_DIRS: $(BINDIR) $(OBJDIR) $(OBJDIR)/Mupen64plusHLE $(OBJDIR)/tests

$(OBJDIR)/tests:
	mkdir -p $@

$(OBJDIR)/Mupen64plusHLE:
	mkdir -p $@

$(OBJDIR):
	mkdir -p $@

$(BINDIR):
	mkdir -p $@

#### Clean target deletes all generated files ####
clean:
	rm -rf $(BINDIR)
	rm -f $(COMMON_OBJS)
	rm -f $(TEST_OBJS)
