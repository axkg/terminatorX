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
 
    File: tX_aduiodevice.cc
 
    Description: Implements Audiodevice handling... 
*/    

#include "tX_audiodevice.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>

#include "tX_endian.h"

void tX_audiodevice :: init()
{
	samples_per_buffer=0;
	set_buffersize_near(globals.audiodevice_buffersize);
}


void tX_audiodevice :: set_latency_near(int milliseconds)
{
	samples_per_buffer=(int) (((float) milliseconds) * 88.2);
}

int tX_audiodevice :: get_latency()
{
	return ((int) (((float) samples_per_buffer) / 88.2));
}

void tX_audiodevice :: set_buffersize_near(int samples)
{
	samples_per_buffer=samples;
}

int tX_audiodevice :: get_buffersize()
{
	return samples_per_buffer;
}

int tX_audiodevice :: open()
{
	fprintf(stderr, "tX: Error: tX_audiodevice::dev_open()\n");
	return 1;
}

int tX_audiodevice :: close()
{
	fprintf(stderr, "tX: Error: tX_audiodevice::dev_close()\n");
	return 1;
}

void tX_audiodevice :: play(int16_t* dummy)
{
	fprintf(stderr, "tX: Error: tX_audiodevice::play()\n");
}

#ifdef USE_OSS

/* this is required as open() overloads 
   ansi open() - better solutions anybody? ;)
*/
inline int open_hack(char *name,  int flags, mode_t mode)
{
	return open(name, flags, mode);
}

int tX_audiodevice_oss :: open()
{
	int i=0;
	int p;
	int buff_cfg;

	if (fd) return (1);
        fd = open_hack(globals.audio_device, O_WRONLY, 0);
	
	/* setting buffer size */	
	buff_cfg=(globals.buff_no<<16) | globals.buff_size;
	
	p=buff_cfg;
		
	i = ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &p);

        ioctl(fd, SNDCTL_DSP_RESET, 0);		

	/* 16 Bits */
	
        p =  16;

        i +=  ioctl(fd, SOUND_PCM_WRITE_BITS, &p);

	/* STEREO :) */
	
        p =  2;
        i += ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &p);
	
	/* 44.1 khz */

        p =  44100;
        i += ioctl(fd, SOUND_PCM_WRITE_RATE, &p);
		
        i += ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &blocksize);
	
	samples_per_buffer=blocksize/sizeof(int16_t);
	globals.true_block_size=samples_per_buffer/2;
	
        ioctl(fd, SNDCTL_DSP_SYNC, 0);

        return(i);	
}

inline int closewrapper(int fd)
{
	return close(fd);
}

int tX_audiodevice_oss :: close()
{
	void *dummy;

	if (!fd)
	{	
		return(1);		
	}
	closewrapper(fd);
	fd=0;
	blocksize=0;
		
	return(0);	
}

tX_audiodevice_oss :: tX_audiodevice_oss()
{
	fd=0;
	blocksize=0;
	init();
}

void tX_audiodevice_oss :: play(int16_t *buffer)
{
#ifdef BIG_ENDIAN_MACHINE
	swapbuffer (buffer, samples);
#endif
	write(fd, buffer, blocksize);	
}

#endif //USE_OSS

#ifdef USE_ALSA

int tX_audiodevice_alsa :: open()
{
	return 1;
}

int tX_audiodevice_alsa :: close()
{
	return 1;
}

tX_audiodevice_alsa :: tX_audiodevice_alsa()
{
	init();
}

void tX_audiodevice_alsa :: play(int16_t *buffer)
{
#ifdef BIG_ENDIAN_MACHINE
	swapbuffer (buffer, samples);
#endif
	/***/
}

#endif //USE_ALSA
