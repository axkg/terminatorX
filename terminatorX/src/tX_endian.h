/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2002  Alexander König
 
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
 
    File: tX_endian.h
 
    Description: header to tX_endian.c
    
    27 apr 1999: include config.h to automatically recognize 
    		 big endian machines via configure.
		 
    7 mar 2000:  moved from endian.{cc,h} to tX_endian.{cc,h} as
                 the glibc includes used my endian.h when they should
		 include <bits/endian.h>
*/    

#ifdef HAVE_CONFIG_H
#include <config.h>
#ifdef WORDS_BIGENDIAN
#define BIG_ENDIAN_MACHINE 1
#else
#undef BIG_ENDIAN_MACHINE
#endif
#endif

#ifdef WORDS_BIGENDIAN

#include "tX_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void swap16(int16_t * val);
extern void swap32(int32_t * val);
extern void swapbuffer(int16_t *buffer, int samples);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
