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
//	puts (globals.audio_device);
	
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

	/* STEREO :) */
	
        p =  2;
        i += ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &p);
	
	/* 44.1 khz */

        p =  44100;
        i += ioctl(fd, SOUND_PCM_WRITE_RATE, &p);
		
        i += ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &blocksize);
	samples=blocksize/sizeof(int16_t);	
	globals.true_block_size=samples/2;
	
//	printf("bs: %i, samples: %i, tbs: %i\n", blocksize,samples,globals.true_block_size);
        ioctl(fd, SNDCTL_DSP_SYNC, 0);

        return(i);	
}

int audiodevice :: getblocksize()
{
	return(blocksize);
}

int audiodevice :: dev_close()
{
	void *dummy;

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
}

void audiodevice :: eat(int16_t *buffer)
{
#ifdef BIG_ENDIAN_MACHINE
	swapbuffer (buffer, samples);
#endif
	write(fd, buffer, blocksize);	
}
