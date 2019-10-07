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

#include "SoundDriver.h"
#include "WaveOut.h"

// Load the buffer from the AI interface to our emulated buffer
void SoundDriver::AI_LenChanged(u8 *start, u32 length)
{
	if (length == 0)
	{
		*AudioInfo.AI_STATUS_REG = 0;
		*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
		AudioInfo.CheckInterrupts();
		return;
	}
	//*********************** Buffer Overflow *****************
#if 1
	// Bleed off some of this buffer to smooth out audio
	if ((length < m_MaxBufferSize) && (Configuration::getSyncAudio() == true) /*&& m_AI_DMAPrimaryBytes > 0*/)
	{
		if (m_MaxBufferSize == m_BufferRemaining)
		{
			DEBUG_OUTPUT("B"); // Debug that we overflowed (shouldn't happen with locked FPS)
		}
		while (m_MaxBufferSize == m_BufferRemaining)
		{
#ifdef _WIN32
			Sleep(1);
#else
			// TODO: We need to support non-windows in another way
			assert(0);
#endif
		}
	}
#endif
	//*********************** Buffer Overflow *****************
	lastLength = length;

#ifdef _WIN32
	WaitForSingleObject(m_hMutex, INFINITE);
#else
	puts("[AI_LenChanged] To do:  Working non-Win32 mutex timing.");
#endif
	BufferAudio();
#if 1
	if (m_AI_DMASecondaryBytes > 0)
	{
		DEBUG_OUTPUT("X");
		s32 tmp = m_AI_DMASecondaryBytes - m_AI_DMAPrimaryBytes;
		if (tmp > 0)
		{
			m_AI_DMAPrimaryBuffer = m_AI_DMASecondaryBuffer + tmp; m_AI_DMASecondaryBuffer = NULL;
			m_AI_DMASecondaryBytes = 0;
		}
		else
		{
			m_AI_DMAPrimaryBuffer = m_AI_DMASecondaryBuffer; m_AI_DMASecondaryBuffer = NULL;
			m_AI_DMAPrimaryBytes = m_AI_DMASecondaryBytes; m_AI_DMASecondaryBytes = 0;
		}

	}
#endif
	m_AI_DMASecondaryBuffer = start;
	m_AI_DMASecondaryBytes = length;
	if (m_AI_DMAPrimaryBytes == 0)
	{
		m_AI_DMAPrimaryBuffer = m_AI_DMASecondaryBuffer; m_AI_DMASecondaryBuffer = NULL;
		m_AI_DMAPrimaryBytes = m_AI_DMASecondaryBytes; m_AI_DMASecondaryBytes = 0; 
	}

	if (Configuration::getAIEmulation() == true)
	{
		*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
		if (m_AI_DMAPrimaryBytes > 0 && m_AI_DMASecondaryBytes > 0)
		{
			*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY | AI_STATUS_FIFO_FULL;
		}
	}
	BufferAudio();

#ifdef _WIN32
	ReleaseMutex(m_hMutex);
#endif
#if 1
	// Bleed off some of this buffer to smooth out audio
	if ((length < m_MaxBufferSize) && (Configuration::getSyncAudio() == true) && m_AI_DMASecondaryBytes > 0)
	{
		if (m_MaxBufferSize == m_BufferRemaining)
		{
			DEBUG_OUTPUT("O"); // Debug that we overflowed (shouldn't happen with locked FPS)
		}
		while (m_MaxBufferSize == m_BufferRemaining)
		{
#ifdef _WIN32
			Sleep(1);
#else
			// TODO: We need to support non-windows in another way
			assert(0);
#endif
		}
	}
#endif
}
//WaveOut test;
void SoundDriver::AI_SetFrequency(u32 Frequency)
{
	m_SamplesPerSecond = Frequency;
	SetFrequency(Frequency);
	m_MaxBufferSize = (u32)((Frequency / Configuration::getBufferFPS())) * 4 * Configuration::getBufferLevel();
	m_BufferRemaining = 0;
	m_CurrentReadLoc = m_CurrentWriteLoc = m_BufferRemaining = 0;
	//test.EndWaveOut();
	//test.BeginWaveOut("D:\\test.wav", 2, 16, m_SamplesPerSecond);
}

u32 SoundDriver::AI_ReadLength()
{
	const u32 LENGTHFACTOR = 32;
	u32 retVal;

	if (Configuration::getAIEmulation() == false || lastLength == 0)
		return 0;
#ifdef _WIN32
	WaitForSingleObject(m_hMutex, INFINITE);
#endif
	if (m_BufferRemaining > (lastLength*2 + lastLength/4))
	{
		retVal = lastLength;  // TODO: I do not remember why I set this...
	}
	else
	{
		retVal = m_AI_DMAPrimaryBytes;
	}
	
	if (retVal == lastReadLength)
	{
		if (retVal > (LENGTHFACTOR * (lastReadCount+1)))
		{
			lastReadCount++;
			retVal -= LENGTHFACTOR * lastReadCount;
		}
	}
	else
	{
		lastReadCount = 0;
		lastReadLength = retVal;
	}
#ifdef _WIN32
	ReleaseMutex(m_hMutex);
#endif
	return (retVal & ~0x7);
}

void SoundDriver::AI_Startup()
{
	Initialize();
	m_AI_DMAPrimaryBytes = m_AI_DMASecondaryBytes = 0;
	m_AI_DMAPrimaryBuffer = m_AI_DMASecondaryBuffer = NULL;

	m_MaxBufferSize = MAX_SIZE;
	m_CurrentReadLoc = m_CurrentWriteLoc = m_BufferRemaining = 0;
	m_DMAEnabled = false;
	lastReadLength = 0;
	lastReadCount = 0;
	lastLength = 0;

#ifdef _WIN32
	if (m_hMutex == NULL)
	{
		m_hMutex = CreateMutex(NULL, FALSE, NULL);
	}
#else
	// to do
#endif
	StartAudio();
	SetVolume(Configuration::getVolume());
}

void SoundDriver::AI_Shutdown()
{
	StopAudio();
	DeInitialize();
	m_BufferRemaining = 0;
#ifdef _WIN32
	if (m_hMutex != NULL)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
#else
	// to do
#endif
	//test.EndWaveOut();
}

void SoundDriver::AI_ResetAudio()
{
	m_BufferRemaining = 0;
	if (m_audioIsInitialized == true) AI_Shutdown();
	DeInitialize();
	m_audioIsInitialized = false;
	AI_Startup();
	m_audioIsInitialized = (Initialize() == FALSE);
	StartAudio();
}

void SoundDriver::AI_Update(Boolean Wait)
{
#ifdef _WIN32
	if (Wait) Sleep(10); // TODO:  Fixme -- Ai Update appears to be problematic
#endif
	AiUpdate(Wait);
}

void SoundDriver::BufferAudio()
{
	m_DMAEnabled = (*AudioInfo.AI_CONTROL_REG & AI_CONTROL_DMA_ON) == AI_CONTROL_DMA_ON;
	if (m_DMAEnabled == false)
		return;
	while ((m_BufferRemaining < m_MaxBufferSize) && (m_AI_DMAPrimaryBytes > 0 || m_AI_DMASecondaryBytes > 0))
	{
		*(u16 *)(m_Buffer + m_CurrentWriteLoc) = *(u16 *)(m_AI_DMAPrimaryBuffer+2);
		*(u16 *)(m_Buffer + m_CurrentWriteLoc+2) = *(u16 *)m_AI_DMAPrimaryBuffer;
		m_CurrentWriteLoc += 4;
		m_AI_DMAPrimaryBuffer += 4;
		m_CurrentWriteLoc %= m_MaxBufferSize;
		assert(m_BufferRemaining <= m_MaxBufferSize);
		m_BufferRemaining += 4;
		assert(m_BufferRemaining <= m_MaxBufferSize);
		m_AI_DMAPrimaryBytes -= 4;
		if (m_AI_DMAPrimaryBytes == 0)
		{
			m_AI_DMAPrimaryBytes = m_AI_DMASecondaryBytes; m_AI_DMAPrimaryBuffer = m_AI_DMASecondaryBuffer; // Switch
			m_AI_DMASecondaryBytes = 0; m_AI_DMASecondaryBuffer = NULL;
			if (Configuration::getAIEmulation() == true)
			{
				lastReadLength = 0;
				lastReadCount = 0;
				*AudioInfo.AI_STATUS_REG = AI_STATUS_DMA_BUSY;
				*AudioInfo.AI_STATUS_REG &= ~AI_STATUS_FIFO_FULL;
				*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
				AudioInfo.CheckInterrupts();
				if (m_AI_DMAPrimaryBytes == 0) *AudioInfo.AI_STATUS_REG = 0;
			}
		}
	}
}

// Copies data to the audio playback buffer
u32 SoundDriver::LoadAiBuffer(u8 *start, u32 length)
{
	u32 bytesToMove = length;// &0xFFFFFFFC;
	u8 *ptrStart = start;
	u8 nullBuff[MAX_SIZE];
	u32 writePtr = 0;
	static u32 lastSample = 0;
	if (start == NULL)
		ptrStart = nullBuff;

	assert((length & 0x3) == 0);
	assert(bytesToMove <= m_MaxBufferSize);  // We shouldn't be asking for more.

	m_DMAEnabled = (*AudioInfo.AI_CONTROL_REG & AI_CONTROL_DMA_ON) == AI_CONTROL_DMA_ON;

	if (bytesToMove > m_MaxBufferSize)
	{
		memset(ptrStart, 0, length);
		return length;
	}

	// Return silence -- DMA is disabled
	if (m_DMAEnabled == false)
	{
		memset(ptrStart, 0, length);
		return length;
	}

#ifdef _WIN32
	WaitForSingleObject(m_hMutex, INFINITE);
#else
	puts("[LoadAIBuffer] To do:  non-Win32 m_hMutex");
#endif

	// Step 0: Replace depleted stored buffer for next run
	BufferAudio();

	// Step 1: Deplete stored buffer (should equal length size)
	if (bytesToMove <= m_BufferRemaining)
	{
		while (bytesToMove > 0 && m_BufferRemaining > 0)
		{
			lastSample = *(u32 *)(ptrStart + writePtr) = *(u32 *)(m_Buffer + m_CurrentReadLoc);
			m_CurrentReadLoc += 4;
			writePtr += 4;
			m_CurrentReadLoc %= m_MaxBufferSize;
			assert(m_BufferRemaining <= m_MaxBufferSize);
			m_BufferRemaining -= 4;
			assert(m_BufferRemaining <= m_MaxBufferSize);
			bytesToMove -= 4;
		}
		assert(writePtr == length);
	}

	// Step 2: Fill bytesToMove with silence
	if (bytesToMove == length)
		DEBUG_OUTPUT("S");

	while (bytesToMove > 0)
	{
		*(u32 *)(ptrStart + writePtr) = lastSample;
		writePtr += 4;
		bytesToMove -= 4;
	}

	// Step 3: Replace depleted stored buffer for next run
	BufferAudio();	
	//test.WriteData(ptrStart, length);

#ifdef _WIN32
	ReleaseMutex(m_hMutex);
#else
	// to do
#endif
	assert(bytesToMove == 0);
	return (length - bytesToMove);
}
