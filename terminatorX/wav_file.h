/*
    wav_file.h - taken from wav-tools 1.1
    Copyright (C) by Colin Ligertwood

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
    
    Changes:
    
    11 Mar 1999: -added license hint
                 -slight changes for use with terminatorX

    20 Mar 1999: now includes sys/types.h and uses types
                 defined there for interplatform compability. 
*/

#ifndef _H_WAVFILE
#define _H_WAVFILE

#include "tX_types.h"

typedef struct{
	int32_t	srate;
	int8_t	chans;
	int8_t	depth;
	int32_t	bps;
	int8_t	blkalign;
	int32_t	len;
	int32_t	sofar;
	
	int     handle;
	char	name[31];
	char    head[43];
} wav_sig;

extern int init_wav_read(char file_name[], wav_sig *info);
extern int open_wav_rec(wav_sig *info);
extern int rewrite_head(wav_sig *info);

#endif
