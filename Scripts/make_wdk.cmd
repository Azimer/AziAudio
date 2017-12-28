@ECHO OFF
TITLE Windows Driver Kit 7.1.0

set src=%CD%\..\AziAudio
set obj=%CD%

REM set target=i386
set target=amd64

set DDK=C:\WinDDK\7600.16385.1
set MSVC=%DDK%\bin\x86\%target%
set incl=/I"%DDK%\inc\crt" /I"%DDK%\inc\api" /I"%DDK%\inc\api\crt\stl60" /I"%src%\..\3rd Party\directx\include"
set libs=/LIBPATH:"%DDK%\lib\crt\%target%" /LIBPATH:"%DDK%\lib\wnet\%target%"

set C_FLAGS=/W4 /Ox /Ob2 /Gm /Zi /Oi /GS- /EHa /MD
set LINK_FLAGS=%libs% kernel32.lib user32.lib ole32.lib %obj%\resource.res /DLL

set files=%src%\main.cpp^
 %src%\ABI_Adpcm.cpp^
 %src%\ABI_Buffers.cpp^
 %src%\ABI_Envmixer.cpp^
 %src%\ABI_Filters.cpp^
 %src%\ABI_MixerInterleave.cpp^
 %src%\ABI_Resample.cpp^
 %src%\ABI1.cpp^
 %src%\ABI2.cpp^
 %src%\ABI3.cpp^
 %src%\ABI3mp3.cpp^
 %src%\Configuration.cpp^
 %src%\DirectSoundDriver.cpp^
 %src%\HLEMain.cpp^
 %src%\SoundDriver.cpp^
 %src%\SoundDriverFactory.cpp^
 %src%\SoundDriverInterface.cpp^
 %src%\NoSoundDriver.cpp^
 %src%\WaveOut.cpp^
 %src%\XAudio2SoundDriver.cpp^
 %src%\Mupen64plusHLE\audio.c^
 %src%\Mupen64plusHLE\memory.c^
 %src%\Mupen64plusHLE\Mupen64Support.c^
 %src%\Mupen64plusHLE\musyx.c

%DDK%\bin\x86\rc.exe %incl% /fo %obj%\resource.res %src%\resource.rc
%MSVC%\cl.exe %files% %incl% %C_FLAGS% /link /OUT:AziAudio.dll %LINK_FLAGS%

pause
