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
 
    File: tX_vtt.cc
 
    Description: This implements the new virtual turntable class. It replaces
		 the old turntable.c from terminatorX 3.2 and earlier. The lowpass
		 filter is based on some sample code by Paul Kellett
		 <paul.kellett@maxim.abel.co.uk>
*/    
   
#include "tX_vtt.h"
#include "tX_global.h"
#include <stdio.h>
#include "malloc.h"
#include <math.h>
#include "tX_wavfunc.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_3DNOW
#include "3dnow.h"
#endif

extern void build_vtt_gui(vtt_class *);
extern void gui_set_name(vtt_class *vtt, char *newname);
extern void gui_set_filename(vtt_class *vtt, char *newname);
extern void delete_gui(vtt_class *vtt);
extern void gui_update_display(vtt_class *vtt);
extern void gui_clear_master_button(vtt_class *vtt);
extern void cleanup_vtt(vtt_class *vtt);

int vtt_class::vtt_amount=0;
list <vtt_class *> vtt_class::main_list;
list <vtt_class *> vtt_class::render_list;
int16_t* vtt_class::mix_out_buffer=NULL;
f_prec * vtt_class::mix_buffer=NULL;
int vtt_class::samples_in_mix_buffer=0;
pthread_mutex_t vtt_class::render_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vtt_class::main_lock=PTHREAD_MUTEX_INITIALIZER;
f_prec vtt_class::master_volume=1.0;
f_prec vtt_class::res_master_volume=1.0;
//f_prec vtt_class::saturate_fac=((f_prec) SAMPLE_MAX-SAMPLE_BORDER)*1.0/FLT_MAX;
f_prec vtt_class::saturate_fac=0.1;
int vtt_class::do_saturate=0;
vtt_class * vtt_class::sync_master=NULL;
int vtt_class::master_triggered=0;
int vtt_class::master_triggered_at=0;
vtt_class * vtt_class::focused_vtt=NULL;
f_prec vtt_class::mix_max=0;
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
	
	x_control=CONTROL_SCRATCH;
	y_control=CONTROL_CUTOFF;
	
	lp_enable=0;
	lp_reso=0.8;
	lp_freq=0.3;
	lp_gain=1;
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	ec_enable=0;
	ec_length=0.5;
	ec_feedback=0.3;
	ec_clear_buffer();
	ec_set_length(0.5);
	
//	pthread_mutex_lock(&main_lock);
	main_list.push_back(this);
//	pthread_mutex_unlock(&main_lock);
	
	if (do_create_gui)
	{	
		build_vtt_gui(this);
	}
	else have_gui=0;
		
	set_master_volume(globals.volume);
	set_output_buffer_size(samples_in_mix_buffer);
}

vtt_class :: ~vtt_class()
{
	stop();
//	pthread_mutex_lock(&main_lock);
	main_list.remove(this);
//	pthread_mutex_unlock(&main_lock);
	if (buffer) free(buffer);
	if (output_buffer) free(output_buffer);
	
	delete_gui(this);
	vtt_amount--;
}

void vtt_class :: set_name(char *newname)
{
	strcpy(name, newname);
	gui_set_name(this, name);	
}

void vtt_class :: set_file_data(char *newfilename, int16_t *newbuffer, int samples)
{
	if (is_playing) stop();
	
	if (buffer) free(buffer);
	buffer=newbuffer;
	samples_in_buffer=samples;
	maxpos=samples;
	
	strcpy(filename, newfilename);
	if (have_gui)
	{
		gui_set_filename(this, newfilename);
		gui_update_display(this);
	}
	ec_set_length(ec_length);
}

int vtt_class :: set_output_buffer_size(int newsize)
{
	if (output_buffer) free(output_buffer);
	output_buffer = (float *) malloc (sizeof(float)*newsize);
	end_of_outputbuffer = output_buffer + newsize; //size_t(sizeof(float)*(newsize));
	
	samples_in_outputbuffer=newsize;
	inv_samples_in_outputbuffer=1.0/samples_in_outputbuffer;
	
	if (output_buffer) return(0);	
	else return(0);
}

void vtt_class :: set_volume(f_prec newvol)
{
	rel_volume=newvol;
	res_volume=rel_volume*res_master_volume;
}

void vtt_class :: recalc_volume()
{
	res_volume=rel_volume*res_master_volume;
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

void vtt_class :: set_controls (int x, int y)
{
	x_control=x;
	y_control=y;
}

void vtt_class :: set_mute(int newstate)
{
	mute=newstate;
}

void vtt_class :: lp_set_enable (int newstate)
{
	lp_enable=newstate;
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
	
	lp_a=1.0-freq;
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
}

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
//	printf("le: %f\n", ec_res_length);	
	
	if (ec_res_length>=EC_MAX_BUFFER) ec_res_length=EC_MAX_BUFFER;
	
	delay=(int )floor(ec_res_length);
	delay-=2;
	ec_delay=&ec_buffer[delay];
}

void vtt_class :: ec_set_feedback(f_prec feedback)
{
	ec_feedback=feedback;
}

void vtt_class :: ec_clear_buffer()
{
	int i;
	
	for (i=0; i<EC_MAX_BUFFER; i++)
	{
		ec_buffer[i]=0.0;
	}
	ec_ptr=ec_buffer;
}

void vtt_class :: render()
{
	if (do_scratch)
	{
		if (sense_cycles>0)
		{
			sense_cycles--;
			if (sense_cycles==0) speed=0;
		}
	}
	render_scratch();
	if (lp_enable) render_lp();
	if (ec_enable) render_ec();
}

f_prec speed_step_last=0;
f_prec temp;

void vtt_class :: render_scratch()
{
	int16_t *ptr;
	
	int sample;
	int fade_in=0;
	int fade_out=0;
	int do_mute=0;
	
	f_prec next_speed;
	f_prec pos_a_f;
//	unsigned int pos_a_i;
	
	f_prec amount_a;
	f_prec amount_b;

	f_prec sample_a;
	f_prec sample_b;
	
	f_prec sample_res;
	
	f_prec *out;
	f_prec fade_vol;	
	
//	if (speed != speed_last) printf("%s: from %f to %f.\n", name, speed, speed_last);
	
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


/*	if (speed_real != speed_step_last)
	{
		printf("last: %f now: %f \n", speed_step_last, speed_real);
		speed_step_last=speed_real;
	}*/
	
	speed_step_last=pos_f;
	
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

	if (mute != mute_old)
	{
		if (mute)
		{
			fade_out=1; fade_in=0;
			fade=NEED_FADE_IN;
		}
		else
		{
			fade_in=1; fade_out=0;
			fade=NEED_FADE_OUT;
		}
		mute_old=mute;
	}
	else
	{
		if (mute) do_mute=1;
	}
	
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
	speed_last = speed_real;
}	

/*
	The following lowpass filter is based on some sample code by
	Paul Kellett <paul.kellett@maxim.abel.co.uk>
*/

void vtt_class :: render_lp()
{
#ifdef NOT_NOT_NOT_USE_3DNOW
	mmx_t *sample;
	f_prec in;
	int i;
	mmx_t mmx_gain;
	
	mmx_gain.s[0]=lp_resgain;
	mmx_gain.s[1]=lp_resgain;
	
	for (sample = (mmx_t *) output_buffer, sample<(mmx_t*) end_of_outputbuffer; sample++)
	{
		movq_m2r(*sample, mm0);
		movq_m2r(mmx_gain, mm1);
		
		pfmul_r2r(mm1, mm0);
		
		
		lp_buf0 = lp_a * lp_buf0 + lp_freq * (in + lp_b * (lp_buf0 - lp_buf1));
		lp_buf1 = lp_a * lp_buf1 + lp_freq * lp_buf0;
		
		*sample=lp_buf1;
	}
#else
	f_prec *sample;
		
	for (sample = output_buffer; sample<end_of_outputbuffer; sample++)
	{
		lp_buf0 = lp_a * lp_buf0 + lp_freq * ((*sample)*lp_resgain + lp_b * (lp_buf0 - lp_buf1));
		lp_buf1 = lp_a * lp_buf1 + lp_freq * lp_buf0;
		
		*sample=lp_buf1;
	}
#endif
}

void vtt_class :: render_ec()
{
#ifdef USE_3DNOW
	mmx_t *sample;
	mmx_t feed;

	feed.s[0]=ec_feedback;
	feed.s[1]=ec_feedback;
	
	movq_m2r(feed, mm0);
	
	for (sample = (mmx_t*) output_buffer; sample<(mmx_t*) end_of_outputbuffer; sample++, ec_ptr+=2)
	{
	
		if (ec_ptr>ec_delay) ec_ptr=ec_buffer;
		
		movq_m2r(*sample, mm1);
		movq_m2r(*ec_ptr, mm2);
		
		pfmul_r2r(mm0, mm2);
		pfadd_r2r(mm1, mm2);
		
		movq_r2m(mm2, *sample);
		movq_r2m(mm2, *ec_ptr);	
	}	
	
	femms();
#else
	f_prec *sample;
	f_prec temp;
	int i;


	for (i=0, sample = output_buffer; i<samples_in_outputbuffer; i++, sample++, ec_ptr++)
	{
		if (ec_ptr>ec_delay) ec_ptr=ec_buffer;
		
		temp= *sample + (*ec_ptr) *ec_feedback;
		*sample=temp;
		*ec_ptr=temp;
	}	
#endif
}

int vtt_class :: set_mix_buffer_size(int no_samples)
{
	list <vtt_class *> :: iterator vtt;
	int res=0;
	
	if (mix_buffer) free(mix_buffer);
	mix_buffer=(float *) malloc (sizeof(float)*no_samples);
	if (mix_out_buffer) free(mix_out_buffer);
	mix_out_buffer=(int16_t *) malloc (sizeof(int16_t)*no_samples);
	samples_in_mix_buffer=no_samples;
	
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
	f_prec temp;
#ifdef USE_FLASH
	f_prec max;
	f_prec min;
#endif	
	
	pthread_mutex_lock(&render_lock);
	
	switch (render_list.size())
	{
		case 0: for (sample=0; sample<samples_in_mix_buffer; sample++)
			{
				mix_out_buffer[sample]=0;
			}
			break;
/*		case 1:	vtt=render_list.begin();
			(*vtt)->render();
			
			if (do_saturate)
			for (sample=0; sample<samples_in_mix_buffer; sample++)
			{
				temp=((*vtt)->output_buffer[sample]*(*vtt)->res_volume);
				if (temp>SAMPLE_BORDER)
				{
					temp*=saturate_fac;
					temp+=SAMPLE_BORDER;
				}
				else
				{
					if (temp<-SAMPLE_BORDER)
					{
						temp*=saturate_fac;
						temp-=SAMPLE_BORDER;
					}
				}
				mix_out_buffer[sample]=(int16_t) temp;
			}
			else
			for (sample=0; sample<samples_in_mix_buffer; sample++)
			{
				mix_out_buffer[sample]=(int16_t) ((*vtt)->output_buffer[sample]*(*vtt)->res_volume);
			}
			break;*/
		default:
			vtt=render_list.begin();
			(*vtt)->render();			
#ifdef USE_FLASH
			max=(*vtt)->max_value;
			min=max;
			
			for (sample=0; sample<samples_in_mix_buffer; sample++)
			{				
				temp=(*vtt)->output_buffer[sample];
				mix_buffer[sample]=temp*(*vtt)->res_volume;
				
				if (temp>max) max=temp;
				else if (temp<min) min=temp;
			}
			
			min*=-1.0;
			if (min>max) (*vtt)->max_value=min; else (*vtt)->max_value=max;

#else		
			for (sample=0; sample<samples_in_mix_buffer; sample++)
			{				
				mix_buffer[sample]=(*vtt)->output_buffer[sample]*(*vtt)->res_volume;
			}
#endif			
			if (master_triggered)
			{
				pthread_mutex_unlock(&render_lock);
//				pthread_mutex_lock(&main_lock);
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
//				pthread_mutex_unlock(&main_lock);
				pthread_mutex_lock(&render_lock);
			}
			vtt=render_list.begin();
			for (vtt++; vtt!=render_list.end(); vtt++)
			{
				(*vtt)->render();					
#ifdef USE_FLASH
				max=(*vtt)->max_value;
				min=max;
				
				for (sample=0; sample<samples_in_mix_buffer; sample++)
				{				
					temp=(*vtt)->output_buffer[sample];
					mix_buffer[sample]+=temp*(*vtt)->res_volume;
				
					if (temp>max) max=temp;
					else if (temp<min) min=temp;
				}
				
				min*=-1.0;
				if (min>max) (*vtt)->max_value=min; else (*vtt)->max_value=max;
#else				
				for (sample=0; sample<samples_in_mix_buffer; sample++)
				{
					mix_buffer[sample]+=(*vtt)->output_buffer[sample]*(*vtt)->res_volume;
				}
#endif				
			}
			
			if (do_saturate)
			for (sample=0; sample<samples_in_mix_buffer; sample++)
			{
				temp=(int16_t)mix_buffer[sample];
				if (temp>SAMPLE_BORDER)
				{
					printf("sat: %f -> ", temp);
					temp=SAMPLE_BORDER+(saturate_fac*(temp-SAMPLE_BORDER));
					printf("%hi\n", int16_t(temp));									
				}
				else
				{
					if (temp<-SAMPLE_BORDER)
					{
						printf("sat: %f -> ", temp);
						temp=-SAMPLE_BORDER+(saturate_fac*(temp+ SAMPLE_BORDER));
					}
				}
				mix_out_buffer[sample]=(int16_t) temp;
			}
			else
			{
#ifdef 	USE_FLASH		
				max=mix_max;
				for (sample=0; sample<samples_in_mix_buffer; sample++)
				{				
					temp=mix_buffer[sample];
					mix_out_buffer[sample]=(int16_t) temp;
				
					if (temp>max) max=temp;
					else if (temp<min) min=temp;
				}
				min*=-1.0;
				if (min>max) mix_max=min; else mix_max=max;
#else
				for (sample=0; sample<samples_in_mix_buffer; sample++)
				{
					mix_out_buffer[sample]=(int16_t)mix_buffer[sample];
				}
#endif				
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
	pthread_mutex_unlock(&render_lock);
	
	return(mix_out_buffer);
}

int vtt_class :: trigger()
{
	if (!buffer) return (1);

	if (!is_playing) pthread_mutex_lock(&render_lock);
	
	if (res_pitch>=0) pos_f=0;
	else pos_f=maxpos;
	fade=NEED_FADE_OUT;
	speed=res_pitch;
	speed_real=res_pitch;
	speed_target=res_pitch;
/*	mute=0;
	mute_old=0;*/
	want_stop=0;

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
//	pthread_mutex_lock(&render_lock);

	if (!is_playing) 
	{
		pthread_mutex_unlock(&render_lock);
		return(1);
	}
	render_list.remove(this);
	want_stop=0;

	is_playing=0;
//	pthread_mutex_unlock(&render_lock);

#ifdef USE_FLASH
	max_value=0;
#endif
	cleanup_vtt(this);
	sync_countdown=0;
	
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

void vtt_class :: set_master_volume(f_prec new_volume)
{
	list <vtt_class *> :: iterator vtt;

	master_volume=new_volume;
	globals.volume=new_volume;
	
	if (main_list.size()>0)
	{
//		res_master_volume=master_volume/((f_prec) main_list.size());
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

int vtt_class :: enable_saturate (int newstate)
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
	
	for (vtt=main_list.begin(); vtt!=main_list.end() ; vtt++)
	{
		if ((*vtt)==focused_vtt)
		{
			vtt++;
			if (vtt==main_list.end())
			{			
				focused_vtt=(*main_list.begin());
				return;
			}
			else
			{
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
		speed=0;
		do_scratch=1;
		sense_cycles=globals.sense_cycles;
	}
	else
	{
		speed=res_pitch;
		do_scratch=0;
	}
}

#define MAGIC 0.05

void vtt_class :: handle_input(int control, f_prec value)
{
	f_prec temp;
	
	switch (control)
	{
		case CONTROL_SCRATCH:
		if (do_scratch) speed=value*globals.mouse_speed;
		sense_cycles=globals.sense_cycles;
		break;
		
		case CONTROL_VOLUME:
		temp=rel_volume+MAGIC*value*globals.mouse_speed;
		if (temp>1.0) temp=1.0;
		else if (temp<0) temp=0;
		set_volume(temp);
		break;
		
		case CONTROL_CUTOFF:
		temp=lp_freq+MAGIC*value*globals.mouse_speed;
		if (temp>0.99) temp=0.99;
		else if (temp<0) temp=0;
		lp_set_freq(temp);
		break;
		
		case CONTROL_FEEDBACK:
		temp=ec_feedback+MAGIC*value*globals.mouse_speed;
		if (temp>1.0) temp=1.0;
		else if (temp<0) temp=0;
		ec_set_feedback(temp);
		break;
	}
}

void vtt_class :: unfocus()
{
	focused_vtt=NULL;
}

void vtt_class :: xy_input(f_prec x_value, f_prec y_value)
{
	handle_input(x_control, x_value);
	handle_input(y_control, y_value);
}


#define store(data); if (fwrite((void *) &data, sizeof(data), 1, output)!=1) res+=1;

int  vtt_class :: save(FILE * output)
{
	int res=0;
	
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
	store(x_control);
	store(y_control);	
	
	store(lp_enable);
	store(lp_gain);
	store(lp_reso);
	store(lp_freq);
	
	store(ec_enable);
	store(ec_length);
	store(ec_feedback);
	
	return(res);
}

#define atload(data); if (fread((void *) &data, sizeof(data), 1, input)!=1) res+=1;

int vtt_class :: load(FILE * input)
{
	int res=0;
	
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
	atload(x_control);
	atload(y_control);	
	
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

int  vtt_class :: save_all(FILE* output)
{
	int res=0;
	list <vtt_class *> :: iterator vtt;
	
	store(vtt_amount);
	store(master_volume);
	store(globals.pitch);
	

	for (vtt=main_list.begin(); vtt!=main_list.end(); vtt++)
	{
		res+=(*vtt)->save(output);
	}
	
	return(res);
}

int  vtt_class :: load_all(FILE* input)
{
	int res=0, restmp=0;
	list <vtt_class *> :: iterator vtt;
	unsigned int i, max, size;
	int16_t *newbuffer;
	vtt_class *newvtt;
	
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

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(0);
		res+=newvtt->load(input);
		if (strlen(newvtt->filename))
		{
			restmp=load_wav(newvtt->filename, &newbuffer, &size);
			if (!restmp) newvtt->set_file_data(newvtt->filename, newbuffer, size/sizeof(int16_t));
			res+=restmp;
		}
	}
	
	return(res);
}

int add_vtt(GtkWidget *daddy)
{
	vtt_class *hmmpg;
	hmmpg = new vtt_class(1);
	gtk_box_pack_start(GTK_BOX(daddy), hmmpg->gui.frame, TRUE, TRUE, 0);
	gtk_widget_show(hmmpg->gui.frame);
}
