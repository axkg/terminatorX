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
 
    File: tX_audiofile.h
 
    Description: Header to audiofile.cc
*/   

#ifndef _h_tx_audiofile
#define _h_tx_audiofile 1

#define SOX_BLOCKSIZE 32000

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tX_endian.h>

#ifdef USE_SOX_INPUT
#define SOX_STR "sox \"%s\" -t raw -c 1 -r 44100 -s -w -"
#endif

#ifdef USE_MPG123_INPUT
/* The Original MPG123_STR - probably slightly faster than the one above but
but mpg321 doesn't support -m yet.
#define MPG123_STR "mpg123 -qms \"%s\""
*/

#ifdef BIG_ENDIAN_MACHINE
/* This works with mpg321 only... */
#define MPG123_STR "mpg123 -qs \"%s\" | sox -x -t raw -s -w -r 44100 -c 2 - -t raw -c 1 -r 44100 -s -w -"
#else
#define MPG123_STR "mpg123 -qs \"%s\" | sox -t raw -s -w -r 44100 -c 2 - -t raw -c 1 -r 44100 -s -w -"
#endif
#endif

#ifdef USE_OGG123_INPUT
#define OGG123_STR "ogg123 -q -d wav -f - \"%s\" | sox -t wav - -t raw -c 1 -r 44100 -s -w -"
/* -o file:/dev/stdout because ogg123 doesn't interpret - as stdout */
/* 20010907: i take that back, it seems that newer versions don't
 * have that problem */
#endif /* USE_OGG123_INPUT */


#define TX_AUDIO_SUCCESS 0
#define TX_AUDIO_ERR_ALLOC 1
#define TX_AUDIO_ERR_PIPE_READ 2
#define TX_AUDIO_ERR_SOX 3
#define TX_AUDIO_ERR_MPG123 4
#define TX_AUDIO_ERR_WAV_NOTFOUND 5
#define TX_AUDIO_ERR_NOT_16BIT 6
#define TX_AUDIO_ERR_NOT_MONO 7
#define TX_AUDIO_ERR_WAV_READ 8
#define TX_AUDIO_ERR_NOT_SUPPORTED 9
#define TX_AUDIO_ERR_OGG123 10

#define TX_AUDIO_UNDEFINED 0
#define TX_AUDIO_MMAP 1
#define TX_AUDIO_LOAD 2

#define TX_FILE_UNDEFINED 0
#define TX_FILE_WAV 1
#define TX_FILE_MPG123 2
#define TX_FILE_OGG123 3

#include <limits.h>
#include "tX_types.h"
#include <stdio.h>

class tx_audiofile
{
	private:
	int mem_type;
	int file_type;
	
	FILE *file;
	char filename[PATH_MAX];
	int16_t *mem;
	size_t memsize;
	long no_samples;	

#ifdef USE_BUILTIN_WAV
	int load_wav();
#endif
#ifdef USE_SOX_INPUT	
	int load_sox();
#define NEED_PIPED 1	
#endif
#ifdef USE_MPG123_INPUT	
	int load_mpg123();
#define NEED_PIPED 1	
#endif
#ifdef USE_OGG123_INPUT
	int load_ogg123();
#define NEED_PIPED 1
#endif

#ifdef NEED_PIPED
	int load_piped();
#endif
	void figure_file_type();
	
	public:
	tx_audiofile();
	
	int load(char *p_file_name);
	int16_t *get_buffer() { return mem; };
	long get_no_samples() { return no_samples; };
	
	~tx_audiofile();
};

#endif
