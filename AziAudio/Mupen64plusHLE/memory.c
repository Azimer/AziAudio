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
*   Mupen64plus-rsp-hle - memory.c                                          *
*   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/             *
*   Copyright (C) 2013 Bobby Smiles                                         *
*                                                                           *
*   This program is free software; you can redistribute it and/or modify    *
*   it under the terms of the GNU General Public License as published by    *
*   the Free Software Foundation; either version 2 of the License, or       *
*   (at your option) any later version.                                     *
*                                                                           *
*   This program is distributed in the hope that it will be useful,         *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
*   GNU General Public License for more details.                            *
*                                                                           *
*   You should have received a copy of the GNU General Public License       *
*   along with this program; if not, write to the                           *
*   Free Software Foundation, Inc.,                                         *
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.            *
*****************************************************************************/

#include <string.h>
#include <assert.h>

#include "common.h"

/*
 * 2017.02.09:  helpers taken from "memory.h"
 *
 * We're not including "memory.h" itself because it's full of static function
 * definitions--all but 3 of which are never used here--which causes tens of
 * unused function warnings from unclean practice.  We could try for a more
 * modular design, but the below 3 functions are straightforward to paste in.
 */

static uint8_t* pt_u8(const unsigned char* buffer, unsigned address)
{
    return (uint8_t *)(buffer + (address ^ ENDIAN_SWAP_BYTE));
}

static uint16_t* pt_u16(const unsigned char* buffer, unsigned address)
{
    assert((address & 1) == 0);
    return (uint16_t *)(buffer + (address ^ ENDIAN_SWAP_HALF));
}

static uint32_t* pt_u32(const unsigned char* buffer, unsigned address)
{
    assert((address & 3) == 0);
    return (uint32_t *)(buffer + address);
}


/* Global functions */
void load_u8(uint8_t* dst, const unsigned char* buffer, unsigned address, size_t count)
{
    while (count != 0) {
        *(dst++) = *pt_u8(buffer, address);
        address += 1;
        --count;
    }
}

void load_u16(uint16_t* dst, const unsigned char* buffer, unsigned address, size_t count)
{
    while (count != 0) {
        *(dst++) = *pt_u16(buffer, address);
        address += 2;
        --count;
    }
}

void load_u32(uint32_t* dst, const unsigned char* buffer, unsigned address, size_t count)
{
    /* Optimization for uint32_t */
    memcpy(dst, pt_u32(buffer, address), count * sizeof(uint32_t));
}

void store_u8(unsigned char* buffer, unsigned address, const uint8_t* src, size_t count)
{
    while (count != 0) {
        *pt_u8(buffer, address) = *(src++);
        address += 1;
        --count;
    }
}

void store_u16(unsigned char* buffer, unsigned address, const uint16_t* src, size_t count)
{
    while (count != 0) {
        *pt_u16(buffer, address) = *(src++);
        address += 2;
        --count;
    }
}

void store_u32(unsigned char* buffer, unsigned address, const uint32_t* src, size_t count)
{
    /* Optimization for uint32_t */
    memcpy(pt_u32(buffer, address), src, count * sizeof(uint32_t));
}
