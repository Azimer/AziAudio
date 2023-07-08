## -*- Makefile -*-
## mingw Makefile for AziAudio Project64 Audio Plugin
##
## Build can be configured to a small amount with environment variables.
## Set BUILD_ARCH to x86 for a 32 bit build or x86_64 for a 64 bit build.
## If BUILD_ARCH is not set, we will attempt to detect the architecture.
## Set BUILD_DEBUG to 1 for a debug build. Anything else for a release build.


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
COMMON_FLAGS = -msse2 -DSSE2_SUPPORT -mstackrealign -I"3rd Party/directx/include" -I"3rd Party" -Wall -Wno-attributes -Wno-unknown-pragmas
CFLAGS = $(BASICOPTS) $(COMMON_FLAGS)
CXXFLAGS = $(BASICOPTS) $(COMMON_FLAGS) $(CPPFLAGS)
LDFLAGS = -static-libstdc++ -static-libgcc -static -lole32 -lcomctl32 -lwinmm -ldsound -lksuser

# Define the target directories.
BINDIR=bin
SRCDIR=AziAudio

all: $(BINDIR)/$(PLUGIN_FILE)

COMMON_OBJS =  \
	$(SRCDIR)/WaveOut.o \
	$(SRCDIR)/ABI_Resample.o \
	$(SRCDIR)/ABI_MixerInterleave.o \
	$(SRCDIR)/ABI_Filters.o \
	$(SRCDIR)/ABI_Envmixer.o \
	$(SRCDIR)/ABI_Buffers.o \
	$(SRCDIR)/ABI_Adpcm.o \
	$(SRCDIR)/ABI3mp3.o \
	$(SRCDIR)/ABI3.o \
	$(SRCDIR)/ABI2.o \
	$(SRCDIR)/ABI1.o \
	$(SRCDIR)/Configuration.o \
	$(SRCDIR)/Mupen64plusHLE/musyx.o \
	$(SRCDIR)/Mupen64plusHLE/Mupen64Support.o \
	$(SRCDIR)/Mupen64plusHLE/memory.o \
	$(SRCDIR)/Mupen64plusHLE/audio.o \
	$(SRCDIR)/SoundDriverFactory.o \
	$(SRCDIR)/SoundDriverInterface.o \
	$(SRCDIR)/SoundDriver.o \
	$(SRCDIR)/SoundDriverLegacy.o \
	$(SRCDIR)/WaveOutSoundDriver.o \
	$(SRCDIR)/XAudio2SoundDriver.o \
	$(SRCDIR)/XAudio2SoundDriverLegacy.o \
	$(SRCDIR)/DirectSoundDriver.o \
	$(SRCDIR)/DirectSoundDriverLegacy.o \
	$(SRCDIR)/WASAPISoundDriver.o \
	$(SRCDIR)/NoSoundDriver.o \
	$(SRCDIR)/HLEMain.o \
	$(SRCDIR)/main.o \
	$(SRCDIR)/resource.o

# Link or archive
$(BINDIR)/$(PLUGIN_FILE): $(BINDIR) $(OBJDIR) $(XA_OBJS) $(COMMON_OBJS)
	$(CXX) -shared $(CXXFLAGS) $(CPPFLAGS) -o $@ $(COMMON_OBJS) $(LDFLAGS)

# Compile source files into .o files

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.o: %.rc
	$(WINDRES) $(RESFLAGS) $< $@

$(BINDIR):
	mkdir -p $@

#### Clean target deletes all generated files ####
clean:
	rm -rf $(BINDIR)
	rm -f $(COMMON_OBJS)
