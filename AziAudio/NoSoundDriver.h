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
/*
NoSound Driver to demonstrate how to use the SoundDriver interface
*/

#pragma once
#include "common.h"
#include "SoundDriverLegacy.h"

#if !defined(_WIN32) && !defined(_XBOX)
typedef union _LARGE_INTEGER {
#if defined(ANONYMOUS_STRUCTS_ARE_NOT_ALLOWED)
    struct {
        u32 LowPart;
        s32 HighPart;
    }; /* ...but Microsoft uses them anyway. */
#endif
    struct {
        u32 LowPart;
        s32 HighPart;
    } u;
    s64 QuadPart;
} LARGE_INTEGER;
#endif

class NoSoundDriver :
	public SoundDriverLegacy
{
public:
	NoSoundDriver() {};
	~NoSoundDriver() {};

	// Setup and Teardown Functions
	Boolean Initialize();
	void DeInitialize();

	// Management functions
	void AiUpdate(Boolean Wait);
	void StopAudio();
	void StartAudio();
	void SetFrequency(u32 Frequency);

	static SoundDriverInterface* CreateSoundDriver() { return new NoSoundDriver(); }

	u32 GetReadStatus() { return 0;  }
	u32 AddBuffer(u8 *, u32) { return 0; }


protected:
	bool dllInitialized;
	/*
	LARGE_INTEGER perfTimer;
	LARGE_INTEGER perfFreq;
	LARGE_INTEGER perfLast;
	LARGE_INTEGER countsPerSample;
	*/
	bool isPlaying;
};
