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

#pragma once

#include "common.h"
#if defined(_WIN32) && !defined(_XBOX)
#include <mmreg.h>
#endif
#include <dsound.h>
#include "SoundDriver.h"
#include "SoundDriverInterface.h"

#define DS_SEGMENTS     4
#define LOCK_SIZE sLOCK_SIZE
// 0x600
//sLOCK_SIZE
//0x700

#define TOTAL_SIZE (LOCK_SIZE*DS_SEGMENTS)

#define MAXBUFFER 27000
//27000
//(LOCK_SIZE*DS_SEGMENTS+LOCK_SIZE)
// LOCKSIZE must not be fractional
//#define LOCK_SIZE    (ac->SegmentSize)

#define BUFFSIZE (writeLoc-readLoc)

class DirectSoundDriver :
	public SoundDriver
{
protected:
	DWORD dwFreqTarget; // Frequency of the Nintendo64 Game Audio
	void(*CallBack)(DWORD);
	BOOL audioIsPlaying;
	HANDLE handleAudioThread;
	DWORD  dwAudioThreadId;
	HANDLE hMutex;
	LPDIRECTSOUNDBUFFER  lpdsbuf;
	LPDIRECTSOUND8        lpds;
	bool audioIsDone;
	// Buffer Variables
	BYTE SoundBuffer[500000];//[MAXBUFFER];
	DWORD readLoc;
	DWORD writeLoc;
	volatile DWORD remainingBytes;
	DWORD SampleRate;
	DWORD SegmentSize;

public:

	friend DWORD WINAPI AudioThreadProc(DirectSoundDriver *ac);

	DirectSoundDriver() { lpdsbuf = NULL; lpds = NULL; audioIsDone = false; hMutex = NULL; handleAudioThread = NULL; audioIsPlaying = FALSE; readLoc = writeLoc = remainingBytes = 0; SampleRate = 0; };
	//DirectSoundDriver() {};
	~DirectSoundDriver() { };

	// Setup and Teardown Functions
	BOOL Initialize();
	void DeInitialize();

	// Buffer Functions for the Audio Code
	void SetFrequency(u32 Frequency);           // Sets the Nintendo64 Game Audio Frequency
	void SetSegmentSize(DWORD length);

	// Management functions
	void AiUpdate(BOOL Wait);
	void StopAudio();							// Stops the Audio PlayBack (as if paused)
	void StartAudio();							// Starts the Audio PlayBack (as if unpaused)

	void SetVolume(u32 volume);

	static SoundDriverInterface* CreateSoundDriver() { return new DirectSoundDriver(); }
};
