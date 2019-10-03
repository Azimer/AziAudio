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
//#define _WIN32_WINNT 0x0601
#ifdef _WIN32
#include <Windows.h>
#include <mmsystem.h>
#include <amstream.h>
#endif

#include "SoundDriver.h"

class WaveOutSoundDriver :
	public SoundDriver
{
public:
	WaveOutSoundDriver();
	~WaveOutSoundDriver();

	// Setup and Teardown Functions
	BOOL Initialize();
	void DeInitialize();
	void Setup();
	void Teardown();

	// Buffer Functions for the Audio Code
	void SetFrequency(u32 Frequency);           // Sets the Nintendo64 Game Audio Frequency

	// Management functions
	void AiUpdate(BOOL Wait) { if (Wait) WaitMessage(); };
	void StopAudio();							// Stops the Audio PlayBack (as if paused)
	void StartAudio();							// Starts the Audio PlayBack (as if unpaused)

	void SetVolume(u32 volume);

	static SoundDriverInterface* CreateSoundDriver() { return new WaveOutSoundDriver(); }
	static bool ValidateDriver();

protected:

	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	HWAVEOUT       m_hWave;

	int m_numOutputBuffers;
	u32 m_OutputBuffersSize;
	WAVEHDR *m_OutputBuffers;
	u8 *m_BufferMemory;
	bool bIsDone;
	u32 SampleRate;

private:
	static bool ClassRegistered;
};

#if !defined(_MSC_VER)
#undef __in
#undef __out
#endif
