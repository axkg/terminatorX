/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2003  Alexander König
 
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
 
    File: tX_aduiodevice.cc
 
    Description: Implements Audiodevice handling... 
*/    

#define ALSA_PCM_NEW_HW_PARAMS_API

#include "tX_audiodevice.h"
#include "tX_vtt.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <config.h>

#include "tX_endian.h"

#ifndef __USE_XOPEN
#	define __USE_XOPEN // we need this for swab()
#	include <unistd.h>
#	undef __USE_XOPEN
#else
#	include <unistd.h>
#endif

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "tX_engine.h"

tX_audiodevice :: tX_audiodevice() : samples_per_buffer(0),
current_buffer(0), buffer_pos(0)
{
	sample_buffer[0]=NULL;
	sample_buffer[1]=NULL;
	engine=tX_engine::get_instance();
}

void tX_audiodevice :: start() {
	sample_buffer[0]=new int16_t[samples_per_buffer*2];
	sample_buffer[1]=new int16_t[samples_per_buffer*2];
	int current=0;
	
	while (!engine->is_stopped()) {
		current=current ? 0 : 1;
		
		int16_t *current_buffer=sample_buffer[current];
		int16_t *next_buffer=sample_buffer[current ? 0 : 1];
		
		fill_buffer(current_buffer, next_buffer);
		play(current_buffer);
	}
	
	delete [] sample_buffer[0];
	delete [] sample_buffer[1];
}

void tX_audiodevice :: fill_buffer(int16_t *target_buffer, int16_t *next_target_buffer) {
	int vtt_buffer_size=vtt_class::get_mix_buffer_size()<<1;
	int prefill;
	
	while (buffer_pos < samples_per_buffer) {
		int16_t *data=engine->render_cycle();
		
		int rest=samples_per_buffer-(buffer_pos+vtt_buffer_size);		
		
		if (rest>0) {
			memcpy(&target_buffer[buffer_pos], data, samples_per_buffer << 1);
		} else {
			rest*=-1;
					
			memcpy(&target_buffer[buffer_pos], data, (vtt_buffer_size-rest) << 1);
			memcpy(next_target_buffer, &data[vtt_buffer_size-rest], rest << 1);
			prefill=rest;
		}
		
		buffer_pos+=vtt_buffer_size;
	}
	
	buffer_pos=prefill;
}

/* Driver Specific Code follows.. */

#ifdef USE_OSS

int tX_audiodevice_oss :: open()
{
	int i=0;
	int p;
	int buff_cfg;

	if (fd) return (1);
	fd=::open(globals.oss_device, O_WRONLY, 0);
	
	/* setting buffer size */	
	buff_cfg=(globals.oss_buff_no<<16) | globals.oss_buff_size;
	p=buff_cfg;

	tX_debug("tX_adudiodevice_oss::open() - buff_no: %i, buff_size: %i, buff_cfg: %08x", globals.oss_buff_no, globals.oss_buff_size, buff_cfg);
		
	i = ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &p);
	ioctl(fd, SNDCTL_DSP_RESET, 0);		

	/* 16 Bits */
	
	p =  16;
	i +=  ioctl(fd, SOUND_PCM_WRITE_BITS, &p);

	/* STEREO :) */
	
	p =  2;
	i += ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &p);
	
	/* 44.1 khz */

	p =  globals.oss_samplerate;
	i += ioctl(fd, SOUND_PCM_WRITE_RATE, &p);
	
	sample_rate=globals.oss_samplerate;
	
	/* Figure actual blocksize.. */
	
	i += ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &blocksize);

	samples_per_buffer=blocksize/sizeof(int16_t);

	tX_debug("tX_adudiodevice_oss::open() - blocksize: %i, samples_per_buffer: %i", blocksize, samples_per_buffer);
	
	ioctl(fd, SNDCTL_DSP_SYNC, 0);

	return i;
}

int tX_audiodevice_oss :: close()
{
	if (!fd)
	{	
		return(1);		
	}
	::close(fd);
	fd=0;
	blocksize=0;
		
	return 0;
}

tX_audiodevice_oss :: tX_audiodevice_oss() : tX_audiodevice(),
fd(0), blocksize(0) {}

double tX_audiodevice_oss :: get_latency()
{
	return 0;
}

void tX_audiodevice_oss :: play(int16_t *buffer)
{
#ifdef BIG_ENDIAN_MACHINE
	swapbuffer (buffer, samples_per_buffer);
#endif
	int res=write(fd, buffer, blocksize);	
	if (res==-1) {
		tX_error("failed to write to audiodevice: %s", strerror(errno));
		exit(-1);
	}
}

#endif //USE_OSS

#ifdef USE_ALSA

int tX_audiodevice_alsa :: open()
{
	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
	snd_pcm_hw_params_t *hw_params;
	char pcm_name[64];
	char foo[PATH_MAX];
	
	snd_pcm_hw_params_alloca(&hw_params);	
	
	int card;
	int device;
	
	sscanf(globals.alsa_device, "%i-%i: %s", &card, &device, foo);
	sprintf(pcm_name, "hw:%i,%i", card, device);
	
	if (snd_pcm_open(&pcm_handle, pcm_name, stream, 0) < 0) {
		tX_error("ALSA: Failed to access PCM device \"%s\"", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
	
	if (snd_pcm_hw_params_any(pcm_handle, hw_params) < 0) {
		tX_error("ALSA: Failed to configure PCM device \"%s\"", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
  	
	/* Setting INTERLEAVED stereo... */
#ifdef USE_ALSA_MEMCPY
	if (snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
#else
	if (snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_MMAP_INTERLEAVED) < 0) {
#endif	
		tX_error("ALSA: Failed to set interleaved access for PCM device \"%s\"", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
	
	/* Make it 16 Bit LE - we handle converting from BE anyway... */
	if (snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE) < 0) {
		tX_error("ALSA: Error setting 16 Bit sample width for PCM device \"%s\"", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
	
	/* Setting sampling rate */
	unsigned int hw_rate=(unsigned int)globals.alsa_samplerate;
	int dir;
	
	int res = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &hw_rate, &dir);
	
	if (dir != 0) {
		tX_warning("ALSA: The PCM device \"%s\" doesnt support 44100 Hz playback - using %i instead", pcm_name, hw_rate);
	}	

	sample_rate=hw_rate;
	
	/* Using stereo output */
	if (snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 2) < 0) {
		tX_error("ALSA: PCM device \"%s\" does not support stereo operation", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
	
	unsigned int buffer_time=globals.alsa_buffer_time;
	unsigned int period_time=globals.alsa_period_time;
	
	if (snd_pcm_hw_params_set_buffer_time_near(pcm_handle, hw_params, &buffer_time, &dir) < 0) {
		tX_error("ALSA: failed to set the buffer time opf %lu usecs", globals.alsa_buffer_time);
		return -1;
	}

	long unsigned int buffer_size;

	if (snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size) < 0) {
		tX_error("ALSA: failed to retreive buffer size");
		return -1;
	}
	
	tX_debug("ALSA: buffer size is %i", buffer_size);
	
	if (snd_pcm_hw_params_set_period_time_near(pcm_handle, hw_params, &period_time, &dir) < 0) {
		tX_error("ALSA: failed to set period time %i", globals.alsa_period_time);
		return -1;
	}
	
	if (snd_pcm_hw_params_get_period_size(hw_params, &period_size, &dir)<0) {
		tX_error("ALSA: failed to retreive period size");
		return -1;
	}
	
	samples_per_buffer=period_size;
	
	/* Apply all that setup work.. */
	if (snd_pcm_hw_params(pcm_handle, hw_params) < 0) {
		tX_error("ALSA: Failed to apply settings to PCM device \"%s\"", pcm_name);
		snd_pcm_hw_params_free (hw_params);
		return -1;
	}
	
	snd_pcm_hw_params_free (hw_params);
	return 0;
}

int tX_audiodevice_alsa :: close()
{
	snd_pcm_close(pcm_handle);
	
	return 0;
}

double tX_audiodevice_alsa :: get_latency()
{
	return 0;
}


tX_audiodevice_alsa :: tX_audiodevice_alsa() : tX_audiodevice(),
pcm_handle(NULL) {}

void tX_audiodevice_alsa :: play(int16_t *buffer)
{
	snd_pcm_sframes_t pcmreturn;
#ifdef BIG_ENDIAN_MACHINE
	swapbuffer (buffer, samples_per_buffer);
#endif
	
#ifdef USE_ALSA_MEMCPY
	pcmreturn = snd_pcm_writei(pcm_handle, buffer, samples_per_buffer >> 1);
#else	
	pcmreturn = snd_pcm_mmap_writei(pcm_handle, buffer, samples_per_buffer >> 1);
#endif
	
	while (pcmreturn==-EPIPE) {
		snd_pcm_prepare(pcm_handle);
		
#ifdef USE_ALSA_MEMCPY
		pcmreturn = snd_pcm_writei(pcm_handle, buffer, samples_per_buffer >> 1);
#else	
		pcmreturn = snd_pcm_mmap_writei(pcm_handle, buffer, samples_per_buffer >> 1);
#endif
		//tX_warning("ALSA: ** buffer underrun **");
	}
	
	if (pcmreturn<0) {
		printf("snd_pcm_writei says: %s.\n", strerror(-1*pcmreturn));
	}
}

#endif //USE_ALSA
