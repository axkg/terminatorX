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
		 
    Changes:
	
	13 Sept 2002 -	Wrote a seperate loading routine to be used with
				libmad, which is significantly better then piping mpg?1?.
				Rewrote load_piped to use realloc() instead - much easier,
				faster and uses less memory - wish I'd known about realloc()
				when coding load_piped() for the first time ;)
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

#ifdef USE_VORBIS_INPUT
#	include <vorbis/codec.h>
#	include <vorbis/vorbisfile.h>
#	include <errno.h>
#endif

tx_audiofile :: tx_audiofile()
{
	mem_type=TX_AUDIO_UNDEFINED;
	file_type=TX_FILE_UNDEFINED;
	file=NULL;
	strcpy(filename,"");
	mem=NULL;
	no_samples=0;
	sample_rate=44100;
}

void tx_audiofile :: figure_file_type()
{
	char *ext;
	
	ext=strrchr(filename, (int) '.');
	
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
//		if (load_err==TX_AUDIO_SUCCESS) return(load_err);
	}
#endif	

#ifdef USE_MAD_INPUT
	if ((load_err) && (file_type==TX_FILE_MPG123)) {
		load_err=load_mad();
		//if (load_err==TX_AUDIO_SUCCESS) return(load_err);
	}
#endif	
	
#ifdef USE_MPG123_INPUT
	if ((load_err) && (file_type==TX_FILE_MPG123)) {
		load_err=load_mpg123();
		//return(load_err);
	}
#endif	

#ifdef USE_VORBIS_INPUT
	if ((load_err) && (file_type==TX_FILE_OGG123)) {
		load_err=load_vorbis();
		//if (load_err==TX_AUDIO_SUCCESS) return(load_err);
	}
#endif
	
#ifdef USE_OGG123_INPUT
	if ((load_err) && (file_type==TX_FILE_OGG123)) {
		load_err=load_ogg123();
		//return(load_err);
	}
#endif

#ifdef USE_SOX_INPUT
	if (load_err) {
		load_err=load_sox();
	}
#endif	

	if (!load_err) {
		printf("samplerate is :%i\n", sample_rate);
	}
	return(load_err);
}

tx_audiofile :: ~tx_audiofile() {
	switch (mem_type) {
		case TX_AUDIO_MMAP: break;
		case TX_AUDIO_LOAD: {
			free(mem);
			break;
		}
	}	

 	if (file) {
		if (mem_type==TX_AUDIO_MMAP) {
			// free mmap
		}
	}
 }
 
#ifdef NEED_PIPED 

tX_audio_error tx_audiofile :: load_piped()
{
	int16_t *data=NULL;
	ssize_t allbytes=0;
	ssize_t prev_bytes;
	ssize_t bytes=1;
	unsigned char buffer[SOX_BLOCKSIZE];
	unsigned char *ptr;
	
	/* Irritating the user... */
	ld_set_progress(0.5);
	mem_type=TX_AUDIO_LOAD;	
	
	while (bytes) {
		bytes = fread(buffer, 1, SOX_BLOCKSIZE, file);
		
		if (bytes>0) {
			prev_bytes=allbytes;
			allbytes+=bytes;
			int16_t *new_data=(int16_t *) realloc(data, allbytes);
			//printf("All: %i, Bytes: %i, new: %08x, old: %08x\n", allbytes, bytes, new_data, data);
			
			if (!new_data) {
				pclose(file); file=NULL;
				if (data) free(data);
				data=NULL;
				return TX_AUDIO_ERR_ALLOC;
			}
			
			data=new_data;
			ptr=(unsigned char *) data;
			memcpy((void *) &ptr[prev_bytes],(void *) buffer, bytes);
		}
	}
	
	pclose(file); file=NULL;
	
	if (!allbytes) {
		// If we get here we read zero Bytes...
		if (data) free(data);
		return TX_AUDIO_ERR_PIPE_READ;
	}
	
	no_samples=allbytes/sizeof(int16_t);
	mem=data;
	
	/* Irritating the user just a little more ;)*/
	ld_set_progress(1.0);
	
	return TX_AUDIO_SUCCESS;
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

	sample_rate=wav_in.srate;
	
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
	
	mem_type=TX_AUDIO_LOAD;
	
	if (munmap(mp3_file, stat_dat.st_size) == -1) {
		return TX_AUDIO_ERR_MAD_MUNMAP;
	}
	
	return TX_AUDIO_SUCCESS;
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
	unsigned int sample_rate;
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
	buffer->sample_rate = pcm->samplerate;
	
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
	buffer.sample_rate=0;

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

	sample_rate=buffer.sample_rate;
  return result;
}

#endif

/* AARG - OGG loading is not threadsafe !*/
size_t tX_ogg_file_size;
long tX_ogg_file_read;

#ifdef USE_VORBIS_INPUT
typedef struct {
  size_t (*read_func) (void *, size_t, size_t, FILE *);
  int  (*seek_func) (void *, long, int);
  int (*close_func) (FILE *);
  long (*tell_func) (FILE *);
} tX_ov_callbacks;

int ogg_seek(void *ptr, long offset, int whence) {
	/* ogg shall not seek ! */
	return -1;
	errno=EBADF;
}

size_t ogg_read(void  *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t ret_val;
	ret_val=fread(ptr, size, nmemb, stream);
	if (ret_val>0) {
		tX_ogg_file_read+=ret_val*size;
		ld_set_progress((double) tX_ogg_file_read/(double) tX_ogg_file_size);	
	}
	return ret_val;
}

#define VORBIS_BUFF_SIZE 4096 /*recommended*/

tX_audio_error tx_audiofile::load_vorbis() {
	/*  VORBIS Callbacks */
	ov_callbacks org_callbacks;
	/* evil casting - to make g++ shut up */
	tX_ov_callbacks *callbacks=(tX_ov_callbacks *) &org_callbacks;
	
	/* buffer */
	char pcmout[VORBIS_BUFF_SIZE];
	char mono[VORBIS_BUFF_SIZE];
	OggVorbis_File vf;
	bool eof=false;
	int current_section=0;
	
	struct stat stat_dat;
		
	callbacks->read_func=ogg_read;
	callbacks->seek_func=ogg_seek;
	callbacks->close_func=fclose;
	callbacks->tell_func=ftell;
	
	file=fopen(filename, "r");
	if (!file) {
		return TX_AUDIO_ERR_WAV_NOTFOUND;		
	}
	
	if (fstat(fileno(file), &stat_dat)  == -1 || stat_dat.st_size == 0) {
		return TX_AUDIO_ERR_MAD_STAT;
	}
	
	tX_ogg_file_size=stat_dat.st_size;
	tX_ogg_file_read=0;
	
	int res=ov_open_callbacks((void *) file, &vf, NULL, 0, org_callbacks);
	if (res<0) {
		fclose(file); 
		file=NULL;
		return TX_AUDIO_ERR_VORBIS_OPEN;		
	}
	
	vorbis_info *vi=ov_info(&vf,-1);
	sample_rate=vi->rate;

	unsigned int channels=vi->channels;	
	unsigned int samples_read=0;
	int16_t* data=NULL;
	size_t bytes=0;
	
	while((!eof) && (!current_section)){
#ifdef BIG_ENDIAN_MACHINE
#	define ENDIANP 1
#else
#	define ENDIANP 0		
#endif
		long ret=ov_read(&vf,pcmout,VORBIS_BUFF_SIZE,ENDIANP,2,1,&current_section);
		if (ret == 0) {
			eof=true;
		} else if (ret < 0) {
		  /* ignore stream errors */
		} else {
			int16_t *new_data;
			
			bytes+=ret;
			new_data=(int16_t *) realloc(data, bytes/channels);
			if (!new_data) {
				if (data) free(data);
				return TX_AUDIO_ERR_ALLOC;
			}
			data=new_data;
			
			int mono_samples=(ret/2)/channels;
			
			int i=0;			
			while (i<mono_samples) {
				int16_t *src=(int16_t *) &pcmout;				
				double value=0.0;
				
				for (int c=0; c<channels; c++) {
					value+=(double) src[i*channels+c];
				}
				value/=(double) channels;
				
				data[samples_read+i]=(int16_t) value;
				i++;
			}
			samples_read+=mono_samples;
			
			//memcpy(&data[last_pos], pcmout, ret);
		}
		//printf("current %i\n", current_section);
    }
	
	ov_clear(&vf);
	
	mem=(int16_t *) data;
	//no_samples=bytes>>1;
	no_samples=samples_read;
	
	return TX_AUDIO_SUCCESS;
}
#endif
