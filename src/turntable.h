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

    File: turntable.h

    Description: Header to turntable.c

    For a closer desciption of the methods see implementation (tunrtable.cc).
    
    Changes:
    
    20 Mar 1999: Using int16_t instead of short
    
    07 Apr 1999: Moved to gtk's pseudo c++ ;(
*/


#ifndef _H_TURNTABLE
#define _H_TURNTABLE

#define MODE_SCRATCH			1
#define MODE_RECORD_SCRATCH		2
#define MODE_PLAYBACK_RECORDED		3

#define NEED_FADE_OUT 0
#define NEED_FADE_IN 1

#include "tX_types.h"

//#define TX_WRITER_THREAD 1

#ifdef TX_WRITER_THREAD
#include <pthread.h>
#endif

typedef struct 
{
	f_prec real_speed;
	f_prec target_speed;
	f_prec speed_step;
		
	f_prec pos;
	f_prec maxpos;

	f_prec vol_loop;
	f_prec vol_scratch;

	int buff_cfg;	

	int samples_per_block;
	f_prec f_samples_per_block;

	int block_ctr;
	int block_max;
	int last_block_recorded;
	int verbose;

	int realpos;
	int mute_scratch;

	/* Rotation-Speed of Turntable */
	f_prec speed;
	f_prec last_speed;
	f_prec default_speed;	
	
	int devicefd;
	int deviceblocksize;
		
	int16_t *store_pos;
	int16_t *mix_pos;
	int16_t *mix_max;	

	int dev_open;
	int mode;	
		
	int16_t *samplebuffer;

	int fade;
	int old_mute;

#ifdef TX_WRITER_THREAD
	pthread_t writer;
	int16_t *tmp_out;
	int writer_busy;
	
	pthread_mutex_t writer_locked;
	pthread_mutex_t next_write;
#endif

	f_prec lowpass_freq;
	f_prec lowpass_q;
	f_prec lowpass_gainadj;
	
	f_prec lowpass_buf0;
	f_prec lowpass_buf1;
	f_prec lowpass_a;
	f_prec lowpass_b;
	f_prec lowpass_lastin;

	
} Virtual_TurnTable;

extern	Virtual_TurnTable *vtt_new();
		
extern	void vtt_needle_up(Virtual_TurnTable *);
extern	int vtt_needle_down(Virtual_TurnTable *);
	
extern	void vtt_play_block(Virtual_TurnTable *, int16_t *);
extern	int16_t * vtt_get_next_storage_block(Virtual_TurnTable *);
		
extern	void vtt_render_block(Virtual_TurnTable *, int16_t *);

extern	void vtt_toggle_mix(Virtual_TurnTable *);	
extern	void vtt_set_mode(Virtual_TurnTable *, int );
extern	void vtt_add_mix(Virtual_TurnTable *, int16_t *);
	
extern	void vtt_store_rec_pos(Virtual_TurnTable *);
extern	void vtt_reset_rec_pos(Virtual_TurnTable *);
	
extern	int vtt_block_action(Virtual_TurnTable *);
	
extern	void vtt_set_speed(Virtual_TurnTable *, f_prec);
		
extern	int vtt_open_dev(Virtual_TurnTable *, int);
extern	int vtt_close_dev(Virtual_TurnTable *);

extern int vtt_save(Virtual_TurnTable* , char*, int);

extern void vtt_lowpass_conf (Virtual_TurnTable *vtt, f_prec freq, f_prec q);
extern void vtt_lowpass_block (Virtual_TurnTable *vtt, int16_t *buffer);
extern void vtt_lowpass_setfreq (Virtual_TurnTable *vtt, f_prec freq_adj);
#endif
