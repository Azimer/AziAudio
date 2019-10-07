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
/*
	NoSound Driver to demonstrate how to use the SoundDriver interface
*/
#include "NoSoundDriver.h"
#include "SoundDriverFactory.h"
#if !defined(_WIN32) && !defined(_XBOX)
#include <unistd.h>
#endif

bool NoSoundDriver::ClassRegistered = NoSoundDriver::ValidateDriver() ?
		SoundDriverFactory::RegisterSoundDriver(SND_DRIVER_NOSOUND, NoSoundDriver::CreateSoundDriver, "No Sound Driver", 0) :
		false;

bool NoSoundDriver::ValidateDriver()
{
	// No Sound should always be an option.  The only issue is if GetTickCount isn't supported or something similar
	return true;
}

Boolean NoSoundDriver::Initialize()
{
	dllInitialized = true;
	isPlaying = false;
	m_SamplesPerSecond = false;
	lastTick = 0;
	return true;
}

void NoSoundDriver::DeInitialize()
{
	isPlaying = false;
	dllInitialized = false;
	lastTick = 0;
}

// Management functions
void NoSoundDriver::AiUpdate(Boolean Wait)
{
	u32 bytes;
	u32 tick, tickdiff;
	Wait = Wait; // Avoids unreferences parameter warning.  Required as part of the Project64 API

	// GetTickCount - Retrieves the number of milliseconds that have elapsed since the system was started, up to 49.7 days.
#if defined(_WIN32) || defined(_XBOX)
	if (lastTick == 0)
		lastTick = GetTickCount(); 
#endif	

	if (isPlaying == true)
	{
#if defined(_WIN32) || defined(_XBOX)
		Sleep(5);
		tick = GetTickCount();
		tickdiff = tick - lastTick;
		lastTick = tick;
#else
		usleep(5);
		tickdiff = 50;
#endif	
		if (tickdiff > 50)
		{
			tickdiff = 50;
		}
		bytes = (m_SamplesPerSecond / 1000) * 4 * tickdiff; // Play tickdiff ms of audio
		if (bytes > 0) LoadAiBuffer(NULL, bytes);
	}
	else
	{
#if defined(_WIN32) || defined(_XBOX)
		Sleep(1);
#else
		usleep(1);
#endif	
	}
}

void NoSoundDriver::StopAudio()
{
	isPlaying = false;
}

void NoSoundDriver::StartAudio()
{
	isPlaying = true;
}

void NoSoundDriver::SetFrequency(u32 Frequency)
{
#ifdef _WIN32
	UNREFERENCED_PARAMETER(Frequency);
#else
#endif
}
