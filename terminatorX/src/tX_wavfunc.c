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
 
    File: tX_wavfunc.c
 
    Description: Contains routines for loading the loop and scratch
    		 files and allocating the record buffer.
*/    

#include "tX_types.h"
#include "tX_gui.h"
#include "wav_file.h"
#include "tX_wavfunc.h"
#include "tX_global.h"
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <malloc.h>

#define min(a,b) ((a) < (b) ? (a) : (b))

#define SOX_BLOCKSIZE 32000

#ifdef USE_SOX_INPUT
typedef struct
{
	char buffer[SOX_BLOCKSIZE];
	ssize_t used;
	void *next; 
} tempbuff;

tempbuff *newbuff()
{
	tempbuff *nwbf;
	
	nwbf=(tempbuff *) malloc(sizeof(tempbuff));
	nwbf->next=NULL;
	nwbf->used=0;
	
	return(nwbf);
}
#endif

int load_wav(char *name, int16_t **data_ptr, unsigned int *size)
{
	wav_sig wav_in;
	int16_t *data;
	int16_t *p;
	ssize_t allbytes=0;
	ssize_t bytes=0;
	int i;
#ifdef USE_SOX_INPUT
	tempbuff* start=NULL;
	tempbuff* w=NULL;
	tempbuff* new=NULL;
	
//	int blink=0;
//	int buffsum=0;
#endif	
	
	if (!init_wav_read(name, &wav_in))
	{
#ifdef USE_CONSOLE
		printf("[load_wav] Error. Couldn't open \"%s\".\n", name);		
#endif	
		return(LW_ERROR_FILE_NOT_FOUND);
	}

#ifdef USE_CONSOLE
	printf("Loading: %s\n", name);
	if (parms.verbose) printf("File: %i Bytes Data, %i Bit Depth, %i Hz Samplerate.\n", wav_in.len, wav_in.depth, wav_in.srate);	
#endif	

#ifndef USE_SOX_INPUT
	
	if (wav_in.depth != 16)
	{
#ifdef USE_CONSOLE
		puts("[load_wav] Error: Wave-File is not 16 Bit. Fatal. Giving up.");		
#endif		
		return(LW_ERROR_FILE_NOT_16BIT);
	}

	if (wav_in.chans != 1)
	{
#ifdef USE_CONSOLE	
		puts("[load_wav] Error: Wave-File is not Mono. Fatal. Giving up.");
#endif
		return(LW_ERROR_FILE_NOT_MONO);
	}

#ifdef USE_CONSOLE	
	if (wav_in.srate != 44100) 
	{
		puts("[load_wav] Warning: Wave-File was not recorded at 44.100 Hz!");
	}
	if (wav_in.blkalign != 2)
	{
		printf("[load_wav] Warning: Unexpected block alignment: %i.\n", wav_in.blkalign);
	}
#endif

	*size=wav_in.len;
	data = (int16_t *) malloc (*size);
		
	if (!data)
	{
#ifdef USE_CONSOLE
		puts("[load_wav] Error: Failed to allocate sample memory. Fatal. Giving up.");
#endif	
		return(LW_ERROR_ALLOC);
	}
#endif

#ifdef USE_SOX_INPUT
	bytes=1;		
	
	start=newbuff();
	w=start;
	
	wav_progress_update(0.5); /* Yeehha! A realistic Progessbar ;) */
	
	while (bytes)
#else
	p=data;
	
	while (wav_in.len>allbytes)
#endif	
	{	
#ifdef USE_SOX_INPUT	
		bytes = fread(w->buffer, 1, SOX_BLOCKSIZE, wav_in.handle);
		w->used=bytes;
		
/*		buffsum++;
		if (buffsum>5)
		{
			if (blink)
			{
				blink=0;
				wav_progress_update(0.5);
			}
			else
			{
				blink=1;
				wav_progress_update(0.25);
			}
			buffsum=0;
		}*/
		
#ifdef BIG_ENDIAN_MACHINE
		swapbuffer(w->buffer, bytes/sizeof(int16_t));
#endif		
		
		if (bytes)
		{
			new=newbuff();
			if (!new) return (LW_ERROR_ALLOC);
			w->next=new;
			w=new;
		}				
#else
	
		bytes = fread(p, 1, min(1024, wav_in.len-allbytes), wav_in.handle);

#ifdef BIG_ENDIAN_MACHINE
		if (!wav_in.has_host_order) swapbuffer(p, bytes/sizeof(int16_t));
#endif		
		
		if (bytes<=0)
		{
#ifdef USE_CONSOLE
		       puts("[load_wav] Error: Failed to read Wave-Data (Corrupt?). Fatal. Giving up.");
                       if(bytes<0) perror("terminatorX");
#endif		
			free(data);
			wav_progress_update(0);
		      return (LW_ERROR_READ);
		}
				
#endif		// USE_SOX_INPUT

		allbytes+=bytes;
		
#ifndef USE_SOX_INPUT
		wav_progress_update(((gfloat) allbytes)/((gfloat) wav_in.len));
		
		p+=(ssize_t) bytes/sizeof(int16_t);
#endif		
	}
	
	wav_close(wav_in.handle);

#ifdef USE_SOX_INPUT
	if (!allbytes) 	// Nothing read from pipe -> error
	{
		free(start);
		return (LW_ERROR_READ);
	}
#endif

#ifdef USE_CONSOLE
	if (parms.verbose) printf("Read: %i Bytes of Wave-Data.\n", allbytes);
#endif	

#ifdef USE_SOX_INPUT
	*size=allbytes;
	data = (int16_t *) malloc (*size);
	
	if (!data)
	{
		return(LW_ERROR_ALLOC);
	}
	
	w=start;
	p=data;

	bytes=0;
	
	while(w)
	{
		memcpy(p, w->buffer, w->used);
		bytes+=w->used;
		new=(tempbuff*) w->next;
		free(w);
		w=new;
		
		wav_progress_update(0.5 + 0.5*((gfloat) bytes)/((gfloat) allbytes));
		
		p+=(ssize_t) SOX_BLOCKSIZE/sizeof(int16_t);
	}

#endif
	wav_progress_update(0);

	*data_ptr=data;

	return (LW_NO_ERROR);
}

int malloc_recbuffer()
{
	if (globals.rec_buffer) free(globals.rec_buffer);
	globals.rec_buffer=(int16_t*) malloc((ssize_t) globals.rec_size);
	if (globals.rec_buffer!=NULL) return(0);
	else return(1);
}
