@setlocal enableextensions enabledelayedexpansion
@ECHO off
ECHO ÕÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¸
ECHO ³ AziAudio MSYS-less MinGW build script v2.0                             ³
ECHO ÆÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¾


 REM Defaults
SET MinGW_PATH_DEFAULT=C:\MinGW





 REM Load all arguments into variables so we can easily parse them.
FOR %%a IN (%*) do (
    CALL:getargs "%%a" 
)

 REM Parse --help
IF DEFINED --help (
    ECHO ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
    ECHO ³ Use this script to build AziAudio without using MSYS or messing with   ³
    ECHO ³ your PATH. Both versions of the plugin will be built.                  ³
    ECHO ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
    ECHO ³ This script offers some limited Configuration options.                 ³
    ECHO ³                                                                        ³
    ECHO ³ --mingwpath [directory]                                                ³
    ECHO ³   Location of your mingw installation. We look for the bin folder, so  ³
    ECHO ³   do not point this path directly at the location of GCC.              ³
    ECHO ³   DEFAULT: C:\MinGW                                                    ³
    ECHO ³                                                                        ³
    ECHO ³ --debug                                                                ³
    ECHO ³   Pass this argument to make debug builds.                             ³
    ECHO ³                                                                        ³
    ECHO ³ --arch [architecture]                                                  ³
    ECHO ³   Choose which architecture to build for. Valid options are x86 and    ³
    ECHO ³   x86_64.                                                              ³
    ECHO ³   DEFAULT: try to auto-detect                                          ³
    ECHO ³   NOTE: This option only affects the build locations.                  ³
    ECHO ³                                                                        ³
    ECHO ³ --help                                                                 ³
    ECHO ³   You're already here.                                                 ³
    ECHO ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
    GOTO done
)

 REM Parse --mingwpath
IF NOT DEFINED --mingwpath (
    SET --mingwpath=%MinGW_PATH_DEFAULT%
)
CALL:argtopath %--mingwpath% MinGW
SET PATH=%MinGW%\bin;%PATH%

ECHO ³ Verifying presence of needed tools...
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
    ECHO ³ FATAL:    Unsupported architecture specified.
    ECHO ³           Valid architectures are x86 and x86_64.
    ECHO À Stopping.
    exit /b 400
) )
IF NOT %ARCH% == %ARCH_DETECTED% (
    ECHO ³ WARNING:  Detected GCC target architecture does not match the architecture
    ECHO ³           specified.
    ECHO ³           Specified architecture: %ARCH%
    ECHO ³           GCC Target:             %GCC_TRIP%
    ECHO ³                                   (interpreted as %ARCH_DETECTED%^)
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

ECHO ÆÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¸
ECHO ³ Configuration:                                                         ³
ECHO ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
ECHO ³ MinGW location:       %MinGW%
ECHO ³   (Don't worry if this isn't accurate)
ECHO ³ Build architecture:  %ARCH%
ECHO ³ Build type:          %BUILD_TYPE%
ECHO À Starting build...

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
    ECHO ³   %~1 found!
)
GOTO:EOF

:existsinpath2
IF "%~$PATH:1" == "" ( IF "%~$PATH:2" == "" (
	ECHO ³ FATAL:    Could not find required tool %~1.
    ECHO ³           Did you supply the correct --mingwpath?
    ECHO À Stopping.
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
