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

    Description: Header to turntable.cc

    For a closer desciption of the methods see implementation (tunrtable.cc).
    
    Changes:
    
    20 Mar 1999: Using int16_t instead of short
*/


#ifndef _H_TURNTABLE
#define _H_TURNTABLE

#define MODE_SCRATCH			1
#define MODE_RECORD_SCRATCH		2
#define MODE_PLAYBACK_RECORDED		3

#define f_prec double

#include "tX.h"
#include "tX_types.h"

typedef struct {
	f_prec speed;
	f_prec default_speed;
	int buff_cfg;	
	int bsize;	
	int16_t *playback_data;
	unsigned int playback_size;
	int16_t *record_data;
	unsigned int record_size;	
	int16_t *mix_data;
	unsigned int mix_size;
	char devicename[256];
	char file_name[128];
	int verbose;
	tX_Window *win;
} TT_init;

class Virtual_TurnTable
{

	f_prec real_speed;
	f_prec target_speed;
	f_prec speed_step;
		
	f_prec pos;
	f_prec maxpos;

	int buff_cfg;	

	int x_last;
	f_prec spp;

	int samples_per_block;

	int16_t last_sample;
	int block_ctr;
	int block_max;
	int last_block_recorded;
	int verbose;


	public:

#ifndef USE_OLD_MIX
	f_prec vol_loop;
	f_prec vol_scratch;
#endif
	
	/* Rotation-Speed of Turntable */
	f_prec speed;
	f_prec default_speed;	
	
	char devicename[256];
	int devicefd;
	int deviceblocksize;
	
	tX_Window *win;
	
	int file_ctr;
	char file_name[128];
	char last_fn[128];
	
	int16_t *playback_data;
	unsigned int playback_size;
	
	int16_t *record_data;
	int16_t *store_pos;
	unsigned int record_size;
		
	int16_t *mix_data;
	unsigned int mix_size;
	int16_t *mix_pos;
	int16_t *mix_max;	
	int do_mix;

	/* Dynamic Data */
	
	int dev_open;
	int mode;	
		
	public:
	
	int16_t *samplebuffer;

	Virtual_TurnTable(TT_init *);
	~Virtual_TurnTable();
		
	void set_window(tX_Window *);		

	void needle_up();
	void needle_down();
	
	void play_block(int16_t *);
	int16_t * get_next_storage_block();
	
	void render_block(int16_t *);

	void toggle_mix();	
	void set_mode(int );
	void add_mix(int16_t *);
	
	void store_rec_pos();
	void reset_rec_pos();
	
	void save(int);
	
	int block_action();
	
	void set_speed(f_prec);
		
	int open_dev();
	int close_dev();
};

#endif
