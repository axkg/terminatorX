/*
    wav_read.c - taken from wav-tools 1.1
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
    
    11 Mar 1999: added license hint
    
    20 Mar 1999: using types in sys/types for interplatform
                 compability.
		 
    20 Mar 1999: support for big endian machines
    
    07 Apr 1999: had to change the macros ... i wish I hadn't switched to C ..
    
    29 Apr 1999: hacked to work with FILE* instead of file descriptors
    	         and support for sox and mpg123
*/

/* operations for verifying and reading wav files. */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "wav_file.h"
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include "endian.h"
#include "tX_types.h"

/* Read little endian 16bit values little endian
*/
#ifdef BIG_ENDIAN_MACHINE

#define read16(x); \
	p = (int8_t *) &tmp; \
	p[0] = info->head[x]; \
	p[1] = info->head[x+1]; \
	swap16(&tmp); \
	return(tmp);

#else

#define read16(x); \
	p = (int8_t *) &tmp; \
	p[0] = info->head[x]; \
	p[1] = info->head[x+1]; \
	return(tmp);

#endif	
	

/* Read little endian 32bit values little endian
*/
#ifdef BIG_ENDIAN_MACHINE

#define read32(x); \
	p = (int8_t *) &tmp; \
	p[0] = info->head[x]; \
	p[1] = info->head[x+1]; \
	p[2] = info->head[x+2]; \
	p[3] = info->head[x+3]; \
	swap32(&tmp); \
	return (tmp);

#else

#define read32(x); \
	p = (int8_t *) &tmp; \
	p[0] = info->head[x]; \
	p[1] = info->head[x+1]; \
	p[2] = info->head[x+2]; \
	p[3] = info->head[x+3]; \
	return (tmp);

#endif

	/* wav header is 44 bytes long */
FILE* open_wav(char *header, char file_name[], int *mode){
	FILE* handle;
#ifdef USE_SOX_INPUT
	char buffer[PATH_MAX*2];
#ifdef USE_MPG123_INPUT
	char *end;
	
	end=(char *) strrchr(file_name, (int) '.');

	*mode=0;
	
	if (end)
	{
		if (strlen(end)>3)
		{
			if (((end[1]=='m') || (end[1]=='M')) && ((end[2]=='p') || (end[2]=='P')))
			{
				sprintf(buffer, MPG123_STR, file_name);	
#ifdef USE_MPG123_FAST								
				*mode=1;
#endif				
			}
			else
			{
				sprintf(buffer, SOX_STR, file_name);				
			}			
		}
		else
		{
				sprintf(buffer, SOX_STR, file_name);				
		}
	}	
		
#else	
	sprintf(buffer, SOX_STR, file_name);	
#endif
	handle = popen(buffer, "r");
#else	
	handle = fopen(file_name, "r");
	if(handle)
	fread((char *) header, 1, 44, handle);
#endif	
	return(handle);
}

int16_t get_wav_format(wav_sig *info){
	int16_t tmp; 
	int8_t *p; 

	read16(20);
}

	/* mono or stereo */
int8_t get_wav_channels(wav_sig *info){
	return(info->head[22]);
//	read16(22);
}

	/* sample rate */
int32_t get_wav_srate(wav_sig *info){
	int32_t tmp; 
	int8_t *p; 
	
	read32(24);
}

int32_t get_wav_bps(wav_sig *info){
	int32_t tmp; 
	int8_t *p; 

	read32(28);
}

int8_t get_wav_blkalign(wav_sig *info){
	return(info->head[32]);
//	read16(32);
}

	/* sample depth (8bit or 16bit) */
int8_t get_wav_depth(wav_sig *info){
	return(info->head[34]);
//	read16(34);
}

	/* data section only  ==  totalfile - 44 */
int32_t get_wav_len(wav_sig *info){
	int32_t tmp; 
	int8_t *p; 

	read32(40);
}


FILE *init_wav_read(char file_name[], wav_sig *info){
	int mode=0;
	
	info->handle = open_wav(info->head, file_name, &mode);
#ifdef USE_SOX_INPUT
	info->has_host_order=mode;
#endif	
	strcpy(info->name,file_name);
	info->chans = get_wav_channels(info);
 	info->srate = get_wav_srate(info);
 	info->bps   = get_wav_bps(info);
 	info->blkalign = get_wav_blkalign(info);
	info->depth = get_wav_depth(info);
	info->len   = get_wav_len(info);
	return(info->handle);	
}

void wav_close(FILE* wav)
{
	if (wav)
	{
#ifdef USE_SOX_INPUT
	pclose(wav);
#else
	fclose(wav);
#endif
	}
}
