/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2016  Alexander König
 
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
 
    File: tX_endian.c
 
    Description: swap byte order for big endian systems/audiohardware.
*/    

#include "tX_endian.h"
#include "tX_global.h"

#ifdef WORDS_BIGENDIAN

void swap16(int16_t * val) {
	int8_t temp;
	int8_t *p;
	
	p=(int8_t *) val;
	temp=p[0];
	p[0]=p[1];
	p[1]=temp;
}

void swap32(int32_t * val) {
	int8_t temp;
	int8_t *p;
	
	p=(int8_t *) val;
	temp=p[0];
	p[0]=p[3];
	p[3]=temp;
	
	temp=p[1];
	p[1]=p[2];
	p[2]=temp;
}

#endif
