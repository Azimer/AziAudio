/****************************************************************************
*                                                                           *
* Azimer's HLE Audio Plugin for Project64 Compatible N64 Emulators          *
* http://www.apollo64.com/                                                  *
* Copyright (C) 2000-2021 Azimer. All rights reserved.                      *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#include "common.h"
#if defined(ENABLE_BACKEND_WAVEOUT)
#include "WaveOutSoundDriver.h"
#include "AudioSpec.h"
#include <stdio.h>
#include "SoundDriverFactory.h"

#pragma comment(lib, "winmm.lib") // TODO: Move this to the project / propsheets

WaveOutSoundDriver* WaveOutSoundDriver::m_Instance;

bool WaveOutSoundDriver::ClassRegistered = ValidateDriver() ?
		SoundDriverFactory::RegisterSoundDriver(SND_DRIVER_WAVEOUT, WaveOutSoundDriver::CreateSoundDriver, "WaveOut Driver", 1) :
		false;

/*
	Will verify the driver can run in the configured environment
*/
bool WaveOutSoundDriver::ValidateDriver()
{
	// This should be available in all versions of Windows in the last 25 years
	WAVEOUTCAPS caps;
	waveOutGetDevCaps(WAVE_MAPPER, &caps, sizeof(WAVEOUTCAPS));
	if (caps.dwFormats & WAVE_FORMAT_4S16)
		return true;
	else
		return false;
}

WaveOutSoundDriver::WaveOutSoundDriver()
{
	DEBUG_OUTPUT("WO Constructor");
	m_hWave = NULL;
	SampleRate = 0;
	m_numOutputBuffers = 0;
	m_OutputBuffersSize = 0;
	m_OutputBuffers = NULL;
	m_BufferMemory = NULL;
	m_Instance = this;
	DEBUG_OUTPUT("WO Constructor done");
}

WaveOutSoundDriver::~WaveOutSoundDriver()
{
	DEBUG_OUTPUT("WO Deconstructor");
	Teardown();
	DEBUG_OUTPUT("WO Deconstructor done");
}

void WaveOutSoundDriver::Teardown()
{
	DEBUG_OUTPUT("WO: Teardown()\n");
	WaitForSingleObject(m_hMutex, INFINITE);
	bIsDone = true;
	if (m_hWave != NULL)
	{
		waveOutReset(m_hWave);
		waveOutClose(m_hWave);
	}
	if (m_OutputBuffers != NULL)
	{
		delete m_OutputBuffers;
	}
	if (m_BufferMemory != NULL)
	{
		delete m_BufferMemory;
	}
	m_hWave = NULL;
	m_OutputBuffers = NULL;
	m_BufferMemory = NULL;
	ReleaseMutex(m_Instance->m_hMutex);
	DEBUG_OUTPUT("WO: Teardown() done\n");
}

void WaveOutSoundDriver::Setup()
{
	DEBUG_OUTPUT("WO: Setup()\n");
	WAVEFORMATEX wfm;
	memset(&wfm, 0, sizeof(WAVEFORMATEX));

	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = 2;
	wfm.nSamplesPerSec = SampleRate;
	wfm.wBitsPerSample = 16;
	wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

	waveOutOpen(&m_hWave, WAVE_MAPPER, &wfm, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
	bIsDone = false;
	DEBUG_OUTPUT("WO: Setup() done\n");
}

BOOL WaveOutSoundDriver::Initialize()
{
	DEBUG_OUTPUT("WO: Initialize()\n");
	Teardown();
	SampleRate = 0;

	DEBUG_OUTPUT("WO: Initialize() done\n");
	if (m_hWave == NULL)
		return TRUE;
	else
		return FALSE;
}

void WaveOutSoundDriver::DeInitialize()
{
	DEBUG_OUTPUT("WO: DeInitialize()\n");
	Teardown();
	DEBUG_OUTPUT("WO: DeInitialize() done\n");
}

void WaveOutSoundDriver::SetFrequency(u32 Frequency)
{
	DEBUG_OUTPUT("WO: SetFrequency()\n");
	if (SampleRate != Frequency)
	{
		Teardown();
		SampleRate = Frequency;
		Setup();
		if (Configuration::getBackendFPS() <= 60)
		{
			m_OutputBuffersSize = (u32)((Frequency / Configuration::getBackendFPS())) * 4;
		}
		else
		{
			m_OutputBuffersSize = (u32)((Frequency / 60)) * 4;
		}
		m_numOutputBuffers = 3;// Configuration::getBufferLevel(); // TODO: Is this necessary?  It seems "60 FPS" and 3 buffers is lowest before performance impact

		assert(m_OutputBuffers == NULL);
		assert(m_BufferMemory == NULL);
		assert(m_numOutputBuffers > 0);
		m_OutputBuffers = new WAVEHDR[m_numOutputBuffers];
		m_BufferMemory = new u8[m_numOutputBuffers * m_OutputBuffersSize];
		memset(m_BufferMemory, 0, sizeof(u8) * m_numOutputBuffers * m_OutputBuffersSize);
		memset(m_OutputBuffers, 0, sizeof(WAVEHDR) * m_numOutputBuffers);
		for (int i = 0; i < m_numOutputBuffers; i++)
		{
			m_OutputBuffers[i].lpData = (LPSTR)(m_BufferMemory + i*m_OutputBuffersSize);
			m_OutputBuffers[i].dwBufferLength = m_OutputBuffersSize;
			m_OutputBuffers[i].dwUser = i;
			waveOutPrepareHeader(m_hWave, &m_OutputBuffers[i], sizeof(WAVEHDR));
		}
		for (int i = 0; i < m_numOutputBuffers; i++)
		{
			waveOutWrite(m_hWave, &m_OutputBuffers[i], sizeof(WAVEHDR));
		}
	}
}

void WaveOutSoundDriver::StopAudio()
{
	DEBUG_OUTPUT("WO: StopAudio()\n");
	if (m_hWave != NULL)
		waveOutPause(m_hWave);
	DEBUG_OUTPUT("WO: StopAudio() done\n");
}

void WaveOutSoundDriver::StartAudio()
{
	DEBUG_OUTPUT("WO: StartAudio()\n");
	if (m_hWave != NULL)
		waveOutRestart(m_hWave);
	DEBUG_OUTPUT("WO: StartAudio() done\n");
}

void WaveOutSoundDriver::SetVolume(u32 volume)
{
	DWORD level = (DWORD)((((float)(100-volume))/100.0) * 0xFFFF);
	DWORD result = (DWORD)((DWORD)(level & 0xFFFF) | ((DWORD)(level & 0xFFFF) * 0x10000));

	waveOutSetVolume(m_hWave, result);
}


void CALLBACK WaveOutSoundDriver::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	UNREFERENCED_PARAMETER(dwParam2);
	UNREFERENCED_PARAMETER(dwInstance);
	UNREFERENCED_PARAMETER(hwo);
	LPWAVEHDR waveheader;
	DWORD index;

	if (m_Instance->bIsDone) return;
	switch (uMsg)
	{
		case WOM_OPEN:
			DEBUG_OUTPUT("WO: WOM_OPEN\n");
			break;
		case WOM_CLOSE:
			DEBUG_OUTPUT("WO: WOM_CLOSE\n");
			break;
		case WOM_DONE:
			WaitForSingleObject(m_Instance->m_hMutex, INFINITE);
			if (m_Instance->bIsDone) return;
			waveOutUnprepareHeader(m_Instance->m_hWave, (LPWAVEHDR)dwParam1, sizeof(WAVEHDR));
			waveheader = (LPWAVEHDR)dwParam1;
			index = (DWORD)waveheader->dwUser; // We are using the dwUser not as a pointer but a DWORD value
			memset(&m_Instance->m_OutputBuffers[index], 0, sizeof(WAVEHDR));
			m_Instance->m_OutputBuffers[index].lpData = (LPSTR)(m_Instance->m_BufferMemory + index*m_Instance->m_OutputBuffersSize);
			m_Instance->m_OutputBuffers[index].dwBufferLength = m_Instance->m_OutputBuffersSize;
			m_Instance->m_OutputBuffers[index].dwUser = index;
			m_Instance->LoadAiBuffer((u8 *)m_Instance->m_OutputBuffers[index].lpData, m_Instance->m_OutputBuffersSize);
			waveOutPrepareHeader(m_Instance->m_hWave, &m_Instance->m_OutputBuffers[index], sizeof(WAVEHDR));
			waveOutWrite(m_Instance->m_hWave, &m_Instance->m_OutputBuffers[index], sizeof(WAVEHDR));
			ReleaseMutex(m_Instance->m_hMutex);
			break;
	}
}
#endif
