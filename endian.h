/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999  Alexander König
 
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
 
    File: endian.h
 
    Description: header to endian.cc
*/    

#if defined (BIG_ENDIAN_MACHINE) || defined(BIG_ENDIAN_AUDIO)

#include "tX_types.h"

extern void swap16(int16_t * val);
extern void swap32(int32_t * val);
extern void swapbuffer(int16_t *buffer, int samples);

#endif
