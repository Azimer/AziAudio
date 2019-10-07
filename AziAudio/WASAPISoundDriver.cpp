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
#if defined(ENABLE_BACKEND_WASAPI)
#include "WASAPISoundDriver.h"
#include "AudioSpec.h"
#include <stdio.h>
#include "SoundDriverFactory.h"
#include <audioclient.h>
#include <mmdeviceapi.h>

bool WASAPISoundDriver::ClassRegistered = WASAPISoundDriver::ValidateDriver() ?
					SoundDriverFactory::RegisterSoundDriver(SND_DRIVER_WASAPI, WASAPISoundDriver::CreateSoundDriver, "WASAPI Driver (experimental)", 0) :
					false;
// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
              { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

bool WASAPISoundDriver::ValidateDriver()
{
	bool retVal = false;
	/* Validate a windows audio services end point enumerator object will initialize */
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	const GUID CLSID_MMDeviceEnumerator_Test = { 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E };
	const GUID IID_IMMDeviceEnumerator_Test = { 0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6 };
	IUnknown* obj;

	HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator_Test,
		NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator_Test, (void**)&obj);
	if (SUCCEEDED(hr))
	{
		obj->Release();
		retVal = true;
	}
	CoUninitialize();
	return retVal;
}

WASAPISoundDriver::WASAPISoundDriver()
{
	bInitialized = false;
	m_CoUninit = false;
	hAudioThread = NULL;
}

WASAPISoundDriver::~WASAPISoundDriver()
{
	DeInitialize();
}

BOOL WASAPISoundDriver::Initialize()
{
	IMMDeviceEnumerator *testEnumerator = NULL;
	HRESULT hr;
	m_CoUninit = false;
	hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hr) && (hr != RPC_E_CHANGED_MODE))
	{
		EXIT_ON_ERROR(hr);
	}
	else
	{
		m_CoUninit = true;
	}
	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&testEnumerator);
	EXIT_ON_ERROR(hr);
	bInitialized = true;
	return TRUE;
Exit:
	SAFE_RELEASE(testEnumerator);
	if (m_CoUninit == true)
		CoUninitialize();
	return FALSE;
}

void WASAPISoundDriver::DeInitialize()
{
	DWORD exitCode;
	if (bInitialized == true)
	{
		bStopAudioThread = true;
		if (hAudioThread != NULL)
		{
			WaitForSingleObject(hAudioThread, 5000);
			GetExitCodeThread(hAudioThread, &exitCode);
			if (exitCode == STILL_ACTIVE)
			{
				TerminateThread(hAudioThread, (DWORD)-1);
			}
			hAudioThread = NULL;
		}
	}
	if (m_CoUninit == true)
		CoUninitialize();
}

void WASAPISoundDriver::SetFrequency(u32 Frequency)
{
	UNREFERENCED_PARAMETER(Frequency);
	if (hAudioThread == NULL)
	{
		bStopAudioThread = false;
		hAudioThread = CreateThread(NULL, 0, AudioThreadProc, this, 0, NULL);
		assert(hAudioThread != NULL);
	}
}

void WASAPISoundDriver::StopAudio()
{
}

void WASAPISoundDriver::StartAudio()
{
}

void WASAPISoundDriver::SetVolume(u32 volume)
{
	m_Volume = (1.0f - ((float)volume / 100.0f));
}

DWORD WINAPI WASAPISoundDriver::AudioThreadProc(LPVOID lpParameter)
{
	WASAPISoundDriver* driver = (WASAPISoundDriver*)lpParameter;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_MILLISEC * (1000 / Configuration::getBackendFPS()) * Configuration::getBufferLevel();
	REFERENCE_TIME hnsActualDuration;
	HRESULT hr;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	IAudioClient *pAudioClient = NULL;
	IAudioRenderClient *pRenderClient = NULL;
	WAVEFORMATEXTENSIBLE *pwfx = NULL;
	UINT32 bufferFrameCount;
	UINT32 numFramesAvailable;
	UINT32 numFramesPadding;
	BYTE *pData;
	DWORD flags = 0;

	WAVEFORMATEXTENSIBLE AudioFormat = {};

	hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	EXIT_ON_ERROR(hr);

	// Get ourselves a device enumerator.  This can be used to determine which device we want to write to
	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator);
	EXIT_ON_ERROR(hr);

	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	EXIT_ON_ERROR(hr);

	hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient); EXIT_ON_ERROR(hr);
	
	hr = pAudioClient->GetMixFormat((WAVEFORMATEX **)&pwfx);	EXIT_ON_ERROR(hr);
	AudioFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	AudioFormat.Format.nChannels = 2;
	AudioFormat.Format.nSamplesPerSec = pwfx->Format.nSamplesPerSec;
	AudioFormat.Format.wBitsPerSample = 32;
	AudioFormat.Format.nBlockAlign = (AudioFormat.Format.wBitsPerSample / 8) * AudioFormat.Format.nChannels;
	AudioFormat.Format.nAvgBytesPerSec = AudioFormat.Format.nSamplesPerSec * AudioFormat.Format.nBlockAlign;
	AudioFormat.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
	AudioFormat.dwChannelMask = 0x3;
	AudioFormat.Samples.wValidBitsPerSample = 32;
	AudioFormat.Samples.wSamplesPerBlock = 32;
	AudioFormat.Samples.wReserved = 32;
	AudioFormat.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

	hr = pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		0,//AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,
		hnsRequestedDuration,
		0,
		(WAVEFORMATEX *)&AudioFormat, //&AudioFormat,
		NULL);
	EXIT_ON_ERROR(hr)

	REFERENCE_TIME pDefault;
	REFERENCE_TIME pMinimum;
	pAudioClient->GetDevicePeriod(&pDefault, &pMinimum);

	// Get the actual size of the allocated buffer.
	hr = pAudioClient->GetBufferSize(&bufferFrameCount); EXIT_ON_ERROR(hr);
	hr = pAudioClient->GetService(IID_IAudioRenderClient,(void**)&pRenderClient); EXIT_ON_ERROR(hr);

	// Grab the entire buffer for the initial fill operation.
	hr = pRenderClient->GetBuffer(bufferFrameCount, &pData); EXIT_ON_ERROR(hr);
	float ratio;
	ratio = (float)driver->m_SamplesPerSecond / (float)AudioFormat.Format.nSamplesPerSec;

	// Load the initial data into the shared buffer.
	driver->LoadAiBufferResample(pData, bufferFrameCount, ratio);

	hr = pRenderClient->ReleaseBuffer(bufferFrameCount, flags);	EXIT_ON_ERROR(hr);

	// Calculate the actual duration of the allocated buffer.	
	hnsActualDuration = ((REFTIMES_PER_SEC * bufferFrameCount) / AudioFormat.Format.nSamplesPerSec);
	DEBUG_OUTPUT("Requested Duration: %i ms\n", (1000 / Configuration::getBackendFPS()));
	DEBUG_OUTPUT("Actual Duration: %i ms\n", (int)(hnsActualDuration / REFTIMES_PER_MILLISEC));
	DEBUG_OUTPUT("Buffer Frame Count: %i\n", bufferFrameCount);
	// Let's play some shit...

	hr = pAudioClient->Start(); EXIT_ON_ERROR(hr); // Start playing.
	
	SetThreadPriority(driver->hAudioThread, THREAD_PRIORITY_TIME_CRITICAL);
	// Each loop fills about half of the shared buffer.
	while (driver->bStopAudioThread == false)
	{
		// See how much buffer space is available.
		hr = pAudioClient->GetCurrentPadding(&numFramesPadding); EXIT_ON_ERROR(hr);

		numFramesAvailable = bufferFrameCount - numFramesPadding;

		if (numFramesAvailable > 0)
		{
			if (numFramesPadding == 0) DEBUG_OUTPUT("!");
			// Grab all the available space in the shared buffer.

			hr = pRenderClient->GetBuffer(numFramesAvailable, &pData); EXIT_ON_ERROR(hr);

			ratio = (float)driver->m_SamplesPerSecond / (float)AudioFormat.Format.nSamplesPerSec;
			driver->LoadAiBufferResample(pData, numFramesAvailable, ratio);
			hr = pRenderClient->ReleaseBuffer(numFramesAvailable, flags); EXIT_ON_ERROR(hr);
			hnsActualDuration = ((REFTIMES_PER_SEC * numFramesAvailable) / AudioFormat.Format.nSamplesPerSec);
		}
		else
		{
			Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));
			hnsActualDuration = REFTIMES_PER_MILLISEC * 2; // At least 1ms sleep
		}
	}
	SetThreadPriority(driver->hAudioThread, THREAD_PRIORITY_NORMAL);
	DEBUG_OUTPUT("WASAPI: Ending loop\n");

	Sleep(10);

	hr = pAudioClient->Stop(); EXIT_ON_ERROR(hr); // Stop playing.

	// Graceful exit
	goto NiceExit;
Exit:
	DEBUG_OUTPUT("WASAPI: Error on thread\n");
	MessageBox(NULL, "An error occurred in the WASAPI audio thread.  This will result in no audio or a hang if prevent buffer overflow is checked.  As this is an experimental audio code, please submit a bug report to github.  Thank you!", 
		"AziAudio ERROR", MB_OK);
NiceExit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pRenderClient);
	CoUninitialize();

	DEBUG_OUTPUT("WASAPI: Exiting thread\n");
	return 0;
}

// TODO: Same as LoadAiBuffer but with a rudementary resample method added
u32 WASAPISoundDriver::LoadAiBufferResample(u8 *start, u32 frames, float ratio)
{
	u32 samplesToMove = frames;// &0xFFFFFFFC;
	u32 bytesToMove = (u32)(samplesToMove * ratio) * 4;
	float *outp = (float *)(start);
	s16 *inp = (s16 *)(m_Buffer + m_CurrentReadLoc);
	float gain = m_Volume * 1.4f;
	float Cinp0, Cinp1;
	static float Linp0 = 0.0f, Linp1 = 0.0f;
	static float accum = 0.0f;

	if (start == NULL) return 0;

	assert(bytesToMove <= m_MaxBufferSize);  // We shouldn't be asking for more.

	m_DMAEnabled = (*AudioInfo.AI_CONTROL_REG & AI_CONTROL_DMA_ON) == AI_CONTROL_DMA_ON;

	if ((bytesToMove > m_MaxBufferSize) || (m_DMAEnabled == false))
	{
		while (samplesToMove > 0)
		{
			outp[0] = Linp0; outp[1] = Linp1;
			outp += 2;
			samplesToMove -= 1;
		}
		return frames;
	}

#ifdef _WIN32
	WaitForSingleObject(m_hMutex, INFINITE);
#else
	puts("[LoadAIBuffer] To do:  non-Win32 m_hMutex");
#endif

	// Step 0: Replace depleted stored buffer for next run
	BufferAudio();

	// Step 1: Deplete stored buffer (should equal length size)
	if (samplesToMove <= m_BufferRemaining)
	{
		Cinp0 = inp[0] / 65535.0f;
		Cinp1 = inp[1] / 65535.0f;
		while (samplesToMove > 0 && m_BufferRemaining > 0)
		{
			outp[0] = (Linp0 + accum * (Cinp0 - Linp0)) * gain;
			outp[1] = (Linp1 + accum * (Cinp1 - Linp1)) * gain;
			if (outp[0] > 1.0) outp[0] = 1.0; if (outp[0] < -1.0) outp[0] = -1.0;
			if (outp[1] > 1.0) outp[1] = 1.0; if (outp[1] < -1.0) outp[1] = -1.0;
			accum += ratio;
			while (accum >= 1.0)
			{
				accum -= 1.0;
				m_CurrentReadLoc += 4;
				m_CurrentReadLoc %= m_MaxBufferSize;
				inp = (s16 *)(m_Buffer + m_CurrentReadLoc);
				Linp0 = Cinp0; Cinp0 = (inp[0] / 65535.0f);
				Linp1 = Cinp1; Cinp1 = (inp[1] / 65535.0f);
				assert(m_BufferRemaining <= m_MaxBufferSize);
				m_BufferRemaining -= 4;
				assert(m_BufferRemaining <= m_MaxBufferSize);
			}
			samplesToMove -= 1;
			outp += 2;
		}
	}

	// Step 2: Fill bytesToMove with silence
	if (samplesToMove == frames)
		DEBUG_OUTPUT("S");

	while (samplesToMove > 0)
	{
		outp[0] = Linp0; outp[1] = Linp1;
		outp += 2;
		samplesToMove -= 1;
	}

	// Step 3: Replace depleted stored buffer for next run
	BufferAudio();

#ifdef _WIN32
	ReleaseMutex(m_hMutex);
#else
	// to do
#endif
	assert(samplesToMove == 0);
	return (frames - samplesToMove);
}
#endif
