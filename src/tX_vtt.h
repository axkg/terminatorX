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
 
    File: tX_vtt.h
 
    Description: Header to tX_vtt.cc
    
    08 Dec 1999 - added audiofile support
*/    

#ifndef _h_tx_vtt
#define _h_tx_vtt 1

#include <config.h>

#ifdef DONT_USE_FLASH
#undef USE_FLASH
#else
#define USE_FLASH 1
#endif

#include <list>
#include "tX_types.h"
#include "tX_vttgui.h"
#include <pthread.h>
#include <float.h>
#include <stdio.h>
#include "tX_audiofile.h"
#include "tX_seqpar.h"
#include "tX_vttfx.h"
#include "tX_ladspa.h"

#ifdef USE_3DNOW
#include "3dnow.h"
#endif

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
	static f_prec *mix_buffer_end;
	
	static int16_t *mix_out_buffer;
	static f_prec mix_max_l;
	static f_prec mix_max_r;
	static int samples_in_mix_buffer;
	static pthread_mutex_t render_lock;
	
	static f_prec master_volume;
	static f_prec res_master_volume;
	static f_prec vol_channel_adjust;

	static f_prec saturate_fac;
	static int do_saturate;	
	
	static vtt_class * sync_master;
	static int master_triggered;
	static int master_triggered_at;
	static vtt_class * focused_vtt;
	static int solo_ctr;
	
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
	
	bool control_hidden;
	bool audio_hidden;
	
	/* builtin fx */
	vtt_fx* lp_fx;
	vtt_fx* ec_fx;

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
#ifdef USE_3DNOW
	mmx_t mm_res_volume;
#endif	
	f_prec rel_pitch; // The (user-selected) relative pitch
	f_prec res_pitch;
	
	
	f_prec pan; // The logical pan value -1 left, 0 center, 1 right
	f_prec res_volume_left;
	f_prec res_volume_right;
	
	int autotrigger;
	int loop;
	
	d_prec speed;
	d_prec speed_real;
	d_prec speed_target;
	d_prec speed_step;
	d_prec speed_last;
	int fade_in;
	int fade_out;
	int do_mute;
		
	d_prec pos_f;
	unsigned int pos_i;
	d_prec maxpos;
	
	int mute;
	int res_mute;
	int res_mute_old;
	
	int mix_mute;
	int mix_solo;
	int fade;
	
	/* input control vars */
	int x_control;
	int y_control;

	/* seq par mapping for x/y axis */
	tX_seqpar *x_par;
	tX_seqpar *y_par;
	
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
	f_prec *ec_output_buffer;
	f_prec *ec_delay;
	f_prec *ec_ptr;
	int ec_enable;
	f_prec ec_length;
	f_prec ec_res_length;
	f_prec ec_feedback;
	f_prec ec_pan;
	f_prec ec_volume;
	f_prec ec_volume_left;
	f_prec ec_volume_right;
	
	/* sequenceable parameters */
	tX_seqpar_vtt_speed sp_speed;
	tX_seqpar_vtt_volume sp_volume;
	tX_seqpar_vtt_pitch sp_pitch;
	tX_seqpar_vtt_pan sp_pan;
	tX_seqpar_vtt_trigger sp_trigger;
	tX_seqpar_vtt_loop sp_loop;
	tX_seqpar_vtt_sync_client sp_sync_client;
	tX_seqpar_vtt_sync_cycles sp_sync_cycles;
	tX_seqpar_vtt_lp_enable sp_lp_enable;
	tX_seqpar_vtt_lp_gain sp_lp_gain;
	tX_seqpar_vtt_lp_reso sp_lp_reso;
	tX_seqpar_vtt_lp_freq sp_lp_freq;
	tX_seqpar_vtt_ec_enable sp_ec_enable;
	tX_seqpar_vtt_ec_length sp_ec_length;
	tX_seqpar_vtt_ec_feedback sp_ec_feedback;
	tX_seqpar_vtt_ec_pan sp_ec_pan;
	tX_seqpar_vtt_ec_volume sp_ec_volume;
	tX_seqpar_vtt_mute sp_mute;
	tX_seqpar_spin sp_spin;

	tx_audiofile *audiofile;

	list <vtt_fx *> fx_list;
		
	public:
	/* Methods */		
	vtt_class(int);
	~vtt_class();
	
	/* Parameter setup methods */
	void set_name(char *);
	int set_output_buffer_size(int);

	void set_volume(f_prec);
	void recalc_volume();
	
	void set_pan(f_prec);
	
	void set_pitch(f_prec);
	void recalc_pitch();
	
	void set_autotrigger(int);
	void set_loop(int);
	
	void set_mute(int);
	
	void set_controls(int, int);
	void set_y_input_parameter(tX_seqpar *);
	void set_x_input_parameter(tX_seqpar *);
	
	void lp_reset();
	void lp_set_enable(int);
	void lp_set_gain(f_prec);
	void lp_set_reso(f_prec);
	void lp_set_freq(f_prec);
	void lp_setup(f_prec, f_prec, f_prec);
	
	void ec_set_enable(int);
	void ec_set_length(f_prec);	
	void ec_set_feedback(f_prec);
	void ec_set_volume(f_prec);
	void ec_set_pan(f_prec);
	void ec_clear_buffer();
	
	void set_sync_master(int);		
	void set_sync_client(int, int);
	void set_sync_client_ug(int, int); // and update gui
	
	void set_scratch(int);
	void xy_input(f_prec, f_prec);
	void handle_input(int, f_prec);

	vtt_fx_ladspa * add_effect(LADSPA_Plugin *);
	
	void calc_speed();
	void render();
	void forward_turntable();	
	
	static int16_t *render_all_turntables();
	static void forward_all_turntables();
	
	static int set_mix_buffer_size(int);
	static void set_master_volume(f_prec);
	static void set_master_pitch(f_prec);
	static void enable_saturate(int);
	static void focus_no(int);
	static void focus_next();
	static void unfocus();	
	int trigger();
	
	int stop();
	int stop_nolock();
	
	int save(FILE *);
	int load_10(FILE *);
	int load_11(FILE *);
	int load_12(FILE *);
	int load_13(FILE *);
	int load_14(FILE *);
	
	static int load_all_10(FILE *, char *fname); /* fname is for display only*/
	static int load_all_11(FILE *, char *fname); /* fname is for display only*/
	static int load_all_12(FILE *, char *fname); /* fname is for display only*/
	static int load_all_13(FILE *, char *fname); /* fname is for display only*/
	static int load_all_14(FILE *, char *fname); /* fname is for display only*/
	static int save_all(FILE *);
	int load_file(char *name);	

	void render_scratch();
	void render_lp();
	void render_ec();
	
	void effect_up(vtt_fx *effect);
	void effect_down(vtt_fx *effect);
	void effect_remove(vtt_fx_ladspa *effect);
	
	void hide_audio(bool);
	void hide_control(bool);
	
	void set_mix_mute(int newstate);
	void set_mix_solo(int newstate);
	void calc_mute()
	{
		res_mute=((mute) || (mix_mute && (!mix_solo)) || ((solo_ctr>0)&&(!mix_solo)));
	}
};

#endif
