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
 
    File: tX_vtt.cc
 
    Description: This implements the new virtual turntable class. It replaces
		 the old turntable.c from terminatorX 3.2 and earlier. The lowpass
		 filter is based on some sample code by Paul Kellett
		 <paul.kellett@maxim.abel.co.uk>
		 
    08 Dec 1999 - Switched to the new audiofile class		 
*/    
   
#include "tX_vtt.h"
#include "tX_global.h"
#include <stdio.h>
#include "malloc.h"
#include <math.h>
#include "tX_mastergui.h"
#include "tX_sequencer.h"
#include <glib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_3DNOW
#include "3dnow.h"
#endif


#ifdef DEBUG
#define tX_freemem(ptr, varname, comment); fprintf(stderr, "** free() [%s] at %08x. %s.\n", varname, ptr, comment); free(ptr);
#define tX_malloc(ptr, varname, comment, size, type); fprintf(stderr, "**[1/2] malloc() [%s]. Size: %i. %s.\n", varname, size, comment); ptr=type malloc(size); fprintf(stderr, "**[2/2] malloc() [%s]. ptr: %08x.\n", varname, ptr);
#else
#define tX_freemem(ptr, varname, comment); free(ptr);
#define tX_malloc(ptr, varname, comment, size, type); ptr=type malloc(size);
#endif

#include "tX_loaddlg.h"

#define USE_PREFETCH 1

#ifdef USE_PREFETCH
#define my_prefetch(base, index); __asm__  __volatile__ ("prefetch index(%0)\n" : : "r" (base));
#define my_prefetchw(base, index); __asm__  __volatile__ ("prefetchw index(%0)\n" : : "r" (base));
#else
#define my_prefetch(base, index);  /* NOP */;
#define my_prefetchw(base, index); /* NOP */;
#endif

extern void build_vtt_gui(vtt_class *);
extern void gui_set_name(vtt_class *vtt, char *newname);
extern void gui_set_filename(vtt_class *vtt, char *newname);
extern void delete_gui(vtt_class *vtt);
extern void gui_update_display(vtt_class *vtt);
extern void gui_clear_master_button(vtt_class *vtt);
extern void cleanup_vtt(vtt_class *vtt);
extern int vg_get_current_page(vtt_class *vtt);

int vtt_class::vtt_amount=0;
list <vtt_class *> vtt_class::main_list;
list <vtt_class *> vtt_class::render_list;
int16_t* vtt_class::mix_out_buffer=NULL;
f_prec * vtt_class::mix_buffer=NULL;
f_prec * vtt_class::mix_buffer_end=NULL;
int vtt_class::solo_ctr=0;

int vtt_class::samples_in_mix_buffer=0;
pthread_mutex_t vtt_class::render_lock=PTHREAD_MUTEX_INITIALIZER;
f_prec vtt_class::master_volume=1.0;
f_prec vtt_class::res_master_volume=1.0;
f_prec vtt_class::saturate_fac=0.1;
int vtt_class::do_saturate=0;
vtt_class * vtt_class::sync_master=NULL;
int vtt_class::master_triggered=0;
int vtt_class::master_triggered_at=0;
vtt_class * vtt_class::focused_vtt=NULL;
f_prec vtt_class::mix_max_l=0;
f_prec vtt_class::mix_max_r=0;
f_prec vtt_class::vol_channel_adjust=1.0;

#define GAIN_AUTO_ADJUST 0.8

vtt_class :: vtt_class (int do_create_gui)
{	
	vtt_amount++;
	sprintf (name, "Turntable %i", vtt_amount);
	strcpy(filename, "NONE");
	buffer=NULL;
	samples_in_buffer=0;
	
	set_volume(1);
	set_pitch(1);
	
	autotrigger=1;
	loop=1;
	
	is_playing=0;
	is_sync_master=0;
	is_sync_client=0;
	sync_cycles=0,
	sync_countdown=0;
	
	lp_enable=0;
	lp_reso=0.8;
	lp_freq=0.3;
	lp_gain=1;
	lp_setup(lp_gain, lp_reso, lp_freq);
	lp_reset();
	
	ec_enable=0;
	ec_length=0.5;
	ec_feedback=0.3;
	ec_clear_buffer();
	ec_set_length(0.5);
	ec_set_pan(0);
	ec_set_volume(1);
	
	main_list.push_back(this);

	/* "connecting" the seq-parameters */
	
	sp_speed.set_vtt((void *) this);
	sp_volume.set_vtt((void *) this);	
	sp_pitch.set_vtt((void *) this);	
	sp_pan.set_vtt((void *) this);
	sp_trigger.set_vtt((void *) this);	
	sp_loop.set_vtt((void *) this);	
	sp_sync_client.set_vtt((void *) this);	
	sp_sync_cycles.set_vtt((void *) this);	
	sp_lp_enable.set_vtt((void *) this);	
	sp_lp_gain.set_vtt((void *) this);	
	sp_lp_reso.set_vtt((void *) this);	
	sp_lp_freq.set_vtt((void *) this);	
	sp_ec_enable.set_vtt((void *) this);	
	sp_ec_length.set_vtt((void *) this);
	sp_ec_pan.set_vtt((void *) this);
	sp_ec_volume.set_vtt((void *) this);
	sp_ec_feedback.set_vtt((void *) this);		
	sp_mute.set_vtt((void *) this);
	sp_spin.set_vtt((void *) this);

	x_par = &sp_speed;
	y_par = &sp_lp_freq;
	
	lp_fx=new vtt_fx_lp();
	lp_fx->set_vtt((void *) this);
	fx_list.push_back(lp_fx);

	ec_fx=new vtt_fx_ec();
	ec_fx->set_vtt((void *) this);
	fx_list.push_back(ec_fx);
	
	if (do_create_gui)
	{	
		build_vtt_gui(this);
		lp_fx->set_panel_widget(gui.lp_panel->get_widget());	
		ec_fx->set_panel_widget(gui.ec_panel->get_widget());
	}
	else have_gui=0;
		
	set_pan(0);	
	set_master_volume(globals.volume);
	set_output_buffer_size(samples_in_mix_buffer/2);
	
	audiofile = NULL;
	audiofile_pitch_correction=0;
	mix_solo=0;
	mix_mute=0;
	res_mute=mute;
	res_mute_old=0;
	
	audio_hidden=false;
	control_hidden=false;
}

vtt_class :: ~vtt_class()
{
	vtt_fx *effect;
	stop();

	main_list.remove(this);
	if (audiofile) delete audiofile;
	//if (buffer) free(buffer);
	if (output_buffer) tX_freemem(output_buffer, "output_buffer", "vtt Destructor");
	vtt_amount--;
	
	while (fx_list.size())
	{ 
		effect=(*fx_list.begin());
		fx_list.remove(effect);
		delete effect;
	}
	
	delete_gui(this);
}

void vtt_class :: set_name(char *newname)
{
	strcpy(name, newname);
	gui_set_name(this, name);	
}

int vtt_class :: load_file(char *fname)
{
	int res;
	int was_playing=is_playing;
	
	if (is_playing) stop();

	if (audiofile) delete(audiofile);
	
	buffer=NULL;
	samples_in_buffer=0;
	maxpos=0;
	strcpy(filename,"");

	audiofile=new tx_audiofile();
	res=audiofile->load(fname);	
	
	if (res==TX_AUDIO_SUCCESS) {
		buffer=audiofile->get_buffer();
		double file_rate=audiofile->get_sample_rate();
		audiofile_pitch_correction=file_rate/44100.0;
		recalc_pitch();
		samples_in_buffer=audiofile->get_no_samples();
		maxpos=audiofile->get_no_samples();
		strcpy(filename, fname);
		if (was_playing) trigger();
//		printf("Successfully loaded %s, %08x, %i\n", fname, buffer, samples_in_buffer);
	}
	
	if (have_gui)
	{
		gui_update_display(this);
	}
	ec_set_length(ec_length);
	
	return(res);
}

int vtt_class :: set_output_buffer_size(int newsize)
{
	list <vtt_fx *> :: iterator effect;

	if (ec_output_buffer) tX_freemem(ec_output_buffer, "ec_output_buffer", "vtt set_output_buffer_size()");
	tX_malloc(ec_output_buffer, "ec_output_buffer", "vtt set_output_buffer_size()", sizeof(float)*newsize, (float *));

	if (output_buffer) tX_freemem(output_buffer, "output_buffer", "vtt set_output_buffer_size()");
	tX_malloc(output_buffer, "output_buffer", "vtt set_output_buffer_size()", sizeof(float)*newsize, (float *));

	end_of_outputbuffer = output_buffer + newsize; //size_t(sizeof(float)*(newsize));
	
	samples_in_outputbuffer=newsize;
	inv_samples_in_outputbuffer=1.0/samples_in_outputbuffer;

	for (effect=fx_list.begin(); effect != fx_list.end(); effect++)
	{
		(*effect)->reconnect_buffer();
	}
	
	if (output_buffer) return(0);	
	else return(0);
}

void vtt_class :: set_volume(f_prec newvol)
{
	rel_volume=newvol;
	recalc_volume();
}

void vtt_class :: recalc_volume()
{
	res_volume=rel_volume*res_master_volume;
	f_prec ec_res_volume=res_volume*ec_volume;
	
	if (pan>0.0)
	{
		res_volume_left=(1.0-pan)*res_volume;
		res_volume_right=res_volume;
	}
	else if (pan<0.0)
	{
		res_volume_left=res_volume;
		res_volume_right=(1.0+pan)*res_volume;
	}
	else
	{
		res_volume_left=res_volume_right=res_volume;
	}
	
	if (ec_pan>0.0)
	{
		ec_volume_left=(1.0-ec_pan)*ec_res_volume;
		ec_volume_right=ec_res_volume;
	}
	else if (ec_pan<0.0)
	{
		ec_volume_left=ec_res_volume;
		ec_volume_right=(1.0+ec_pan)*ec_res_volume;
	}
	else
	{
		ec_volume_left=ec_volume_right=ec_res_volume;
	}	
//	printf("vtt_volume: %f, %f, l: %f, r: %f\n", rel_volume, res_volume, res_volume_left, res_volume_right);
	
#ifdef USE_3DNOW
	mm_res_volume.s[0]=mm_res_volume.s[1]=res_volume;
#endif	
}

void vtt_class :: set_pan(f_prec newpan)
{
	pan=newpan;
	recalc_volume();
}

void vtt_class :: set_pitch(f_prec newpitch)
{
	rel_pitch=newpitch;
//	res_pitch=fabs(globals.pitch)*rel_pitch;
	res_pitch=globals.pitch*rel_pitch;
	speed=res_pitch;
	ec_set_length(ec_length);
}

void vtt_class :: recalc_pitch()
{
//	res_pitch=fabs(globals.pitch)*rel_pitch;
	res_pitch=globals.pitch*rel_pitch;
	res_pitch*=audiofile_pitch_correction;
	speed=res_pitch;
	ec_set_length(ec_length);
}

void vtt_class :: set_autotrigger(int newstate)
{
	autotrigger=newstate;
}

void vtt_class :: set_loop(int newstate)
{
	loop=newstate;
}

void vtt_class :: set_mute(int newstate)
{
	mute=newstate;
	calc_mute();
}

void vtt_class :: set_mix_mute(int newstate)
{
	mix_mute=newstate;
	calc_mute();
}

void vtt_class :: set_mix_solo(int newstate)
{
	if (mix_solo && !newstate)
	{
		/* turning it off */
		mix_solo=0;
		solo_ctr--;
	}
	else if (!mix_solo && newstate)
	{
		/* turning it on */
		mix_solo=1;
		solo_ctr++;
	}
	calc_mute();

	/* locking ? */
	list <vtt_class *> :: iterator vtt;
	
	for (vtt=main_list.begin(); vtt!=main_list.end() ; vtt++)
	{
		(*vtt)->calc_mute();
	}
}

void vtt_class :: lp_set_enable (int newstate)
{
	lp_enable=newstate;
	lp_reset();
}

void vtt_class :: lp_reset()
{
	lp_buf0=lp_buf1=0;
}

void vtt_class :: lp_set_gain (f_prec gain)
{
	lp_gain=gain;
	lp_resgain=lp_gain*lp_autogain;
}

void vtt_class :: lp_set_reso(f_prec reso)
{
	lp_reso=reso;
	
	lp_b=reso*(1.0+(1.0/lp_a));
	lp_autogain=1.0-reso*GAIN_AUTO_ADJUST;
	lp_resgain=lp_gain*lp_autogain;
}

void vtt_class :: lp_set_freq(f_prec freq)
{
	lp_freq=freq;
	
	lp_a=0.9999-freq;
	lp_b=lp_reso*(1.0+(1.0/lp_a));
}

void vtt_class :: lp_setup(f_prec gain, f_prec reso, f_prec freq)
{
	lp_freq=freq;
	lp_reso=reso;
	
	lp_a=1.0-freq;
	lp_b=reso*(1.0+(1.0/lp_a));
	
	lp_autogain=1.0-reso*GAIN_AUTO_ADJUST;
	lp_resgain=lp_gain*lp_autogain;
}

void vtt_class :: ec_set_enable(int newstate)
{
	ec_enable=newstate;
	ec_clear_buffer();
}


void vtt_class :: ec_set_pan(f_prec pan)
{
	ec_pan=pan;

	recalc_volume();
}

/* Max length is 1.0 */

void vtt_class :: ec_set_length(f_prec length)
{
	int delay;

	ec_length=length;
	if (res_pitch==0) 
	{
		ec_res_length=length*samples_in_buffer;
	}
	else
	{
		ec_res_length=length*samples_in_buffer/res_pitch;	
	}
	
	if (ec_res_length<0) ec_res_length*=-1;
	
	if (ec_res_length>=EC_MAX_BUFFER)
	{
		ec_res_length=EC_MAX_BUFFER*length;
	}
	
	delay=(int )floor(ec_res_length);
	delay-=2;
	ec_delay=&ec_buffer[delay];
}

void vtt_class :: ec_set_feedback(f_prec feedback)
{
	ec_feedback=feedback;
}


void vtt_class :: ec_set_volume(f_prec volume)
{
	ec_volume=volume;
	recalc_volume();
}

void vtt_class :: ec_clear_buffer()
{
	f_prec *ptr;
	
	for (ptr=ec_buffer; ptr<=ec_delay; ptr++)
	{
		*ptr=0.0;
	}
	ec_ptr=ec_buffer;
}

void vtt_class :: render()
{
	list <vtt_fx *> :: iterator effect;

	if (do_scratch)
	{
		if (sense_cycles>0)
		{
			sense_cycles--;
			if (sense_cycles==0) sp_speed.receive_input_value(0);
		}
	}
	render_scratch();
	
	for (effect=fx_list.begin(); effect != fx_list.end(); effect++)
	{
		if ((*effect)->isEnabled()) (*effect)->run();
	}
}

extern void vg_create_fx_gui(vtt_class *vtt, vtt_fx_ladspa *effect, LADSPA_Plugin *plugin);

vtt_fx_ladspa * vtt_class :: add_effect (LADSPA_Plugin *plugin)
{
	vtt_fx_ladspa *new_effect;
	
	new_effect = new vtt_fx_ladspa (plugin, this);
	pthread_mutex_lock(&render_lock);
	fx_list.push_back(new_effect);
	if (is_playing) new_effect->activate();
	pthread_mutex_unlock(&render_lock);
	vg_create_fx_gui(this, new_effect, plugin);
	
	return new_effect;
}

void vtt_class :: calc_speed()
{
	do_mute=fade_out=fade_in=0;

	if (speed != speed_target)
	{
		speed_target=speed;
		speed_step=speed_target-speed_real;
		speed_step/=10.0;
	}
			
	if (speed_target != speed_real)
	{
		speed_real+=speed_step;
		if ((speed_step<0) && (speed_real<speed_target)) speed_real=speed_target;
		else
		if ((speed_step>0) && (speed_real>speed_target)) speed_real=speed_target;			
	}
	
	if (fade)
	{
		if ((speed_last==0) && (speed_real !=0))
		{
			fade_in=1;
			fade=NEED_FADE_OUT;
		}
	}
	else
	{
		if ((speed_last!=0) && (speed_real==0))
		{
			fade_out=1;
			fade=NEED_FADE_IN;
		}
	}

	speed_last = speed_real;

	if (res_mute != res_mute_old)
	{
		if (res_mute)
		{
			fade_out=1; fade_in=0;
			fade=NEED_FADE_IN;
		}
		else
		{
			fade_in=1; fade_out=0;
			fade=NEED_FADE_OUT;
		}
		res_mute_old=res_mute;
	}
	else
	{
		if (res_mute) do_mute=1;
	}	
}

void vtt_class :: render_scratch()
{
	int16_t *ptr;
	
	int sample;
	
	f_prec pos_a_f;
	
	f_prec amount_a;
	f_prec amount_b;

	f_prec sample_a;
	f_prec sample_b;
	
	f_prec sample_res;
	
	f_prec *out;
	f_prec fade_vol;	

	calc_speed();
					
	for (sample =0,out=output_buffer, fade_vol=0.0; sample < samples_in_outputbuffer;sample++, out++, fade_vol+=inv_samples_in_outputbuffer)
	{
		if ((speed_real!=0) || (fade_out))
		{

			pos_f+=speed_real;

			if (pos_f>maxpos)
			{
				pos_f-=maxpos;
				if (res_pitch>0)
				{
					if (loop)
					{
					if (is_sync_master)
					{
						master_triggered=1;
						master_triggered_at=sample;
					}
					}
					else
					{
						want_stop=1;
					}
					
				}
			}
			else if (pos_f<0)
			{
				pos_f+=maxpos;
				if (res_pitch<0)
				{
					if (loop)
					{
					if (is_sync_master)
					{
						master_triggered=1;
						master_triggered_at=sample;
					}
					}
					else
					{
						want_stop=1;
					}
				}
			}
				
			pos_a_f=floor(pos_f);
			pos_i=(unsigned int) pos_a_f;
								
			amount_b=pos_f-pos_a_f;				
			amount_a=1.0-amount_b;				
				
			if (do_mute)
			{
				*out=0.0;
			}
			else
			{
				ptr=&buffer[pos_i];
				sample_a=(f_prec) *ptr;
			
				if (pos_i == samples_in_buffer) 
				{
					sample_b=*buffer;
				}
				else
				{
					ptr++;
					sample_b=(f_prec) *ptr;
				}
				
				sample_res=(sample_a*amount_a)+(sample_b*amount_b);
								
				if (fade_in)
				{
					sample_res*=fade_vol;
				}
				else
				if (fade_out)
				{
					sample_res*=1.0-fade_vol;
				}
 
				*out=sample_res;
			}
		}
		else
		{
				*out=0;
		}
	}
}	

void vtt_class :: forward_turntable()
{
	int sample;
	double pos_f_tmp;
#ifdef pos_f_test
	int show=0;
	double diff;
#endif

	calc_speed();

	if ((speed_real==0) && (!fade_out)) return;
	
	
	/* following code is problematic as adding speed_real*n is
	  different from adding speed_real n times to pos_f.
	  
	  well it speeds things up quite a bit and double precision
	  seems to do a satisfying job.
	  
	  #define pos_f_test to prove that.
	*/
	
	pos_f_tmp=pos_f+speed_real*samples_in_outputbuffer;
	
	if ((pos_f_tmp > 0) && (pos_f_tmp < maxpos))
	{
#ifdef pos_f_test
		show=1;
#else	
		pos_f=pos_f_tmp;
		return;
#endif		
	}
				
	/* now the slow way ;) */
	
	for (sample =0; sample < samples_in_outputbuffer; sample++)
	{
			pos_f+=speed_real;

			if (pos_f>maxpos)
			{
				pos_f-=maxpos;
				if (res_pitch>0)
				{
					if (loop)
					{
					if (is_sync_master)
					{
						master_triggered=1;
						master_triggered_at=sample;
					}
					}
					else
					{
						want_stop=1;
					}
					
				}
			}
			else if (pos_f<0)
			{
				pos_f+=maxpos;
				if (res_pitch<0)
				{
					if (loop)
					{
					if (is_sync_master)
					{
						master_triggered=1;
						master_triggered_at=sample;
					}
					}
					else
					{
						want_stop=1;
					}
				}
			}
		
	}
#ifdef pos_f_test
	if (show)
	{
		diff=pos_f_tmp-pos_f;
		if (diff!=0) printf("fast: %f, slow: %f, diff: %f, tt: %s\n", pos_f_tmp, pos_f, diff, name);
	}
#endif	
}	

/*
	The following lowpass filter is based on some sample code by
	Paul Kellett <paul.kellett@maxim.abel.co.uk>
*/

void vtt_class :: render_lp()
{
	f_prec *sample;
		
	for (sample = output_buffer; sample<end_of_outputbuffer; sample++)
	{
		lp_buf0 = lp_a * lp_buf0 + lp_freq * ((*sample)*lp_resgain + lp_b * (lp_buf0 - lp_buf1));
		lp_buf1 = lp_a * lp_buf1 + lp_freq * lp_buf0;
		
		*sample=lp_buf1;
	}
}

void vtt_class :: render_ec()
{
	f_prec *sample;
	f_prec *ec_sample;
	int i;

	for (i=0, sample = output_buffer, ec_sample=ec_output_buffer; i<samples_in_outputbuffer; i++, ec_sample++,sample++, ec_ptr++)
	{
		if (ec_ptr>ec_delay) ec_ptr=ec_buffer;
		*ec_sample=(*ec_ptr) *ec_feedback;
		*ec_ptr=*sample+*ec_sample;
	}	
}

int vtt_class :: set_mix_buffer_size(int no_samples)
{
	list <vtt_class *> :: iterator vtt;
	int res=0;
	
//	printf("vtt_class::set_mix_buffer_size(), mix_buffer: %12x, mix_out: %12x, samples: %i\n", mix_buffer, mix_out_buffer, no_samples);
	
	if (mix_buffer) tX_freemem(mix_buffer, "mix_buffer", "vtt set_mix_buffer_size()");
	samples_in_mix_buffer=no_samples*2;

	tX_malloc(mix_buffer, "mix_buffer", "vtt set_mix_buffer_size()", sizeof(float)*samples_in_mix_buffer, (float *));
	mix_buffer_end=mix_buffer+samples_in_mix_buffer;

//	printf("mix_buffer: %12x\n", mix_buffer);
//	printf("mix_samples: %i, out_samples: %i", samples_in_mix_buffer, no_samples);
	
	if (mix_out_buffer) tX_freemem(mix_out_buffer, "mix_out_buffer", "vtt set_mix_buffer_size()");
	tX_malloc(mix_out_buffer, "mix_out_buffer", "vtt set_mix_buffer_size()", sizeof(int16_t)*samples_in_mix_buffer + 4, (int16_t *));

//	printf("mix_out_buffer: %12x\n", mix_out_buffer);
	
	for (vtt=main_list.begin(); vtt!=main_list.end(); vtt++)
	{
		res|=(*vtt)->set_output_buffer_size(no_samples);
	}
	
	if ((!mix_buffer) || (!mix_out_buffer) || res) return(1);
	return(0);
}

int16_t * vtt_class :: render_all_turntables()
{
	list <vtt_class *> :: iterator vtt, next;
	int sample;
	int mix_sample;
	f_prec temp;

#ifdef USE_3DNOW
	mmx_t *mix;
	mmx_t *vtt_buffer;
	int32_t *mix_int;
#endif

#ifdef USE_FLASH
	f_prec max;
	f_prec min;
#ifdef USE_3DNOW
	mmx_t mm_max;
	mmx_t mm_min;
	mmx_t mm_volume;
	mmx_t mm_src1;
	mmx_t mm_src2;

#ifndef OVERRIDE_MOVQ_AUTODETECT
#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#endif /* GCC_VERSION */

#if (GCC_VERSION < 2096)
#warning "*************************"
#warning "* gcc < 2.96            *"
#warning "* assuming working movq *"
#warning "*************************"
#undef GCC_MOVQ_BUG_WORKAROUND
#else
#warning "*************************"
#warning "* gcc >= 2.96           *"
#warning "* using movq-workaround *"
#warning "*************************"
#define GCC_MOVQ_BUG_WORKAROUND 1
#endif /* GCC < 2.96 */
#endif /* OVERRIDE MOVQ AUTODETECVT */
	
#ifdef GCC_MOVQ_BUG_WORKAROUND
	/* REQUIRED DUE TO GCC BUG (2.96-3.0.2) */
	mmx_t *mm_src1_ptr=&mm_src1;
	mmx_t *mm_src2_ptr=&mm_src2;
	mmx_t *mm_volume_ptr=&mm_volume;
	mmx_t *mm_max_ptr=&mm_max;
	mmx_t *mm_min_ptr=&mm_min;
	
#define MM_VAR_ACC(var) (* var ## _ptr)
#define MM_VAR_MOVQ(var) * var ## _ptr
#else
#define MM_VAR_ACC(var) var
#define MM_VAR_MOVQ(var) var
#endif	
	int32_t *temp_int=&mm_max.d[1];
#endif	
#endif	
	
	pthread_mutex_lock(&render_lock);
	
	if (render_list.size()==0)
	{
		for (sample=0; sample<samples_in_mix_buffer; sample++)
		{
			mix_out_buffer[sample]=0;
		}
	}
	else
	{
			vtt=render_list.begin();
			(*vtt)->render();			
			max=(*vtt)->max_value;
			min=max;

#ifndef USE_3DNOW
			for (sample=0, mix_sample=0; sample<(*vtt)->samples_in_outputbuffer; sample++)
			{				
				temp=(*vtt)->output_buffer[sample];
				mix_buffer[mix_sample]=temp*(*vtt)->res_volume_left;
				mix_sample++;
				mix_buffer[mix_sample]=temp*(*vtt)->res_volume_right;
				mix_sample++;
				
				if (temp>max) max=temp;
				else if (temp<min) min=temp;
			}
#else
			MM_VAR_ACC(mm_volume).s[0]=(*vtt)->res_volume_left;
			MM_VAR_ACC(mm_volume).s[1]=(*vtt)->res_volume_right;

			MM_VAR_ACC(mm_max).s[1]=MM_VAR_ACC(mm_max).s[0]=max;
			MM_VAR_ACC(mm_min).s[1]=MM_VAR_ACC(mm_min).s[0]=min;
			
			movq_m2r(MM_VAR_MOVQ(mm_max), mm1);
			movq_m2r(MM_VAR_MOVQ(mm_min), mm2);
							
			movq_m2r(MM_VAR_MOVQ(mm_volume), mm0);
			
			mix=(mmx_t*)mix_buffer;
			
			for (f_prec* src=(*vtt)->output_buffer; mix < (mmx_t*) mix_buffer_end;)
			{
				/* first sample */
				MM_VAR_ACC(mm_src1).s[0]=*src;
				MM_VAR_ACC(mm_src1).s[1]=*src;
					
				/* sample * l/r volume */
				movq_m2r(MM_VAR_MOVQ(mm_src1), mm3);
				pfmul_r2r(mm0, mm3);
				movq_r2m(mm3, *mix);
					
				/* next sample */
				src++, mix++;
				MM_VAR_ACC(mm_src2).s[0]=*src;
				MM_VAR_ACC(mm_src2).s[1]=*src;
					
				/* sample * l/r volume */
				movq_m2r(MM_VAR_MOVQ(mm_src2), mm3);
				pfmul_r2r(mm0, mm3);
				movq_r2m(mm3, *mix);
					
				/* calculating min/max */
				MM_VAR_ACC(mm_src1).s[1]=MM_VAR_ACC(mm_src2).s[0];
				movq_m2r(mm_src1, mm3);
				pfmax_r2r(mm3, mm1);
				pfmin_r2r(mm3, mm2);
				
				src++, mix++;
			}

			movq_r2m(mm1, MM_VAR_MOVQ(mm_max));
			movq_r2m(mm2, MM_VAR_MOVQ(mm_min));
			
			femms();
			
			if (MM_VAR_ACC(mm_max).s[0]>MM_VAR_ACC(mm_max).s[1]) max=MM_VAR_ACC(mm_max).s[0]; else max=MM_VAR_ACC(mm_max).s[1];
			if (MM_VAR_ACC(mm_min).s[0]<MM_VAR_ACC(mm_min).s[0]) min=MM_VAR_ACC(mm_min).s[0]; else min=MM_VAR_ACC(mm_min).s[1];
#endif			
			
			min*=-1.0;
			if (min>max) (*vtt)->max_value=min; else (*vtt)->max_value=max;

			if ((*vtt)->ec_enable)
			{
#ifndef USE_3DNOW			
				for (sample=0, mix_sample=0; sample<(*vtt)->samples_in_outputbuffer; sample++)
				{				
					temp=(*vtt)->ec_output_buffer[sample];
					
					mix_buffer[mix_sample]+=temp*(*vtt)->ec_volume_left;
					mix_sample++;
					mix_buffer[mix_sample]+=temp*(*vtt)->ec_volume_right;
					mix_sample++;
				}
#else
				MM_VAR_ACC(mm_volume).s[0]=(*vtt)->ec_volume_left;
				MM_VAR_ACC(mm_volume).s[1]=(*vtt)->ec_volume_right;
						
				movq_m2r(MM_VAR_MOVQ(mm_volume), mm0);
				mix =(mmx_t*)mix_buffer;

				for (f_prec* src=(*vtt)->ec_output_buffer; mix < (mmx_t*) mix_buffer_end; src++, mix++)
				{
					/* first sample */
					MM_VAR_ACC(mm_src1).s[0]=*src;
					MM_VAR_ACC(mm_src1).s[1]=*src;
				
					/* sample * l/r volume */
					movq_m2r(MM_VAR_MOVQ(mm_src1), mm3);
					pfmul_r2r(mm0, mm3);
				
					/* accumulating complete mix */
					movq_m2r(*mix, mm4);
					pfadd_r2r(mm4, mm3);
					movq_r2m(mm3, *mix);
				}
				femms();
#endif				
			}
			
			if (master_triggered)
			{
				pthread_mutex_unlock(&render_lock);
				for (vtt=main_list.begin(); vtt!=main_list.end(); vtt++)
				{
					if ((*vtt)->is_sync_client)
					{
						if ((*vtt)->sync_countdown)
						{
							(*vtt)->sync_countdown--;
						}
						else
						{
							(*vtt)->sync_countdown=(*vtt)->sync_cycles;
							(*vtt)->trigger();
						}
					}
				}
				pthread_mutex_lock(&render_lock);
			}
			
			vtt=render_list.begin();
			for (vtt++; vtt!=render_list.end(); vtt++)
			{
				(*vtt)->render();					
				max=(*vtt)->max_value;
				min=max;

#ifndef USE_3DNOW
				for (sample=0, mix_sample=0; sample<(*vtt)->samples_in_outputbuffer; sample++)
				{				
					temp=(*vtt)->output_buffer[sample];
					mix_buffer[mix_sample]+=temp*(*vtt)->res_volume_left;
					mix_sample++;					
					mix_buffer[mix_sample]+=temp*(*vtt)->res_volume_right;
					mix_sample++;
				
					if (temp>max) max=temp;
					else if (temp<min) min=temp;
				}
#else
				MM_VAR_ACC(mm_volume).s[0]=(*vtt)->res_volume_left;
				MM_VAR_ACC(mm_volume).s[1]=(*vtt)->res_volume_right;

				MM_VAR_ACC(mm_max).s[1]=MM_VAR_ACC(mm_max).s[0]=max;
				MM_VAR_ACC(mm_min).s[1]=MM_VAR_ACC(mm_min).s[0]=min;
			
				movq_m2r(MM_VAR_MOVQ(mm_max), mm1);
				movq_m2r(MM_VAR_MOVQ(mm_min), mm2);
							
				movq_m2r(MM_VAR_MOVQ(mm_volume), mm0);
				mix=(mmx_t*)mix_buffer;

				for (f_prec* src=(*vtt)->output_buffer; mix < (mmx_t*) mix_buffer_end;)
				{
					/* first sample */
					MM_VAR_ACC(mm_src1).s[0]=*src;
					MM_VAR_ACC(mm_src1).s[1]=*src;
					
					/* sample * l/r volume */
					movq_m2r(MM_VAR_MOVQ(mm_src1), mm3);
					pfmul_r2r(mm0, mm3);
					
					/* accumulating complete mix */
					movq_m2r(*mix, mm4);
					pfadd_r2r(mm4, mm3);
					movq_r2m(mm3, *mix);
					
					/* next sample */
					src++, mix++;
					MM_VAR_ACC(mm_src2).s[0]=*src;
					MM_VAR_ACC(mm_src2).s[1]=*src;
					
					/* sample * l/r volume */
					movq_m2r(MM_VAR_MOVQ(mm_src2), mm3);
					pfmul_r2r(mm0, mm3);

					/* accumulating complete mix */
					movq_m2r(*mix, mm4);
					pfadd_r2r(mm4, mm3);
					movq_r2m(mm3, *mix);
					
					/* calculating min/max */
					MM_VAR_ACC(mm_src1).s[1]=MM_VAR_ACC(mm_src2).s[0];
					movq_m2r(MM_VAR_MOVQ(mm_src1), mm3);
					pfmax_r2r(mm3, mm1);
					pfmin_r2r(mm3, mm2);
					
					src++, mix++;
				}

				movq_r2m(mm1, MM_VAR_MOVQ(mm_max));
				movq_r2m(mm2, MM_VAR_MOVQ(mm_min));
				
				femms();
	
				if (MM_VAR_ACC(mm_max).s[0]>MM_VAR_ACC(mm_max).s[1]) max=MM_VAR_ACC(mm_max).s[0]; else max=MM_VAR_ACC(mm_max).s[1];
				if (MM_VAR_ACC(mm_min).s[0]<MM_VAR_ACC(mm_min).s[0]) min=MM_VAR_ACC(mm_min).s[0]; else min=MM_VAR_ACC(mm_min).s[1];
#endif
				
				min*=-1.0;
				if (min>max) (*vtt)->max_value=min; else (*vtt)->max_value=max;
				
				if ((*vtt)->ec_enable)
				{
#ifndef USE_3DNOW
					for (sample=0, mix_sample=0; sample<(*vtt)->samples_in_outputbuffer; sample++)
					{				
						temp=(*vtt)->ec_output_buffer[sample];
						
						mix_buffer[mix_sample]+=temp*(*vtt)->ec_volume_left;
						mix_sample++;
						mix_buffer[mix_sample]+=temp*(*vtt)->ec_volume_right;
						mix_sample++;
					}
#else
					MM_VAR_ACC(mm_volume).s[0]=(*vtt)->ec_volume_left;
					MM_VAR_ACC(mm_volume).s[1]=(*vtt)->ec_volume_right;
						
					movq_m2r(MM_VAR_MOVQ(mm_volume), mm0);
					mix =(mmx_t*)mix_buffer;

					for (f_prec* src=(*vtt)->ec_output_buffer; mix < (mmx_t*) mix_buffer_end; src++, mix++)
					{
						/* first sample */
						MM_VAR_ACC(mm_src1).s[0]=*src;
						MM_VAR_ACC(mm_src1).s[1]=*src;
				
						/* sample * l/r volume */
						movq_m2r(MM_VAR_MOVQ(mm_src1), mm3);
						pfmul_r2r(mm0, mm3);
				
						/* accumulating complete mix */
						movq_m2r(*mix, mm4);
						pfadd_r2r(mm4, mm3);
						movq_r2m(mm3, *mix);
					}

					femms();
#endif					
				}
			}
			
			/* left */
			
			max=mix_max_l;
			min=max;

			for (sample=0; sample<samples_in_mix_buffer; sample+=2)
			{				
				temp=mix_buffer[sample];
				mix_out_buffer[sample]=(int16_t) temp;
			
				if (temp>max) max=temp;
				else if (temp<min) min=temp;
			}
			
			min*=-1.0;
			if (min>max) mix_max_l=min; else mix_max_l=max;		
			
			/* right */
			
			max=mix_max_r;
			min=max;

			for (sample=1; sample<samples_in_mix_buffer; sample+=2)
			{				
				temp=mix_buffer[sample];
				mix_out_buffer[sample]=(int16_t) temp;
			
				if (temp>max) max=temp;
				else if (temp<min) min=temp;
			}
			
			min*=-1.0;
			if (min>max) mix_max_r=min; else mix_max_r=max;		
			
	}
	master_triggered=0;
		
	vtt=render_list.begin();
	while (vtt!=render_list.end())
	{
		next=vtt;
		next++;
		
		if ((*vtt)->want_stop) (*vtt)->stop_nolock();
		vtt=next;
	}
	pthread_mutex_unlock(&render_lock);
	
	return(mix_out_buffer);
}

void vtt_class :: forward_all_turntables()
{
	list <vtt_class *> :: iterator vtt, next;

	if (render_list.size()>0)
	{
 		 vtt=render_list.begin();
 		 (*vtt)->forward_turntable();			 

 		 if (master_triggered)
 		 {
 			 for (vtt=main_list.begin(); vtt!=main_list.end(); vtt++)
 			 {
 				 if ((*vtt)->is_sync_client)
 				 {
 					 if ((*vtt)->sync_countdown)
 					 {
 						 (*vtt)->sync_countdown--;
 					 }
 					 else
 					 {
 						 (*vtt)->sync_countdown=(*vtt)->sync_cycles;
 						 (*vtt)->trigger();
 					 }
 				 }
 			 }
 		 }

 		 vtt=render_list.begin();
 		 for (vtt++; vtt!=render_list.end(); vtt++)
 		 {
 			 (*vtt)->forward_turntable();
 		 }
 		 
	}
	master_triggered=0;
	vtt=render_list.begin();
	while (vtt!=render_list.end())
	{
		next=vtt;
		next++;
		
		if ((*vtt)->want_stop) (*vtt)->stop_nolock();
		vtt=next;
	}
}


int vtt_class :: trigger()
{
	list <vtt_fx *> :: iterator effect;

	if (!buffer) return (1);
	
	if (!is_playing) pthread_mutex_lock(&render_lock);
	
	if (res_pitch>=0) pos_f=0;
	else pos_f=maxpos;
	fade=NEED_FADE_OUT;
	speed=res_pitch;
	speed_real=res_pitch;
	speed_target=res_pitch;
	want_stop=0;

	/* activating plugins */
	for (effect=fx_list.begin(); effect != fx_list.end(); effect++)
	{
		(*effect)->activate();
	}


#ifdef USE_FLASH
	max_value=0;
#endif
	
	if (is_sync_master)
	{
		master_triggered=1;
		master_triggered_at=0;
	}
	
	if (!is_playing)
	{
		is_playing=1;
	
		if (is_sync_master) 
		{
			render_list.push_front(this);		
		}		
		else
		{
			render_list.push_back(this);
		}
		pthread_mutex_unlock(&render_lock);
	}
	return(0);
}

int vtt_class :: stop_nolock()
{
	list <vtt_fx *> :: iterator effect;

	if (!is_playing) 
	{
		pthread_mutex_unlock(&render_lock);
		return(1);
	}
	render_list.remove(this);
	want_stop=0;

	is_playing=0;

#ifdef USE_FLASH
	max_value=0;
#endif
	cleanup_vtt(this);
	sync_countdown=0;
	
	/* deactivating plugins */
	for (effect=fx_list.begin(); effect != fx_list.end(); effect++)
	{
		(*effect)->deactivate();
	}
	
	return(0);
}

int vtt_class :: stop()
{
	int res;
	
	pthread_mutex_lock(&render_lock);

	res=stop_nolock();

	pthread_mutex_unlock(&render_lock);

	return(res);
}

void vtt_class :: set_sync_master(int master)
{
	if (master)
	{
		if (sync_master) sync_master->set_sync_master(0);
		sync_master=this;
		is_sync_master=1;
	}
	else
	{
		if (sync_master==this) sync_master=0;
		is_sync_master=0;
		gui_clear_master_button(this);
	}
}

void vtt_class :: set_sync_client(int slave, int cycles)
{
	is_sync_client=slave;
	sync_cycles=cycles;
//	sync_countdown=cycles; 
	sync_countdown=0;
}

void vtt_class :: set_sync_client_ug(int slave, int cycles)
{
	set_sync_client(slave, cycles);
}

void vtt_class :: set_master_volume(f_prec new_volume)
{
	list <vtt_class *> :: iterator vtt;

	master_volume=new_volume;
	globals.volume=new_volume;
	
	if (main_list.size()>0)
	{
		vol_channel_adjust=sqrt((f_prec) main_list.size());
		res_master_volume=master_volume/vol_channel_adjust;		
	}
		
	for (vtt=main_list.begin(); vtt!=main_list.end(); vtt++)
	{
		(*vtt)->recalc_volume();
	}
}

void vtt_class :: set_master_pitch(f_prec new_pitch)
{
	list <vtt_class *> :: iterator vtt;
	
	globals.pitch=new_pitch;
	for (vtt=main_list.begin(); vtt!=main_list.end(); vtt++)
	{
		(*vtt)->recalc_pitch();
	}
}

void vtt_class :: enable_saturate (int newstate)
{
	do_saturate=newstate;
}

void vtt_class :: focus_no(int no)
{
	list <vtt_class *> :: iterator vtt;
	int i;

	for (i=0, vtt=main_list.begin(); vtt!=main_list.end(); vtt++, i++)
	{
		if (i==no)
		{
			focused_vtt=(*vtt);
		}
	}
}

void vtt_class :: focus_next()
{
	list <vtt_class *> :: iterator vtt;
	
	if (!focused_vtt)
	{
		if (main_list.size())
		{
			focused_vtt=(*main_list.begin());
		}
		return;
	}
	
	for (vtt=main_list.begin(); vtt!=main_list.end() ; vtt++) {
		if ((*vtt)==focused_vtt) {
			/* Ok, we found ourselves.. */
			
			vtt++;
			while ((vtt!=main_list.end()) && ((*vtt)->audio_hidden) ) {
				vtt++;
			}
			
			if (vtt==main_list.end()) {
				/* No other "focusable" after this vtt so we're looking for the next */
				
				for (vtt=main_list.begin(); vtt!=main_list.end() ; vtt++) {
					if (! (*vtt)->audio_hidden) {
						focused_vtt=(*vtt);
						return;
					}
				}
				/* When we get here there's no "focusable" vtt at all... damn */
				focused_vtt=NULL;
				return;
			} else {
				focused_vtt=(*vtt);
				return;
			}
		}
	}
	
	focused_vtt=(*main_list.begin());
}

void vtt_class :: set_scratch(int newstate)
{
	if (newstate)
	{
		sp_spin.receive_input_value(0);
		do_scratch=1;
		sense_cycles=globals.sense_cycles;
	}
	else
	{
		sp_spin.receive_input_value(1);
		do_scratch=0;
	}
}


void vtt_class :: unfocus()
{
	focused_vtt=NULL;
}

void vtt_class :: set_x_input_parameter(tX_seqpar *sp)
{
	x_par = sp;
}

void vtt_class :: set_y_input_parameter(tX_seqpar *sp)
{
	y_par = sp;
}

void vtt_class :: xy_input(f_prec x_value, f_prec y_value)
{
	if (x_par) x_par->handle_mouse_input(x_value*globals.mouse_speed);
	if (y_par) y_par->handle_mouse_input(y_value*globals.mouse_speed);
}

#define store(data); if (fwrite((void *) &data, sizeof(data), 1, output)!=1) res+=1;

int  vtt_class :: save(FILE * output)
{
	list <vtt_fx *> :: iterator effect;

	int res=0;
	guint32 pid;
	int32_t counter;
	guint8 hidden;
	
	store(name);
	store(filename);
	store(is_sync_master);
	store(is_sync_client);
	store(sync_cycles);
	store(rel_volume);
	store(rel_pitch);
	
	store(autotrigger);
	store(loop);
	
	store(mute);
	store(pan);
	
	store(lp_enable);
	store(lp_gain);
	store(lp_reso);
	store(lp_freq);
	
	store(ec_enable);
	store(ec_length);
	store(ec_feedback);
	store(ec_pan);
	store(ec_volume);
	
	store(audio_hidden);
	store(control_hidden);

	pid=sp_speed.get_persistence_id();
	store(pid);
	pid=sp_volume.get_persistence_id();
	store(pid);
	pid=sp_pitch.get_persistence_id();
	store(pid);
	pid=sp_trigger.get_persistence_id();
	store(pid);
	pid=sp_loop.get_persistence_id();
	store(pid);
	pid=sp_sync_client.get_persistence_id();
	store(pid);
	pid=sp_sync_cycles.get_persistence_id();
	store(pid);
	pid=sp_lp_enable.get_persistence_id();
	store(pid);
	pid=sp_lp_gain.get_persistence_id();
	store(pid);
	pid=sp_lp_reso.get_persistence_id();
	store(pid);
	pid=sp_lp_freq.get_persistence_id();
	store(pid);
	pid=sp_ec_enable.get_persistence_id();
	store(pid);
	pid=sp_ec_length.get_persistence_id();
	store(pid);
	pid=sp_ec_feedback.get_persistence_id();
	store(pid);
	pid=sp_ec_volume.get_persistence_id();
	store(pid);
	pid=sp_ec_pan.get_persistence_id();
	store(pid);
	pid=sp_mute.get_persistence_id();
	store(pid);
	pid=sp_spin.get_persistence_id();
	store(pid);
	pid=sp_pan.get_persistence_id();
	store(pid);
		
	counter=fx_list.size();
	store(counter);

	for (effect=fx_list.begin(); effect!=fx_list.end(); effect++)
	{
		(*effect)->save(output);
	}
	
	if (x_par)
	{
		pid=1;
		store(pid);
		pid=x_par->get_persistence_id();
		store(pid);
	}
	else
	{
		pid=0;
		store(pid);
	}

	if (y_par)
	{
		pid=1;
		store(pid);
		pid=y_par->get_persistence_id();
		store(pid);
	}
	else
	{
		pid=0;
		store(pid);
	}
		
	hidden=gui.main_panel->is_hidden();
	store(hidden);

	hidden=gui.trigger_panel->is_hidden();
	store(hidden);

	hidden=gui.lp_panel->is_hidden();
	store(hidden);

	hidden=gui.ec_panel->is_hidden();
	store(hidden);
	
	return(res);
}

#define atload(data); if (fread((void *) &data, sizeof(data), 1, input)!=1) res+=1;

int vtt_class :: load_10(FILE * input)
{
	int res=0;
	int obsolete_int;
	
	atload(name);
	atload(filename);
	atload(is_sync_master);
	atload(is_sync_client);
	atload(sync_cycles);
	atload(rel_volume);
	recalc_volume();
	atload(rel_pitch);
	recalc_pitch();
	
	atload(autotrigger);
	atload(loop);
	
	atload(mute);
	atload(obsolete_int); //x_control
	atload(obsolete_int); //y_control
	
	atload(lp_enable);
	atload(lp_gain);
	atload(lp_reso);
	atload(lp_freq);
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	atload(ec_enable);
	atload(ec_length);
	ec_set_length(ec_length);
	atload(ec_feedback);
	ec_set_feedback(ec_feedback);
	
	return(res);
}


int vtt_class :: load_11(FILE * input)
{
	int res=0;
	guint32 pid;
	int32_t gui_page;
	int obsolete_int;
	
	atload(name);
	atload(filename);
	atload(is_sync_master);
	atload(is_sync_client);
	atload(sync_cycles);
	atload(rel_volume);
	recalc_volume();
	atload(rel_pitch);
	recalc_pitch();
	
	atload(autotrigger);
	atload(loop);
	
	atload(mute);
	atload(obsolete_int); //x_control
	atload(obsolete_int); //y_control
	
	atload(lp_enable);
	atload(lp_gain);
	atload(lp_reso);
	atload(lp_freq);
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	atload(ec_enable);
	atload(ec_length);
	ec_set_length(ec_length);
	atload(ec_feedback);
	ec_set_feedback(ec_feedback);

	atload(pid);
	sp_speed.set_persistence_id(pid);
	atload(pid);
	sp_volume.set_persistence_id(pid);
	atload(pid);
	sp_pitch.set_persistence_id(pid);
	atload(pid);
	sp_trigger.set_persistence_id(pid);
	atload(pid);
	sp_loop.set_persistence_id(pid);
	atload(pid);
	sp_sync_client.set_persistence_id(pid);
	atload(pid);
	sp_sync_cycles.set_persistence_id(pid);
	atload(pid);
	sp_lp_enable.set_persistence_id(pid);
	atload(pid);
	sp_lp_gain.set_persistence_id(pid);
	atload(pid);
	sp_lp_reso.set_persistence_id(pid);
	atload(pid);
	sp_lp_freq.set_persistence_id(pid);
	atload(pid);
	sp_ec_enable.set_persistence_id(pid);
	atload(pid);
	sp_ec_length.set_persistence_id(pid);
	atload(pid);
	sp_ec_feedback.set_persistence_id(pid);
	atload(pid);
	sp_mute.set_persistence_id(pid);
	atload(pid);
	sp_spin.set_persistence_id(pid);
	
	atload(gui_page);
	
	return(res);
}

int vtt_class :: load_12(FILE * input)
{
	int res=0;
	guint32 pid;
	int32_t counter;
	int32_t type;
	long id;
	int i;
	unsigned int t;
	LADSPA_Plugin *plugin;
	char buffer[256];
	vtt_fx_ladspa *ladspa_effect;
	guint8 hidden;
	
	atload(buffer);
	this->set_name(buffer);
	atload(filename);
	atload(is_sync_master);
	atload(is_sync_client);
	atload(sync_cycles);
	atload(rel_volume);
	recalc_volume();
	atload(rel_pitch);
	recalc_pitch();
	
	atload(autotrigger);
	atload(loop);
	
	atload(mute);
	
	atload(lp_enable);
	atload(lp_gain);
	atload(lp_reso);
	atload(lp_freq);
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	atload(ec_enable);
	atload(ec_length);
	ec_set_length(ec_length);
	atload(ec_feedback);
	ec_set_feedback(ec_feedback);

	atload(pid);
	sp_speed.set_persistence_id(pid);
	atload(pid);
	sp_volume.set_persistence_id(pid);
	atload(pid);
	sp_pitch.set_persistence_id(pid);
	atload(pid);
	sp_trigger.set_persistence_id(pid);
	atload(pid);
	sp_loop.set_persistence_id(pid);
	atload(pid);
	sp_sync_client.set_persistence_id(pid);
	atload(pid);
	sp_sync_cycles.set_persistence_id(pid);
	atload(pid);
	sp_lp_enable.set_persistence_id(pid);
	atload(pid);
	sp_lp_gain.set_persistence_id(pid);
	atload(pid);
	sp_lp_reso.set_persistence_id(pid);
	atload(pid);
	sp_lp_freq.set_persistence_id(pid);
	atload(pid);
	sp_ec_enable.set_persistence_id(pid);
	atload(pid);
	sp_ec_length.set_persistence_id(pid);
	atload(pid);
	sp_ec_feedback.set_persistence_id(pid);
	atload(pid);
	sp_mute.set_persistence_id(pid);
	atload(pid);
	sp_spin.set_persistence_id(pid);
		
	atload(counter);
	
	for (i=0; i<counter; i++)
	{
		atload(type);
		switch(type)
		{
			case TX_FX_BUILTINCUTOFF:
				for (t=0; t<fx_list.size(); t++) effect_down(lp_fx);
			break;
			
			case TX_FX_BUILTINECHO:
				for (t=0; t<fx_list.size(); t++) effect_down(ec_fx);
			break;
			
			case TX_FX_LADSPA:
				atload(id);
				plugin=LADSPA_Plugin::getPluginByUniqueID(id);
				if (plugin)
				{
					ladspa_effect=add_effect(plugin);
					ladspa_effect->load(input);
				}
				else
				{
					sprintf(buffer,"Fatal Error: Couldn't find required plugin with ID [%li].", id);
					tx_note(buffer);
					res++;
				}
			break;
			
			default:
				tx_note("Fatal Error loading set: unknown effect type!");
				res++;
		}		
	}

	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_x_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_x_input_parameter(NULL);
	
	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_y_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_y_input_parameter(NULL);

	atload(hidden);
	gui.main_panel->hide(hidden);

	atload(hidden);
	gui.trigger_panel->hide(hidden);

	atload(hidden);
	gui.lp_panel->hide(hidden);

	atload(hidden);
	gui.ec_panel->hide(hidden);
	
	return(res);
}

int vtt_class :: load_13(FILE * input)
{
	int res=0;
	guint32 pid;
	int32_t counter;
	int32_t type;
	long id;
	int i;
	unsigned int t;
	LADSPA_Plugin *plugin;
	char buffer[256];
	vtt_fx_ladspa *ladspa_effect;
	guint8 hidden;
	
	atload(buffer);
	this->set_name(buffer);
	atload(filename);
	atload(is_sync_master);
	atload(is_sync_client);
	atload(sync_cycles);
	atload(rel_volume);
	atload(rel_pitch);
	recalc_pitch();
	
	atload(autotrigger);
	atload(loop);
	
	atload(mute);
	atload(pan);
	
	atload(lp_enable);
	atload(lp_gain);
	atload(lp_reso);
	atload(lp_freq);
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	atload(ec_enable);
	atload(ec_length);
	ec_set_length(ec_length);
	atload(ec_feedback);
	ec_set_feedback(ec_feedback);
	atload(ec_pan);
	ec_set_pan(ec_pan);
	atload(ec_volume);
	ec_set_volume(ec_volume);

	recalc_volume();

	atload(pid);
	sp_speed.set_persistence_id(pid);
	atload(pid);
	sp_volume.set_persistence_id(pid);
	atload(pid);
	sp_pitch.set_persistence_id(pid);
	atload(pid);
	sp_trigger.set_persistence_id(pid);
	atload(pid);
	sp_loop.set_persistence_id(pid);
	atload(pid);
	sp_sync_client.set_persistence_id(pid);
	atload(pid);
	sp_sync_cycles.set_persistence_id(pid);
	atload(pid);
	sp_lp_enable.set_persistence_id(pid);
	atload(pid);
	sp_lp_gain.set_persistence_id(pid);
	atload(pid);
	sp_lp_reso.set_persistence_id(pid);
	atload(pid);
	sp_lp_freq.set_persistence_id(pid);
	atload(pid);
	sp_ec_enable.set_persistence_id(pid);
	atload(pid);
	sp_ec_length.set_persistence_id(pid);
	atload(pid);
	sp_ec_feedback.set_persistence_id(pid);
	atload(pid);
	sp_ec_volume.set_persistence_id(pid);
	atload(pid);
	sp_ec_pan.set_persistence_id(pid);
	atload(pid);
	sp_mute.set_persistence_id(pid);
	atload(pid);
	sp_spin.set_persistence_id(pid);
	atload(pid);
	sp_pan.set_persistence_id(pid);
		
	atload(counter);
	
	for (i=0; i<counter; i++)
	{
		atload(type);
		switch(type)
		{
			case TX_FX_BUILTINCUTOFF:
				for (t=0; t<fx_list.size(); t++) effect_down(lp_fx);
			break;
			
			case TX_FX_BUILTINECHO:
				for (t=0; t<fx_list.size(); t++) effect_down(ec_fx);
			break;
			
			case TX_FX_LADSPA:
				atload(id);
				plugin=LADSPA_Plugin::getPluginByUniqueID(id);
				if (plugin)
				{
					ladspa_effect=add_effect(plugin);
					ladspa_effect->load(input);
				}
				else
				{
					sprintf(buffer,"Fatal Error: Couldn't find required plugin with ID [%li].", id);
					tx_note(buffer);
					res++;
				}
			break;
			
			default:
				tx_note("Fatal Error loading set: unknown effect type!");
				res++;
		}		
	}

	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_x_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_x_input_parameter(NULL);
	
	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_y_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_y_input_parameter(NULL);

	atload(hidden);
	gui.main_panel->hide(hidden);

	atload(hidden);
	gui.trigger_panel->hide(hidden);

	atload(hidden);
	gui.lp_panel->hide(hidden);

	atload(hidden);
	gui.ec_panel->hide(hidden);
	
	return(res);
}

int vtt_class :: load_14(FILE * input)
{
	int res=0;
	guint32 pid;
	int32_t counter;
	int32_t type;
	long id;
	int i;
	unsigned int t;
	LADSPA_Plugin *plugin;
	char buffer[256];
	vtt_fx_ladspa *ladspa_effect;
	guint8 hidden;
	
	atload(buffer);
	this->set_name(buffer);
	atload(filename);
	atload(is_sync_master);
	atload(is_sync_client);
	atload(sync_cycles);
	atload(rel_volume);
	atload(rel_pitch);
	recalc_pitch();
	
	atload(autotrigger);
	atload(loop);
	
	atload(mute);
	atload(pan);
	
	atload(lp_enable);
	atload(lp_gain);
	atload(lp_reso);
	atload(lp_freq);
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	atload(ec_enable);
	atload(ec_length);
	ec_set_length(ec_length);
	atload(ec_feedback);
	ec_set_feedback(ec_feedback);
	atload(ec_pan);
	ec_set_pan(ec_pan);
	atload(ec_volume);
	ec_set_volume(ec_volume);

	atload(audio_hidden);
	atload(control_hidden);
	
	recalc_volume();

	atload(pid);
	sp_speed.set_persistence_id(pid);
	atload(pid);
	sp_volume.set_persistence_id(pid);
	atload(pid);
	sp_pitch.set_persistence_id(pid);
	atload(pid);
	sp_trigger.set_persistence_id(pid);
	atload(pid);
	sp_loop.set_persistence_id(pid);
	atload(pid);
	sp_sync_client.set_persistence_id(pid);
	atload(pid);
	sp_sync_cycles.set_persistence_id(pid);
	atload(pid);
	sp_lp_enable.set_persistence_id(pid);
	atload(pid);
	sp_lp_gain.set_persistence_id(pid);
	atload(pid);
	sp_lp_reso.set_persistence_id(pid);
	atload(pid);
	sp_lp_freq.set_persistence_id(pid);
	atload(pid);
	sp_ec_enable.set_persistence_id(pid);
	atload(pid);
	sp_ec_length.set_persistence_id(pid);
	atload(pid);
	sp_ec_feedback.set_persistence_id(pid);
	atload(pid);
	sp_ec_volume.set_persistence_id(pid);
	atload(pid);
	sp_ec_pan.set_persistence_id(pid);
	atload(pid);
	sp_mute.set_persistence_id(pid);
	atload(pid);
	sp_spin.set_persistence_id(pid);
	atload(pid);
	sp_pan.set_persistence_id(pid);
		
	atload(counter);
	
	for (i=0; i<counter; i++)
	{
		atload(type);
		switch(type)
		{
			case TX_FX_BUILTINCUTOFF:
				for (t=0; t<fx_list.size(); t++) effect_down(lp_fx);
			break;
			
			case TX_FX_BUILTINECHO:
				for (t=0; t<fx_list.size(); t++) effect_down(ec_fx);
			break;
			
			case TX_FX_LADSPA:
				atload(id);
				plugin=LADSPA_Plugin::getPluginByUniqueID(id);
				if (plugin)
				{
					ladspa_effect=add_effect(plugin);
					ladspa_effect->load(input);
				}
				else
				{
					sprintf(buffer,"Fatal Error: Couldn't find required plugin with ID [%li].", id);
					tx_note(buffer);
					res++;
				}
			break;
			
			default:
				tx_note("Fatal Error loading set: unknown effect type!");
				res++;
		}		
	}

	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_x_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_x_input_parameter(NULL);
	
	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_y_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_y_input_parameter(NULL);

	atload(hidden);
	gui.main_panel->hide(hidden);

	atload(hidden);
	gui.trigger_panel->hide(hidden);

	atload(hidden);
	gui.lp_panel->hide(hidden);

	atload(hidden);
	gui.ec_panel->hide(hidden);
	
	return(res);
}


int  vtt_class :: save_all(FILE* output)
{
	int res=0;
	list <vtt_class *> :: iterator vtt;
	guint32 pid;
	
	tX_seqpar :: create_persistence_ids();
	
	store(vtt_amount);
	store(master_volume);
	store(globals.pitch);
	pid=sp_master_volume.get_persistence_id();
	store(pid);
	pid=sp_master_pitch.get_persistence_id();
	store(pid);

	for (vtt=main_list.begin(); vtt!=main_list.end(); vtt++)
	{
		res+=(*vtt)->save(output);
	}
	
	sequencer.save(output);
	
	return(res);
}

int  vtt_class :: load_all_10(FILE* input, char *fname)
{
	int res=0, restmp=0;
	unsigned int i, max;
	vtt_class *newvtt;
	char ftmp[PATH_MAX];
	
	while (main_list.size())
	{
		delete((*main_list.begin()));
	}
		
	atload(max);
	atload(master_volume);
	set_master_volume(master_volume);
	globals.volume=master_volume;
	atload(globals.pitch);	
	set_master_pitch(globals.pitch);

	ld_create_loaddlg(TX_LOADDLG_MODE_MULTI, max);
	ld_set_setname(fname);

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(1);
		res+=newvtt->load_10(input);
		
		if (strlen(newvtt->filename))
		{
			/* ftmp IS NECESSARY !!! */
			strcpy(ftmp, newvtt->filename);
			ld_set_filename(ftmp);
			
			//restmp=load_wav(newvtt->filename, &newbuffer, &size);
			restmp=newvtt->load_file(ftmp);
			res+=restmp;
		}
		gtk_box_pack_start(GTK_BOX(control_parent), newvtt->gui.control_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(audio_parent), newvtt->gui.audio_box, TRUE, TRUE, 0);
	}
	
	sequencer.clear();
	
	ld_destroy();
	
	return(res);
}


int  vtt_class :: load_all_11(FILE* input, char *fname)
{
	int res=0, restmp=0;
	unsigned int i, max;
	vtt_class *newvtt;
	char ftmp[PATH_MAX];
	guint32 pid;
	
	while (main_list.size())
	{
		delete((*main_list.begin()));
	}
		
	atload(max);
	atload(master_volume);
	set_master_volume(master_volume);
	globals.volume=master_volume;
	atload(globals.pitch);	
	set_master_pitch(globals.pitch);
	atload(pid);
	sp_master_volume.set_persistence_id(pid);
	atload(pid);
	sp_master_pitch.set_persistence_id(pid);

	ld_create_loaddlg(TX_LOADDLG_MODE_MULTI, max);
	ld_set_setname(fname);

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(1);
		res+=newvtt->load_11(input);
		
		if (strlen(newvtt->filename))
		{
			/* ftmp IS NECESSARY !!! */
			strcpy(ftmp, newvtt->filename);
			ld_set_filename(ftmp);
			
			//restmp=load_wav(newvtt->filename, &newbuffer, &size);
			restmp=newvtt->load_file(ftmp);
			res+=restmp;
		}
		gtk_box_pack_start(GTK_BOX(control_parent), newvtt->gui.control_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(audio_parent), newvtt->gui.audio_box, TRUE, TRUE, 0);
		
	}
	
	sequencer.load(input);
	
	ld_destroy();
	
	return(res);
}


int  vtt_class :: load_all_12(FILE* input, char *fname)
{
	int res=0, restmp=0;
	unsigned int i, max;
	vtt_class *newvtt;
	char ftmp[PATH_MAX];
	guint32 pid;
	
	while (main_list.size())
	{
		delete((*main_list.begin()));
	}
		
	atload(max);
	atload(master_volume);
	set_master_volume(master_volume);
	globals.volume=master_volume;
	atload(globals.pitch);	
	set_master_pitch(globals.pitch);
	atload(pid);
	sp_master_volume.set_persistence_id(pid);
	atload(pid);
	sp_master_pitch.set_persistence_id(pid);

	ld_create_loaddlg(TX_LOADDLG_MODE_MULTI, max);
	ld_set_setname(fname);

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(1);
		res+=newvtt->load_12(input);
		
		if (strlen(newvtt->filename))
		{
			/* ftmp IS NECESSARY !!! */
			strcpy(ftmp, newvtt->filename);
			ld_set_filename(ftmp);
			
			//restmp=load_wav(newvtt->filename, &newbuffer, &size);
			restmp=newvtt->load_file(ftmp);
			res+=restmp;
		}
		gtk_box_pack_start(GTK_BOX(control_parent), newvtt->gui.control_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(audio_parent), newvtt->gui.audio_box, TRUE, TRUE, 0);
		
	}
	
	sequencer.load(input);
	
	ld_destroy();
	
	return(res);
}

int  vtt_class :: load_all_13(FILE* input, char *fname)
{
	int res=0, restmp=0;
	unsigned int i, max;
	vtt_class *newvtt;
	char ftmp[PATH_MAX];
	guint32 pid;
	
	while (main_list.size())
	{
		delete((*main_list.begin()));
	}
		
	atload(max);
	atload(master_volume);
	set_master_volume(master_volume);
	globals.volume=master_volume;
	atload(globals.pitch);	
	set_master_pitch(globals.pitch);
	atload(pid);
	sp_master_volume.set_persistence_id(pid);
	atload(pid);
	sp_master_pitch.set_persistence_id(pid);

	ld_create_loaddlg(TX_LOADDLG_MODE_MULTI, max);
	ld_set_setname(fname);

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(1);
		res+=newvtt->load_13(input);
		
		if (strlen(newvtt->filename))
		{
			/* ftmp IS NECESSARY !!! */
			strcpy(ftmp, newvtt->filename);
			ld_set_filename(ftmp);
			
			//restmp=load_wav(newvtt->filename, &newbuffer, &size);
			restmp=newvtt->load_file(ftmp);
			res+=restmp;
		}
		gtk_box_pack_start(GTK_BOX(control_parent), newvtt->gui.control_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(audio_parent), newvtt->gui.audio_box, TRUE, TRUE, 0);
		
	}
	
	sequencer.load(input);
	
	ld_destroy();
	
	return(res);
}

int  vtt_class :: load_all_14(FILE* input, char *fname)
{
	int res=0, restmp=0;
	unsigned int i, max;
	vtt_class *newvtt;
	char ftmp[PATH_MAX];
	guint32 pid;
	
	while (main_list.size())
	{
		delete((*main_list.begin()));
	}
		
	atload(max);
	atload(master_volume);
	set_master_volume(master_volume);
	globals.volume=master_volume;
	atload(globals.pitch);	
	set_master_pitch(globals.pitch);
	atload(pid);
	sp_master_volume.set_persistence_id(pid);
	atload(pid);
	sp_master_pitch.set_persistence_id(pid);

	ld_create_loaddlg(TX_LOADDLG_MODE_MULTI, max);
	ld_set_setname(fname);

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(1);
		res+=newvtt->load_14(input);
		
		if (strlen(newvtt->filename))
		{
			/* ftmp IS NECESSARY !!! */
			strcpy(ftmp, newvtt->filename);
			ld_set_filename(ftmp);
			
			//restmp=load_wav(newvtt->filename, &newbuffer, &size);
			restmp=newvtt->load_file(ftmp);
			res+=restmp;
		}
		gtk_box_pack_start(GTK_BOX(control_parent), newvtt->gui.control_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(audio_parent), newvtt->gui.audio_box, TRUE, TRUE, 0);
	    if (newvtt->audio_hidden) newvtt->hide_audio(newvtt->audio_hidden);
	    if (newvtt->control_hidden) newvtt->hide_control(newvtt->control_hidden);
	}
	
	sequencer.load(input);
	
	ld_destroy();
	
	return(res);
}


void add_vtt(GtkWidget *ctrl, GtkWidget *audio, char *fn)
{
	vtt_class *hmmpg;
	hmmpg = new vtt_class(1);
	gtk_box_pack_start(GTK_BOX(ctrl), hmmpg->gui.control_box, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(audio), hmmpg->gui.audio_box, TRUE, TRUE, 0);
	if (fn) hmmpg->load_file(fn);
}

extern void vg_move_fx_panel_up(GtkWidget *wid, vtt_class *vtt);
extern void vg_move_fx_panel_down(GtkWidget *wid, vtt_class *vtt);

//#define debug_fx_stack(); for (i=fx_list.begin(); i != fx_list.end(); i++) puts((*i)->get_info_string());
#define debug_fx_stack();

void vtt_class :: effect_up(vtt_fx *effect)
{
	list <vtt_fx *> :: iterator i;
	list <vtt_fx *> :: iterator previous;
	int ok=0;
	
	debug_fx_stack();
	
	if ((*fx_list.begin())==effect) return;
	
	for (previous=i=fx_list.begin(); i != fx_list.end(); i++)
	{
		if ((*i) == effect)
		{
			ok=1;
			break;
		}
		previous=i;
	}
	
	if (ok)
	{	
		pthread_mutex_lock(&render_lock);
		fx_list.remove(effect);
		fx_list.insert(previous, effect);
		pthread_mutex_unlock(&render_lock);

		vg_move_fx_panel_up(effect->get_panel_widget(), this);
	}
	
	debug_fx_stack();
}

void vtt_class :: effect_down(vtt_fx *effect)
{
	list <vtt_fx *> :: iterator i;
	int ok=0;

	debug_fx_stack();
		
	for (i=fx_list.begin(); i != fx_list.end(); i++)
	{
		if ((*i) == effect)
		{
			ok=1;
			break;
		}
	}
	
	if ((ok) && (i!=fx_list.end()))
	{
		i++;
		if (i==fx_list.end()) return;
		i++;

		pthread_mutex_lock(&render_lock);
		fx_list.remove(effect);
		
		fx_list.insert(i, effect);
		vg_move_fx_panel_down(effect->get_panel_widget(), this);
		pthread_mutex_unlock(&render_lock);
	}
	
debug_fx_stack();	
}

void vtt_class ::  effect_remove(vtt_fx_ladspa *effect)
{
	pthread_mutex_lock(&render_lock);
	fx_list.remove(effect);
	pthread_mutex_unlock(&render_lock);
	
	delete effect;
}

extern void gui_hide_control_panel(vtt_class *vtt, bool hide);
extern void gui_hide_audio_panel(vtt_class *vtt, bool hide);

void vtt_class :: hide_audio(bool hide) {
	audio_hidden=hide;
	gui_hide_audio_panel(this, hide);
}

void vtt_class :: hide_control(bool hide) {
	control_hidden=hide;
	gui_hide_control_panel(this, hide);	
}
