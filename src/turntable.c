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

    File: turntable.c

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
		 
    07 Apr 1999: moved to gtk's pseudo C++		
    
    21 Apr 1999: applied new "NO_CLICK" strategy to render_block. 
    
    27 Apr 1999: <config.h> support
    
    09 May 1999: some render_block speedups.
    
    01 Jun 1999: fixed stdout-output. It's a good idea to use bigger
                 buffersizes though..
		 
    02 Jun 1999: rehacked de-clicking code (fade_in/fade_out) for simplicity
                 and reliability.
		 
    20 Jul 1999: implemented the lowpass filter - the code is
                 based on Paul Kellett's "democode" in reso_lop.txt,
		 that I found somewhere on the net.
    
    21 Jul 1999: moved the filter to render_block for speedups
*/

#define GAIN_ADJUSTMENT_VALUE 0.8

#include "tX_gui.h"
#include "turntable.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <fcntl.h>

#ifndef WIN32
#include <sys/soundcard.h>
#endif

#include <stdio.h>
#include <malloc.h>
#include "wav_file.h"
#include <math.h>

#include "endian.h"
#include "tX_types.h"
#include "tX_global.h"
#include "tX_engine.h"

/* For KEEP_DEV_OPEN */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define NON_RT_BUFF 13

Virtual_TurnTable *vtt_new()
{
	Virtual_TurnTable *vtt;
	
	vtt=(Virtual_TurnTable *) malloc(sizeof(Virtual_TurnTable));
	
	vtt->vol_scratch=globals.scratch_vol;
	vtt->vol_loop=1.0-globals.scratch_vol;
	
	vtt->default_speed=globals.vtt_default_speed;
	vtt->speed=globals.vtt_default_speed;
	
	vtt->last_block_recorded=0;
	
	vtt->dev_open=0;
	vtt->devicefd=0;
		
	vtt->mode=MODE_SCRATCH;

#ifdef TX_WRITER_THREAD
	vtt->writer = 0;	
	vtt->tmp_out=NULL;
	vtt->writer_busy=0;
	
	pthread_mutex_init(&vtt->writer_locked, NULL);
	pthread_mutex_init(&vtt->next_write, NULL);	
#endif

	vtt->lowpass_q=globals.lowpass_reso;
	
	return(vtt);
}

int vtt_open_dev(Virtual_TurnTable *vtt, int use_rt_buffsize)
{
#ifdef USE_CONSOLE
	if (verbose) puts("[VTT:open_dev] Trying to open device.");
#endif
	int i=0;
	int p;
#ifndef WIN32
	
#ifdef KEEP_DEV_OPEN
	if (vtt->dev_open) return(0);
#else	
	if (vtt->dev_open) 
	{
#ifdef USE_CONSOLE
	puts("[VTT:open_dev] Error: Device already open.");
#endif
		return (1);
	}
#endif

	if (globals.use_stdout)
	{
		vtt->devicefd=STDOUT_FILENO;
		vtt->dev_open=1;		
		vtt->deviceblocksize=1 << globals.buff_size;
		return(0);
	}

        vtt->devicefd = open(globals.audio_device, O_WRONLY, 0);
	
#ifndef KEEP_DEV_OPEN
	/* setting buffer size */	
	if (use_rt_buffsize)
	{
#endif	
		vtt->buff_cfg=(globals.buff_no<<16) | globals.buff_size;
#ifndef KEEP_DEV_OPEN		
	}	
	else
	{
		/* somewhat more conservative buffersettings to
		 reduce destortion in filedialogs */
		vtt->buff_cfg=(globals.buff_no<<16) | NON_RT_BUFF;	
	}
#endif	
		p=vtt->buff_cfg;
		
		i = ioctl(vtt->devicefd, SNDCTL_DSP_SETFRAGMENT, &p);

        ioctl(vtt->devicefd, SNDCTL_DSP_RESET, 0);		

	/* 16 Bits */
	
        p =  16;

        i +=  ioctl(vtt->devicefd, SOUND_PCM_WRITE_BITS, &p);

	/* MONO */
	
        p =  1;
        i += ioctl(vtt->devicefd, SOUND_PCM_WRITE_CHANNELS, &p);
	
	/* 44.1 khz */

        p =  44100;
        i += ioctl(vtt->devicefd, SOUND_PCM_WRITE_RATE, &p);
		
        i += ioctl(vtt->devicefd, SNDCTL_DSP_GETBLKSIZE, &vtt->deviceblocksize);

#ifdef ENABLE_DEBUG_OUTPUT	
	fprintf(stderr, "[vtt_open_dev()] Deviceblocksize: %i\n", vtt->deviceblocksize);
#endif
	
        ioctl(vtt->devicefd, SNDCTL_DSP_SYNC, 0);

	vtt->dev_open=!i;

#endif // WIN32
        return(i);
}

/* Virtual_TurnTable::close_dev() : Closes audio device */

int vtt_close_dev(Virtual_TurnTable *vtt)
{
#ifndef WIN32
#ifdef KEEP_DEV_OPEN
	return(ioctl(vtt->devicefd, SNDCTL_DSP_POST, 0));
#else
#ifdef USE_CONSOLE
	if (verbose) puts("[VTT:close_dev] Trying to close device.");
#endif
	if (!vtt->dev_open)
	{	
#ifdef USE_CONSOLE
	puts("[VTT:close_dev] Error: Device not open.");
#endif	
		return(1);		
	}

	if (!globals.use_stdout)
	{
		close(vtt->devicefd);
	}
	vtt->devicefd=0;
	vtt->dev_open=0;
	
	return(0);
#endif
#endif // WIN32
}

#ifdef TX_WRITER_THREAD
void *vtt_block_writer(void *tt)
{
	Virtual_TurnTable *vtt;
	
	vtt=(void *) tt;
	
	while (vtt->writer_busy)
	{
		pthread_mutex_lock(&vtt->next_write);
		if (vtt->writer_busy)
		{
			pthread_mutex_lock(&vtt->writer_locked);		
			pthread_mutex_unlock(&vtt->next_write);
			write(vtt->devicefd, vtt->tmp_out, vtt->deviceblocksize);
			free(vtt->tmp_out);
			pthread_mutex_unlock(&vtt->writer_locked);
		}
		else
		{
			pthread_mutex_unlock(&vtt->next_write);
		}
	}	
	pthread_exit(NULL);
}
#endif

/* Virtual_TurnTable::play_block() : plays the given audio buffer size must
   be deviceblocksize.
*/

void vtt_play_block(Virtual_TurnTable *vtt, int16_t *buffer)
{
#ifdef BIG_ENDIAN_MACHINE
#ifndef BIG_ENDIAN_AUDIO
	swapbuffer(buffer, vtt->samples_per_block);
#endif	
#else
#ifdef BIG_ENDIAN_AUDIO
	swapbuffer(buffer, vtt->samples_per_block);
#endif
#endif

#ifdef TX_WRITER_THREAD	
	vtt->tmp_out=(int16_t*) malloc(vtt->deviceblocksize);

	pthread_mutex_lock(&vtt->writer_locked);
	pthread_mutex_unlock(&vtt->next_write);
	memcpy(vtt->tmp_out, buffer, vtt->deviceblocksize);
	pthread_mutex_unlock(&vtt->writer_locked);
	pthread_mutex_lock(&vtt->next_write);
#else
	write(vtt->devicefd, buffer, vtt->deviceblocksize);
#endif	
}

/* Virtual_TurnTable::render_block() : "renders" one block of audio (scratch)
   data into given buffer (size == deviceblocksize)
*/

void vtt_render_block(Virtual_TurnTable *vtt, int16_t *buffer)
{	
	int16_t *ptr;
	
	int sample;
	int fade_in=0;
	int fade_out=0;
	int do_mute=0;
	
	f_prec next_speed;
	f_prec true_pos_a;
	
//	f_prec diff; 	/* dropped diff for speed improvements now this value is stored tp amount_b */
	f_prec amount_a;
	f_prec amount_b;

#ifndef USE_TIME_DISPLAY	
	unsigned int real_pos_a;
#endif

	f_prec sample_a;
	f_prec sample_b;
	
	f_prec sample_res;
	
// 	Now the lowpass vars:	
	f_prec f=vtt->lowpass_freq;
	f_prec adj=vtt->lowpass_gainadj;
	f_prec buf0=vtt->lowpass_buf0;
	f_prec buf1=vtt->lowpass_buf1;
	f_prec a=vtt->lowpass_a;
	f_prec b=vtt->lowpass_b;	
	int lowpass=globals.lowpass_enable;
	
	real_pos_a=vtt->realpos;
	
	if (vtt->speed != vtt->target_speed)
	{
		vtt->target_speed=vtt->speed;
		vtt->speed_step=vtt->target_speed-vtt->real_speed;
		vtt->speed_step/=10.0;
	}
			
	if (vtt->target_speed != vtt->real_speed)
	{
		vtt->real_speed+=vtt->speed_step;
		if ((vtt->speed_step<0) && (vtt->real_speed<vtt->target_speed)) vtt->real_speed=vtt->target_speed;
		else
		if ((vtt->speed_step>0) && (vtt->real_speed>vtt->target_speed)) vtt->real_speed=vtt->target_speed;			
	}

	if (vtt->fade)
	{
		if ((vtt->last_speed==0) && (vtt->real_speed !=0))
		{
			fade_in=1;
			vtt->fade=NEED_FADE_OUT;
		}
	}
	else
	{
		if ((vtt->last_speed!=0) && (vtt->real_speed==0))
		{
			fade_out=1;
			vtt->fade=NEED_FADE_IN;
		}
	}

	if (vtt->mute_scratch != vtt->old_mute)
	{
		if (vtt->mute_scratch)
		{
			fade_out=1; fade_in=0;
			vtt->fade=NEED_FADE_IN;
		}
		else
		{
			fade_in=1; fade_out=0;
			vtt->fade=NEED_FADE_OUT;
		}
		vtt->old_mute=vtt->mute_scratch;
	}
	else
	{
		if (vtt->mute_scratch) do_mute=1;
	}
			
	for (sample=0; sample < vtt->samples_per_block; sample++)
	{
		if ((vtt->real_speed!=0) || (fade_out))
		{
			vtt->pos+=vtt->real_speed;
			if (vtt->pos>vtt->maxpos) vtt->pos-=vtt->maxpos;
			else if (vtt->pos<0) vtt->pos+=vtt->maxpos;
				
			true_pos_a=floor(vtt->pos);
			real_pos_a=(unsigned int) true_pos_a;
								
			amount_b=vtt->pos-true_pos_a;				
			amount_a=1.0-amount_b;				
				
			if (do_mute)
			{
				*buffer=0;
			}
			else
			{
				ptr=&globals.scratch_data[real_pos_a];
				sample_a=(f_prec) *ptr;
			
				if (real_pos_a == globals.scratch_len) 
				{
					sample_b=*globals.scratch_data;
				}
				else
				{
					ptr++;
					sample_b=(f_prec) *ptr;
				}
				
				sample_res=(sample_a*amount_a)+(sample_b*amount_b);
				
				if (lowpass)
				{
					sample_res*=adj;
					buf0 = a * buf0 + f * (sample_res + b * (buf0 - buf1));
					buf1 = a * buf1 + f * buf0;
					sample_res=buf1;
				}
				
				if (fade_in)
				{
					sample_res*=(((f_prec)sample)/vtt->f_samples_per_block);
				}
				else
				if (fade_out)
				{
					sample_res*=1.0-(((f_prec) sample)/vtt->f_samples_per_block);
				}
 
				*buffer=(int16_t) sample_res;
			}
		}
		else
		{
				*buffer=0;
		}
		buffer++;
	}
	vtt->realpos = real_pos_a;
	vtt->last_speed = vtt->real_speed;
	vtt->lowpass_buf0=buf0;
	vtt->lowpass_buf1=buf1;
}

/* Virtual_TurnTable::needle_down() : Starts playback */

int vtt_needle_down(Virtual_TurnTable *vtt)
{
	int result;
	
	vtt->pos=0;
	vtt->maxpos=(f_prec) globals.scratch_size / 2.0;

	vtt->mix_pos=globals.loop_data;
	vtt->mix_max=globals.loop_data+(size_t) globals.loop_len;
	vtt->store_pos=globals.rec_buffer;
	vtt->fade=NEED_FADE_OUT;

	vtt->speed=globals.vtt_default_speed;
	vtt->real_speed=globals.vtt_default_speed;
	vtt->target_speed=globals.vtt_default_speed;
	vtt->speed_step=0;
#ifdef TX_WRITER_THREAD	
	pthread_mutex_lock(&vtt->next_write);
	
	vtt->writer_busy=1;
	if (!pthread_create(&vtt->writer, NULL, vtt_block_writer, (void *) vtt))
	{
		puts("failed");
	}
#endif	

	vtt->lowpass_freq=0.99;
	vtt_lowpass_conf(vtt, vtt->lowpass_freq, vtt->lowpass_q);
	
#ifdef HANDLE_UNSTOP	
	vtt->fade=0;
#endif

	result = vtt_open_dev(vtt, 1);	
	
	if (result) 
	{
#ifdef USE_CONSOLE
		puts("[VTT:needle_down] Error: Failed opening device. Fatal. Giving up.");
#endif		
		return(1);
	}

#ifdef USE_CONSOLE
	if (verbose) printf("[VTT:needle_down] Using Blocksize of %i Bytes.\n", deviceblocksize);
#endif
	vtt->samples_per_block=vtt->deviceblocksize/sizeof(int16_t);
	vtt->f_samples_per_block=(f_prec) vtt->samples_per_block;
	
	vtt->samplebuffer=(int16_t *) malloc(vtt->deviceblocksize);

	if (!vtt->samplebuffer)
	{
#ifdef USE_CONSOLE
		puts("[VTT:needle_down] Error. Failed to allocate sample_buffer. Fatal. Giving up.");
#endif
		return(1);
	}
	
	vtt->store_pos=globals.rec_buffer;
	vtt->mix_pos=globals.loop_data;
	vtt->block_ctr=0;
	vtt->block_max=globals.rec_size/vtt->deviceblocksize;

	vtt->mute_scratch=0;
	vtt->old_mute=0;
	
	return(0);
}

/* Virtual_TurnTable::needle_up() : Stops playback. */

void vtt_needle_up(Virtual_TurnTable *vtt)
{
	int result;
	void *ptr;

#ifdef TX_WRITER_THREAD	
	vtt->writer_busy=0;
	pthread_mutex_unlock(&vtt->next_write);
	pthread_join(vtt->writer, &ptr);
	vtt->writer=0;
	vtt->tmp_out=NULL;
#endif	
	
	free(vtt->samplebuffer);	
	
	result = vtt_close_dev(vtt);	
	
#ifdef USE_CONSOLE
	if (result)
	puts("[VTT:needle_up] Error: Failed closing device.");
#endif	
	
}

/* Virtual_TurnTable::set_speed() : Sets the rotation speed of the
   Virtual TurnTable. Rarely used as writing to speed directly is 
   faster (and uglier). 
*/

void vtt_set_speed(Virtual_TurnTable *vtt, f_prec targetspeed)
{
#ifdef FUSE_CONSOLE
	if (verbose) printf("[VTT:set_speed] Speed setup: %f.\n", targetspeed);
#endif
	vtt->speed=targetspeed;
}

/* Virtual_TurnTable::set_mode() : Just what it says ..*/

void vtt_set_mode(Virtual_TurnTable *vtt, int newmode)
{
	vtt->mode=newmode;
}

/* Virtual_TurnTable::get_next_storage_block() : Returns a pointer to the next
   block in recordbuffer.
*/

int16_t * vtt_get_next_storage_block(Virtual_TurnTable *vtt)
{
	if (vtt->block_ctr)
	{
		vtt->block_ctr++;
		if (vtt->block_ctr > vtt->block_max) return(NULL);
		vtt->store_pos+=(size_t) vtt->samples_per_block;	
		return(vtt->store_pos);
	}
	else
	{
		vtt->block_ctr++;
		return(vtt->store_pos);
	}

}

void vtt_lowpass_setfreq (Virtual_TurnTable *vtt, f_prec freq_adj)
{
	if (freq_adj > 0)
	{
		vtt->lowpass_freq+=freq_adj;
		if (vtt->lowpass_freq >= 0.99) vtt->lowpass_freq=0.99;
	}
	else
	{
		vtt->lowpass_freq+=freq_adj;
		if (vtt->lowpass_freq<0) vtt->lowpass_freq=0;
	}
	
	vtt->lowpass_a=1.0-vtt->lowpass_freq;
	vtt->lowpass_b=vtt->lowpass_q*(1.0+(1.0/vtt->lowpass_a));
}

void vtt_lowpass_conf (Virtual_TurnTable *vtt, f_prec freq, f_prec q)
{
//	printf("lowpass confed: f %f, %f q\n", freq, q);
	vtt->lowpass_freq=freq;
	vtt->lowpass_q=q;
	
	vtt->lowpass_a=1.0-freq;
	vtt->lowpass_b=q*(1.0+(1.0/vtt->lowpass_a));
	
	vtt->lowpass_gainadj=1.0-q*GAIN_ADJUSTMENT_VALUE;	
}

void vtt_lowpass_block (Virtual_TurnTable *vtt, int16_t *buffer)
{
	int sample;
	f_prec f=vtt->lowpass_freq;
	f_prec adj=vtt->lowpass_gainadj;
	f_prec in;
	f_prec buf0=vtt->lowpass_buf0;
	f_prec buf1=vtt->lowpass_buf1;
	f_prec a=vtt->lowpass_a;
	f_prec b=vtt->lowpass_b;	
	
	for (sample=0; sample < vtt->samples_per_block; sample++, buffer++)
	{
		in=*buffer*adj;	
		buf0 = a * buf0 + f * (in + b * (buf0 - buf1));
		buf1 = a * buf1 + f * buf0;
		*buffer=buf1;
	}
	vtt->lowpass_buf0=buf0;
	vtt->lowpass_buf1=buf1;
}

/* Virtual_TurnTable::() : This triggers all required actions depending
   on current mode.
*/

int vtt_block_action(Virtual_TurnTable *vtt)
{
	int16_t *ptr;

	switch(vtt->mode)
	{
		case MODE_SCRATCH:
			vtt_render_block(vtt, vtt->samplebuffer);
			/*if (globals.lowpass_enable)
			{
				vtt_lowpass_block(vtt, vtt->samplebuffer);
			}*/
			if (globals.do_mix) vtt_add_mix(vtt, vtt->samplebuffer);
			vtt_play_block(vtt, vtt->samplebuffer);
			return(0);

		case MODE_RECORD_SCRATCH:
			ptr=vtt_get_next_storage_block(vtt);
			if (ptr)
			{
				vtt_render_block(vtt, ptr);
				/*if (globals.lowpass_enable)
				{
					vtt_lowpass_block(vtt, ptr);
				}*/
				
				if (globals.do_mix) 
				{
					memcpy(vtt->samplebuffer, ptr, vtt->deviceblocksize);
					vtt_add_mix(vtt, vtt->samplebuffer);
					vtt_play_block(vtt, vtt->samplebuffer);
				}
				else
				{
					vtt_play_block(vtt, ptr);
				}
				return(0);
			}
			else
			{
				return(1);
			}
		
		case MODE_PLAYBACK_RECORDED:
			if (vtt->block_ctr>vtt->last_block_recorded) return (1);
			ptr=vtt_get_next_storage_block(vtt);
			
			if (ptr)
			{
				vtt->realpos = ptr-globals.rec_buffer;
			
				if (globals.do_mix)
				{
					memcpy(vtt->samplebuffer, ptr, vtt->deviceblocksize);
					vtt_add_mix(vtt,vtt->samplebuffer);
					vtt_play_block(vtt, vtt->samplebuffer);
				}
				else
				{
					vtt_play_block(vtt,ptr);
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

void vtt_add_mix(Virtual_TurnTable *vtt, int16_t * buffer)
{
	int sample;
	int16_t *scratch_pos;
	
	f_prec f_val_s, f_val_l;	
	
	for (sample=0, scratch_pos=buffer; sample < vtt->samples_per_block; sample ++, scratch_pos++)
	{
		f_val_l=(f_prec) *vtt->mix_pos;
		f_val_l*=vtt->vol_loop;
		f_val_s=(f_prec) *scratch_pos;
		f_val_s*=vtt->vol_scratch;
		*scratch_pos=(int16_t) (f_val_s + f_val_l);

		vtt->mix_pos++;
		if (vtt->mix_pos>=vtt->mix_max) vtt->mix_pos=globals.loop_data;		
	}
}

/* Virtual_TurnTable::toggle_mix() : Dis- or enable mixing of loop data.*/
/*
void vtt_toggle_mix(Virtual_TurnTable *vtt)
{
	if (globals.loop_data) vtt->do_mix=!vtt->do_mix;
	else
	{
#ifdef USE_CONSOLE
		puts("[VTT:toggle_mix] Error: Can't enable mixing: no loop loaded.");
#endif		
		vtt->do_mix=0;
	}
}
*/
/* Virtual_TurnTable::store_rec_pos() : Stores current recording position
   for later playback or saving.
*/

void vtt_store_rec_pos(Virtual_TurnTable *vtt)
{
	if (vtt->block_ctr > vtt->block_max) vtt->last_block_recorded=vtt->block_max;
	else vtt->last_block_recorded=vtt->block_ctr;
	
	globals.rec_len=vtt->last_block_recorded*vtt->samples_per_block;
}

/* Virtual_TurnTable::() : See above and guess ;) */

void vtt_reset_rec_pos(Virtual_TurnTable *vtt)
{
	vtt->last_block_recorded=0;
	globals.rec_len=0;
}

/* Virtual_TurnTable::save() : Saves a recorded scratch. If the mix == 1
   then with loop data mixed else unmixed raw scratch.
*/

int vtt_save(Virtual_TurnTable *vtt, char* filename, int mix)
{
	wav_sig out_wav;
	int16_t *ptr;
	
	out_wav.srate=44100;
	out_wav.chans=1;
	out_wav.depth=16;
	out_wav.bps=88200;
	out_wav.blkalign=2;
	out_wav.len=vtt->last_block_recorded*vtt->deviceblocksize;
	out_wav.sofar=out_wav.len;
	
//	sprintf(out_wav.name, "%s%02i.wav", file_name, file_ctr);	
	strcpy(out_wav.name, filename);
	
	if (!open_wav_rec(&out_wav))
	{
		return(1);
	}
	
	vtt->block_ctr=0;
	vtt->store_pos=globals.rec_buffer;
	vtt->mix_pos=globals.loop_data;
	
#ifndef BIG_ENDIAN_MACHINE
	if (mix)
#endif	
	vtt->samplebuffer=(int16_t *) malloc(vtt->deviceblocksize);
	
	while (vtt->block_ctr<vtt->last_block_recorded)
	{
		ptr=vtt_get_next_storage_block(vtt);
		if (mix)
		{
			memcpy(vtt->samplebuffer, ptr, vtt->deviceblocksize);
			vtt_add_mix(vtt, vtt->samplebuffer);
#ifdef BIG_ENDIAN_MACHINE
			swapbuffer(vtt->samplebuffer, vtt->samples_per_block);
#endif			
			fwrite(vtt->samplebuffer, vtt->deviceblocksize, 1, out_wav.handle); 
		}
		else
		{
#ifdef BIG_ENDIAN_MACHINE
			memcpy(vtt->samplebuffer, ptr, vtt->deviceblocksize);
			swapbuffer(vtt->samplebuffer, vtt->samples_per_block);
			fwrite(vtt->samplebuffer, vtt->deviceblocksize, 1, out_wav.handle);			
#else
			fwrite(ptr, vtt->deviceblocksize, 1, out_wav.handle);

#endif
		}
		wav_progress_update(((gfloat) vtt->block_ctr)/((gfloat) vtt->last_block_recorded));
	}

#ifndef BIG_ENDIAN_MACHINE	
	if (mix)
#endif	
		free(vtt->samplebuffer);
	
	wav_progress_update(0);
	
	wav_close(out_wav.handle);
	return(0);
}
