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
//#include "rsp.h"
//#define SAFE_MEMORY
/*
#ifndef SAFE_MEMORY
#	define wr8 (src , address);
#	define rd8 (dest, address);
#	define wr16 (src, address);
#	define rd16 (dest, address);
#	define wr32 (src, address);
#	define rd32 (dest, address);
#	define wr64 (src, address);
#	define rd64 (dest, address);
#	define dmamem (dest, src, size) memcpy (dest, src, size);
#	define clrmem (dest, size)		memset (dest, 0, size);
#else
	void wr8 (u8 src, void *address);
	void rd8 (u8 dest, void *address);
	void wr16 (u16 src, void *address);
	void rd16 (u16 dest, void *address);
	void wr32 (u16 src, void *address);
	void rd32 (u16 dest, void *address);
	void wr64 (u16 src, void *address);
	void rd64 (u16 dest, void *address);
	void dmamem (void *dest, void *src, int size);
	void clrmem (void *dest, int size);
#endif
*/
/******** DMEM Memory Map for ABI 1 ***************
Address/Range		Description
-------------		-------------------------------
0x000..0x2BF		UCodeData
	0x000-0x00F		Constants  - 0000 0001 0002 FFFF 0020 0800 7FFF 4000
	0x010-0x02F		Function Jump Table (16 Functions * 2 bytes each = 32) 0x20
	0x030-0x03F		Constants  - F000 0F00 00F0 000F 0001 0010 0100 1000
	0x040-0x03F		Used by the Envelope Mixer (But what for?)
	0x070-0x07F		Used by the Envelope Mixer (But what for?)
0x2C0..0x31F		<Unknown>
0x320..0x35F		Segments
0x360				Audio In Buffer (Location)
0x362				Audio Out Buffer (Location)
0x364				Audio Buffer Size (Location)
0x366				Initial Volume for Left Channel
0x368				Initial Volume for Right Channel
0x36A				Auxillary Buffer #1 (Location)
0x36C				Auxillary Buffer #2 (Location)
0x36E				Auxillary Buffer #3 (Location)
0x370				Loop Value (shared location)
0x370				Target Volume (Left)
0x372				Ramp?? (Left)
0x374				Rate?? (Left)
0x376				Target Volume (Right)
0x378				Ramp?? (Right)
0x37A				Rate?? (Right)
0x37C				Dry??
0x37E				Wet??
0x380..0x4BF		Alist data
0x4C0..0x4FF		ADPCM CodeBook
0x500..0x5BF		<Unknown>
0x5C0..0xF7F		Buffers...
0xF80..0xFFF		<Unknown>
***************************************************/



// T8 = 0x360

p_func ABI1[NUM_ABI_COMMANDS] = {
    SPNOOP    ,ADPCM     ,CLEARBUFF ,ENVMIXER  ,LOADBUFF  ,RESAMPLE  ,SAVEBUFF  ,UNKNOWN   ,
    SETBUFF   ,SETVOL    ,DMEMMOVE  ,LOADADPCM ,MIXER     ,INTERLEAVE,POLEF     ,SETLOOP   ,
    SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,
    SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,
};

p_func ABI1GE[NUM_ABI_COMMANDS] = {
    SPNOOP    ,ADPCM     ,CLEARBUFF,ENVMIXER_GE,LOADBUFF  ,RESAMPLE  ,SAVEBUFF  ,UNKNOWN   ,
    SETBUFF   ,SETVOL    ,DMEMMOVE  ,LOADADPCM ,MIXER     ,INTERLEAVE,POLEF     ,SETLOOP   ,
    SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,
    SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,SPNOOP    ,
};
