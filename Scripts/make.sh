mkdir -p gccbuild
mkdir -p gccbuild/Mupen64plusHLE

src="./../AziAudio"
obj="./gccbuild"

FLAGS_x86="\
 -fPIC\
 -DSSE2_SUPPORT\
 -masm=intel\
 -msse2\
 -mstackrealign\
 -ansi\
 -pedantic\
 -Wall\
"
C_FLAGS=$FLAGS_x86

echo Compiling sources...
cc -o $obj/Mupen64plusHLE/audio.asm          $src/Mupen64plusHLE/audio.c          -S $C_FLAGS -O2
cc -o $obj/Mupen64plusHLE/memory.asm         $src/Mupen64plusHLE/memory.c         -S $C_FLAGS -O2
cc -o $obj/Mupen64plusHLE/Mupen64Support.asm $src/Mupen64plusHLE/Mupen64Support.c -S $C_FLAGS -Os
cc -o $obj/Mupen64plusHLE/musyx.asm          $src/Mupen64plusHLE/musyx.c          -S $C_FLAGS -O2
g++ -o $obj/ABI1.asm                  -x c++ $src/ABI1.cpp                  -S $C_FLAGS -O2
g++ -o $obj/ABI2.asm                  -x c++ $src/ABI2.cpp                  -S $C_FLAGS -O2
g++ -o $obj/ABI3.asm                  -x c++ $src/ABI3.cpp                  -S $C_FLAGS -O2
g++ -o $obj/ABI3mp3.asm               -x c++ $src/ABI3mp3.cpp               -S $C_FLAGS -O2
g++ -o $obj/ABI_Adpcm.asm             -x c++ $src/ABI_Adpcm.cpp             -S $C_FLAGS -O2
g++ -o $obj/ABI_Buffers.asm           -x c++ $src/ABI_Buffers.cpp           -S $C_FLAGS -O2
g++ -o $obj/ABI_Envmixer.asm          -x c++ $src/ABI_Envmixer.cpp          -S $C_FLAGS -O2
g++ -o $obj/ABI_Filters.asm           -x c++ $src/ABI_Filters.cpp           -S $C_FLAGS -O2
g++ -o $obj/ABI_MixerInterleave.asm   -x c++ $src/ABI_MixerInterleave.cpp   -S $C_FLAGS -O2
g++ -o $obj/ABI_Resample.asm          -x c++ $src/ABI_Resample.cpp          -S $C_FLAGS -O2
g++ -o $obj/HLEMain.asm               -x c++ $src/HLEMain.cpp               -S $C_FLAGS -Os

g++ -o $obj/main.asm                  -x c++ $src/main.cpp                  -S $C_FLAGS -Os
g++ -o $obj/Configuration.asm         -x c++ $src/Configuration.cpp         -S $C_FLAGS -Os
g++ -o $obj/SoundDriver.asm           -x c++ $src/SoundDriver.cpp           -S $C_FLAGS -Os
g++ -o $obj/SoundDriverFactory.asm    -x c++ $src/SoundDriverFactory.cpp    -S $C_FLAGS -Os
g++ -o $obj/SoundDriverInterface.asm  -x c++ $src/SoundDriverInterface.cpp  -S $C_FLAGS -Os
g++ -o $obj/NoSoundDriver.asm         -x c++ $src/NoSoundDriver.cpp         -S $C_FLAGS -Os
# To do:  We currently don't have any sound-playing drivers for this plugin on Linux.

echo Assembling compiled sources...
as -o $obj/ABI1.o                          $obj/ABI1.asm
as -o $obj/ABI2.o                          $obj/ABI2.asm
as -o $obj/ABI3.o                          $obj/ABI3.asm
as -o $obj/ABI3mp3.o                       $obj/ABI3mp3.asm
as -o $obj/ABI_Adpcm.o                     $obj/ABI_Adpcm.asm
as -o $obj/ABI_Buffers.o                   $obj/ABI_Buffers.asm
as -o $obj/ABI_Envmixer.o                  $obj/ABI_Envmixer.asm
as -o $obj/ABI_Filters.o                   $obj/ABI_Filters.asm
as -o $obj/ABI_MixerInterleave.o           $obj/ABI_MixerInterleave.asm
as -o $obj/ABI_Resample.o                  $obj/ABI_Resample.asm
as -o $obj/HLEMain.o                       $obj/HLEMain.asm
as -o $obj/Configuration.o                 $obj/Configuration.asm
as -o $obj/main.o                          $obj/main.asm

as -o $obj/Mupen64plusHLE/audio.o          $obj/Mupen64plusHLE/audio.asm
as -o $obj/Mupen64plusHLE/memory.o         $obj/Mupen64plusHLE/memory.asm
as -o $obj/Mupen64plusHLE/Mupen64Support.o $obj/Mupen64plusHLE/Mupen64Support.asm
as -o $obj/Mupen64plusHLE/musyx.o          $obj/Mupen64plusHLE/musyx.asm

as -o $obj/SoundDriver.o                   $obj/SoundDriver.asm
as -o $obj/SoundDriverFactory.o            $obj/SoundDriverFactory.asm
as -o $obj/SoundDriverInterface.o          $obj/SoundDriverInterface.asm
as -o $obj/NoSoundDriver.o                 $obj/NoSoundDriver.asm

OBJ_LIST="\
$obj/ABI1.o \
$obj/ABI2.o \
$obj/ABI3.o \
$obj/ABI3mp3.o \
$obj/ABI_Adpcm.o \
$obj/ABI_Buffers.o \
$obj/ABI_Envmixer.o \
$obj/ABI_Filters.o \
$obj/ABI_MixerInterleave.o \
$obj/ABI_Resample.o \
$obj/HLEMain.o \
$obj/Configuration.o \
$obj/main.o \
$obj/Mupen64plusHLE/audio.o \
$obj/Mupen64plusHLE/memory.o \
$obj/Mupen64plusHLE/Mupen64Support.o \
$obj/Mupen64plusHLE/musyx.o \
$obj/NoSoundDriver.o \
$obj/SoundDriver.o \
$obj/SoundDriverFactory.o \
$obj/SoundDriverInterface.o"

echo Linking assembled objects...
g++ -o $obj/AziAudio.so $OBJ_LIST -s -shared
