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

/* memset() and memcpy() */
#include <string.h>

#include "audiohle.h"
//#include "RSP/rsp.h"

// For pseudo-HLE code... :)
//u32 r0, at, v0, v1, a0, a1, a2, a3;
//u32 t0, t1, t2, t3, t4, t5, t6, t7;
//u32 s0, s1, s2, s3, s4, s5, s6, s7;
//u32 t8, t9, k0, k1, gp, sp, s8, ra;
u32 t9, k0;

u64 ProfileStartTimes[30];
u64 ProfileTimes[30];

u8 * DMEM;
u8 * IMEM;
u8 * DRAM;

// Variables needed for ABI HLE
u8 BufferSpace[0x10000];
short hleMixerWorkArea[256];
u32 SEGMENTS[0x10];		// 0x0320
u16 AudioInBuffer;		// 0x0000(T8)
u16 AudioOutBuffer;		// 0x0002(T8)
u16 AudioCount;			// 0x0004(T8)
u16 AudioAuxA;			// 0x000A(T8)
u16 AudioAuxC;			// 0x000C(T8)
u16 AudioAuxE;			// 0x000E(T8)
u32 loopval;			// 0x0010(T8) // Value set by A_SETLOOP : Possible conflict with SETVOLUME???
bool isMKABI = false;
bool isZeldaABI = false;

s32 acc[32][N];
s16 acc_clamped[N];

// Audio UCode lists
//     Dummy UCode Handler for UCode detection... (Will always assume UCode1 until the nth list is executed)
extern p_func SafeABI[NUM_ABI_COMMANDS];
//---------------------------------------------------------------------------------------------
//
//     ABI 1 : Mario64, WaveRace USA, Golden Eye 007, Quest64, SF Rush
//				 60% of all games use this.  Distributed 3rd Party ABI
//
extern p_func ABI1[NUM_ABI_COMMANDS];
extern p_func ABI1GE[NUM_ABI_COMMANDS];
//---------------------------------------------------------------------------------------------
//
//     ABI 2 : WaveRace JAP, MarioKart 64, Mario64 JAP RumbleEdition, 
//				 Yoshi Story, Pokemon Games, Zelda64, Zelda MoM (miyamoto) 
//				 Most NCL or NOA games (Most commands)
extern p_func ABI2[NUM_ABI_COMMANDS];
//---------------------------------------------------------------------------------------------
//
//     ABI 3 : DK64, Perfect Dark, Banjo Kazooi, Banjo Tooie
//				 All RARE games except Golden Eye 007
//
extern p_func ABI3[NUM_ABI_COMMANDS];
//---------------------------------------------------------------------------------------------
//
//     ABI 5 : Factor 5 - MoSys/MusyX
//				 Rogue Squadron, Tarzan, Hydro Thunder, and TWINE
//				 Indiana Jones and Battle for Naboo (?)
//extern p_func ABI5[NUM_ABI_COMMANDS];
//---------------------------------------------------------------------------------------------
//
//     ABI ? : Unknown or unsupported UCode
//
extern p_func ABIUnknown[NUM_ABI_COMMANDS];
//---------------------------------------------------------------------------------------------

p_func ABI[NUM_ABI_COMMANDS];
bool locklistsize = false;

//---------------------------------------------------------------------------------------------

void SPU () {
}

void SPNOOP() {
#if 0//_DEBUG
	static char buff[] = "Unknown/Unimplemented Audio Command %i in ABI 3";
	char * sprintf_offset;
	const u8 command = (unsigned char)((k0 & 0xFF000000ul) >> 24);

	sprintf_offset = strchr(&buff[0], '%'); /* Overwrite "%i" with decimal. */
	*(sprintf_offset + 0) = '0' + (command / 10 % 10);
	*(sprintf_offset + 1) = '0' + (command / 1 % 10);
	if (sprintf_offset[0] == '0')
		sprintf_offset[0] = ' '; /* Leading 0's may confuse decimal w/ octal. */
	MessageBox(NULL, buff, PLUGIN_VERSION, MB_OK);
#endif
}

u32 UCData, UDataLen;

//#define ENABLELOG
#ifdef ENABLELOG
#pragma message ("Logging of Timing info is enabled!!!")
FILE *dfile = fopen ("d:\\HLEInfo.txt", "wt");
#endif

extern "C"
{
	// MusyX HLE provided by Mupen64Plus authored by Bobby Smiles
	void ProcessMusyX_v1();
	void ProcessMusyX_v2();
}

u32 base, dmembase;

void HLEStart() {
	u32 List = ((u32*)DMEM)[0xFF0 / 4], ListLen = ((u32*)DMEM)[0xFF4 / 4];
	u32 *HLEPtr = (u32 *)(DRAM + List);

	UCData = ((u32*)DMEM)[0xFD8 / 4];
	UDataLen = ((u32*)DMEM)[0xFDC / 4];
	base = ((u32*)DMEM)[0xFD0 / 4];
	dmembase = ((u32*)DMEM)[0xFD8 / 4];

	loopval = 0;
	memset(SEGMENTS, 0, 0x10 * 4);
	isMKABI = false;
	isZeldaABI = false;

	u8 * UData = DRAM + UCData;

	// Detect uCode
	if (((u32*)UData)[0] != 0x1) {
		switch (*(u32*)(UData + (0x10)))
		{
			case 0x00000001: // MusyX v1
				// RogueSquadron, ResidentEvil2, PolarisSnoCross,
				// TheWorldIsNotEnough, RugratsInParis, NBAShowTime,
				// HydroThunder, Tarzan, GauntletLegend, Rush2049
				ProcessMusyX_v1();
				return;
			default:
				return;

			case 0x0000127c:  break; // naudio (many games)
			case 0x00001280:  break; // BanjoKazooie
			case 0x1c58126c:  break; // DonkeyKong
			case 0x1ae8143c:  break; // BanjoTooie, JetForceGemini, MickeySpeedWayUSA, PerfectDark
			case 0x1ab0140c:  break; // ConkerBadFurDay
		}
		memcpy(ABI, ABI3, NUM_ABI_COMMANDS * sizeof(p_func));
	}
	else 
	{
		if (*(u32*)(UData + (0x30)) == 0xF0000F00) 
		{ // Should be common in ABI 1
			switch (*(u32*)(UData + (0x28)))
			{
				case 0x1e24138c:
					memcpy(ABI, ABI1, NUM_ABI_COMMANDS * sizeof(p_func));
					break;
				case 0x1dc8138c: // GoldenEye
					memcpy(ABI, ABI1GE, NUM_ABI_COMMANDS * sizeof(p_func));
					break;
				case 0x1e3c1390: // BlastCorp, DiddyKongRacing
					memcpy(ABI, ABI1GE, NUM_ABI_COMMANDS * sizeof(p_func));
					break;
				default: return;
			}
		}
		else  
		{ 
			switch (*(u32*)(UData + (0x10))) // ABI2 and MusyX
			{
				case 0x00010010: // MusyX v2 (IndianaJones, BattleForNaboo)
					ProcessMusyX_v2();
					return;
				default:
					return;

				case 0x11181350:  break; // MarioKart, WaveRace (E)
				case 0x111812e0:  break; // StarFox (J)
				case 0x110412ac:  break; // WaveRace (J RevB)
				case 0x110412cc:  break; // StarFox/LylatWars (except J)
				case 0x1cd01250:  break; // FZeroX
				case 0x1f08122c:  break; // YoshisStory
				case 0x1f38122c:  break; // 1080° Snowboarding
				case 0x1f681230:  break; // Zelda OoT / Zelda MM (J, J RevA)
				case 0x1f801250:  break; // Zelda MM (except J, J RevA, E Beta), PokemonStadium 2
				case 0x109411f8:  break; // Zelda MM (E Beta)
				case 0x1eac11b8:  break; // AnimalCrossing
			}
			memcpy(ABI, ABI2, NUM_ABI_COMMANDS * sizeof(p_func));
		}
	}

	//memcpy (imem+0x80, rdram+((u32*)dmem)[0xFD0/4], ((u32*)dmem)[0xFD4/4]);

#ifdef ENABLELOG
	u32 times[NUM_ABI_COMMANDS];
	u32 calls[NUM_ABI_COMMANDS];
	u32 list=0;
	u32 total;

	//memcpy (dmem, rdram+UCData, UDataLen); // Load UCode Data (for RSP stuff)

	memset (times, 0, NUM_ABI_COMMANDS * sizeof(u32));
	memset (calls, 0, NUM_ABI_COMMANDS * sizeof(u32));
	u64 start, end;
#endif

	ListLen = ListLen >> 2;

#if 0
	if (*(u32*)(rdram + UCData + 0x30) == 0x0B396696) {
		//printf ("MusyX Detected\n");
		void smDetect();
		if (ABI[0] != ABI5[0])
			smDetect();
		//ChangeABI (5); // This will be replaced with ProcessMusyX
		//RspClosed();
		//*RSPInfo.SP_PC_REG = 0x1000;
		//DoRspCycles(100);
		return;
	}
#endif

	for (u32 x = 0; x < ListLen; x += 2) {
		unsigned char command;

		k0 = HLEPtr[x + 0];
		t9 = HLEPtr[x + 1];
		command = (unsigned char)((k0 >> 24) & 0xFF);
#if 0
		assert(command == command % NUM_ABI_COMMANDS);
		command %= NUM_ABI_COMMANDS;
#endif

//		fprintf (dfile, "k0: %08X  t9: %08X\n", k0, t9);
#ifdef ENABLELOG
		__asm {
			rdtsc;
			mov dword ptr [start+0], eax;
			mov dword ptr [start+4], edx;
		}
#endif
		StartProfile (2 + command);
		ABI[command]();
		EndProfile (2 + command);
#ifdef ENABLELOG
		__asm {
			rdtsc;
			mov dword ptr [end+0], eax;
			mov dword ptr [end+4], edx;
		}
		calls[command]++;
		times[command] += (u32)(end - start);
#endif
	}
#ifdef ENABLELOG
	fprintf (dfile, "List #%i\n", list++);
	total = 0;
	for (x = 0; x < NUM_ABI_COMMANDS; x++)
		total += times[x];
	for (x = 0; x < NUM_ABI_COMMANDS; x++) {
		if (calls[x] != 0)
			fprintf (dfile, 
			"Command: %02X - Calls: %3i - Total Time: %6i - Avg Time: %8.2f - Percent: %5.2f%%\n", 
			x, 
			calls[x], 
			times[x], 
			(float)times[x]/(float)calls[x],
			((float)times[x]/(float)total)*100.0);
	}
#endif

//	fclose (dfile);
//	assert(0);
}

#ifndef PREFER_MACRO_FUNCTIONS
INLINE s32 sats_over(s32 slice)
{
#ifdef TWOS_COMPLEMENT_NEGATION
    s32 adder, mask;

    adder  = +32767 - slice;
    mask  =  ((s32)adder >> 31); /* if (+32767 - x < 0 */
    mask &= ~((s32)slice >> 31); /*  && x >= 0) */
    adder &= mask;
    return (s32)(slice + adder); /* slice + (+32767 - slice) == +32767 */
#else
    if (slice > +32767)
        return +32767;
    return (slice);
#endif
}
INLINE s32 sats_under(s32 slice)
{
#ifdef TWOS_COMPLEMENT_NEGATION
    s32 adder, mask;

    adder  = +32768 + slice;
    mask  =  ((s32)adder >> 31); /* if (x + 32768 < 0 */
    mask &=  ((s32)slice >> 31); /*  && x < 0) */
    adder &= mask;
    return (s32)(slice - adder); /* slice - (slice + 32768) == -32768 */
#else
    if (slice < -32768)
        return -32768;
    return (slice);
#endif
}

s16 pack_signed(s32 slice)
{
#ifdef SSE2_SUPPORT
    __m128i xmm;

    xmm = _mm_cvtsi32_si128(slice);
    xmm = _mm_packs_epi32(xmm, xmm);
    return (s16)_mm_cvtsi128_si32(xmm); /* or:  return _mm_extract_epi16(xmm, 0); */
#else
    s32 result;

    result = slice;
    result = sats_under(result);
    result = sats_over (result);
    return (s16)(result & 0x0000FFFFul);
#endif
}

void vsats128(s16* vd, s32* vs)
{
#ifdef SSE2_SUPPORT
    __m128i result, xmm_hi, xmm_lo;

    xmm_hi = _mm_loadu_si128((__m128i *)&vs[0]);
    xmm_lo = _mm_loadu_si128((__m128i *)&vs[4]);
    result = _mm_packs_epi32(xmm_hi, xmm_lo);
    _mm_storeu_si128((__m128i *)vd, result);
#else
    register size_t i;

    for (i = 0; i < 8; i++)
        vd[i] = pack_signed(vs[i]);
#endif
}
#endif

void copy_vector(void * vd, const void * vs)
{
#if defined(SSE2_SUPPORT)
 /* MOVDQU  XMMWORD PTR[vd], XMMWORD PTR[vs] */
    _mm_storeu_si128((__m128i *)vd, _mm_loadu_si128((__m128i *)vs));
#elif defined(SSE1_SUPPORT)
 /* MOVUPS  XMMWORD PTR[vd], XMMWORD PTR[vs] */
    _mm_storeu_ps((float *)vd, _mm_loadu_ps((float *)vs));
#elif 0
 /* MOVDQA  XMMWORD PTR[vd], XMMWORD PTR[vs] */
 /* MOVAPS  XMMWORD PTR[vd], XMMWORD PTR[vs] */
    *(__m128 *)vd = *(__m128 *)vs; /* Crash if vd or vs not 128-bit aligned! */
#else
    memcpy(vd, vs, 8 * sizeof(i16));
#endif
}

void swap_elements(void * vd, const void * vs)
{
#ifdef SSE2_SUPPORT
    __m128i RSP_as_XMM;

    RSP_as_XMM = _mm_loadu_si128((__m128i *)vs);
    RSP_as_XMM = _mm_shufflehi_epi16(RSP_as_XMM, _MM_SHUFFLE(2, 3, 0, 1));
    RSP_as_XMM = _mm_shufflelo_epi16(RSP_as_XMM, _MM_SHUFFLE(2, 3, 0, 1));
    _mm_storeu_si128((__m128i *)vd, RSP_as_XMM);
#else
    i16 temp_vector[8];
    register size_t i;

    for (i = 0; i < 8; i++)
        temp_vector[i] = ((i16 *)vs)[i ^ 1];
    copy_vector(vd, temp_vector);
#endif
}
