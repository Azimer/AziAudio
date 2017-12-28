/****************************************************************************
*                                                                           *
* Azimer's HLE Audio Plugin for Project64 Compatible N64 Emulators          *
* http://www.apollo64.com/                                                  *
* Copyright (C) 2000-2017 Azimer. All rights reserved.                      *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

//************ Configuration Section ************** (to be moved to compile time defines)

// Configure the plugin to have a console window for informational output -- should be used for debugging only
//#define USE_PRINTF

#ifndef _COMMON_DOT_H_
#define _COMMON_DOT_H_

#include <stddef.h> // size_t definition

#if defined (_XBOX)
#include <xtl.h>
#include "../3rd Party/XBox/xbox_depp.h"
#elif defined(_WIN32)
#include <windows.h>
#include <commctrl.h>
#endif

#ifdef USE_PRINTF
#include <stdio.h>
#endif
#include <assert.h>

#if 0
#define ENABLEPROFILING
#endif

#if defined(_MSC_VER)
#define SEH_SUPPORTED
#endif

#ifdef USE_PRINTF
#define DEBUG_OUTPUT printf
#else
#define DEBUG_OUTPUT //
#endif




#include "my_types.h"

enum SoundDriverType
{
	SND_DRIVER_NOSOUND = 0x0000,
	SND_DRIVER_DS8L = 0x1000,
	SND_DRIVER_DS8 = 0x1001,
	SND_DRIVER_XA2L = 0x1002,
	SND_DRIVER_XA2 = 0x1003,
};


typedef struct {
	u16 Version;
	u32 BufferSize;
	Boolean doAIHACK;
	Boolean syncAudio;
	Boolean fillAudio;
	Boolean oldStyle;
	Boolean Reserved2;
	Boolean Reserved3;
	u32  Reserved4;
	u32  Reserved5;
	u32  Reserved6;
} rSettings;
extern rSettings RegSettings;
#endif

#define AUDIOCODE 0
#define HLECODE   1
#define CPUCODE   2

unsigned long GenerateCRC (unsigned char *data, int size);

#define PLUGIN_NAME     "Audio"

#ifdef DEVBUILD
#ifdef __GNUC__
#define PLUGIN_BUILDSYS "Mingw"
#else
#define PLUGIN_BUILDSYS "MSVC"
#endif
#ifdef _DEBUG
#define PLUGIN_DEBUG " (" PLUGIN_BUILDSYS " Debug)"
#else
#define PLUGIN_DEBUG " (" PLUGIN_BUILDSYS ")"
#endif
#else
#ifdef _DEBUG
#define PLUGIN_DEBUG " (Debug r23)"
#else
#define PLUGIN_DEBUG ""
#endif
#endif

#define PLUGIN_RELEASE " v0.70 "
#define PLUGIN_BUILD "WIP 8" \
	   PLUGIN_DEBUG 

#define PLUGIN_VERSION \
"Azimer's " \
PLUGIN_NAME \
PLUGIN_RELEASE \
PLUGIN_BUILD


#ifdef ENABLEPROFILING

	extern u64 ProfileStartTimes[30];
	extern u64 ProfileTimes[30];

	inline void StartProfile (int profile) {
		u64 start;
		__asm {
			rdtsc;
			mov dword ptr [start+0], eax;
			mov dword ptr [start+4], edx;
		}
		ProfileStartTimes[profile] = start;
	}

	inline void EndProfile (int profile) {
		u64 end;
		__asm {
			rdtsc;
			mov dword ptr [end+0], eax;
			mov dword ptr [end+4], edx;
		}
		ProfileTimes[profile] = ProfileTimes[profile] + (end - ProfileStartTimes[profile]);
	}
	inline void PrintProfiles () {
		FILE *dfile = fopen ("d:\\profile.txt", "wt");
		u64 totalTimes = 0;
		for (int x = 0; x < 30; x++) {
			if (ProfileTimes[x] != 0) {
				fprintf (dfile, "Times for %i is: %08X %08X\n", x, (u32)(ProfileTimes[x] >> 32), (u32)ProfileTimes[x]);
				totalTimes += ProfileTimes[x];
			}
		}
		for (x = 0; x < 30; x++) {
			if (ProfileTimes[x] != 0) {
				fprintf (dfile, "Percent Time for %i is: %i%%\n", x, (u32)((ProfileTimes[x]*100) / totalTimes));			
			}
		}
		fclose (dfile);
	}
	inline void ClearProfiles () {
		for (int x = 0; x < 30; x++) {
			ProfileTimes[x] = 0;
		}
	}
#else
#	define StartProfile(profile) //
#	define EndProfile(profile) //
#	define PrintProfiles() //
#	define ClearProfiles()//
#endif

/*
 * `strcpy` with bounds checking
 * This basically is a portable variation of Microsoft's `strcpy_s`.
 */
extern int safe_strcpy(char* dst, size_t limit, const char* src);
