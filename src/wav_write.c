/*
    wav_write.cc - taken from wav-tools 1.1
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
                 -added return statement in rewrite_head
		 
    20 Mar 1999: -using sys/types.h for type sizes for interplatform
		 compatibility

    22 Mar 1999: removed some wav-tools bugs (handling 8-Bit
                 values as 16Bit integers) 
		 
    29 Apr 1999: hacked to use FILE* instead of file descriptors
*/

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>

#ifndef WIN32
#include <sys/soundcard.h>
#include <unistd.h>
#endif

#include "wav_file.h"

#include "endian.h"

void init_head(wav_sig *info){
	int16_t tmp16;
	int32_t tmp32;
	
	strcpy(info->head, "RIFF    WAVEfmt                     data    ");
	
	tmp32=(info->sofar + 32);	
#ifdef BIG_ENDIAN_MACHINE	
	swap32(&tmp32);
#endif
	*(int32_t  *)&info->head[4]  = tmp32;
	
	
	tmp32=16;
#ifdef BIG_ENDIAN_MACHINE	
	swap32(&tmp32);
#endif
	*(int32_t *)&info->head[16] = tmp32;
	
	tmp16=1;
#ifdef BIG_ENDIAN_MACHINE	
	swap16(&tmp16);
#endif
	*(int16_t *)&info->head[20] = tmp16;
	
  	info->head[22] = info->chans;
	info->head[23] = 0;	
	
	tmp32=info->srate;
#ifdef BIG_ENDIAN_MACHINE	
	swap32(&tmp32);
#endif
	*(int32_t  *)&info->head[24] = tmp32;
	
	tmp32=info->bps;
#ifdef BIG_ENDIAN_MACHINE	
	swap32(&tmp32);
#endif
	*(int32_t  *)&info->head[28] = tmp32;
	
        info->head[32] = info->blkalign;
	info->head[33] = 0;
		
	info->head[34] = info->depth;
	info->head[35] = 0;
	
	tmp32=info->sofar;
#ifdef BIG_ENDIAN_MACHINE	
	swap32(&tmp32);
#endif
	*(int32_t  *)&info->head[40] = tmp32;
}

FILE* open_wav_rec(wav_sig *info){
	info->handle = fopen(info->name, "w");
	init_head(info);
	fwrite(info->head, 44, 1, info->handle);
	return(info->handle);
}

/* obsolete 
int rewrite_head(wav_sig *info){
//	lseek(info->handle, 0, SEEK_SET);
//	init_head(info);
//	write(info->handle, info->head, 44);

	return(0);
}*/
