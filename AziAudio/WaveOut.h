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
#include <Windows.h>

typedef struct 
{
	// RIFF Header
	char ChunkID[4]; // "RIFF"
	DWORD ChunkSize; // FileSize - 8
	char format[4]; // "WAVE"
	// fmt Header
	char Subchunk1ID[4]; // "fmt "
	DWORD Subchunk1Size; // 16 for PCM
	WORD AudioFormat; // PCM=1
	WORD NumChannels; // Mono=1, Stereo=2
	DWORD SampleRate; // 8000, 44100, etc.
	DWORD ByteRate; // SampleRate*NumChannels*BitsPerSample/8
	WORD BlockAlign; // NumChannels * BitsPerSample/8
	WORD BitsPerSample; // 8bits = 8, 16 bits = 16
	// Data Header
	char Subchunk2ID[4]; // "data"
	DWORD Subchunk2Size; // NumSamples * NumChannels * BitsPerSample/8

	// Everything after is Data

} WaveHeader;

class WaveOut
{
protected:
	WaveHeader header;
	FILE *waveoutput;
	DWORD datasize;
public:
	WaveOut();
	void BeginWaveOut(char *filename, WORD channels, WORD bitsPerSample, DWORD sampleRate);
	void EndWaveOut();
	void WriteData (unsigned char*data, DWORD size);
};
