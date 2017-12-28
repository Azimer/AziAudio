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

#include <memory.h>
#include <stdio.h>
#include "WaveOut.h"	

WaveOut::WaveOut() : waveoutput(NULL) {
	waveoutput = NULL;
};
void WaveOut::BeginWaveOut(char *filename, WORD channels, WORD bitsPerSample, DWORD sampleRate)
{
	if (waveoutput != NULL) return;
	// Clear header
	memset(&header, 0, sizeof(WaveHeader));
	header.AudioFormat = 1;
	header.BitsPerSample = bitsPerSample;
	header.NumChannels = channels;
	header.SampleRate = sampleRate;
	header.BlockAlign = header.NumChannels * (header.BitsPerSample/8);
	header.ByteRate = header.SampleRate * header.BlockAlign;
	memcpy (header.ChunkID, "RIFF", 4);
	header.ChunkSize=0; // TODO at EndWaveOut
	memcpy (header.format, "WAVE", 4);
	memcpy (header.Subchunk1ID, "fmt ", 4);
	header.Subchunk1Size = 16;
	memcpy(header.Subchunk2ID,"data",4);
	header.Subchunk2Size=0; // TODO at EndWaveOut

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
	fopen_s(&waveoutput, filename, "wb");
#else
	waveoutput = fopen(filename, "wb");
#endif
	fwrite(&header, sizeof(WaveHeader), 1, waveoutput);
	datasize = 0;
}

void WaveOut::WriteData (unsigned char*data, DWORD size)
{
	if (waveoutput == NULL) return;
	datasize += size;
	fwrite(data, size, 1, waveoutput);
}

void WaveOut::EndWaveOut()
{
	if (waveoutput == NULL) return;
	header.ChunkSize = ftell(waveoutput)-8;
	header.Subchunk2Size = datasize;
	fseek(waveoutput, 0, SEEK_SET);
	fwrite(&header, sizeof(WaveHeader), 1, waveoutput);
	fclose(waveoutput);
	waveoutput = NULL;
}
