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

#include "common.h"
#if defined(ENABLE_BACKEND_XAUDIO2)
#include "XAudio2SoundDriver.h"
#include "AudioSpec.h"
#include <stdio.h>
#include "SoundDriverFactory.h"

bool XAudio2SoundDriver::ClassRegistered = ValidateDriver() ?
		SoundDriverFactory::RegisterSoundDriver(SND_DRIVER_XA2, XAudio2SoundDriver::CreateSoundDriver, "XAudio2 Driver", 15) :
		false;

static IXAudio2* g_engine;
static IXAudio2SourceVoice* g_source;
static IXAudio2MasteringVoice* g_master;

static bool audioIsPlaying = false;
static bool canPlay = false;

static u8 bufferData[4][44100 * 4];
static int writeBuffer = 0;
static int readBuffer = 0;
static int filledBuffers;
static int bufferBytes;
//static int lastLength = 1;

static int cacheSize = 0;
static int interrupts = 0;
static VoiceCallback voiceCallback;

bool XAudio2SoundDriver::ValidateDriver()
{
	bool retVal = false;
	/* Validate an XAudio2 2.7 object will initialize */
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	const GUID CLSID_XAudio2_Test = { 0x5a508685, 0xa254, 0x4fba, 0x9b, 0x82, 0x9a, 0x24, 0xb0, 0x03, 0x06, 0xaf };
	const GUID IID_IXAudio2_Test = { 0x8bcf1f58, 0x9fe7, 0x4583, 0x8a, 0xc6, 0xe2, 0xad, 0xc4, 0x65, 0xc8, 0xbb };
	IUnknown* obj;

	HRESULT hr = CoCreateInstance(CLSID_XAudio2_Test,
		NULL, CLSCTX_INPROC_SERVER, IID_IXAudio2_Test, (void**)&obj);
	if (SUCCEEDED(hr))
	{
		obj->Release();
		retVal = true;
	}
	CoUninitialize();
	return retVal;
}

XAudio2SoundDriver::XAudio2SoundDriver()
{
	g_engine = NULL;
	g_source = NULL;
	g_master = NULL;
	dllInitialized = false;
	bStopAudioThread = false;
	hAudioThread = NULL;
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}


XAudio2SoundDriver::~XAudio2SoundDriver()
{
	DeInitialize();
	//Teardown();
	CoUninitialize();
}
static HANDLE hMutex;


BOOL XAudio2SoundDriver::Initialize()
{
	if (g_source != NULL)
	{
		g_source->Start();
	}
	audioIsPlaying = false;
	writeBuffer = 0;
	readBuffer = 0;
	filledBuffers = 0;
	bufferBytes = 0;
	lastLength = 1;

	cacheSize = 0;
	interrupts = 0;
	return false;
}

BOOL XAudio2SoundDriver::Setup()
{
	if (dllInitialized == true) return true;
	dllInitialized = true;
	hAudioThread = NULL;
	audioIsPlaying = false;
	writeBuffer = 0;
	readBuffer = 0;
	filledBuffers = 0;
	bufferBytes = 0;
	lastLength = 1;

	cacheSize = 0;
	interrupts = 0;

	hMutex = CreateMutex(NULL, FALSE, NULL);
	if (FAILED(XAudio2Create(&g_engine)))
	{
		CoUninitialize();
		return -1;
	}

	if (FAILED(g_engine->CreateMasteringVoice(&g_master)))
	{
		g_engine->Release();
		CoUninitialize();
		return -2;
	}
	canPlay = true;

	// Load Wave File

	WAVEFORMATEX wfm;

	memset(&wfm, 0, sizeof(WAVEFORMATEX));

	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = 2;
	wfm.nSamplesPerSec = 44100;
	wfm.wBitsPerSample = 16; // TODO: Allow 8bit audio...
	wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;


	if (FAILED(g_engine->CreateSourceVoice(&g_source, &wfm, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceCallback, NULL, NULL)))
	{
		g_engine->Release();
		CoUninitialize();
		return -3;
	}

	g_source->Start();
	SetVolume(Configuration::getVolume());

	return FALSE;
}
void XAudio2SoundDriver::DeInitialize()
{
	Teardown();
}

void XAudio2SoundDriver::Teardown()
{
	if (dllInitialized == false) return;
	canPlay = false;
	StopAudioThread();
	if (hMutex != NULL)
		WaitForSingleObject(hMutex, INFINITE);
	if (g_source != NULL)
	{
		g_source->Stop();
		g_source->FlushSourceBuffers();
		if (hMutex != NULL) ReleaseMutex(hMutex);
		g_source->DestroyVoice();
	}
	if (g_master != NULL) g_master->DestroyVoice();
	if (g_engine != NULL)
	{
		g_engine->StopEngine();
		g_engine->Release();
	}
	g_engine = NULL;
	g_master = NULL;
	g_source = NULL;
	if (hMutex != NULL)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}
	hMutex = NULL;
	dllInitialized = false;
	canPlay = false;
}

void XAudio2SoundDriver::PlayBuffer(u8* data, int bufferSize)
{
	XAUDIO2_BUFFER xa2buff;

	xa2buff.Flags = XAUDIO2_END_OF_STREAM; // Suppress XAudio2 warnings
	xa2buff.PlayBegin = 0;
	xa2buff.PlayLength = 0;
	xa2buff.LoopBegin = 0;
	xa2buff.LoopLength = 0;
	xa2buff.LoopCount = 0;
	xa2buff.pContext = NULL;
	xa2buff.AudioBytes = bufferSize;
	xa2buff.pAudioData = data;
	if (canPlay)
		g_source->SubmitSourceBuffer(&xa2buff);

	XAUDIO2_VOICE_STATE xvs;
	g_source->GetState(&xvs);

	assert(xvs.BuffersQueued < 5);

}

void XAudio2SoundDriver::SetFrequency(u32 Frequency)
{
	if (Setup() < 0) /* failed to apply a sound device */
		return;
	cacheSize = (u32)((Frequency / Configuration::getBackendFPS())) * 4;
	g_source->FlushSourceBuffers();
	g_source->SetSourceSampleRate(Frequency);

	StartAudioThread();
}

void XAudio2SoundDriver::AiUpdate(BOOL Wait)
{
	if (Wait)
		WaitMessage();
}

DWORD WINAPI XAudio2SoundDriver::AudioThreadProc(LPVOID lpParameter)
{
	XAudio2SoundDriver* driver = (XAudio2SoundDriver*)lpParameter;
	static int idx = 0;  // TODO: This needs to be moved...

	while (driver->bStopAudioThread == false)
	{
		if (g_source != NULL)
		{
			XAUDIO2_VOICE_STATE xvs;
			g_source->GetState(&xvs);
			// # of BuffersQueued is a knob we can turn for latency vs buffering
			// 2 is minimum.  Maximum is the size of bufferData which is still TBD (Current 5)
			// It is always possible to new a buffer prior to submission then free it on completion.  Worth it?
			while (xvs.BuffersQueued < 3 && driver->bStopAudioThread == false) // Doubled this in hopes it would help... shouldn't cause too much additional latency
			{
				u32 len = driver->LoadAiBuffer(bufferData[idx], cacheSize);
				if (len > 0)
				{
					driver->PlayBuffer(bufferData[idx], len);
					idx = (idx + 1) % 4;
				}
				else
				{
					if (Configuration::getDisallowSleepXA2() == false) 
						Sleep(0); // Give up timeslice - prevents a 2ms sleep potential
				}
				g_source->GetState(&xvs);
			}
		}
		if (Configuration::getDisallowSleepXA2() == false)
			Sleep(1);
	}
	return 0;
}

void XAudio2SoundDriver::StartAudioThread()
{
	if (hAudioThread == NULL && dllInitialized == true)
	{
		DEBUG_OUTPUT("Audio Thread created\n");
		bStopAudioThread = false;
		hAudioThread = CreateThread(NULL, 0, AudioThreadProc, this, 0, NULL);
		assert(hAudioThread != NULL);
	}
}

void XAudio2SoundDriver::StopAudioThread()
{
	if (hAudioThread != NULL)
	{
		bStopAudioThread = true;
		DWORD result = WaitForSingleObject(hAudioThread, 5000);
		if (result != WAIT_OBJECT_0)
		{
			TerminateThread(hAudioThread, 0);
		}
		DEBUG_OUTPUT("Audio Thread terminated\n");
	}
	hAudioThread = NULL;
	bStopAudioThread = false;
}

void XAudio2SoundDriver::StopAudio()
{
	
	audioIsPlaying = false;
	StopAudioThread();
	if (g_source != NULL)
	{
		g_source->Stop();
		g_source->FlushSourceBuffers();
	}
		
}

void XAudio2SoundDriver::StartAudio()
{
	audioIsPlaying = true;
	StartAudioThread();
}

// 100 - Mute to 0 - Full Volume
void XAudio2SoundDriver::SetVolume(u32 volume)
{
	float xaVolume = 1.0f - ((float)volume / 100.0f);
	if (g_source != NULL) g_source->SetVolume(xaVolume);
	//XAUDIO2_MAX_VOLUME_LEVEL
}


void __stdcall VoiceCallback::OnBufferEnd(void * pBufferContext)
{
	UNREFERENCED_PARAMETER(pBufferContext);
}

void __stdcall VoiceCallback::OnVoiceProcessingPassStart(UINT32 SamplesRequired) 
{
	UNREFERENCED_PARAMETER(SamplesRequired);
}
#endif