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

#ifdef USE_WRITER_THREAD

void* writer_thread(void *parm)
{
	audiodevice *audio=(audiodevice *) parm;
	int16_t *buffer;

	puts("writer thread");

	pthread_mutex_lock(&audio->write_mutex);
	
	while (pthread_mutex_trylock(&audio->stop_mutex))
	{
	
		pthread_mutex_lock(&audio->buffer_read_mutex);
		pthread_mutex_unlock(&audio->write_mutex);
		pthread_mutex_lock(&audio->buffer_ready_mutex);
		buffer=audio->current_buffer;
		pthread_mutex_unlock(&audio->buffer_read_mutex);
		pthread_mutex_unlock(&audio->buffer_ready_mutex);
		pthread_mutex_lock(&audio->write_mutex);
		
		write(audio->fd, buffer, audio->blocksize);	
	}
	pthread_mutex_unlock(&audio->stop_mutex);
	pthread_mutex_unlock(&audio->write_mutex);
	
	puts("wt quits");
	return NULL;
}

int audiodevice :: eat(int16_t *buffer)
{
	if (pthread_mutex_trylock(&stop_mutex))
	{
		pthread_mutex_lock(&write_mutex);

		current_buffer=buffer;
		pthread_mutex_unlock(&buffer_ready_mutex);	
	
		pthread_mutex_lock(&buffer_read_mutex);
	
		pthread_mutex_lock(&buffer_ready_mutex);
	
		pthread_mutex_unlock(&buffer_read_mutex);
		pthread_mutex_unlock(&write_mutex);	
	}
}

#endif

int audiodevice :: dev_open(int dont_use_rt_buffsize)
{
	int i=0;
	int p;
	int buff_cfg;

	if (fd) return (1);

	if (globals.use_stdout)
	{
		fd=STDOUT_FILENO;
		blocksize=1 << globals.buff_size;
		return(0);
	}

        fd = open(globals.audio_device, O_WRONLY, 0);
	
	/* setting buffer size */	
	if (dont_use_rt_buffsize)
	{
		buff_cfg=(globals.buff_no<<16) | NON_RT_BUFF;	
	}	
	else
	{
		buff_cfg=(globals.buff_no<<16) | globals.buff_size;
	}
	
	p=buff_cfg;
		
	i = ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &p);

        ioctl(fd, SNDCTL_DSP_RESET, 0);		

	/* 16 Bits */
	
        p =  16;

        i +=  ioctl(fd, SOUND_PCM_WRITE_BITS, &p);

	/* MONO */
	
        p =  1;
        i += ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &p);
	
	/* 44.1 khz */

        p =  44100;
        i += ioctl(fd, SOUND_PCM_WRITE_RATE, &p);
		
        i += ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &blocksize);
	
        ioctl(fd, SNDCTL_DSP_SYNC, 0);

#ifdef USE_WRITER_THREAD
	
	puts("A");
	pthread_mutex_lock(&stop_mutex);
	puts("A");
	pthread_mutex_trylock(&buffer_ready_mutex);
	puts("A");
	pthread_create(&writer, NULL, writer_thread, (void *) this);
#endif	
	
        return(i);	
}

int audiodevice :: getblocksize()
{
	return(blocksize);
}

int audiodevice :: dev_close()
{
	void *dummy;

#ifdef USE_WRITER_THREAD
	pthread_mutex_unlock(&buffer_ready_mutex);
	pthread_mutex_unlock(&stop_mutex);
	
	pthread_join(writer, &dummy);	
	puts("okidoki");
#endif	

	if (!fd)
	{	
		return(1);		
	}

	if (!globals.use_stdout)
	{
		close(fd);
	}
	fd=0;
	blocksize=0;
	
	return(0);	
}

audiodevice :: audiodevice()
{
	fd=0;
	blocksize=0;
#ifdef USE_WRITER_THREAD	
	pthread_mutex_init(&stop_mutex, NULL);
	pthread_mutex_init(&write_mutex, NULL);
	pthread_mutex_init(&buffer_read_mutex, NULL);
	pthread_mutex_init(&buffer_ready_mutex, NULL);
#endif	
}

#ifndef USE_WRITER_THREAD
int audiodevice :: eat(int16_t *buffer)
{
	write(fd, buffer, blocksize);	
}
#endif
