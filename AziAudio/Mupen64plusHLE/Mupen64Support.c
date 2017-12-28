/****************************************************************************
*                                                                           *
* Azimer's HLE Audio Plugin for Project64 Compatible N64 Emulators          *
* http://www.apollo64.com/                                                  *
* Copyright (C) 2000-2017 Azimer. All rights reserved.                      *
*                                                                           *
* License:                                                                  *
*                                                                           *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
*****************************************************************************/
#include <stdarg.h>
#include <stdio.h>

#include "common.h"

#include "hle_external.h"
#include "hle_internal.h"
#include "../AudioSpec.h"

void HleWarnMessage(void* user_defined, const char *message, ...)
{
	va_list args;
	va_start(args, message);
#if 0
	DebugMessage(M64MSG_WARNING, message, args);
#endif
	va_end(args);

	if (user_defined == NULL)
		return;
 /* user_defined possibly as a HWND, FILE pointer, window ID, ... ? */
}


void HleVerboseMessage(void* user_defined, const char *message, ...)
{
	va_list args;
	va_start(args, message);
#if 0
	DebugMessage(M64MSG_VERBOSE, message, args);
#endif
	va_end(args);

	if (user_defined == NULL)
		return;
 /* user_defined possibly as a HWND, FILE pointer, window ID, ... ? */
}


static struct hle_t _hle;

void SetupMusyX()
{
	struct hle_t *hle = &_hle;

	hle->dram = AudioInfo.RDRAM;
	hle->dmem = AudioInfo.DMEM;
	hle->imem = AudioInfo.IMEM;
	/*hle->mi_intr = NULL;
	hle->sp_mem_addr = NULL;
	hle->sp_dram_addr = NULL;
	hle->sp_rd_length = NULL;
	hle->sp_wr_length = NULL;
	hle->sp_status = NULL;
	hle->sp_dma_full = NULL;
	hle->sp_dma_busy = NULL;
	hle->sp_pc = NULL;
	hle->sp_semaphore = NULL;
	hle->dpc_start = NULL;
	hle->dpc_end = NULL;
	hle->dpc_current = NULL;
	hle->dpc_status = NULL;
	hle->dpc_clock = NULL;
	hle->dpc_bufbusy = NULL;
	hle->dpc_pipebusy = NULL;
	hle->dpc_tmem = NULL;
	hle->user_defined = NULL;*/
}

void ProcessMusyX_v1()
{
	SetupMusyX();
	musyx_v1_task(&_hle);
}

void ProcessMusyX_v2()
{
	SetupMusyX();
	musyx_v2_task(&_hle);
}
