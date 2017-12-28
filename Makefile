## -*- Makefile -*-
## mingw Makefile for AziAudio Project64 Audio Plugin
##
## Build can be configured to a small amount with environment variables.
## Set BUILD_ARCH to x86 for a 32 bit build or x86_64 for a 64 bit build.
## If BUILD_ARCH is not set, we will attempt to detect the architecture.
## Set BUILD_DEBUG to YES for a debug build. Anything else for a release build.


#### Compiler and tool definitions shared by all build targets #####
CC = gcc
CXX = g++
WINDRES = windres
CFLAGS = $(BASICOPTS) -msse2 -DSSE2_SUPPORT -mstackrealign
CXXFLAGS = $(BASICOPTS) -msse2 -DSSE2_SUPPORT -mstackrealign
LDFLAGS = -static-libstdc++ -static-libgcc
XA_FLAGS = -I3rd\ Party/directx/include -I3rd\ Party
DS_FLAGS = -DXAUDIO_LIBRARIES_UNAVAILABLE -I3rd\ Party/directx/include

# Cfg
ifeq ($(BUILD_ARCH),)
GCC_ARCH = $(shell gcc -dumpmachine | sed s/-.*//)
ifeq ($(GCC_ARCH),x86_64)
BUILD_ARCH = x86_64
else
BUILD_ARCH = x86
endif
endif

ifneq ($(BUILD_DEBUG),YES)
BUILD_DEBUG = NO
DS_PLUGIN_FILE = AziAudioDS8_m.dll
XA_PLUGIN_FILE = AziAudioXA2_m.dll
RESFLAGS =
BASICOPTS = -O2
BUILD_TYPE = Release
else
BUILD_DEBUG = YES
DS_PLUGIN_FILE = AziAudioDS8_md.dll
XA_PLUGIN_FILE = AziAudioXA2_md.dll
RESFLAGS = -D_DEBUG
BASICOPTS = -g -D_DEBUG
BUILD_TYPE = Debug
endif

# Define the target directories.
#OBJDIR=obj
#BUILDDIR=build
ifeq ($(BUILD_ARCH),x86_64)
FOLDER = $(BUILD_TYPE)_mingw64
else
FOLDER = $(BUILD_TYPE)_mingw32
endif
OBJDIR = build/$(FOLDER)/AziAudio
BUILDDIR = bin/$(FOLDER)
SRCDIR=AziAudio

all: ds xa

ds: $(BUILDDIR)/$(DS_PLUGIN_FILE)

xa: $(BUILDDIR)/$(XA_PLUGIN_FILE)

directsound: ds

directsound8: ds

ds8: ds

xaudio: xa

xaudio2: xa

xa2: xa

release: all

#debug: debugvar all

#debugvar:
#	$(eval BASICOPTS := -g -D_DEBUG)

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
	$(OBJDIR)/Mupen64plusHLE/musyx.o \
	$(OBJDIR)/Mupen64plusHLE/Mupen64Support.o \
	$(OBJDIR)/Mupen64plusHLE/memory.o \
	$(OBJDIR)/Mupen64plusHLE/audio.o \
	$(OBJDIR)/resource.o

XA_OBJS =	\
	$(OBJDIR)/XA/XAudio2SoundDriver.o \
	$(OBJDIR)/XA/HLEMain.o \
	$(OBJDIR)/XA/SoundDriver.o \
	$(OBJDIR)/XA/main.o

DS_OBJS =	\
	$(OBJDIR)/DS/DirectSoundDriver.o \
	$(OBJDIR)/DS/HLEMain.o \
	$(OBJDIR)/DS/SoundDriver.o \
	$(OBJDIR)/DS/main.o
	

# Link or archive
$(BUILDDIR)/$(XA_PLUGIN_FILE): $(BUILDDIR) $(OBJDIR) $(XA_OBJS) $(COMMON_OBJS)
	$(CXX) -shared $(CXXFLAGS) $(CPPFLAGS) -o $@ $(XA_OBJS) $(COMMON_OBJS) $(LDFLAGS) -lole32

$(BUILDDIR)/$(DS_PLUGIN_FILE): $(BUILDDIR) $(OBJDIR) $(DS_OBJS) $(COMMON_OBJS)
	$(CXX) -shared $(CXXFLAGS) $(CPPFLAGS) -o $@ $(DS_OBJS) $(COMMON_OBJS) $(LDFLAGS) -ldsound


# Compile source files into .o files

$(OBJDIR)/XA/XAudio2SoundDriver.o: $(OBJDIR)/XA $(SRCDIR)/XAudio2SoundDriver.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(XA_FLAGS) -o $@ -c $(SRCDIR)/XAudio2SoundDriver.cpp

$(OBJDIR)/DS/DirectSoundDriver.o: $(OBJDIR)/DS $(SRCDIR)/DirectSoundDriver.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DS_FLAGS) -o $@ -c $(SRCDIR)/DirectSoundDriver.cpp

$(OBJDIR)/XA/SoundDriver.o: $(OBJDIR)/DS $(SRCDIR)/SoundDriver.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DS_FLAGS) -o $@ -c $(SRCDIR)/SoundDriver.cpp

$(OBJDIR)/DS/SoundDriver.o: $(OBJDIR)/DS $(SRCDIR)/SoundDriver.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DS_FLAGS) -o $@ -c $(SRCDIR)/SoundDriver.cpp

$(OBJDIR)/XA/NoSoundDriver.o: $(OBJDIR)/DS $(SRCDIR)/NoSoundDriver.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DS_FLAGS) -o $@ -c $(SRCDIR)/NoSoundDriver.cpp

$(OBJDIR)/DS/NoSoundDriver.o: $(OBJDIR)/DS $(SRCDIR)/NoSoundDriver.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DS_FLAGS) -o $@ -c $(SRCDIR)/NoSoundDriver.cpp

$(OBJDIR)/XA/main.o: $(OBJDIR)/XA $(SRCDIR)/main.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(XA_FLAGS) -o $@ -c $(SRCDIR)/main.cpp

$(OBJDIR)/DS/main.o: $(OBJDIR)/DS $(SRCDIR)/main.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DS_FLAGS) -o $@ -c $(SRCDIR)/main.cpp

$(OBJDIR)/XA/HLEMain.o: $(OBJDIR)/XA $(SRCDIR)/HLEMain.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(XA_FLAGS) -o $@ -c $(SRCDIR)/HLEMain.cpp

$(OBJDIR)/DS/HLEMain.o: $(OBJDIR)/DS $(SRCDIR)/HLEMain.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DS_FLAGS) -o $@ -c $(SRCDIR)/HLEMain.cpp

$(OBJDIR)/WaveOut.o: $(OBJDIR) $(SRCDIR)/WaveOut.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/WaveOut.cpp

$(OBJDIR)/ABI_Resample.o: $(OBJDIR) $(SRCDIR)/ABI_Resample.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/ABI_Resample.cpp

$(OBJDIR)/ABI_MixerInterleave.o: $(OBJDIR) $(SRCDIR)/ABI_MixerInterleave.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/ABI_MixerInterleave.cpp

$(OBJDIR)/ABI_Filters.o: $(OBJDIR) $(SRCDIR)/ABI_Filters.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/ABI_Filters.cpp

$(OBJDIR)/ABI_Envmixer.o: $(OBJDIR) $(SRCDIR)/ABI_Envmixer.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/ABI_Envmixer.cpp

$(OBJDIR)/ABI_Buffers.o: $(OBJDIR) $(SRCDIR)/ABI_Buffers.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/ABI_Buffers.cpp

$(OBJDIR)/ABI_Adpcm.o: $(OBJDIR) $(SRCDIR)/ABI_Adpcm.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/ABI_Adpcm.cpp

$(OBJDIR)/ABI3mp3.o: $(OBJDIR) $(SRCDIR)/ABI3mp3.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/ABI3mp3.cpp

$(OBJDIR)/ABI3.o: $(OBJDIR) $(SRCDIR)/ABI3.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/ABI3.cpp

$(OBJDIR)/ABI2.o: $(OBJDIR) $(SRCDIR)/ABI2.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/ABI2.cpp

$(OBJDIR)/ABI1.o: $(OBJDIR) $(SRCDIR)/ABI1.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/ABI1.cpp

$(OBJDIR)/Mupen64plusHLE/musyx.o: $(OBJDIR)/Mupen64plusHLE $(SRCDIR)/Mupen64plusHLE/musyx.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/Mupen64plusHLE/musyx.c

$(OBJDIR)/Mupen64plusHLE/Mupen64Support.o: $(OBJDIR)/Mupen64plusHLE $(SRCDIR)/Mupen64plusHLE/Mupen64Support.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/Mupen64plusHLE/Mupen64Support.c

$(OBJDIR)/Mupen64plusHLE/memory.o: $(OBJDIR)/Mupen64plusHLE $(SRCDIR)/Mupen64plusHLE/memory.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/Mupen64plusHLE/memory.c

$(OBJDIR)/Mupen64plusHLE/audio.o: $(OBJDIR)/Mupen64plusHLE $(SRCDIR)/Mupen64plusHLE/audio.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $(SRCDIR)/Mupen64plusHLE/audio.c

$(OBJDIR)/resource.o: $(OBJDIR) $(SRCDIR)/resource.rc
	$(WINDRES) $(RESFLAGS) $(SRCDIR)/resource.rc $@


#### Clean target deletes all generated files ####
clean:
	rm -f -r \
		$(BUILDDIR) \
		$(OBJDIR)


# Create the target directory (if needed)
$(BUILDDIR):
	mkdir -p $@

$(OBJDIR):
	mkdir -p $@

$(OBJDIR)/Mupen64plusHLE:
	mkdir -p $@

$(OBJDIR)/XA:
	mkdir -p $@

$(OBJDIR)/DS:
	mkdir -p $@

# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state
