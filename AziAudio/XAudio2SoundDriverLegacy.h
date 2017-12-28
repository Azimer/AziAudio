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
#endif

#include "SoundDriverLegacy.h"
#include <xaudio2.h>

class VoiceCallbackLegacy : public IXAudio2VoiceCallback
{
public:
	//HANDLE hBufferEndEvent;
	VoiceCallbackLegacy() /*: hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL))*/{}
	~VoiceCallbackLegacy(){/* CloseHandle(hBufferEndEvent); */}

	//Called when the voice has just finished playing a contiguous audio stream.
	void __stdcall OnStreamEnd() {/* SetEvent(hBufferEndEvent); */}

	//Unused methods are stubs
	
	void __stdcall OnVoiceProcessingPassEnd() { }
	void __stdcall OnVoiceProcessingPassStart(UINT32 SamplesRequired);// {}
	void __stdcall OnBufferEnd(void * pBufferContext);//    {}
	void __stdcall OnBufferStart(void * pBufferContext) { UNREFERENCED_PARAMETER(pBufferContext); }
	void __stdcall OnLoopEnd(void * pBufferContext) { UNREFERENCED_PARAMETER(pBufferContext); }
	void __stdcall OnVoiceError(void * pBufferContext, HRESULT Error) { UNREFERENCED_PARAMETER(pBufferContext); UNREFERENCED_PARAMETER(Error); }
	
	/*
	STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired);
	STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS);
	STDMETHOD_(void, OnStreamEnd) (THIS);
	STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext);
	STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext);
	STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext);
	STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error);
	*/
};

class XAudio2SoundDriverLegacy :
	public SoundDriverLegacy
{	
public:
	XAudio2SoundDriverLegacy();
	~XAudio2SoundDriverLegacy();
	
	// Setup and Teardown Functions
	BOOL Initialize();
	void DeInitialize();

	BOOL Setup();
	void Teardown();

	// Buffer Functions for the Audio Code
	void SetFrequency(u32 Frequency);           // Sets the Nintendo64 Game Audio Frequency
	u32 AddBuffer(u8 *start, u32 length);       // Uploads a new buffer and returns status

	// Management functions
	void AiUpdate(BOOL Wait);
	void StopAudio();							// Stops the Audio PlayBack (as if paused)
	void StartAudio();							// Starts the Audio PlayBack (as if unpaused)
	u32 GetReadStatus();						// Returns the status on the read pointer

	void SetVolume(u32 volume);

	//void PlayBuffer(int bufferNumber, u8* bufferData, int bufferSize);

	static SoundDriverInterface* CreateSoundDriver() { return new XAudio2SoundDriverLegacy(); }

protected:

	bool dllInitialized;
	static DWORD WINAPI AudioThreadProc(LPVOID lpParameter);

private:
	HANDLE hAudioThread;
	bool   bStopAudioThread;
};

/*
 * The GNU C++ compiler (ported to Windows through MinGW, for example)
 * references system C++ headers that use `__in` and `__out` for things
 * related to C++, which conflicts with Microsoft's <sal.h> driver macros for
 * the XAudio2 API.  Perhaps either side could be blamed for this, but I
 * think that it shouldn't hurt to un-define the __in and __out stuff after
 * we have finished prototyping everything relevant to XAudio2.  -- cxd4
 */
#if !defined(_MSC_VER)
#undef __in
#undef __out
#endif
