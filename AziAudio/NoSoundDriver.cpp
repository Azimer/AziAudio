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
#include "NoSoundDriver.h"
#include "SoundDriverFactory.h"

static bool ClassRegistered = SoundDriverFactory::RegisterSoundDriver(SND_DRIVER_NOSOUND, NoSoundDriver::CreateSoundDriver, "No Sound Driver", 0);

Boolean NoSoundDriver::Initialize()
{
	/*
	Boolean result;
#ifdef _WIN32
	result = QueryPerformanceFrequency(&perfFreq);
	result = QueryPerformanceCounter(&perfLast) || result;
#else
	result = false;
#endif
	*/
	dllInitialized = true;
	isPlaying = false;
	m_SamplesPerSecond = false;
	return true;
}

void NoSoundDriver::DeInitialize()
{
	isPlaying = false;
	dllInitialized = false;
}

// Management functions
void NoSoundDriver::AiUpdate(Boolean Wait)
{
	//LARGE_INTEGER sampleInterval;
	long samples;

	//if (Wait)
	//	WaitMessage(); 
#if defined(_WIN32) || defined(_XBOX)
	if (Wait == TRUE)
		Sleep(1);
#else
	if (Wait == TRUE)
		SDL_Delay(10);
#endif
	
	if (isPlaying == true)
	{
		samples = m_SamplesPerSecond / (1000 / 1);
		//LoadAiBuffer(NULL, samples * 4);
	}
	
	/*
	if (isPlaying == true && countsPerSample.QuadPart > 0)
	{
#ifdef _WIN32
		QueryPerformanceCounter(&perfTimer);
#else
		// To do:  Replace this with SDL, or Linux audio won't play.
#endif
		sampleInterval.QuadPart = perfTimer.QuadPart - perfLast.QuadPart;
		samples = (long)(sampleInterval.QuadPart / countsPerSample.QuadPart);
		if (samples > 0)
		{
			perfLast.QuadPart = perfTimer.QuadPart;// += countsPerSample.QuadPart * samples;
			LoadAiBuffer(NULL, samples * 4); // NULL means it won't actually try to fill a buffer
		}
	}
	*/
}

void NoSoundDriver::StopAudio()
{
	isPlaying = false;
}

void NoSoundDriver::StartAudio()
{
	/*
#ifdef _WIN32
	QueryPerformanceCounter(&perfLast);
#endif
	*/
	isPlaying = true;
}

void NoSoundDriver::SetFrequency(u32 Frequency)
{
#ifdef _WIN32
	UNREFERENCED_PARAMETER(Frequency);
	//int SamplesPerSecond = Frequency; // 16 bit * stereo

	// Must determine the number of Counter units per Sample
	/*QueryPerformanceFrequency(&perfFreq); // Counters per Second

	countsPerSample.QuadPart = perfFreq.QuadPart / SamplesPerSecond;
	QueryPerformanceCounter(&perfTimer);
	perfLast.QuadPart = perfTimer.QuadPart;*/
#else
	// To do:  Replace this with SDL, or Linux audio won't play.
#endif
}
