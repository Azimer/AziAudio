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
#if defined(ENABLE_BACKEND_DIRECTSOUND8)
#include <stdio.h>
#include "DirectSoundDriver.h"
#include "AudioSpec.h"
//#include "WaveOut.h"
#include "SoundDriverFactory.h"

bool DirectSoundDriver::ClassRegistered = DirectSoundDriver::ValidateDriver() ?
			SoundDriverFactory::RegisterSoundDriver(SND_DRIVER_DS8, DirectSoundDriver::CreateSoundDriver, "DirectSound 8 Driver", 6) :
			false;

// TODO: Clean this up a bit...
static DWORD sLOCK_SIZE;
static DWORD last_pos = 0, write_pos = 0, play_pos = 0, temp = 0, next_pos = 0;
static DWORD last_play = 0;
static DWORD last_write = ~0u;
static LPVOID lpvPtr1, lpvPtr2;
static DWORD dwBytes1, dwBytes2;
static int AudioInterruptTime = -1;
static DWORD lastLength = 0;
static DWORD DMALen[3] = { 0, 0, 0 };
static BYTE *DMAData[3] = { NULL, NULL, NULL };

static LPDIRECTSOUNDBUFFER lpdsbuff = NULL;
static LPDIRECTSOUNDBUFFER lpdsb = NULL;

static DWORD buffsize = 0;
static DWORD laststatus = 0;
static DWORD interruptcnt = 0;

//WaveOut test;

bool DirectSoundDriver::ValidateDriver()
{
	bool retVal = false;
	const GUID CLSID_DirectSound8_Test = { 0x3901cc3f, 0x84b5, 0x4fa4, 0xba, 0x35, 0xaa, 0x81, 0x72, 0xb8, 0xa0, 0x9b };
	const GUID IID_IDirectSound8_Test = { 0xC50A7E93, 0xF395, 0x4834, 0x9E, 0xF6, 0x7F, 0xA9, 0x9D, 0xE5, 0x09, 0x66 };

	/* Validate an DirectSound8 object will initialize */
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	IUnknown* obj;
	HRESULT hr = CoCreateInstance(CLSID_DirectSound8_Test,
		NULL, CLSCTX_INPROC_SERVER, IID_IDirectSound8_Test, (void**)&obj);
	if (SUCCEEDED(hr))
	{
		obj->Release();
		retVal = true;
	}
	CoUninitialize();
	return retVal;
}

DWORD WINAPI AudioThreadProc(DirectSoundDriver *ac) {
	DWORD dwStatus;
	//DWORD last_play_pos = 0, bytesMoved = 0;
	//LPDIRECTSOUNDBUFFER8  lpdsbuf = ac->lpdsbuf;
//	LPDIRECTSOUND8        lpds = ac->lpds;

	lpdsbuff = ac->lpdsbuf;

	while (lpdsbuff == NULL)
		Sleep(10);
	DEBUG_OUTPUT("DS8: Audio Thread Started...\n");
	IDirectSoundBuffer_GetStatus(lpdsbuff, &dwStatus);
	if ((dwStatus & DSBSTATUS_PLAYING) == 0) {
		IDirectSoundBuffer_Play(lpdsbuff, 0, 0, 0);
	}

	SetThreadPriority(ac->handleAudioThread, THREAD_PRIORITY_HIGHEST);

	while (ac->audioIsDone == false) { // While the thread is still alive
		while (last_pos == write_pos) { // Cycle around until a new buffer position is available
			if (lpdsbuff == NULL)
				ExitThread(~0u);
			// Check to see if the audio pointer moved on to the next segment
			if (write_pos == last_pos) {
				if ((Configuration::getDisallowSleepDS8() == false) || (write_pos == 0))
					Sleep(1);
			}
			WaitForSingleObject(ac->hMutex, INFINITE);
			if FAILED(lpdsbuff->GetCurrentPosition((unsigned long*)&play_pos, NULL)) {
				MessageBox(NULL, "Error getting audio position...", PLUGIN_VERSION, MB_OK | MB_ICONSTOP);
				goto _exit_;
			}
			ReleaseMutex(ac->hMutex);
			// *** Cached method ***
			// Determine our write position by where our play position resides
			if (play_pos < LOCK_SIZE) write_pos = (LOCK_SIZE * DS_SEGMENTS) - LOCK_SIZE;
			else write_pos = ((play_pos / LOCK_SIZE) * LOCK_SIZE) - LOCK_SIZE;
			// *** JIT ***
			//write_pos = ((play_pos / LOCK_SIZE) * LOCK_SIZE) + (LOCK_SIZE*2);
			//if (write_pos >= TOTAL_SIZE) {
			//	write_pos -= TOTAL_SIZE;
			//}
			//if (play_pos >= (TOTAL_SIZE-LOCK_SIZE)) write_pos = LOCK_SIZE;
			//else write_pos = ((play_pos / LOCK_SIZE) * LOCK_SIZE) + LOCK_SIZE;

			//			if (write_pos == last_pos) {
			last_play = play_pos; // Store the last play position
			//				Sleep (1);
			//			}


#ifdef STREAM_DMA
#ifdef SEH_SUPPORTED
			__try // PJ64 likes to close objects before it shuts down the DLLs completely...
			{
#endif
			if (play_pos > last_play_pos) bytesMoved = play_pos - last_play_pos; else bytesMoved = TOTAL_SIZE - last_play_pos + play_pos;
			last_play_pos = play_pos;
			if (DMALen[0] != 0 && (*AudioInfo.AI_CONTROL_REG & 0x01) == 1)
			{
				//WaitForSingleObject(ac->hMutex, INFINITE);
				DWORD writeCnt = 0;
				DWORD writeOut = 0;
				// Write new data to play buffer from DMA
				if (DMALen[0] + DMALen[1] < LOCK_SIZE)
				{
					//*AudioInfo.AI_STATUS_REG &= ~0x80000001;
					//DEBUG_OUTPUT(";");
				}
				if (ac->remainingBytes < (MAXBUFFER - LOCK_SIZE))
				{
					//writeOut = bytesMoved;
					writeOut = LOCK_SIZE;
				}
				else
				{
					//if (DMALen[1] != 0)
					writeOut = 0;
				}
				writeOut &= ~3;
				/*
				else if (ac->remainingBytes >= LOCK_SIZE * 3)
				writeOut = 0;
				else
				writeOut = LOCK_SIZE/8;			*/
			//	DWORD SavedDMALen0 = DMALen[0];
			//	DWORD SavedDMALen1 = DMALen[1];
				while (writeCnt != writeOut && DMAData[0] > 0)
				{
					ac->SoundBuffer[ac->writeLoc++] = DMAData[0][2];
					ac->SoundBuffer[ac->writeLoc++] = DMAData[0][3];
					ac->SoundBuffer[ac->writeLoc++] = DMAData[0][0];
					ac->SoundBuffer[ac->writeLoc++] = DMAData[0][1];
					DMAData[0] += 4;
					DMALen[0] -= 4;
					writeCnt += 4;
					ac->remainingBytes += 4;
					if (ac->writeLoc == MAXBUFFER)
						ac->writeLoc = 0;
					if (DMALen[0] == 0)
					{ // Swap buffer and invoke an interrupt -- potential issue should the lock_size be greater than the buffer size
						DMALen[0] = DMALen[1]; DMAData[0] = DMAData[1];
						DMALen[1] = DMALen[2]; DMAData[1] = DMAData[2];
						DMALen[2] = 0; DMAData[2] = NULL;

						break;
					}
				}

				/*
				if (writeCnt != writeOut) DEBUG_OUTPUT("#");
				while (writeCnt != writeOut)
				{
				ac->SoundBuffer[ac->writeLoc++] = 0;
				ac->SoundBuffer[ac->writeLoc++] = 0;
				ac->SoundBuffer[ac->writeLoc++] = 0;
				ac->SoundBuffer[ac->writeLoc++] = 0;
				writeCnt += 4;
				ac->remainingBytes += 4;
				if (ac->writeLoc == MAXBUFFER)
				ac->writeLoc = 0;
				}*/
				if (ac->remainingBytes > MAXBUFFER)
					DEBUG_OUTPUT("M");
				//ReleaseMutex(ac->hMutex);
			}
			else
			{
				//if (last_pos == write_pos)	Sleep(1);
			}
#ifdef SEH_SUPPORTED
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}
#endif
#endif
		}
		// This means we had a buffer segment skipped skip
		if (next_pos != write_pos) {
			DEBUG_OUTPUT("A");
		}

		// Store our last position
		last_pos = write_pos;

		// Set out next anticipated segment
		next_pos = write_pos + LOCK_SIZE;
		if (next_pos >= (LOCK_SIZE*DS_SEGMENTS)) {
			next_pos -= (LOCK_SIZE*DS_SEGMENTS);
		}
		if (ac->audioIsDone == true) break;
		// Fill queue buffer here with LOCK_SIZE
		// TODO: Add buffer processing command here....
		// Time to write out to the buffer
		WaitForSingleObject(ac->hMutex, INFINITE);
		if (DS_OK != lpdsbuff->Lock(write_pos, LOCK_SIZE, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0)) {
			MessageBox(NULL, "Error locking sound buffer", PLUGIN_VERSION, MB_OK | MB_ICONSTOP);
			goto _exit_;
		}
		// Fills dwBytes to the Sound Buffer
		ac->LoadAiBuffer((BYTE *)lpvPtr1, dwBytes1);
		if (dwBytes2) { ac->LoadAiBuffer((BYTE *)lpvPtr2, dwBytes2); DEBUG_OUTPUT("P"); }
		//
		if FAILED(lpdsbuff->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2)) {
			MessageBox(NULL, "Error unlocking sound buffer", PLUGIN_VERSION, MB_OK | MB_ICONSTOP);
			goto _exit_;
		}
		ReleaseMutex(ac->hMutex);

		//Sleep(10);
	}

_exit_:
	DEBUG_OUTPUT("DS8: Audio Thread Terminated...\n");
	ReleaseMutex(ac->hMutex);
	//ac->handleAudioThread = NULL;
	ExitThread(0);
//	return 0;
}





//------------------------------------------------------------------------

// Setup and Teardown Functions

// Generates nice alignment with N64 samples...
void DirectSoundDriver::SetSegmentSize(DWORD length) {

	DSBUFFERDESC        dsbdesc;
	WAVEFORMATEX        wfm;
	HRESULT             hr;

	if (SampleRate == 0) { return; }
	SegmentSize = length;

	WaitForSingleObject(hMutex, INFINITE);
	memset(&wfm, 0, sizeof(WAVEFORMATEX));

	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = 2;
	wfm.nSamplesPerSec = SampleRate;
	wfm.wBitsPerSample = 16;
	wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	//dsbdesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
#if defined(_XBOX)
	dsbdesc.dwFlags	= DSBCAPS_CTRLPOSITIONNOTIFY;
#else
	dsbdesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE;
#endif
	dsbdesc.dwBufferBytes = SegmentSize * DS_SEGMENTS;
	dsbdesc.lpwfxFormat = &wfm;

	hr = IDirectSound_CreateSoundBuffer(lpds, &dsbdesc, &lpdsbuf, NULL);
	assert(!FAILED(hr));

	IDirectSoundBuffer_Play(lpdsbuf, 0, 0, DSBPLAY_LOOPING);
	lpdsbuff = this->lpdsbuf;
	ReleaseMutex(hMutex);
}

// TODO: Should clear out AI registers on romopen and initialize
BOOL DirectSoundDriver::Initialize() {
	//audioIsPlaying = FALSE;

	DSBUFFERDESC        dsPrimaryBuff;
	WAVEFORMATEX        wfm;
	HRESULT             hr;

	DeInitialize(); // Release just in case...
	SampleRate = 0; // -- Disabled due to reset bug 

	DEBUG_OUTPUT("DS8: Initialize()\n");
	hMutex = CreateMutex(NULL, FALSE, NULL);

	WaitForSingleObject(hMutex, INFINITE);

#if defined(_XBOX)
	hr = DirectSoundCreate(NULL, &lpds, NULL);
#else
	hr = DirectSoundCreate8(NULL, &lpds, NULL);
#endif
//	assert(!FAILED(hr)); // This happens if there is no sound device.
	if (FAILED(hr))
	{
		DEBUG_OUTPUT("DS8: Unable to DirectSoundCreate\n");
		return -2;
	}

	if (FAILED(hr = IDirectSound_SetCooperativeLevel(lpds, AudioInfo.hwnd, DSSCL_PRIORITY))) {
		DEBUG_OUTPUT("DS8: Failed to SetCooperativeLevel\n");
		return -1;
	}

	if (lpdsbuf) {
		IDirectSoundBuffer_Release(lpdsbuf);
		lpdsbuf = NULL;
	}
	memset(&dsPrimaryBuff, 0, sizeof(DSBUFFERDESC));

	dsPrimaryBuff.dwSize = sizeof(DSBUFFERDESC);
#if defined(_XBOX)
	dsPrimaryBuff.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY;
#else
	dsPrimaryBuff.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
#endif
	dsPrimaryBuff.dwBufferBytes = 0;
	dsPrimaryBuff.lpwfxFormat = NULL;
	memset(&wfm, 0, sizeof(WAVEFORMATEX));

	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = 2;
	wfm.nSamplesPerSec = 44100;
	wfm.wBitsPerSample = 16;
	wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

	hr = IDirectSound_CreateSoundBuffer(lpds, &dsPrimaryBuff, &lpdsb, NULL);

	if (SUCCEEDED(hr)) {
		IDirectSoundBuffer_SetFormat(lpdsb, &wfm);
		IDirectSoundBuffer_Play(lpdsb, 0, 0, DSBPLAY_LOOPING);
	}

	ReleaseMutex(hMutex);

	if (SampleRate > 0)
		SetFrequency(SampleRate);
//	SetSegmentSize(LOCK_SIZE);

	DEBUG_OUTPUT("DS8: Init Success...\n");

	DMALen[0] = DMALen[1] = 0;
	DMAData[0] = DMAData[1] = NULL;
	return FALSE;
}

void DirectSoundDriver::DeInitialize() {

	DEBUG_OUTPUT("DS8: DeInitialize()\n");
	audioIsDone = true;
	StopAudio();
	if (lpdsbuf) {
		IDirectSoundBuffer_Stop(lpdsbuf);
		IDirectSoundBuffer_Release(lpdsbuf);
		lpdsbuf = NULL;
	}
	if (lpds) {
		IDirectSound_Release(lpds);
		lpds = NULL;
	}
	if (hMutex) {
		CloseHandle(hMutex);
		hMutex = NULL;
	}

	lpdsbuf = NULL; lpds = NULL; audioIsDone = false; hMutex = NULL; audioIsPlaying = FALSE; readLoc = writeLoc = remainingBytes = 0;
	DMALen[0] = DMALen[0] = 0;
	DMAData[0] = DMAData[0] = NULL;
	DEBUG_OUTPUT("DS8: DeInitialize() complete\n");
}

// ---------BLAH--------

// Buffer Functions for the Audio Code
void DirectSoundDriver::SetFrequency(u32 Frequency2) {

	DWORD Frequency = Frequency2;

	printf("DS8: SetFrequency()\n");
	// MusyX - (Frequency / 80) * 4
	// 
	StopAudio();

	sLOCK_SIZE = (u32)((Frequency / Configuration::getBackendFPS())) * 4; //(Frequency / 80) * 4;// 0x600;// (22050 / 30) * 4;// 0x4000;// (Frequency / 60) * 4;
	SampleRate = Frequency;
	SegmentSize = 0; // Trash it... we need to redo the Frequency anyway...
	SetSegmentSize(LOCK_SIZE);
	DEBUG_OUTPUT("DS8: Frequency: %i - SegmentSize: %i\n", Frequency, SegmentSize);
	lastLength = 0;
	writeLoc = 0x0000;
	readLoc = 0x0000;
	remainingBytes = 0;
	//DMAData[0] = DMAData[1] = NULL;
	//DMALen[0] = DMALen[1] = NULL;
	StartAudio();
	DEBUG_OUTPUT("DS8: SetFrequency() Complete\n");
}

void DirectSoundDriver::AiUpdate(BOOL Wait) {

	if (Wait)
		WaitMessage();
}


// Management functions
// TODO: For silent emulation... the Audio should still be "processed" somehow...
void DirectSoundDriver::StopAudio() {
	if (!audioIsPlaying) return;
	DEBUG_OUTPUT("DS8: StopAudio()\n");
	//test.EndWaveOut();
	if (lpdsbuf != NULL)
	{
		lpdsbuf->Stop();
	}
	audioIsPlaying = FALSE;
	audioIsDone = true;
	Sleep(100);
	TerminateThread(this->handleAudioThread, 0);
	this->handleAudioThread = NULL;
	DEBUG_OUTPUT("DS8: StopAudio() complete\n");
}

void DirectSoundDriver::StartAudio() {
	if (audioIsPlaying) return;
	DEBUG_OUTPUT("DS8: StartAudio()\n");
	audioIsPlaying = TRUE;
	audioIsDone = false;
	writeLoc = 0x0000;
	readLoc = 0x0000;
	remainingBytes = 0;
	if (this->handleAudioThread != NULL)
	{
		DEBUG_OUTPUT("Audiothread != NULL");
		assert(0);
	}
	else
	{
		this->handleAudioThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)AudioThreadProc, this, NULL, &this->dwAudioThreadId);
	}
	//test.BeginWaveOut("D:\\test.wav", 2, 16, SampleRate);
	if (lpdsbuf != NULL)
	{
		IDirectSoundBuffer_Play(lpdsbuf, 0, 0, DSBPLAY_LOOPING);
	}
	DEBUG_OUTPUT("DS8: StartAudio() Complete\n");
}

void DirectSoundDriver::SetVolume(u32 volume) {
	DWORD dsVolume = ((DWORD)volume * -25);
	if (volume == 100) dsVolume = (DWORD)DSBVOLUME_MIN;
	if (volume == 0) dsVolume = DSBVOLUME_MAX;
	if (lpdsb != NULL) lpdsb->SetVolume(dsVolume);
}


#endif
