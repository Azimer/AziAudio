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

#include "audiohle.h"

/*
typedef struct {
	BYTE sync;

	BYTE error_protection	: 1;	//  0=yes, 1=no
	BYTE lay				: 2;	// 4-lay = layerI, II or III
	BYTE version			: 1;	// 3=mpeg 1.0, 2=mpeg 2.5 0=mpeg 2.0
	BYTE sync2				: 4;

	BYTE extension			: 1;    // Unknown
	BYTE padding			: 1;    // padding
	BYTE sampling_freq		: 2;	// see table below
	BYTE bitrate_index		: 4;	//     see table below

	BYTE emphasis			: 2;	//see table below
	BYTE original			: 1;	// 0=no 1=yes
	BYTE copyright			: 1;	// 0=no 1=yes
	BYTE mode_ext			: 2;    // used with "joint stereo" mode
	BYTE mode				: 2;    // Channel Mode
} mp3struct;

mp3struct mp3;
FILE *mp3dat;
*/

//static FILE *fp = fopen ("d:\\mp3info.txt", "wt");

/*
 {
//	return;
	// Setup Registers...
	mp3setup (k0, t9, 0xFA0);
	
	// Setup Memory Locations...
	//u32 base = ((u32*)dmem)[0xFD0/4]; // Should be 000291A0
	memcpy (BufferSpace, dmembase+rdram, 0x10);
	((u32*)BufferSpace)[0x0] = base;
	((u32*)BufferSpace)[0x008/4] += base;
	((u32*)BufferSpace)[0xFFC/4] = loopval;
	((u32*)BufferSpace)[0xFF8/4] = dmembase;
	//assert(0);
	memcpy (imem+0x238, rdram+((u32*)BufferSpace)[0x008/4], 0x9C0);
	((u32*)BufferSpace)[0xFF4/4] = setaddr;
	pDMEM = (char *)BufferSpace;
	rsp_run ();
	dmembase = ((u32*)BufferSpace)[0xFF8/4];
	loopval  = ((u32*)BufferSpace)[0xFFC/4];
//0x1A98  SW       S1, 0x0FF4 (R0)
//0x1A9C  SW       S0, 0x0FF8 (R0)
//0x1AA0  SW       T7, 0x0FFC (R0)
//0x1AA4  SW       T3, 0x0FF0 (R0)
	//fprintf (fp, "mp3: k0: %08X, t9: %08X\n", k0, t9);
}*/
/*
FFT = Fast Fourier Transform
DCT = Discrete Cosine Transform
MPEG-1 Layer 3 retains Layer 2’s 1152-sample window, as well as the FFT polyphase filter for 
backward compatibility, but adds a modified DCT filter. DCT’s advantages over DFTs (discrete 
Fourier transforms) include half as many multiply-accumulate operations and half the 
generated coefficients because the sinusoidal portion of the calculation is absent, and DCT 
generally involves simpler math. The finite lengths of a conventional DCTs’ bandpass impulse 
responses, however, may result in block-boundary effects. MDCTs overlap the analysis blocks 
and lowpass-filter the decoded audio to remove aliases, eliminating these effects. MDCTs also 
have a higher transform coding gain than the standard DCT, and their basic functions 
correspond to better bandpass response. 

MPEG-1 Layer 3’s DCT sub-bands are unequally sized, and correspond to the human auditory 
system’s critical bands. In Layer 3 decoders must support both constant- and variable-bit-rate 
bit streams. (However, many Layer 1 and 2 decoders also handle variable bit rates). Finally, 
Layer 3 encoders Huffman-code the quantized coefficients before archiving or transmission for 
additional lossless compression. Bit streams range from 32 to 320 kbps, and 128-kbps rates 
achieve near-CD quality, an important specification to enable dual-channel ISDN 
(integrated-services-digital-network) to be the future high-bandwidth pipe to the home. 

*/

// Disables the command because it's not used?
static void DISABLE () {}
static void WHATISTHIS () {}

p_func ABI3[NUM_ABI_COMMANDS] = {
    DISABLE   ,ADPCM3    ,CLEARBUFF3,ENVMIXER3 ,LOADBUFF3 ,RESAMPLE3 ,SAVEBUFF3 ,MP3       ,
    MP3ADDY   ,SETVOL3   ,DMEMMOVE3 ,LOADADPCM3,MIXER3   ,INTERLEAVE3,WHATISTHIS,SETLOOP3  ,
    SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,
    SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,
};
