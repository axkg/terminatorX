/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2011  Alexander König
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
    File: tX_types.h
 
    Description: Use correct type sizes. If <sys/types.h> is not
                 available define USE_X86_TYPES on i386 machines
*/    

#ifndef _H_TX_TYPES
#define _H_TX_TYPES

#define f_prec float
#define d_prec double

#ifndef USE_X86_TYPES

#include <sys/types.h>

#else

#define int8_t char
#define int16_t short
#define int32_t long

#endif

#endif
