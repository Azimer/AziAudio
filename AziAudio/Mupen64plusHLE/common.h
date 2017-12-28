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
*   Mupen64plus-rsp-hle - common.h                                          *
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

#ifndef COMMON_H
#define COMMON_H

#include "../my_types.h"

/* macro for unused variable warning suppression */
#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

/* macro for inline keyword */
#ifdef _MSC_VER
#define inline __inline
#elif defined(__GNUC_GNU_INLINE__)
#define inline
#endif

#endif
