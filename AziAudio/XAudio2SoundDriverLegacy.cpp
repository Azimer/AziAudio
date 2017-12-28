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

#include "common.h"
#include "XAudio2SoundDriverLegacy.h"
#include "AudioSpec.h"
#include <stdio.h>
#include "SoundDriverFactory.h"

static bool ClassRegistered = SoundDriverFactory::RegisterSoundDriver(SND_DRIVER_XA2L, XAudio2SoundDriverLegacy::CreateSoundDriver, "XAudio2 Legacy Driver", 11);

static IXAudio2* g_engine;
static IXAudio2SourceVoice* g_source;
static IXAudio2MasteringVoice* g_master;

static bool audioIsPlaying = false;
static bool canPlay = false;

static u8 bufferData[10][44100 * 4];
static u8* bufferLocation[10];
static int bufferLength[10];
static int writeBuffer = 0;
static int readBuffer = 0;
static int filledBuffers;
static int bufferBytes;
static int lastLength = 1;

static int cacheSize = 0;
static int interrupts = 0;
static VoiceCallbackLegacy voiceCallback;
XAudio2SoundDriverLegacy::XAudio2SoundDriverLegacy()
{
	g_engine = NULL;
	g_source = NULL;
	g_master = NULL;
	dllInitialized = false;
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}


XAudio2SoundDriverLegacy::~XAudio2SoundDriverLegacy()
{
	DeInitialize();
	//Teardown();
	CoUninitialize();
}
static HANDLE hMutex;


BOOL XAudio2SoundDriverLegacy::Initialize()
{
	if (g_source != NULL)
	{
		g_source->Start();
	}
	bufferLength[0] = bufferLength[1] = bufferLength[2] = bufferLength[3] = bufferLength[4] = bufferLength[5] = 0;
	bufferLength[6] = bufferLength[7] = bufferLength[8] = bufferLength[9] = 0;
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

BOOL XAudio2SoundDriverLegacy::Setup()
{
	if (dllInitialized == true) return true;
	dllInitialized = true;
	bufferLength[0] = bufferLength[1] = bufferLength[2] = bufferLength[3] = bufferLength[4] = bufferLength[5] = 0;
	bufferLength[6] = bufferLength[7] = bufferLength[8] = bufferLength[9] = 0;
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
void XAudio2SoundDriverLegacy::DeInitialize()
{
	Teardown();
}

void XAudio2SoundDriverLegacy::Teardown()
{
	if (dllInitialized == false) return;
	canPlay = false;
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

void XAudio2SoundDriverLegacy::SetFrequency(u32 Frequency)
{
	if (Setup() < 0) /* failed to apply a sound device */
		return;
	cacheSize = (Frequency / 25) * 4;// (((Frequency * 4) / 100) & ~0x3) * 8;
	g_source->FlushSourceBuffers();
	g_source->SetSourceSampleRate(Frequency);
}

u32 XAudio2SoundDriverLegacy::AddBuffer(u8 *start, u32 length)
{
	if (length == 0 || g_source == NULL) {
		*AudioInfo.AI_STATUS_REG = 0;
		*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
		AudioInfo.CheckInterrupts();
		return 0;
	}
	lastLength = length;

	// Gracefully waiting for filled buffers to deplete
	if (Configuration::getSyncAudio() == true || Configuration::getForceSync() == true)
		while (filledBuffers == 10) Sleep(1);
	else 
		if (filledBuffers == 10) return 0;
		
	WaitForSingleObject(hMutex, INFINITE);
	for (DWORD x = 0; x < length; x += 4)
	{
		bufferData[writeBuffer][x] = start[x + 2];
		bufferData[writeBuffer][x+1] = start[x + 3];
		bufferData[writeBuffer][x+2] = start[x];
		bufferData[writeBuffer][x+3] = start[x + 1];
	}
	// TODO: HatCat suggestion to get the compiler to optimize it rather than unrolling the loop.
	//for (size_t x = 0; x < length; x++)
	//	bufferData[writeBuffer][x] = start[x ^ 2];
	bufferLength[writeBuffer] = length;
	bufferBytes += length;
	filledBuffers++;

	XAUDIO2_BUFFER xa2buff;

	xa2buff.Flags = XAUDIO2_END_OF_STREAM; // Suppress XAudio2 warnings
	xa2buff.PlayBegin = 0;
	xa2buff.PlayLength = 0;
	xa2buff.LoopBegin = 0;
	xa2buff.LoopLength = 0;
	xa2buff.LoopCount = 0;
	xa2buff.pContext = &bufferLength[writeBuffer];
	xa2buff.AudioBytes = length;
	xa2buff.pAudioData = bufferData[writeBuffer];
	if (canPlay)
		g_source->SubmitSourceBuffer(&xa2buff);

	++writeBuffer;
	writeBuffer %= 10;

	if (bufferBytes < cacheSize || Configuration::getForceSync() == true)
	{
		*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
		*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
		AudioInfo.CheckInterrupts();
	}
	else
	{
		if (filledBuffers >= 2)
			*AudioInfo.AI_STATUS_REG |= AI_STATUS_FIFO_FULL;
		interrupts++;
	}
	ReleaseMutex(hMutex);
	
	return 0;
}

void XAudio2SoundDriverLegacy::AiUpdate(BOOL Wait)
{
	if (Wait)
		WaitMessage();
}

void XAudio2SoundDriverLegacy::StopAudio()
{
	
	audioIsPlaying = false;
	if (g_source != NULL)
	{
		g_source->Stop();
		g_source->FlushSourceBuffers();
	}
		
}

void XAudio2SoundDriverLegacy::StartAudio()
{
	audioIsPlaying = true;
}

u32 XAudio2SoundDriverLegacy::GetReadStatus()
{
	XAUDIO2_VOICE_STATE xvs;
	int retVal;

	if (canPlay)
		g_source->GetState(&xvs);
	else
		return 0;

	if (xvs.BuffersQueued == 0 || Configuration::getForceSync() == true) return 0;

	if (bufferBytes + lastLength < cacheSize)
		return 0;
	else
		retVal = (lastLength - xvs.SamplesPlayed * 4) & ~0x7;

	if (retVal < 0) return 0; else return retVal % lastLength;
}

// 100 - Mute to 0 - Full Volume
void XAudio2SoundDriverLegacy::SetVolume(u32 volume)
{
	float xaVolume = 1.0f - ((float)volume / 100.0f);
	if (g_source != NULL) g_source->SetVolume(xaVolume);
	//XAUDIO2_MAX_VOLUME_LEVEL
}


void __stdcall VoiceCallbackLegacy::OnBufferEnd(void * pBufferContext)
{
	UNREFERENCED_PARAMETER(pBufferContext);
	WaitForSingleObject(hMutex, INFINITE);
#ifdef SEH_SUPPORTED
	__try // PJ64 likes to close objects before it shuts down the DLLs completely...
	{
#endif
		*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
		if (interrupts > 0)
		{
			interrupts--;
			*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
			AudioInfo.CheckInterrupts();
		}
#ifdef SEH_SUPPORTED
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#endif
	bufferBytes -= *(int *)(pBufferContext);
	filledBuffers--;
	ReleaseMutex(hMutex);
}

void __stdcall VoiceCallbackLegacy::OnVoiceProcessingPassStart(UINT32 SamplesRequired) 
{
	UNREFERENCED_PARAMETER(SamplesRequired);
	//if (SamplesRequired > 0)
	//	DEBUG_OUTPUT("SR: %i FB: %i BB: %i  CS:%i\n", SamplesRequired, filledBuffers, bufferBytes, cacheSize);
}
