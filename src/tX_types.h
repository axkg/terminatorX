/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2022  Alexander König

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File: tX_types.h

    Description: Use correct type sizes. If <sys/types.h> is not
                 available define USE_X86_TYPES on i386 machines
*/

#pragma once

#define f_prec float
#define d_prec double

#ifndef USE_X86_TYPES

#include <sys/types.h>

#else

#define int8_t char
#define int16_t short
#define int32_t long

#endif
