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
*   Mupen64plus-rsp-hle - arithmetics.h                                     *
*   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/             *
*   Copyright (C) 2014 Bobby Smiles                                         *
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

#ifndef ARITHMETICS_H
#define ARITHMETICS_H

#include "common.h"

/*
 * 2015.05.05 modified by cxd4
 * The <stdint.h> `INT16_MIN' macro is a dependency of this RSP HLE code.
 * In practice, there are two problems with this:
 *     1.  <stdint.h> can (rather, must) define `INT16_MIN' as -32767 instead
 *         of the necessary -32768, as <stdint.h> is irrelevant to RSP
 *         hardware limitations and is really more about the host's CPU.
 *         http://www.open-std.org/jtc1/sc22/wg14/www/docs/n761.htm
 *     2.  Not everybody has a working <stdint.h>, nor should they need to,
 *         as its existence is based on the opinion that the C language
 *         itself should arbitrate traditional-size types for the programmer.
 *         As none of the <stdint.h> types are scientifically essential to
 *         valid CPU hardware, such C impl. requirements are for laziness.
 */
#if !defined(INT16_MIN) && !defined(INT16_MAX)
#define INT16_MIN               -32768
#define INT16_MAX               +32767
#endif

static inline int16_t clamp_s16(int32_t x)
{
    x = (x < INT16_MIN) ? INT16_MIN: x;
    x = (x > INT16_MAX) ? INT16_MAX: x;

    return (int16_t)x;
}

#endif
