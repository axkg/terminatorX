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
		
    28 Apr 1999: switch from filedescriptors to FILE*
    
    29 Apr 1999: added sox and mpg123 support
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef _H_WAVFILE
#define _H_WAVFILE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_SOX_INPUT
#define SOX_STR "sox \"%s\" -t raw -c 1 -r 44100 -s -w -"
#endif

#ifdef USE_MPG123_INPUT
/*#include "endian.h"	// I don't think this is required
#ifdef BIG_ENDIAN_MACHINE
#define MPG123_STR "mpg123 -s \"%s\" | sox -t raw -r 44100 -c 2 -s -w -x - -t wav -c 1 -r 44100 -s -w -"
#else*/
#define MPG123_STR "mpg123 -qms \"%s\""
/*#else
#define MPG123_STR "mpg123 -qs \"%s\" | sox -t raw -r 44100 -c 2 -s -w - -t raw -c 1 -r 44100 -s -w -"
#endif
/*#endif*/
#endif


#include "tX_global.h"
#include "tX_types.h"

typedef struct{
	int32_t	srate;
	int8_t	chans;
	int8_t	depth;
	int32_t	bps;
	int8_t	blkalign;
	int32_t	len;
	int32_t	sofar;
	
	FILE*   handle;
	char	name[PATH_MAX];
	char    head[43];
#ifdef USE_SOX_INPUT
	int	has_host_order;
#endif	
} wav_sig;

FILE* init_wav_read(char file_name[], wav_sig *info);
FILE* open_wav_rec(wav_sig *info);
extern int rewrite_head(wav_sig *info);
extern void wav_close(FILE* wav);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
