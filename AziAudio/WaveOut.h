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

#pragma once
#include <cstdio>
#if defined(_WIN32)
#include <windows.h>
#endif

typedef struct 
{
	// RIFF Header
	char ChunkID[4]; // "RIFF"
	unsigned long ChunkSize; // FileSize - 8
	char format[4]; // "WAVE"
	// fmt Header
	char Subchunk1ID[4]; // "fmt "
	unsigned long Subchunk1Size; // 16 for PCM
	unsigned short AudioFormat; // PCM=1
	unsigned short NumChannels; // Mono=1, Stereo=2
	unsigned long SampleRate; // 8000, 44100, etc.
	unsigned long ByteRate; // SampleRate*NumChannels*BitsPerSample/8
	unsigned short BlockAlign; // NumChannels * BitsPerSample/8
	unsigned short BitsPerSample; // 8bits = 8, 16 bits = 16
	// Data Header
	char Subchunk2ID[4]; // "data"
	unsigned long Subchunk2Size; // NumSamples * NumChannels * BitsPerSample/8

	// Everything after is Data

} WaveHeader;

class WaveOut
{
protected:
	WaveHeader header;
	FILE *waveoutput;
	unsigned long datasize;
public:
	WaveOut();
	void BeginWaveOut(char *filename, unsigned short channels, unsigned short bitsPerSample, unsigned long sampleRate);
	void EndWaveOut();
	void WriteData (unsigned char*data, unsigned long size);
};
