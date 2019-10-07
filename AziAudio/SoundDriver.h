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

#if defined(_WIN32)
#include <windows.h>
#else
#include <SDL/SDL.h>
#include <pthread.h>
#include <unistd.h>
#endif

/* strcpy() */
#include <string.h>

#include "common.h"
#include "AudioSpec.h"
#include "SoundDriverInterface.h"

#define SND_IS_NOT_EMPTY 0x4000000
#define SND_IS_FULL		 0x8000000

class SoundDriver :
	public SoundDriverInterface
{
public:

	// Buffer Management methods
	u32 LoadAiBuffer(u8 *start, u32 length); // Reads in length amount of audio bytes
	void BufferAudio();

	// Sound Driver Factory method
	static SoundDriver* SoundDriverFactory();

	virtual void SetVolume(u32 volume) { UNREFERENCED_PARAMETER(volume); }; // We could potentially do this ourselves within the buffer copy method
	virtual ~SoundDriver() {};

	void AI_SetFrequency(u32 Frequency);
	void AI_LenChanged(u8 *start, u32 length);
	u32 AI_ReadLength();
	void AI_Startup();
	void AI_Shutdown();
	void AI_ResetAudio();
	void AI_Update(Boolean Wait);

protected:
	// Temporary (to allow for incremental development)
	bool m_audioIsInitialized;
	bool m_isValid;

	// Mutex Handle
#ifdef _WIN32
	HANDLE m_hMutex;
#else
	pthread_mutex_t m_Mutex;
#endif

	// Variables for AI DMA emulation
	//int m_AI_CurrentDMABuffer; // Currently playing AI Buffer
	//int m_AI_WriteDMABuffer;   // Which set of registers will be written to
	u8 *m_AI_DMAPrimaryBuffer, *m_AI_DMASecondaryBuffer;
	u32 m_AI_DMAPrimaryBytes, m_AI_DMASecondaryBytes;

	// Variables for Buffering audio samples from AI DMA
	static const int MAX_SIZE = 44100 * 2 * 2; // Max Buffer Size (44100Hz * 16bit * Stereo)
	//static const int NUM_BUFFERS = 4; // Number of emulated buffers
	u32 m_MaxBufferSize;   // Variable size determined by Playback rate
	u32 m_CurrentReadLoc;   // Currently playing Buffer
	u32 m_CurrentWriteLoc;  // Currently writing Buffer
	u8 m_Buffer[MAX_SIZE]; // Emulated buffers
	u32 m_BufferRemaining; // Buffer remaining
	bool m_DMAEnabled;  // Sets to true when DMA is enabled
	u32 m_SamplesPerSecond;

	// Needed to smooth out the ReadLength
	u32 lastReadLength;
	u32 lastReadCount;
	u32 lastLength;

	SoundDriver(){
		m_audioIsInitialized = false;
#ifdef _WIN32
		m_hMutex = NULL;
#else
		m_Mutex = NULL;
#endif
	}
};
