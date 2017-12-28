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
*   Mupen64plus-rsp-hle - audio.c                                           *
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

#include <assert.h>
#include <stddef.h>

#define S16(hex)   \
    ((hex) & 0x8000u) \
  ? -(signed)((~(hex##u) + 1) & 0x7FFFu) \
  : +(hex) \

#include "arithmetics.h"
const i16 RESAMPLE_LUT[64 * 4] = {
    0x0c39, 0x66ad, 0x0d46, S16(0xffdf),
    0x0b39, 0x6696, 0x0e5f, S16(0xffd8),
    0x0a44, 0x6669, 0x0f83, S16(0xffd0),
    0x095a, 0x6626, 0x10b4, S16(0xffc8),
    0x087d, 0x65cd, 0x11f0, S16(0xffbf),
    0x07ab, 0x655e, 0x1338, S16(0xffb6),
    0x06e4, 0x64d9, 0x148c, S16(0xffac),
    0x0628, 0x643f, 0x15eb, S16(0xffa1),
    0x0577, 0x638f, 0x1756, S16(0xff96),
    0x04d1, 0x62cb, 0x18cb, S16(0xff8a),
    0x0435, 0x61f3, 0x1a4c, S16(0xff7e),
    0x03a4, 0x6106, 0x1bd7, S16(0xff71),
    0x031c, 0x6007, 0x1d6c, S16(0xff64),
    0x029f, 0x5ef5, 0x1f0b, S16(0xff56),
    0x022a, 0x5dd0, 0x20b3, S16(0xff48),
    0x01be, 0x5c9a, 0x2264, S16(0xff3a),
    0x015b, 0x5b53, 0x241e, S16(0xff2c),
    0x0101, 0x59fc, 0x25e0, S16(0xff1e),
    0x00ae, 0x5896, 0x27a9, S16(0xff10),
    0x0063, 0x5720, 0x297a, S16(0xff02),
    0x001f, 0x559d, 0x2b50, S16(0xfef4),
    S16(0xffe2), 0x540d, 0x2d2c, S16(0xfee8),
    S16(0xffac), 0x5270, 0x2f0d, S16(0xfedb),
    S16(0xff7c), 0x50c7, 0x30f3, S16(0xfed0),
    S16(0xff53), 0x4f14, 0x32dc, S16(0xfec6),
    S16(0xff2e), 0x4d57, 0x34c8, S16(0xfebd),
    S16(0xff0f), 0x4b91, 0x36b6, S16(0xfeb6),
    S16(0xfef5), 0x49c2, 0x38a5, S16(0xfeb0),
    S16(0xfedf), 0x47ed, 0x3a95, S16(0xfeac),
    S16(0xfece), 0x4611, 0x3c85, S16(0xfeab),
    S16(0xfec0), 0x4430, 0x3e74, S16(0xfeac),
    S16(0xfeb6), 0x424a, 0x4060, S16(0xfeaf),
    S16(0xfeaf), 0x4060, 0x424a, S16(0xfeb6),
    S16(0xfeac), 0x3e74, 0x4430, S16(0xfec0),
    S16(0xfeab), 0x3c85, 0x4611, S16(0xfece),
    S16(0xfeac), 0x3a95, 0x47ed, S16(0xfedf),
    S16(0xfeb0), 0x38a5, 0x49c2, S16(0xfef5),
    S16(0xfeb6), 0x36b6, 0x4b91, S16(0xff0f),
    S16(0xfebd), 0x34c8, 0x4d57, S16(0xff2e),
    S16(0xfec6), 0x32dc, 0x4f14, S16(0xff53),
    S16(0xfed0), 0x30f3, 0x50c7, S16(0xff7c),
    S16(0xfedb), 0x2f0d, 0x5270, S16(0xffac),
    S16(0xfee8), 0x2d2c, 0x540d, S16(0xffe2),
    S16(0xfef4), 0x2b50, 0x559d, 0x001f,
    S16(0xff02), 0x297a, 0x5720, 0x0063,
    S16(0xff10), 0x27a9, 0x5896, 0x00ae,
    S16(0xff1e), 0x25e0, 0x59fc, 0x0101,
    S16(0xff2c), 0x241e, 0x5b53, 0x015b,
    S16(0xff3a), 0x2264, 0x5c9a, 0x01be,
    S16(0xff48), 0x20b3, 0x5dd0, 0x022a,
    S16(0xff56), 0x1f0b, 0x5ef5, 0x029f,
    S16(0xff64), 0x1d6c, 0x6007, 0x031c,
    S16(0xff71), 0x1bd7, 0x6106, 0x03a4,
    S16(0xff7e), 0x1a4c, 0x61f3, 0x0435,
    S16(0xff8a), 0x18cb, 0x62cb, 0x04d1,
    S16(0xff96), 0x1756, 0x638f, 0x0577,
    S16(0xffa1), 0x15eb, 0x643f, 0x0628,
    S16(0xffac), 0x148c, 0x64d9, 0x06e4,
    S16(0xffb6), 0x1338, 0x655e, 0x07ab,
    S16(0xffbf), 0x11f0, 0x65cd, 0x087d,
    S16(0xffc8), 0x10b4, 0x6626, 0x095a,
    S16(0xffd0), 0x0f83, 0x6669, 0x0a44,
    S16(0xffd8), 0x0e5f, 0x6696, 0x0b39,
    S16(0xffdf), 0x0d46, 0x66ad, 0x0c39,
};

int32_t rdot(size_t n, const int16_t *x, const int16_t *y)
{
    int32_t accu = 0;

    y += n;

    while (n != 0) {
        accu += *(x++) * *(--y);
        --n;
    }

    return accu;
}

void adpcm_compute_residuals(int16_t* dst, const int16_t* src,
        const int16_t* cb_entry, const int16_t* last_samples, size_t count)
{
    const int16_t* const book1 = cb_entry;
    const int16_t* const book2 = cb_entry + 8;

    const int16_t l1 = last_samples[0];
    const int16_t l2 = last_samples[1];

    size_t i;

    assert(count <= 8);

    for(i = 0; i < count; ++i) {
        int32_t accu = (int32_t)src[i] << 11;
        accu += book1[i]*l1 + book2[i]*l2 + rdot(i, book2, src);
        dst[i] = clamp_s16(accu >> 11);
   }
}
