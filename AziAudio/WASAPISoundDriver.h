/****************************************************************************
*                                                                           *
* Azimer's HLE Audio Plugin for Project64 Compatible N64 Emulators          *
* http://www.apollo64.com/                                                  *
* Copyright (C) 2000-2019 Azimer. All rights reserved.                      *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#pragma once
//#define _WIN32_WINNT 0x0601
#ifdef _WIN32
#include <Windows.h>
#endif

#include "SoundDriver.h"

class WASAPISoundDriver :
	public SoundDriver
{
	friend DWORD WINAPI WASAPISoundDriver::AudioThreadProc(LPVOID lpParameter);
public:
	WASAPISoundDriver();
	~WASAPISoundDriver();

	// Setup and Teardown Functions
	BOOL Initialize();
	void DeInitialize();

	// Buffer Functions for the Audio Code
	void SetFrequency(u32 Frequency);           // Sets the Nintendo64 Game Audio Frequency

	// Management functions
	void AiUpdate(BOOL Wait) { if (Wait) WaitMessage(); };
	void StopAudio();							// Stops the Audio PlayBack (as if paused)
	void StartAudio();							// Starts the Audio PlayBack (as if unpaused)

	void SetVolume(u32 volume);

	static SoundDriverInterface* CreateSoundDriver() { return new WASAPISoundDriver(); }

	u32 LoadAiBufferResample(u8 *start, u32 length, float ratio);

	// Override the default in SoundDriver
	//u32 WASAPISoundDriver::LoadAiBuffer(u8 *start, u32 length);

	static bool ValidateDriver();

protected:
	static DWORD WINAPI WASAPISoundDriver::AudioThreadProc(LPVOID lpParameter);

	//bool dllInitialized;
	bool bInitialized;
	float m_Volume;

private:
	HANDLE hAudioThread;
	bool   bStopAudioThread;
	bool   m_CoUninit;
	static bool ClassRegistered;
};

#if !defined(_MSC_VER)
#undef __in
#undef __out
#endif
