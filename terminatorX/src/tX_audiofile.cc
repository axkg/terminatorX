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
#include "tX_endian.h"

#ifdef USE_MAD_INPUT
#	include <mad.h>
#	include <sys/types.h>
#	include <unistd.h>
#	ifndef _POSIX_MAPPED_FILES
#		define _POSIX_MAPPED_FILES
#	endif
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <sys/mman.h>
#endif

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

tX_audio_error tx_audiofile :: load(char *p_file_name)
{
	tX_audio_error load_err=TX_AUDIO_ERR_NOT_SUPPORTED;
	
	strcpy(filename, p_file_name);
	
	ld_set_progress(0);
		
	figure_file_type();
	
#ifdef USE_BUILTIN_WAV
	if ((load_err) && (file_type==TX_FILE_WAV)) {
		load_err=load_wav();	
		if (load_err==TX_AUDIO_SUCCESS) return(load_err);
	}
#endif	

#ifdef USE_MAD_INPUT
	if ((load_err) && (file_type==TX_FILE_MPG123)) {
		load_err=load_mad();
		if (load_err==TX_AUDIO_SUCCESS) return(load_err);
	}
#endif	

	
#ifdef USE_MPG123_INPUT
	if ((load_err) && (file_type==TX_FILE_MPG123)) {
		load_err=load_mpg123();
		return(load_err);
	}
#endif	

#ifdef USE_OGG123_INPUT
	if ((load_err) && (file_type==TX_FILE_OGG123)) {
		load_err=load_ogg123();
		return(load_err);
	}
#endif

#ifdef USE_SOX_INPUT
	if (load_err) {
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

tX_audio_error tx_audiofile :: load_piped()
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
		
		if (bytes)
		{
			newb=newbuff();
			if (!newb) return (TX_AUDIO_ERR_ALLOC);
			w->next=newb;
			w=newb;
		}				

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
tX_audio_error tx_audiofile :: load_sox()
{
	char command[PATH_MAX*2];

	sprintf(command, SOX_STR, filename);
	file = popen(command, "r");
	
	if (!file) return TX_AUDIO_ERR_SOX;
	
	return load_piped();
	
}
#endif	

#ifdef USE_MPG123_INPUT
tX_audio_error tx_audiofile :: load_mpg123()
{
	char command[PATH_MAX*2];
	
	sprintf(command, MPG123_STR, filename);
	file = popen(command, "r");
	
	if (!file) return TX_AUDIO_ERR_MPG123;
	
	return load_piped();
}
#endif	

#ifdef USE_OGG123_INPUT
tX_audio_error tx_audiofile :: load_ogg123()
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
tX_audio_error tx_audiofile :: load_wav()
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
#ifdef ENABLE_DEBUG_OUTPUT
	int output=1;
	unsigned char *debug_p=(unsigned char *) p;
#endif	
	while (wav_in.len>allbytes)
	{	
		bytes = fread(p, 1, min(1024, wav_in.len-allbytes), wav_in.handle);

#ifdef ENABLE_DEBUG_OUTPUT
		if (output) { tX_debug("tX_audiofile::load_wav() read %i Bytes [%04x %04x %04x %04x %04x %04x ..]", bytes, (unsigned int) p[0],  (unsigned int) p[1], (unsigned int) p[2], (unsigned int) p[3], (unsigned int) p[4], (unsigned int) p[5]); }
#endif

		if (bytes<=0) {
			free(data);
			return (TX_AUDIO_ERR_WAV_READ);
		}

#ifdef BIG_ENDIAN_MACHINE
		swapbuffer(p, bytes/sizeof(int16_t));
#	ifdef ENABLE_DEBUG_OUTPUT
		if (output) { tX_debug("tX_audiofile::load_wav() swapped %i Bytes [%04x %04x %04x %04x %04x %04x ..]",
		bytes, (unsigned int) p[0],  (unsigned int) p[1], (unsigned int) p[2], (unsigned int) p[3], (unsigned int) p[4], (unsigned int) p[5]); }
#	endif
#endif		

#ifdef ENABLE_DEBUG_OUTPUT
		output=0;
#endif

		allbytes+=bytes;
		
		ld_set_progress((float) allbytes/(float)wav_in.len);
		
		p+=(ssize_t) bytes/sizeof(int16_t);
	}
	
	wav_close(wav_in.handle);

	mem=data;
	no_samples=memsize/sizeof(int16_t);
	
	return (TX_AUDIO_SUCCESS);
}
#endif

#ifdef USE_MAD_INPUT
tX_audio_error tx_audiofile::load_mad() {
	struct stat stat_dat;
	int fd;
	int res;
	void *mp3_file;
	
	fd=open(filename, O_RDONLY);
	if (!fd) return TX_AUDIO_ERR_MAD_OPEN;
	
	if (fstat(fd, &stat_dat) == -1 ||
      stat_dat.st_size == 0) {
		return TX_AUDIO_ERR_MAD_STAT;
	}
	
	mp3_file = mmap(0, stat_dat.st_size, PROT_READ, MAP_SHARED, fd, 0);
	
	if (mp3_file == MAP_FAILED) {
		return TX_AUDIO_ERR_MAD_MMAP;
	}
	
	res=mad_decode((const unsigned char *) mp3_file, stat_dat.st_size);
	
	if (res) {
		return TX_AUDIO_ERR_MAD_DECODE;
	}
	
	if (munmap(mp3_file, stat_dat.st_size) == -1) {
		return TX_AUDIO_ERR_MAD_MUNMAP;
	}
}

#define TX_MAD_BLOCKSIZE 8096

typedef struct {
	const unsigned char *start;
	const unsigned char *end;
	const unsigned char *last_frame;
	bool first_call;
	ssize_t size;
	int16_t *target_buffer;
	unsigned int target_samples;
	unsigned int current_sample;
} tX_mad_buffer;

const unsigned char *last_current=NULL;

static enum mad_flow tX_mad_input(void *data, struct mad_stream *stream) {
	tX_mad_buffer *buffer = (tX_mad_buffer *) data;
	ssize_t bs;
	unsigned int pos;

	if (buffer->first_call) {
		bs=min(TX_MAD_BLOCKSIZE, buffer->size);
		mad_stream_buffer(stream, buffer->start, bs);
		buffer->first_call=false;
		return MAD_FLOW_CONTINUE;
	} else {
		if (!stream->next_frame) return MAD_FLOW_STOP;
		
		pos=stream->next_frame-buffer->start;
		bs=min(TX_MAD_BLOCKSIZE, buffer->size-pos);
		//tX_debug("last: %08x, new %08x, bs: %i, pos: %i",  buffer->last_frame, stream->next_frame, bs, pos);
		
		mad_stream_buffer(stream, stream->next_frame, bs);
		if (stream->next_frame==buffer->last_frame) {
			//tX_debug("tX_mad_input(): No new frame? Stopping.");
			return MAD_FLOW_STOP;
		}
		ld_set_progress((float) pos/(float) buffer->size);
		buffer->last_frame=stream->next_frame;

		return MAD_FLOW_CONTINUE;
	}
	
	
/*	if (buffer->next_block>=buffer->blocks)
		return MAD_FLOW_STOP;
	mad_stream_buffer(stream, buffer->start, buffer->size);
	
	buffer->next_block=buffer->blocks;
	return MAD_FLOW_CONTINUE;
	
	current=&buffer->start[buffer->next_block*TX_MAD_BLOCKSIZE];
	bs=min(TX_MAD_BLOCKSIZE, buffer->size-(buffer->next_block*TX_MAD_BLOCKSIZE));
	tX_debug("tX_mad_input() current %08x, bs %i, diff %i\n", current, bs, current-last_current);
	buffer->next_block++;
	mad_stream_buffer(stream, current, bs);

	ld_set_progress((float) buffer->next_block/(float) buffer->blocks);	
	last_current=current;	
	return MAD_FLOW_CONTINUE;*/
}

static enum mad_flow tX_mad_error(void *data, struct mad_stream *stream, struct mad_frame *frame) {
	tX_mad_buffer *buffer = (tX_mad_buffer *) data;
	tX_error("Error 0x%04x loading via mad: (%s)\n", stream->error, mad_stream_errorstr(stream));
	return MAD_FLOW_CONTINUE;
}

/* From minimad.c of mad */
static inline signed int scale(mad_fixed_t sample) {
#ifdef BIG_ENDIAN_MACHINE
	swap32_inline(&sample);
#endif
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow tX_mad_output(void *data, struct mad_header const *header, struct mad_pcm *pcm) {
	tX_mad_buffer *buffer=(tX_mad_buffer *) data;
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;	

	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];
	
	buffer->target_samples+=nsamples;

	buffer->target_buffer=(int16_t *) realloc(buffer->target_buffer, buffer->target_samples<<1);
	if (!buffer->target_buffer) { 
			tX_error("Failed allocating sample memory!\n");
			return MAD_FLOW_STOP;
	}
		
	while (nsamples--) {
		signed int sample;

		if (nchannels==1) {
			sample=scale(*left_ch++);
		} else {
			double sample_l=(double) (*left_ch++);
			double sample_r=(double) (*right_ch++); 
			double res=(sample_l+sample_r)/2.0;
			mad_fixed_t mad_res=(mad_fixed_t) res;
			
			sample=scale(mad_res);
		}
		
		buffer->target_buffer[buffer->current_sample]=sample;
		buffer->current_sample++;
	}

	return MAD_FLOW_CONTINUE;
}

int tx_audiofile::mad_decode(unsigned char const *start, unsigned long length) {
	tX_mad_buffer buffer;
	struct mad_decoder decoder;
	int result;

	buffer.start  = start;
	buffer.end = &start[length];
	buffer.last_frame = NULL;
	buffer.size = length;
	//buffer.next_block = 0;  
	//buffer.blocks = length/TX_MAD_BLOCKSIZE + (length%TX_MAD_BLOCKSIZE ? 1 : 0);
	buffer.target_buffer = NULL;
	buffer.target_samples = 0;
	buffer.current_sample = 0;
	buffer.first_call=true;

	tX_debug("tx_audiofile::mad_decode() - start %08x, length %i", buffer.start, buffer.size);
	/* configure input, output, and error functions */

	mad_decoder_init(&decoder, &buffer, tX_mad_input, NULL, NULL, tX_mad_output, tX_mad_error, NULL);
	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	if (!result) {
		this->mem=buffer.target_buffer;
		this->no_samples=buffer.target_samples;
	} else {
		if (buffer.target_buffer) free(buffer.target_buffer);
	}
	
	/* release the decoder */
	mad_decoder_finish(&decoder);  

  return result;
}

#endif
