@setlocal enableextensions enabledelayedexpansion
@ECHO off
ECHO ������������������������������������������������������������������������͸
ECHO � AziAudio MSYS-less MinGW build script v2.0                             �
ECHO ������������������������������������������������������������������������;


 REM Defaults
SET MinGW_PATH_DEFAULT=C:\MinGW





 REM Load all arguments into variables so we can easily parse them.
FOR %%a IN (%*) do (
    CALL:getargs "%%a" 
)

 REM Parse --help
IF DEFINED --help (
    ECHO ������������������������������������������������������������������������Ŀ
    ECHO � Use this script to build AziAudio without using MSYS or messing with   �
    ECHO � your PATH. Both versions of the plugin will be built.                  �
    ECHO ������������������������������������������������������������������������Ĵ
    ECHO � This script offers some limited Configuration options.                 �
    ECHO �                                                                        �
    ECHO � --mingwpath [directory]                                                �
    ECHO �   Location of your mingw installation. We look for the bin folder, so  �
    ECHO �   do not point this path directly at the location of GCC.              �
    ECHO �   DEFAULT: C:\MinGW                                                    �
    ECHO �                                                                        �
    ECHO � --debug                                                                �
    ECHO �   Pass this argument to make debug builds.                             �
    ECHO �                                                                        �
    ECHO � --arch [architecture]                                                  �
    ECHO �   Choose which architecture to build for. Valid options are x86 and    �
    ECHO �   x86_64.                                                              �
    ECHO �   DEFAULT: try to auto-detect                                          �
    ECHO �   NOTE: This option only affects the build locations.                  �
    ECHO �                                                                        �
    ECHO � --help                                                                 �
    ECHO �   You're already here.                                                 �
    ECHO ��������������������������������������������������������������������������
    GOTO done
)

 REM Parse --mingwpath
IF NOT DEFINED --mingwpath (
    SET --mingwpath=%MinGW_PATH_DEFAULT%
)
CALL:argtopath %--mingwpath% MinGW
SET PATH=%MinGW%\bin;%PATH%

ECHO � Verifying presence of needed tools...
CALL:existsinpath gcc
IF errorlevel 404 (
    exit /b 404
)
CALL:existsinpath g++
IF errorlevel 404 (
    exit /b 404
)
CALL:existsinpath windres
IF errorlevel 404 (
    exit /b 404
)

 REM Parse --debug
IF NOT DEFINED --debug_PASSED (
	SET CFLAGS_BUILD= -O2
	SET RESFLAGS_BUILD=
	SET BUILD_TYPE=Release
	SET XA_PLUGIN_FILE=AziAudioXA2_m.dll
	SET DS_PLUGIN_FILE=AziAudioDS8_m.dll
) ELSE (
	SET CFLAGS_BUILD= -g -D_DEBUG
	SET RESFLAGS_BUILD= -D_DEBUG
	SET BUILD_TYPE=Debug
	SET XA_PLUGIN_FILE=AziAudioXA2_md.dll
	SET DS_PLUGIN_FILE=AziAudioDS8_md.dll
)

 REM Parse --arch
CALL:detectarch
IF NOT DEFINED --arch (
    SET --arch=%ARCH_DETECTED%
) ELSE (
    ECHO --arch is %--arch%
)
IF "%--arch%" == "x86" (
    SET ARCH=x86
    SET OUTDIR=%BUILD_TYPE%_mingw32
) ELSE ( IF "%--arch%" == "x86_64" (
    SET ARCH=x86_64
    SET OUTDIR=%BUILD_TYPE%_mingw64
) ELSE (
    ECHO � FATAL:    Unsupported architecture specified.
    ECHO �           Valid architectures are x86 and x86_64.
    ECHO � Stopping.
    exit /b 400
) )
IF NOT %ARCH% == %ARCH_DETECTED% (
    ECHO � WARNING:  Detected GCC target architecture does not match the architecture
    ECHO �           specified.
    ECHO �           Specified architecture: %ARCH%
    ECHO �           GCC Target:             %GCC_TRIP%
    ECHO �                                   (interpreted as %ARCH_DETECTED%^)
)

 REM Set variables
set OBJDIR=build/%OUTDIR%/AziAudio
set BUILDDIR=bin/%OUTDIR%
set CFLAGS= %CFLAGS_BUILD% -msse2 -DSSE2_SUPPORT -mstackrealign
set LDFLAGS= -static-libstdc++ -static-libgcc
set RESFLAGS= %RESFLAGS_BUILD%
set SRCDIR=../AziAudio
set XA_FLAGS= -I"%SRCDIR%/../3rd Party/directx/include" -I"%SRCDIR%/../3rd Party" -Wno-attributes
set DS_FLAGS= -DXAUDIO_LIBRARIES_UNAVAILABLE -I"%SRCDIR%/../3rd Party/directx/include" -Wno-conversion-null
SET OBJS=
SET XA_OBJS=
SET DS_OBJS=

ECHO ������������������������������������������������������������������������͸
ECHO � Configuration:                                                         �
ECHO ��������������������������������������������������������������������������
ECHO � MinGW location:       %MinGW%
ECHO �   (Don't worry if this isn't accurate)
ECHO � Build architecture:  %ARCH%
ECHO � Build type:          %BUILD_TYPE%
ECHO � Starting build...

 REM Make directories
CALL:mkdir %OBJDIR%/Mupen64plusHLE
CALL:mkdir %OBJDIR%/XA
CALL:mkdir %OBJDIR%/DS
CALL:mkdir %BUILDDIR%

ECHO Compiling Sources...
CALL:gppcust HLEMain ALL
CALL:gppcust main ALL
CALL:gppcust Configuration ALL
CALL:gppcust SoundDriver ALL
CALL:gppcust SoundDriverFactory ALL
CALL:gppcust SoundDriverInterface ALL
CALL:gppcust NoSoundDriver ALL
CALL:gppcust XAudio2SoundDriver XA
CALL:gppcust DirectSoundDriver DS
CALL:gpp WaveOut
CALL:gpp ABI_Resample
CALL:gpp ABI_MixerInterleave
CALL:gpp ABI_Filters
CALL:gpp ABI_Envmixer
CALL:gpp ABI_Buffers
CALL:gpp ABI_Adpcm
CALL:gpp ABI3mp3
CALL:gpp ABI3
CALL:gpp ABI2
CALL:gpp ABI1
CALL:gcc Mupen64plusHLE/musyx
CALL:gcc Mupen64plusHLE/Mupen64Support
CALL:gcc Mupen64plusHLE/memory
CALL:gcc Mupen64plusHLE/audio
ECHO Compiling resources...
windres %RESFLAGS% "%SRCDIR%/resource.rc" "%OBJDIR%/resource.o"
SET OBJS=%OBJS% "%OBJDIR%/resource.o"

ECHO Linking...
ECHO     %XA_PLUGIN_FILE%
g++ -shared %CFLAGS% -o "%BUILDDIR%\%XA_PLUGIN_FILE%" %XA_OBJS% %OBJS% %LDFLAGS% -lole32
ECHO     %DS_PLUGIN_FILE%
g++ -shared %CFLAGS% -o "%BUILDDIR%\%DS_PLUGIN_FILE%" %DS_OBJS% %OBJS% %LDFLAGS% -ldsound

GOTO done

:getargs
ECHO.%~1 | findstr /C:"--" 1>nul

 REM Next we check the errorlevel return to see if it contains a key or a value
 REM and set the appropriate action.

IF NOT errorlevel 1 (
  SET KEY=%~1
  SET %~1_PASSED=TRUE
) ELSE (
  SET VALUE=%~1
)
IF DEFINED VALUE (
    SET %KEY%=%~1
     REM It's important to clear the definitions for the the key and value in order to
     REM search for the next key value set.
    SET KEY=
    SET VALUE=
)
GOTO:EOF

:argtopath
SET %~2=%~f1
GOTO:EOF

:existsinpath
CALL:existsinpath2 %~1 %~1.exe
IF errorlevel 404 (
    exit /b 404
) ELSE (
    ECHO �   %~1 found!
)
GOTO:EOF

:existsinpath2
IF "%~$PATH:1" == "" ( IF "%~$PATH:2" == "" (
	ECHO � FATAL:    Could not find required tool %~1.
    ECHO �           Did you supply the correct --mingwpath?
    ECHO � Stopping.
	exit /b 404
) )
GOTO:EOF

:detectarch
 REM We get target triplicate from GCC and use that to try and figure out the
 REM target architecture.
FOR /F "delims=" %%i IN ('gcc -dumpmachine') DO set GCC_TRIP=%%i
FOR /F "delims=-" %%i IN ('gcc -dumpmachine') DO set ARCH_GCC=%%i
IF "%ARCH_GCC%" == "x86_64" (
	SET ARCH_DETECTED=x86_64
) ELSE (
	SET ARCH_DETECTED=x86
)
GOTO:EOF

:mkdir
IF NOT EXIST "%~f1" (
    mkdir "%~f1"
)
GOTO:EOF

:gcc
ECHO     %~1.c...
gcc %CFLAGS% -o "%OBJDIR%/%~1.o" -c "%SRCDIR%/%~1.c"
SET OBJS=%OBJS% "%OBJDIR%/%~1.o"
GOTO:EOF

:gpp
ECHO     %~1.cpp...
g++ %CFLAGS% -o "%OBJDIR%/%~1.o" -c "%SRCDIR%/%~1.cpp"
SET OBJS=%OBJS% "%OBJDIR%/%~1.o"
GOTO:EOF

:gppcust
IF "%~2" == "ALL" (
    ECHO     %~1.cpp (for XAudio2 plugin^)...
    g++ %CFLAGS% %XA_FLAGS% -o "%OBJDIR%/XA/%~1.o" -c "%SRCDIR%/%~1.cpp"
    SET XA_OBJS="%OBJDIR%/XA/%~1.o" %XA_OBJS%
    ECHO     %~1.cpp (for DirectSound8 plugin^)...
    g++ %CFLAGS% %DS_FLAGS% -o "%OBJDIR%/DS/%~1.o" -c "%SRCDIR%/%~1.cpp"
    SET DS_OBJS="%OBJDIR%/DS/%~1.o" %DS_OBJS%
) ELSE (
    ECHO     %~1.cpp...
    SET TEMPVAR=%~2
    g++ %CFLAGS% !%~2_FLAGS! -o "%OBJDIR%/%~2/%~1.o" -c "%SRCDIR%/%~1.cpp"
    SET %~2_OBJS="%OBJDIR%/%~2/%~1.o" !%~2_OBJS!
)
GOTO:EOF

:done
endlocal
