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
 
    File: tX_vtt.h
 
    Description: Header to tX_vtt.cc
*/    

#ifndef _h_tx_vtt
#define _h_tx_vtt 1

#ifndef DONT_USE_FLASH
#define USE_FLASH
#endif

#include <list>
#include "tX_types.h"
#include "tX_vttgui.h"
#include <pthread.h>
#include <float.h>
#include <stdio.h>

#define EC_MAX_BUFFER 256000

#define CONTROL_NOTHING 0
#define CONTROL_SCRATCH 1
#define CONTROL_VOLUME 2
#define CONTROL_CUTOFF 3
#define CONTROL_FEEDBACK 4

#define NEED_FADE_OUT 0
#define NEED_FADE_IN 1

#define SAMPLE_MAX    32760.0
#define SAMPLE_BORDER 30000.0


class vtt_class
{
	public:
	/* class vars */
	static int vtt_amount;
	static list <vtt_class *> main_list;
	static list <vtt_class *> render_list;

	static f_prec *mix_buffer;
	static int16_t *mix_out_buffer;
	static f_prec mix_max;
	static int samples_in_mix_buffer;
	static pthread_mutex_t render_lock;
	static pthread_mutex_t main_lock;
	
	static f_prec master_volume;
	static f_prec res_master_volume;
	static f_prec vol_channel_adjust;

	static f_prec saturate_fac;
	static int do_saturate;	
	
	static vtt_class * sync_master;
	static int master_triggered;
	static int master_triggered_at;
	static vtt_class * focused_vtt;
	
	/* the gui */
	vtt_gui gui;
	int have_gui;
	
	/* main object vars */
	char name[256]; // Turntable's name
	char filename[PATH_MAX]; // The corresponding audiofile
	
	int is_playing;
	int is_sync_master;
	int is_sync_client;
	int sync_cycles;
	int sync_countdown;
	int want_stop;
	int sense_cycles;

#ifdef USE_FLASH
	f_prec max_value;
#endif
	
	int16_t *buffer;	// Actual audio data
	int samples_in_buffer;  // No. of samples in audio data
	int do_scratch;
	
	f_prec *output_buffer; 
	f_prec *end_of_outputbuffer;
	f_prec samples_in_outputbuffer;
	f_prec inv_samples_in_outputbuffer;
	
	/* main playback vars */
	f_prec rel_volume; // The (user-selected) relative volume
	f_prec res_volume; // The resulting volume
	f_prec rel_pitch; // The (user-selected) relative pitch
	f_prec res_pitch;
	
	int autotrigger;
	int loop;
	
	d_prec speed;
	d_prec speed_real;
	d_prec speed_target;
	d_prec speed_step;
	d_prec speed_last;
//	d_prec speed_default;	
	
	d_prec pos_f;
	unsigned int pos_i;
	d_prec maxpos;
	
	int mute;
	int mute_old;
	int fade;
	
	/* input control vars */
	int x_control;
	int y_control;
	
	/* lp vars */
	int lp_enable;
	f_prec lp_gain;
	f_prec lp_reso;
	f_prec lp_freq;
	
	f_prec lp_buf0;
	f_prec lp_buf1;
	f_prec lp_a;
	f_prec lp_b;
	f_prec lp_last;
	f_prec lp_autogain;
	f_prec lp_resgain;
	
	/* echo vars */
	f_prec ec_buffer[EC_MAX_BUFFER];
	f_prec *ec_delay;
	f_prec *ec_ptr;
	int ec_enable;
	f_prec ec_length;
	f_prec ec_res_length;
	f_prec ec_feedback;

	public:
	/* Methods */		
	vtt_class(int);
	~vtt_class();
	
	/* Parameter setup methods */
	void set_name(char *);
	void set_file_data(char *, int16_t *, int);
	int set_output_buffer_size(int);

	void set_volume(f_prec);
	void recalc_volume();
	
	void set_pitch(f_prec);
	void recalc_pitch();
	
	void set_autotrigger(int);
	void set_loop(int);
	
	void set_mute(int);
	
	void set_controls(int, int);
	
	void lp_set_enable(int);
	void lp_set_gain(f_prec);
	void lp_set_reso(f_prec);
	void lp_set_freq(f_prec);
	void lp_setup(f_prec, f_prec, f_prec);
	
	void ec_set_enable(int);
	void ec_set_length(f_prec);	
	void ec_set_feedback(f_prec);
	void ec_clear_buffer();
	
	void set_sync_master(int);		
	void set_sync_client(int, int);
	
	void set_scratch(int);
	void xy_input(f_prec, f_prec);
	void handle_input(int, f_prec);
	
	void render();
	
	static int16_t *render_all_turntables();
	static int set_mix_buffer_size(int);
	static void set_master_volume(f_prec);
	static void set_master_pitch(f_prec);
	static int enable_saturate(int);
	static void focus_no(int);
	static void focus_next();
	static void unfocus();	
	int trigger();
	
	int stop();
	int stop_nolock();
	
	int save(FILE *);
	int load(FILE *);
	
	static int load_all(FILE *);
	static int save_all(FILE *);
	private:
	void render_scratch();
	void render_lp();
	void render_ec();
	
};

#endif
