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
 
    File: tX_wavfunc.h
 
    Description: Header to tX_wavfunc.c
*/    

#ifndef _TX_WAVFUNC_H
#define _TX_WAVFUNC_H

#define LW_NO_ERROR 0
#define LW_ERROR_FILE_NOT_MONO 1
#define LW_ERROR_FILE_NOT_16BIT 2
#define LW_ERROR_FILE_NOT_FOUND 3
#define LW_ERROR_ALLOC 4
#define LW_ERROR_READ 5

int load_wav(char *name, int16_t **data_ptr, unsigned int *size);
extern int malloc_recbuffer();
#endif
