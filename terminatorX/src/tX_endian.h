/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2004  Alexander König
 
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

#ifndef _H_TX_ENDIAN_
#define _H_TX_ENDIAN_

#ifdef HAVE_CONFIG_H
#	include <config.h>
#	ifdef WORDS_BIGENDIAN
#		define BIG_ENDIAN_MACHINE 1
#	else
#		undef BIG_ENDIAN_MACHINE
#	endif
#endif

#ifdef WORDS_BIGENDIAN

#	include "tX_types.h"

#define __USE_XOPEN // we need this for swab()
#	include <unistd.h>
#undef __USE_XOPEN

#define swapbuffer(b, s) swab((void *) b, (void *) b, (ssize_t) s<<1)

static inline void swap32_inline(int32_t *val) {
	int8_t temp;
	int8_t *p=(int8_t *) val;
	
	temp=p[0];
	p[0]=p[3];
	p[3]=temp;
	
	temp=p[1];
	p[1]=p[2];
	p[2]=temp;	
}

#	ifdef __cplusplus
extern "C" {
#	endif /* __cplusplus */

extern void swap16(int16_t * val);
extern void swap32(int32_t * val);
	
#	ifdef __cplusplus
}
#	endif /* __cplusplus */

#endif /* WORDS_BIGENDIAN */

#endif /* _H_TX_ENDIAN_ */
