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

    File: turntable.cc

    Description: This implements the virtual turntable. Contains
    all audio related stuff.

    I no longer have -DUSE_ALIASING as I don´t have time to maintain the same
    code twice. Aliasing is now default. 
    
    Changes:
    
    20 Mar 1999: Setting the buffer size in open_device() no
                 happens before the DSP_RESET. This might
		 help fighting latency with some audio drivers.
		 
    20 Mar 1999: If KEEP_DEV_OPEN is defined the audio device
                 will be opened only once and is only closed
		 by termination of the terminatorX-process via
		 the OS. This helps if you have problems with
		 memory fragmentation and the sounddriver fails
		 to malloc the audiobuffers. If your sounddriver
		 causes awfull clicks on opening the device this
		 is nice too. Of course other processes can not access
		 the audio device while terminatorX is running.
		 see open_dev() and close_dev().
		 
    20 Mar 1999: Big endian support
    
    20 Mar 1999: Removed audio clicks. Use -DHANDLE_STOP to re-enable.
                 This is not the solution I want as it produces
		 what I consider corrupt audio.
    
    21 Mar 1999: do_mix is enabled automatically now if mix_data is
                 available.
		 
    23 Mar 1999: new mixing routine allows to set the loop to scratch
                 ratio. Old mixing (is faster) can be re-enabled
		 via -DUSE_OLD_MIX. I consider this code as obsolete
		 if nobody tells me he/she uses that switch I'll drop the
		 old code.
*/

#include "turntable.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <stdio.h>
#include <malloc.h>
#include "wav_file.h"
#include <math.h>

#include "endian.h"
#include "tX_types.h"

/* Virtual_TurnTable::Virtual_TurnTable() : Does what a constructor does...
   intialization.
*/

Virtual_TurnTable :: Virtual_TurnTable(TT_init *init_data)
{
#ifdef USE_CONSOLE
	if (init_data->verbose) puts("[VTT] Initializing.");
#endif	

#ifndef USE_OLD_MIX
	vol_loop=0.5;
	vol_scratch=0.5;
#endif
	speed=init_data->speed;
	strcpy(devicename, init_data->devicename);

	default_speed=init_data->default_speed;
	buff_cfg=init_data->buff_cfg;
	
	playback_data=init_data->playback_data;
	playback_size=init_data->playback_size;
	
	record_data=init_data->record_data;
	record_size=init_data->record_size;

	mix_data=init_data->mix_data;
	mix_size=init_data->mix_size;
	
	mix_pos=mix_data;
	mix_max=mix_data+(size_t) (mix_size/sizeof(int16_t));
	
	last_block_recorded=0;
	
	verbose=init_data->verbose;

	if (!mix_data) do_mix=0; else do_mix=1;
	
	dev_open=0;
	devicefd=0;
	win=init_data->win;
	last_sample=0;
	
	strcpy(file_name, init_data->file_name);
	file_ctr=0;
	
	mode=MODE_SCRATCH;
	
	store_pos=record_data;
	strcpy(last_fn,"");
}

/* Virtual_TurnTable::open_dev() : Opens and configures the audio device */

int Virtual_TurnTable :: open_dev()
{
#ifdef USE_CONSOLE
	if (verbose) puts("[VTT:open_dev] Trying to open device.");
#endif
	int i=0;
	int p;
	
#ifdef KEEP_DEV_OPEN
	if (dev_open) return(0);
#else	
	if (dev_open) 
	{
#ifdef USE_CONSOLE
	puts("[VTT:open_dev] Error: Device already open.");
#endif
		return (1);
	}
#endif

        devicefd = open(devicename, O_WRONLY, 0);
	
	/* setting buffer size */
	
	p=buff_cfg;
		
	i = ioctl(devicefd, SNDCTL_DSP_SETFRAGMENT, &p);

        ioctl(devicefd, SNDCTL_DSP_RESET, 0);		

	/* 16 Bits */
	
        p =  16;
        i +=  ioctl(devicefd, SOUND_PCM_WRITE_BITS, &p);

	/* MONO */
	
        p =  1;
        i += ioctl(devicefd, SOUND_PCM_WRITE_CHANNELS, &p);
	
	/* 44.1 khz */

        p =  44100;
        i += ioctl(devicefd, SOUND_PCM_WRITE_RATE, &p);
	
        i += ioctl(devicefd, SNDCTL_DSP_GETBLKSIZE, &deviceblocksize);
	
        ioctl(devicefd, SNDCTL_DSP_SYNC, 0);

	dev_open=!i;

        return(i);
}

/* Virtual_TurnTable::close_dev() : Closes audio device */

int Virtual_TurnTable :: close_dev()
{
#ifdef KEEP_DEV_OPEN
	return(ioctl(devicefd, SNDCTL_DSP_POST, 0));
#else
#ifdef USE_CONSOLE
	if (verbose) puts("[VTT:close_dev] Trying to close device.");
#endif
	if (!dev_open)
	{	
#ifdef USE_CONSOLE
	puts("[VTT:close_dev] Error: Device not open.");
#endif	
		return(1);		
	}

	close(devicefd);
	devicefd=0;
	dev_open=0;
	
	return(0);
#endif
}

/* Virtual_TurnTable::set_window() : UGLY. Sets the win pointer of
   Virutal_TurnTable. Required for position updates.
*/

void Virtual_TurnTable :: set_window(tX_Window *window)
{
	win=window;
}

/* Virtual_TurnTable::play_block() : plays the given audio buffer size must
   be deviceblocksize.
*/

void Virtual_TurnTable :: play_block(int16_t *buffer)
{
#ifdef BIG_ENDIAN_MACHINE
#ifndef BIG_ENDIAN_AUDIO
	swapbuffer(buffer, samples_per_block);
#endif	
#else
#ifdef BIG_ENDIAN_AUDIO
	swapbuffer(buffer, samples_per_block);
#endif
#endif
	write(devicefd, buffer, deviceblocksize);
}

/* Virtual_TurnTable::render_block() : "renders" one block of audio (scratch)
   data into given buffer (size == deviceblocksize)
*/

void Virtual_TurnTable :: render_block(int16_t *buffer)
{	
	int16_t *ptr;
	
	int sample;
	
	int x_upd;	
	
	f_prec true_pos_a;
	
	f_prec diff;
	f_prec amount_a;
	f_prec amount_b;
	
	unsigned int real_pos_a;

	f_prec sample_a;
	f_prec sample_b;
	
	f_prec sample_res;
	
	if (win) 
	{
		x_upd=(int) (pos/spp);
		if (x_upd!=x_last)
		{
			win->set_pos(x_upd);			
			x_last=x_upd;
		}
	}
		
	if (speed != target_speed)
	{
		target_speed=speed;
		speed_step=target_speed-real_speed;
		speed_step/=10.0;
	}
		
	if (target_speed != real_speed)
	{
		real_speed+=speed_step;
		if ((speed_step<0) && (real_speed<target_speed)) real_speed=target_speed;
		else
		if ((speed_step>0) && (real_speed>target_speed)) real_speed=target_speed;			
	}
		
	for (sample=0; sample < samples_per_block; sample++)
	{
#ifdef HANDLE_STOP	
		if (real_speed!=0)
		{
#endif		
			pos+=real_speed;
			if (pos>maxpos)	pos-=maxpos;
			else if (pos<0) pos+=maxpos;
				
			true_pos_a=floor(pos);
								
			diff=pos-true_pos_a;
				
			amount_b=diff;
			amount_a=1.0-diff;
				
			real_pos_a=(unsigned int) true_pos_a;
				
			ptr=&playback_data[real_pos_a];
			sample_a=(f_prec) *ptr;
			
			if (real_pos_a == playback_size) 
			{
				sample_b=*playback_data;
			}
			else
			{
				ptr++;
				sample_b=(f_prec) *ptr;
			}
				
			sample_res=(sample_a*amount_a)+(sample_b*amount_b);
#ifdef HANDLE_STOP			
			last_sample=(int16_t) sample_res;			
			buffer[sample]=last_sample;
#else
			buffer[sample]=(int16_t) sample_res;
#endif
#ifdef HANDLE_STOP
		}
		else
		{
			if (last_sample==1) last_sample=0;
			if (last_sample!=0)
			{
				last_sample=(int16_t) (((f_prec) last_sample) * 0.9);
				buffer[sample]=last_sample;
			}
			else
			{			
				buffer[sample]=0;
			}
		}
#endif		
	}
}

/* Virtual_TurnTable::needle_down() : Starts playback */

void Virtual_TurnTable :: needle_down()
{
	int result;
	
	spp=(f_prec) win->get_samples_per_pixel();
	pos=0;
	x_last=-1;
	maxpos=(f_prec) playback_size / 2.0;

	real_speed=speed;
	target_speed=speed;
	speed_step=0;
/*	
#ifdef USE_CONSOLE
	puts("[VTT:needle_down] Trying to open device.");
#endif	*/
	result = open_dev();
	
	if (result) 
	{
#ifdef USE_CONSOLE
		puts("[VTT:needle_down] Error: Failed opening device. Fatal. Giving up.");
#endif		
		exit(1);
	}

#ifdef USE_CONSOLE
	if (verbose) printf("[VTT:needle_down] Using Blocksize of %i Bytes.\n", deviceblocksize);
#endif
	samples_per_block=deviceblocksize/sizeof(int16_t);
	
	samplebuffer=(int16_t *) malloc(deviceblocksize);

	if (!samplebuffer)
	{
#ifdef USE_CONSOLE
		puts("[VTT:needle_down] Error. Failed to allocate sample_buffer. Fatal. Giving up.");
#endif
		exit(1);	
	}
	
	store_pos=record_data;
	mix_pos=mix_data;
	block_ctr=0;
	block_max=record_size/deviceblocksize;
}

/* Virtual_TurnTable::needle_up() : Stops playback. */

void Virtual_TurnTable :: needle_up()
{
	int result;
	
	free(samplebuffer);	
	
/*#ifdef USE_CONSOLE
	puts("[VTT:needle_up] Trying to close device.");
#endif	*/
	result = close_dev();
#ifdef USE_CONSOLE
	if (result)
	puts("[VTT:needle_up] Error: Failed closing device.");
#endif	
	
}

/* Virtual_TurnTable::set_speed() : Sets the rotation speed of the
   Virtual TurnTable. Rarely used as writing to speed directly is 
   faster (and uglier). 
*/

void Virtual_TurnTable :: set_speed(f_prec targetspeed)
{
#ifdef FUSE_CONSOLE
	if (verbose) printf("[VTT:set_speed] Speed setup: %f.\n", targetspeed);
#endif
	speed=targetspeed;
}

/* Virtual_TurnTable::set_mode() : Just what it says ..*/

void Virtual_TurnTable :: set_mode(int newmode)
{
	mode=newmode;
}

/* Virtual_TurnTable::get_next_storage_block() : Returns a pointer to the next
   block in recordbuffer.
*/

int16_t * Virtual_TurnTable :: get_next_storage_block()
{
	if (block_ctr)
	{
		block_ctr++;
		if (block_ctr > block_max) return(NULL);
		store_pos+=(size_t) samples_per_block;	
		return(store_pos);
	}
	else
	{
		block_ctr++;
		return(store_pos);
	}

}

/* Virtual_TurnTable::() : This triggers all required actions depending
   on current mode.
*/

int Virtual_TurnTable :: block_action()
{
	int16_t *ptr;

	switch(mode)
	{
		case MODE_SCRATCH:
			render_block(samplebuffer);
			if (do_mix) add_mix(samplebuffer);
			play_block(samplebuffer);
			return(0);

		case MODE_RECORD_SCRATCH:
			ptr=get_next_storage_block();
			if (ptr)
			{
				render_block(ptr);
				if (do_mix) 
				{
					memcpy(samplebuffer, ptr, deviceblocksize);
					add_mix(samplebuffer);
					play_block(samplebuffer);
				}
				else
				{
					play_block(ptr);
				}
				return(0);
			}
			else
			{
				return(1);
			}
		
		case MODE_PLAYBACK_RECORDED:
			if (block_ctr>last_block_recorded) return (1);
			ptr=get_next_storage_block();
			if (ptr)
			{
				if (do_mix)
				{
					memcpy(samplebuffer, ptr, deviceblocksize);
					add_mix(samplebuffer);
					play_block(samplebuffer);
				}
				else
				{
					play_block(ptr);
				}
				return(0);
			}
			else
			{
				return(1);
			}
	}
	return(1);
}

/* Virtual_TurnTable::add_mix() : This mixes the loop file date to the
   given buffer (size = deviceblocksize)
*/

void Virtual_TurnTable :: add_mix(int16_t * buffer)
{
	int sample;
	int16_t *scratch_pos;
	
#ifdef USE_OLD_MIX
	int16_t value;
#else
	f_prec f_val_s, f_val_l;	
#endif
	
	for (sample=0, scratch_pos=buffer; sample < samples_per_block; sample ++, scratch_pos++)
	{
#ifdef USE_OLD_MIX	
		value=(*mix_pos)>>1;
		value+=(*scratch_pos)>>1;
		*scratch_pos=value;
#else
		f_val_l=(f_prec) *mix_pos;
		f_val_l*=vol_loop;
		f_val_s=(f_prec) *scratch_pos;
		f_val_s*=vol_scratch;
		*scratch_pos=(int16_t) (f_val_s + f_val_l);
#endif				
		mix_pos++;
		if (mix_pos>=mix_max) mix_pos=mix_data;		
	}
}

/* Virtual_TurnTable::toggle_mix() : Dis- or enable mixing of loop data.*/

void Virtual_TurnTable :: toggle_mix()
{
	if (mix_data) do_mix=!do_mix;
	else
	{
#ifdef USE_CONSOLE
		puts("[VTT:toggle_mix] Error: Can't enable mixing: no loop loaded.");
#endif		
		do_mix=0;
	}
}

/* Virtual_TurnTable::store_rec_pos() : Stores current recording position
   for later playback or saving.
*/

void Virtual_TurnTable :: store_rec_pos()
{
	if (block_ctr > block_max) last_block_recorded=block_max;
	else last_block_recorded=block_ctr;	
}

/* Virtual_TurnTable::() : See above and guess ;) */

void Virtual_TurnTable :: reset_rec_pos()
{
	last_block_recorded=0;
}

/* Virtual_TurnTable::save() : Saves a recorded scratch. If the mix == 1
   then with loop data mixed else unmixed raw scratch.
*/

void Virtual_TurnTable :: save(int mix)
{
	wav_sig out_wav;
	int16_t *ptr;

	if ((mix) && (!mix_data))
	{
#ifdef USE_CONSOLE
		puts("[VTT:save] Error: Can't save a mixed file: no loop loaded.");
#endif	
		return;
	}
	
	file_ctr++;
	
	out_wav.srate=44100;
	out_wav.chans=1;
	out_wav.depth=16;
	out_wav.bps=88200;
	out_wav.blkalign=2;
	out_wav.len=last_block_recorded*deviceblocksize;
	out_wav.sofar=out_wav.len;
	
	sprintf(out_wav.name, "%s%02i.wav", file_name, file_ctr);	
	strcpy(last_fn, out_wav.name);

#ifdef USE_CONSOLE
	if (!mix)
	{
		printf("[VTT:save] Saving RAW recorded scratch to %s.\n", out_wav.name);
	}
	else
	{
		printf("[VTT:save] Saving MIXED recorded scratch to %s.\n", out_wav.name);
	}
#endif
	
	if (!open_wav_rec(&out_wav))
	{
#ifdef USE_CONSOLE
		puts("[VTT:save] Failed to open output file.");
#endif	
		return;
	}
	
	block_ctr=0;
	store_pos=record_data;
	mix_pos=mix_data;
	
#ifndef BIG_ENDIAN_MACHINE
	if (mix)
#endif	
	samplebuffer=(int16_t *) malloc(deviceblocksize);
	
	while (block_ctr<last_block_recorded)
	{
		ptr=get_next_storage_block();
		if (mix)
		{
			memcpy(samplebuffer, ptr, deviceblocksize);
			add_mix(samplebuffer);
#ifdef BIG_ENDIAN_MACHINE
			swapbuffer(samplebuffer, samples_per_block);
#endif			
			write(out_wav.handle, samplebuffer, deviceblocksize); 
		}
		else
		{
#ifdef BIG_ENDIAN_MACHINE
			memcpy(samplebuffer, ptr, deviceblocksize);
			swapbuffer(samplebuffer, samples_per_block);
			write(out_wav.handle, samplebuffer, deviceblocksize);			
#else
			write(out_wav.handle, ptr, deviceblocksize);

#endif
		}
	}

#ifndef BIG_ENDIAN_MACHINE	
	if (mix)
#endif	
		free(samplebuffer);
	
	close(out_wav.handle);
	
#ifdef USE_CONSOLE
	if (verbose) puts("[VTT:save] Saving done.");
#endif	
}







