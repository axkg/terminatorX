/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999, 2000  Alexander König
 
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
 
    File: tX_audiofile.cc
 
    Description: This implements the new audiofile class. Unlike earlier
    		 versions of terminatorX where wav_read.c was evilishly
		 "#ifdef"ed to support multiple loading strategies this new
		 design allows multiple load_*()-methods/strategies at
		 the same time. (e.g. tx can now try to load a wavfile
		 with the builtin routines and if that fails it'll try
		 to load the file through sox (if available)).
*/   

#include "tX_audiofile.h"

#include <string.h>
#include <malloc.h>
#include "wav_file.h"
#include "tX_loaddlg.h"

tx_audiofile :: tx_audiofile()
{
	mem_type=TX_AUDIO_UNDEFINED;
	file_type=TX_FILE_UNDEFINED;
	file=NULL;
	strcpy(filename,"");
	mem=NULL;
	no_samples=0;
}

void tx_audiofile :: figure_file_type()
{
	char *ext;
	
	ext=strrchr(filename, (int) '.');
	
//	puts(ext);
	if (ext)
	{
		if (strlen(ext) >3)
		{
			ext++;
			if (!strcasecmp("wav", ext)) file_type=TX_FILE_WAV;
			else if (!strncasecmp("mp", ext, 2)) file_type=TX_FILE_MPG123;
			else if (!strncasecmp("ogg", ext, 2)) file_type=TX_FILE_OGG123;
		}
	}
}

int tx_audiofile :: load(char *p_file_name)
{
	int load_err=TX_AUDIO_ERR_NOT_SUPPORTED;
	
	strcpy(filename, p_file_name);
	
	ld_set_progress(0);
		
	figure_file_type();
	
#ifdef USE_BUILTIN_WAV
	if ((load_err) && (file_type==TX_FILE_WAV))
		load_err=load_wav();
	
	if (!load_err) return(load_err);	
#endif	

#ifdef USE_MPG123_INPUT
	if ((load_err) && (file_type==TX_FILE_MPG123))
	{
		load_err=load_mpg123();
		return(load_err);
	}
#endif	

#ifdef USE_OGG123_INPUT
	if ((load_err) && (file_type==TX_FILE_OGG123))
	{
		load_err=load_ogg123();
		return(load_err);
	}
#endif

#ifdef USE_SOX_INPUT
	if (load_err)
	{
		load_err=load_sox();
	}
#endif	

	return(load_err);
}

 tx_audiofile :: ~tx_audiofile()
 {

	// clear mem

	switch (mem_type)
	{
		case TX_AUDIO_MMAP:
		{
			// unmap stuff
		}
		
		case TX_AUDIO_LOAD:
		{
			//munlock(void *mem, memsize);
			free(mem);
		}
		
	}	

	// clear file

 	if (file)
	{
		if (mem_type==TX_AUDIO_MMAP)
		{
			// free mmap
		}
	}
	
 }
 
typedef struct tembpuff
{
	char buffer[SOX_BLOCKSIZE];
	ssize_t used;
	void *next; 
} tempbuff;

static tempbuff *newbuff()
{
	tempbuff *nwbf;
	
	nwbf=(tempbuff *) malloc(sizeof(tempbuff));
	nwbf->next=NULL;
	nwbf->used=0;
	
	return(nwbf);
}

#ifdef NEED_PIPED 

int tx_audiofile :: load_piped()
{
	int16_t *data;
	int16_t *p;
	ssize_t allbytes=0;
	ssize_t bytes=1;
	int i;
	tempbuff* start=NULL;
	tempbuff* w=NULL;
	tempbuff* newb=NULL;
	
	start=newbuff();
	w=start;

	mem_type=TX_AUDIO_LOAD;

	ld_set_progress(0.5);
		
	while (bytes)
	{	
		bytes = fread(w->buffer, 1, SOX_BLOCKSIZE, file);
		w->used=bytes;
#ifdef BIG_ENDIAN_MACHINE
		swapbuffer((int16_t *) w->buffer, bytes/sizeof(int16_t));
#endif		
		
		if (bytes)
		{
			newb=newbuff();
			if (!newb) return (TX_AUDIO_ERR_ALLOC);
			w->next=newb;
			w=newb;
		}				

#ifdef BIG_ENDIAN_MACHINE
		//if (!wav_in.has_host_order) swapbuffer(p, bytes/sizeof(int16_t));
#endif		
		allbytes+=bytes;			
	}
	
	pclose(file); file=NULL;
	
	if (!allbytes) 	// Nothing read from pipe -> error
	{
		free(start);
		return (TX_AUDIO_ERR_PIPE_READ);
	}

	no_samples=allbytes/sizeof(int16_t);
	memsize=allbytes;
	data = (int16_t *) malloc (memsize);
	
	if (!data)
	{
		w=start;
		while (w)
		{
			newb=(tempbuff*)w->next;
			free(w);
			w=newb;
		}
		return(TX_AUDIO_ERR_ALLOC);
	}
	
	w=start;
	p=data;

	bytes=0;
	
	while(w)
	{
		memcpy(p, w->buffer, w->used);
		bytes+=w->used;
		newb=(tempbuff*) w->next;
		free(w);
		w=newb;
		
		ld_set_progress(0.5 + 0.5*((gfloat) bytes)/((gfloat) allbytes));
		
		p+=(ssize_t) SOX_BLOCKSIZE/sizeof(int16_t);
	}

	mem=data;
	return (TX_AUDIO_SUCCESS);
}

#endif
	
#ifdef USE_SOX_INPUT
int tx_audiofile :: load_sox()
{
	char command[PATH_MAX*2];

	sprintf(command, SOX_STR, filename);
	file = popen(command, "r");
	
	if (!file) return TX_AUDIO_ERR_SOX;
	
	return load_piped();
	
}
#endif	

#ifdef USE_MPG123_INPUT
int tx_audiofile :: load_mpg123()
{
	char command[PATH_MAX*2];
	
	sprintf(command, MPG123_STR, filename);
	file = popen(command, "r");
	
	if (!file) return TX_AUDIO_ERR_MPG123;
	
	return load_piped();	
}
#endif	

#ifdef USE_OGG123_INPUT
int tx_audiofile :: load_ogg123()
{
	char command[PATH_MAX*2];

	sprintf(command, OGG123_STR, filename);
	file = popen(command, "r");

	if (!file) return TX_AUDIO_ERR_OGG123;

	return load_piped();
}
#endif

#ifdef USE_BUILTIN_WAV
#define min(a,b) ((a) < (b) ? (a) : (b))
int tx_audiofile :: load_wav()
{
	wav_sig wav_in;
	int16_t *data;
	int16_t *p;
	ssize_t allbytes=0;
	ssize_t bytes=0;
	int i;

	mem_type=TX_AUDIO_LOAD;
	
	if (!init_wav_read(filename, &wav_in))
	{
		return(TX_AUDIO_ERR_WAV_NOTFOUND);
	}

#ifdef USE_CONSOLE
	printf("Loading: %s\n", filename);
	if (parms.verbose) printf("File: %i Bytes Data, %i Bit Depth, %i Hz Samplerate.\n", wav_in.len, wav_in.depth, wav_in.srate);	
#endif	

	if (wav_in.depth != 16)
	{
		return(TX_AUDIO_ERR_NOT_16BIT);
	}

	if (wav_in.chans != 1)
	{
		return(TX_AUDIO_ERR_NOT_MONO);
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

	memsize=wav_in.len;
	data = (int16_t *) malloc (memsize);
		
	if (!data)
	{
		return(TX_AUDIO_ERR_ALLOC);
	}

	p=data;
	
	while (wav_in.len>allbytes)
	{	
		bytes = fread(p, 1, min(1024, wav_in.len-allbytes), wav_in.handle);

#ifdef BIG_ENDIAN_MACHINE
//		if (!wav_in.has_host_order) swapbuffer(p, bytes/sizeof(int16_t));
#endif		
		if (bytes<=0)
		{
			free(data);
			//wav_progress_update(0);
		      return (TX_AUDIO_ERR_WAV_READ);
		}
		allbytes+=bytes;
		
		ld_set_progress((float) allbytes/(float)wav_in.len);
		
		p+=(ssize_t) bytes/sizeof(int16_t);
	}
	
	wav_close(wav_in.handle);

	mem=data;
	no_samples=memsize/sizeof(int16_t);
	
//	printf("WAV: data: %08x, size %i, len: %i\n", data, memsize, no_samples);	

	return (TX_AUDIO_SUCCESS);
}
#endif
