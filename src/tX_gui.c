/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999  Alexander K÷nig
 
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
 
    File: tX_gui.c
 
    Description: Contains code for
    		 - terminatorX main window.
		 - the file dialogs
		 - the "pre-listen" feature
		 - and the position update schedule code

    Changes:
    
    21 Jul 1999: Added the necessary GUI stuff for the Lowpass Filter
*/    

#include <stdio.h>
#include <time.h>
#include "gtk/gtk.h"
#include "tX_types.h"
#include "tX_global.h"
#include "tX_widget.h"
#include "tX_gui.h"
#include "tX_wavfunc.h"
#include <malloc.h>
#include <string.h>
#include "turntable.h"
#include "tX_engine.h"
#include "tX_dialog.h"
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <pthread.h>
#include "wav_file.h"
#include "endian.h"
#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
int disk_status=DISK_IDLE;

#define set_disk_status(i); disk_status=i;

/* Here come 'em widgets */

Window xwindow=(Window) 0;
GtkWidget *window;
GtkWidget *scratch_btn;
GtkWidget *mix_toggle_btn;
GtkWidget *mix_slider;
GtkWidget *loop_btn;
//GtkWidget *xinput_combo;
GtkWidget *wav_display;
GtkWidget *wav_progress=NULL;
GtkWidget *pos_time;

GSList	  *mode_radios=NULL;
GtkWidget *mode_normal;
GtkWidget *mode_record;
GtkWidget *mode_playback;

GtkWidget *action_btn;

GtkWidget *options_btn;
GtkWidget *about_btn;

GtkWidget *save_fast;
GtkWidget *save_as;
GtkWidget *save_fast_mix;
GtkWidget *save_as_mix;
GtkTooltips *tooltips;

GtkWidget *lowpass_toggle_btn;
GtkWidget *lowpass_slider;

GtkAdjustment *lowpass_freq;
GtkAdjustment *mix_adjustment;

gint idle_tag=0;

int engine_busy=0;

int scratch_busy=0;
GdkWindow *scratch_win=NULL;
int loop_busy=0;
GdkWindow *loop_win=NULL;

GdkWindow *save_win=NULL;
int save_busy=0;

GdkWindow *save_mix_win=NULL;
int save_mix_busy=0;

char last_scratch[PATH_MAX]="";
char last_loop[PATH_MAX]="";

int display_scratch=0;
//GList *xinput_list=NULL;

pthread_t prelis_thread=0;
pthread_mutex_t prelis_play=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t prelis_stop=PTHREAD_MUTEX_INITIALIZER;

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0

int posctr=0;

int db_save_mix=1;
int db_save_raw=0;
int prelis_host_order=0;

FILE* prelis_file=NULL;

void *prelis_run(void *ptr)
{
	char buffer[4096];
	int bytes=1;

	pthread_mutex_lock(&prelis_play);
		
	vtt_open_dev(vttgl, 0);
			
	while ((pthread_mutex_trylock(&prelis_stop)) && (bytes))
	{
		bytes=fread(&buffer, 1, 2048, prelis_file);
#ifdef BIG_ENDIAN_MACHINE
		if (prelis_host_order) swapbuffer((int16_t *) buffer, 1024);
#endif
		if (bytes) write(vttgl->devicefd, buffer, bytes);
	}
			
	vtt_close_dev(vttgl);
	
	if (!bytes) pthread_mutex_lock(&prelis_stop);	
		
	pthread_mutex_unlock(&prelis_stop);
	pthread_mutex_unlock(&prelis_play);
	
	pthread_exit(NULL);
}

void prelis_halt()
{
	void *dummy;
		
	pthread_mutex_unlock(&prelis_stop);
	pthread_join(prelis_thread, &dummy);
	
	wav_close(prelis_file);
	
	prelis_thread=0;
	prelis_file=NULL;
}

void prelis_start(char *name)
{
	wav_sig wav_in;
	
	if (!globals.prelis) return;
	
	if (prelis_thread) prelis_halt();
	
	if (init_wav_read(name, &wav_in))
#ifndef USE_SOX_INPUT	
	if ((wav_in.depth==16) && (wav_in.chans==1))
#endif	
	{
#ifdef USE_SOX_INPUT
	if (wav_in.has_host_order) prelis_host_order=1;
	else prelis_host_order=0;
#endif	
		pthread_mutex_lock(&prelis_play);
		pthread_mutex_lock(&prelis_stop);
		if (!pthread_create(&prelis_thread, NULL, prelis_run, NULL))
		{
			prelis_file=wav_in.handle;
			pthread_mutex_unlock(&prelis_play);			
		}
		else
		{
			pthread_mutex_unlock(&prelis_play);
			prelis_thread=0;
			wav_close(wav_in.handle);
		}
		
	}
	else wav_close(wav_in.handle);
}

void display_engine_error(int err);

int time_ctr=0;

int check_busy()
{
	if (engine_busy)
	{
		tx_note("You have to stop playback first.");
		return(1);
	}
	return(0);
}

int idle_ctr=0;

gint pos_update(gpointer data)
{
	int ret;
	char buffer[40];
	unsigned long int minu,sec,hun;
	int pos;	
	
	ret=get_engine_status();
	
//	usleep(globals.update_idle); // usleep and pthreads ... strange strange

	if (!ret)
	{
		pos=vttgl->realpos;
		gtk_tx_update_pos_display(GTK_TX(wav_display), pos, vttgl->mute_scratch);			
				
		if (globals.time_enable)
		{
			time_ctr++;	
		
			if (time_ctr>globals.time_update)
			{
				time_ctr=0;
				if (pos>0)
				{
					minu=pos/2646000;
					pos-=2646000*minu;
	
					sec=pos/44100;
					pos-=44100*sec;
	
					hun=pos/441;
				}
				else
				{
					minu=sec=hun=0;
				}
	
				sprintf(buffer, "%02li:%02li.%02li", minu, sec, hun);
				gtk_label_set_text(GTK_LABEL(GTK_BUTTON(pos_time)->child), buffer);
				gdk_flush();
			}
		}
		return (TRUE);
	}
	else if(ret==ENG_INIT)
	{
		idle_ctr++;
		if (idle_ctr>1000)
		{
			stop_engine();
			tx_note("Error: Position update idle for 1000 cycles. Giving up!\nMaybe you should increase the \"update idle\" value in the option menu.");
			engine_busy=0;
			gtk_tx_cleanup_pos_display(GTK_TX(wav_display));
			time_ctr=0;			
			return (FALSE);
		}
		return(TRUE);
	}
	else
	{
		stop_engine();
		engine_busy=0;
	
		if (ret>3)
		{
			display_engine_error(ret);			
		}
		
		gtk_tx_cleanup_pos_display(GTK_TX(wav_display));
		time_ctr=0;
		return(FALSE);
	}	
}

void lw_msg(int err, char *buffer)
{
	switch (err)
	{
#ifdef USE_SOX_INPUT	
		case 1: strcat (buffer, "Wave-File is not mono."); break;
		case 2: strcat (buffer, "Wave-File is not 16 Bit."); break;
		case 3: strcat (buffer, "Wave-File could not be accessed."); break;
#else
		case 1: strcat (buffer, "Couldn't open audio file."); break;
		case 2: strcat (buffer, "Couldn't open audio file."); break;
		case 3: strcat (buffer, "Couldn't open audio file."); break;
#endif		
		case 4: strcat (buffer, "Could not allocate memory."); break;
		case 5: strcat (buffer, "Error while reading data. File corrupt?"); break;
		default: strcat (buffer, "Internal Error?");
	}
}

void set_mix(int mix)
{
	if (mix)
	{
		if (globals.loop_data)
		{
			globals.do_mix=1;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mix_toggle_btn), 1);
		}
		else
		{
			globals.do_mix=0;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mix_toggle_btn), 0);
	
			tx_note("Can not enable mix: no loop data available.");	
		}
	}
	else
	{
		globals.do_mix=0;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mix_toggle_btn), 0);
	}
}

void load_scratch(GtkWidget *widget, GtkFileSelection *fs)
{
	int ret;
	char newfile[PATH_MAX];
	char buffer[1024]="Couldn't open scratch file: ";
	char *fn;

	prelis_halt();

	if (check_busy()) return;
	
	set_disk_status(DISK_READING);
	
	strcpy(newfile, gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)));
	strcpy(last_scratch, newfile);
	
	gtk_widget_destroy(GTK_WIDGET(fs));

	if (globals.scratch_data) free(globals.scratch_data);

	ret = load_wav(newfile, &globals.scratch_data, &globals.scratch_size);
	
	if (ret)
	{
		strcpy(globals.scratch_name, "");
		globals.scratch_data=NULL;
		globals.scratch_size=0;
		globals.scratch_len=0;
		gtk_label_set(GTK_LABEL(GTK_BUTTON(scratch_btn)->child), "< NONE >");
		
		lw_msg(ret, buffer);
		tx_note(buffer);				
	}
	else
	{
		strcpy(globals.scratch_name, newfile);
		globals.scratch_len=globals.scratch_size/sizeof(int16_t);
		
		fn=strrchr(newfile, '/');
		if (fn) fn++;
		else fn=newfile;
		gtk_label_set(GTK_LABEL(GTK_BUTTON(scratch_btn)->child), fn);
	}	
	
	if (!display_scratch)
	{
		gtk_tx_set_data(GTK_TX(wav_display), globals.scratch_data, globals.scratch_len);
	}
	
	scratch_busy=0;
	scratch_win=NULL;
	
	set_disk_status(DISK_IDLE);
}

void load_loop(GtkWidget *widget, GtkFileSelection *fs)
{
	int ret;
	char newfile[PATH_MAX];
	char buffer[1024]="Couldn't open loop file: ";
	char *fn;

	prelis_halt();

	if (check_busy()) return;
	
	set_disk_status(DISK_READING);
	
	strcpy(newfile, gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)));
	strcpy(last_loop, newfile);
	gtk_widget_destroy(GTK_WIDGET(fs));

	if (globals.loop_data) free(globals.loop_data);

	ret = load_wav(newfile, &globals.loop_data, &globals.loop_size);
	
	if (ret)
	{
		strcpy(globals.loop_name, "");
		globals.loop_data=NULL;
		globals.loop_size=0;
		globals.loop_len=0;
		gtk_label_set(GTK_LABEL(GTK_BUTTON(loop_btn)->child), "< NONE >");

		set_mix(0);
		
		lw_msg(ret, buffer);
		tx_note(buffer);				
	}
	else
	{
		strcpy(globals.loop_name, newfile);
		globals.loop_len=globals.loop_size/sizeof(int16_t);

		fn=strrchr(newfile, '/');
		if (fn) fn++;
		else fn=newfile;
		
		gtk_label_set(GTK_LABEL(GTK_BUTTON(loop_btn)->child), fn);
		set_mix(1);
	}	
	loop_busy=0;
	loop_win=NULL;
	
	set_disk_status(DISK_IDLE);
}

int scratch_destroy(GtkWidget *widget, GtkFileSelection *fs)
{
	prelis_halt();
	scratch_busy=0;
	scratch_win=NULL;	
	if (fs) gtk_widget_destroy(GTK_WIDGET(fs));
	return(0);
}

int scratch_delete(GtkWidget *widget)
{
	return(scratch_destroy(widget, NULL));
}

int loop_destroy(GtkWidget *widget, GtkFileSelection *fs)
{
	prelis_halt();
	loop_busy=0;
	loop_win=NULL;	
	if (fs) gtk_widget_destroy(GTK_WIDGET(fs));
	return(0);
}

int loop_delete(GtkWidget *widget)
{
	return(loop_destroy(widget, NULL));
}

void sel_change(GtkWidget *w)
{
	GtkFileSelection *fs;
	
	fs=GTK_FILE_SELECTION(gtk_widget_get_toplevel(w));
	
	prelis_start(gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)));

}


void select_scratch (GtkWidget *widget, gpointer data)
{
	GtkWidget *fs;
	
	if (scratch_busy)
	{
		gdk_window_raise(scratch_win);
		return;
	}
	
	fs=gtk_file_selection_new("Select Scratch File");	
	
	if (strlen(last_scratch) >0)
	{
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(fs), last_scratch);
	}
	else
	{
		if (strlen(globals.scratch_name) > 0)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(fs), globals.scratch_name);
	}

	gtk_widget_show(fs);
	
	
	scratch_win=fs->window;		
	scratch_busy=1;

	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button), "clicked", load_scratch, fs);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button), "clicked", GTK_SIGNAL_FUNC (scratch_destroy), fs);	
	gtk_signal_connect (GTK_OBJECT(fs), "delete-event", GTK_SIGNAL_FUNC(scratch_delete), NULL);	
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->file_list), "select_row", sel_change, fs);	
}


void select_loop (GtkWidget *widget, gpointer data)
{
	GtkWidget *fs;

	if (loop_busy)
	{
		gdk_window_raise(loop_win);
		return;
	}

	fs=gtk_file_selection_new("Select Loop File");

	if (strlen(last_loop) >0)
	{
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(fs), last_loop);
	}
	else
	{
		if (strlen(globals.loop_name) > 0)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(fs), globals.loop_name);
	}
	
	gtk_widget_show(fs);
		
	loop_win=fs->window;		
	loop_busy=1;
		
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button), "clicked", load_loop, fs);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button), "clicked", GTK_SIGNAL_FUNC(loop_destroy), fs);	
	gtk_signal_connect (GTK_OBJECT(fs), "delete-event", GTK_SIGNAL_FUNC(loop_delete), NULL);	
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->file_list), "select_row", sel_change, fs);
}

void mix_clicked(GtkWidget *widget)
{
	if (check_busy()) return;
	
	if (GTK_TOGGLE_BUTTON (widget)->active)
	{
		set_mix(1);
	}	
	else
	{
		set_mix(0);
	}
}

void mix_changed (GtkWidget *widget)
{
	f_prec value;
	
	value=(f_prec) GTK_ADJUSTMENT(widget)->value;

	vttgl->vol_loop=value;
	vttgl->vol_scratch=1.0-value;
	globals.scratch_vol=1.0-value;
}

void quit()
{
	globals.width=window->allocation.width;
	globals.height=window->allocation.height;

	gtk_main_quit();
}

void set_mode(GtkWidget *w)
{
	if (GTK_TOGGLE_BUTTON(mode_normal)->active)
	{
		vttgl->mode=MODE_SCRATCH;
		if (display_scratch)
		{
			display_scratch=0;
			gtk_tx_set_data(GTK_TX(wav_display), globals.scratch_data, globals.scratch_len);
		}		 
	}
	else if (GTK_TOGGLE_BUTTON(mode_record)->active)
	{ 
		vttgl->mode=MODE_RECORD_SCRATCH;
		
		if (display_scratch)
		{
			display_scratch=0;
			gtk_tx_set_data(GTK_TX(wav_display), globals.scratch_data, globals.scratch_len);
		}		 	
	}
	else if (GTK_TOGGLE_BUTTON(mode_playback)->active)
	{
		vttgl->mode=MODE_PLAYBACK_RECORDED;

		if (!display_scratch)
		{
			display_scratch=1;
			gtk_tx_set_data(GTK_TX(wav_display), globals.rec_buffer, globals.rec_len);
		}		 		
	}
}

void display_engine_error(int err)
{
	char buffer[2048]="Engine Error: ";
	
	if (err < ENG_ERR) return;
	
	switch (err)
	{
		case ENG_ERR_XOPEN:
			strcat(buffer, "Couldn't connect to X-Display.");
			break;

		case ENG_ERR_XINPUT:
			strcat(buffer, "Couldn't open XInput device.");
			break;
			
		case ENG_ERR_DGA:
			strcat(buffer, "Failed to enable DGA (DirectMouse).");
			break;
			
		case ENG_ERR_SOUND:
			strcat(buffer, "Failed to open audio device.");
			break;

		case ENG_ERR_THREAD:
			strcat(buffer, "Failed to run engine thread.");
			break;
			
		case ENG_ERR_GRABMOUSE:
			strcat(buffer, "Couldn't grab the mouse.");
			break;

		case ENG_ERR_GRABKEY:
			strcat(buffer, "Couldn't grab the keyboard.");
			break;
			
		case ENG_ERR_BUSY:
			strcat(buffer, "Oops! Engine running already?");
			break;
			
		default:
			strcat(buffer, "Oops. Internal Error :(");
	}
	tx_note(buffer);
}

int action()
{
	GtkWidget *top;
	int ret;

	if ((vttgl->mode==MODE_PLAYBACK_RECORDED) && (globals.rec_len<=0))
	{
		tx_note("Error: Nothing recorded - nothing to playback ;)");
		return(0);
	}
	
	if (!globals.scratch_data) 
	{
		tx_note("Error: No scratch data availabe. Please load a sample to scratch on first.");
		return(0);
	}

	if ((globals.do_mix) && (!globals.loop_data))
	{
		tx_note("Error: No mix data availabe. Disable mix.");
		return(0);
	}

	if (check_busy()) return(0);

	if (disk_status)
	{
		switch(disk_status)
		{
			case DISK_WRITING:
			tx_note("Fast User Warning: Please wait for terminatorX to finish writing to disk. Thanks!");
			break;
			
			case DISK_READING:
			tx_note("Fast User Warning: Please wait for terminatorX to finish reading from disk. Thanks!");			
			break;
		}
		return(0);
	}

	if (!xwindow)
	{
		top=gtk_widget_get_toplevel(window);
		xwindow=GDK_WINDOW_XWINDOW(top->window);
	}

	idle_ctr=0;
	
	ret=run_engine();
	
	if (!ret)
	{
		engine_busy=1;
		gtk_tx_prepare_pos_display(GTK_TX(wav_display));
		idle_tag = gtk_timeout_add(globals.update_idle, (GtkFunction) pos_update, NULL);
	}
	else
	{
		display_engine_error(ret);
		stop_engine();		
	}
	return(0);
}

int check_save_sanity(mix)
{
	if (globals.rec_len==0) 
	{
		tx_note("Save Error: Nothing recorded!");
		return(1);
	}
	
	if ((mix) && (globals.loop_data==NULL))
	{
		tx_note("Save Error: No loop file loaded. Try saving unmixed.");
		return(1);
	}
	
	return(0);
}

void do_save_fast(GtkWidget *w, int* mixgl)
{
	int mix=*mixgl;
	char filename[PATH_MAX];
	
	if(check_save_sanity(mix)) return;

	set_disk_status(DISK_WRITING);
	
	sprintf(filename, "%s%04i.wav", globals.prefix, globals.filectr);
	globals.filectr++;
	if (vtt_save(vttgl, filename, mix)) tx_note("Save Error: Error writing file.");

	set_disk_status(DISK_IDLE);
	
}

int save_destroy(GtkWidget *w, GtkFileSelection *fs)
{
	save_win=NULL;
	save_busy=0;
	if (fs) gtk_widget_destroy(GTK_WIDGET(fs));
	return(0);	
}

int save_delete(GtkWidget *w)
{
	return (save_destroy(w, NULL));
}


void save(GtkWidget *w, GtkFileSelection *fs)
{
	char newfile[PATH_MAX];
	
	if(check_save_sanity(1)) return;
		
	set_disk_status(DISK_WRITING);	
		
	strcpy(newfile, gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)));
	if (strlen(newfile))
	{
		if (vtt_save(vttgl, newfile, 0)) tx_note("Save Error: Error writing file.");	
		strcpy(globals.last_fn, newfile);		
	}
	save_destroy(w, fs);
	
	set_disk_status(DISK_IDLE);	
}

int save_mix_destroy(GtkWidget *w, GtkFileSelection *fs)
{
	save_mix_win=NULL;
	save_mix_busy=0;
	if (fs) gtk_widget_destroy(GTK_WIDGET(fs));
	return(0);	
}

int save_mix_delete(GtkWidget *w)
{
	return (save_mix_destroy(w, NULL));
}

void save_mix(GtkWidget *w, GtkFileSelection *fs)
{
	char newfile[PATH_MAX];
	
	if(check_save_sanity(1)) return;

	set_disk_status(DISK_WRITING);
	
	strcpy(newfile, gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)));
	if (strlen(newfile))
	{
		if (vtt_save(vttgl, newfile, 1)) tx_note("Save Error: Error writing file.");	
		strcpy(globals.last_fn, newfile);		
	}
	save_mix_destroy(w, fs);

	set_disk_status(DISK_IDLE);
	
}

void do_save_as(GtkWidget *w, int* mixgl)
{
	int mix=*mixgl;
	GtkWidget *fs;
	
	if(check_save_sanity(mix)) return;
	
	if ((mix) && (save_mix_busy))
	{
		gdk_window_raise(save_mix_win);
		return;
	}

	if ((!mix) && (save_busy))
	{
		gdk_window_raise(save_win);
		return;
	}

	if (mix)
	{	
		fs=gtk_file_selection_new("Save mixed Scratch");	
	}
	else
	{
		fs=gtk_file_selection_new("Save Scratch");		
	}
	
	
	if (strlen(globals.last_fn) > 0)
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(fs), globals.last_fn);

	gtk_widget_show(fs);
	
	if (!mix)
	{	
		save_win=fs->window;		
		save_busy=1;
	}
	else
	{
		save_mix_win=fs->window;		
		save_mix_busy=1;	
	}

	if (!mix)
	{
		gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button), "clicked", save, fs);
		gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button), "clicked", GTK_SIGNAL_FUNC(save_destroy), fs);	
		gtk_signal_connect (GTK_OBJECT(fs), "delete-event", GTK_SIGNAL_FUNC(save_delete), NULL);	
	}
	else
	{
		gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button), "clicked", save_mix, fs);
		gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button), "clicked", GTK_SIGNAL_FUNC(save_mix_destroy), fs);	
		gtk_signal_connect (GTK_OBJECT(fs), "delete-event", GTK_SIGNAL_FUNC(save_mix_delete), NULL);	
	}
}

void mplcfitx()
/* Most Proabably Least Called Function In Terminator X :) */
{
	show_about(0);
}

void scratch_drop(GtkWidget *w)
{
	puts("drop");
}

void lowpass_toggled(GtkWidget *widget)
{	
	if (GTK_TOGGLE_BUTTON (widget)->active)
	{
		globals.lowpass_enable=1;
	}	
	else
	{
		globals.lowpass_enable=0;
	}
}

void lowpass_reso_changed (GtkWidget *widget)
{
	f_prec value;
	
	value=(f_prec) GTK_ADJUSTMENT(widget)->value;

	vtt_lowpass_conf(vttgl, 1.0, value);
	globals.lowpass_reso=value;
}

void setup_signals()
{
	gtk_signal_connect (GTK_OBJECT(scratch_btn), "clicked", (GtkSignalFunc) select_scratch, NULL);
	gtk_signal_connect (GTK_OBJECT(loop_btn), "clicked", (GtkSignalFunc) select_loop, NULL);
	gtk_signal_connect (GTK_OBJECT(mix_toggle_btn), "clicked", (GtkSignalFunc) mix_clicked, NULL);
	gtk_signal_connect (GTK_OBJECT(mix_adjustment), "value_changed", (GtkSignalFunc) mix_changed, NULL);
	gtk_signal_connect (GTK_OBJECT(window), "destroy", (GtkSignalFunc) quit, NULL);
	gtk_signal_connect (GTK_OBJECT(mode_normal), "clicked", (GtkSignalFunc) set_mode, NULL);
	gtk_signal_connect (GTK_OBJECT(mode_record), "clicked", (GtkSignalFunc) set_mode, NULL);
	gtk_signal_connect (GTK_OBJECT(mode_playback), "clicked", (GtkSignalFunc) set_mode, NULL);
	gtk_signal_connect (GTK_OBJECT(action_btn), "clicked", (GtkSignalFunc) action, NULL);
	gtk_signal_connect (GTK_OBJECT(options_btn), "clicked", (GtkSignalFunc) display_options, NULL);
	gtk_signal_connect (GTK_OBJECT(save_fast), "clicked", (GtkSignalFunc) do_save_fast, &db_save_raw);	
	gtk_signal_connect (GTK_OBJECT(save_fast_mix), "clicked", (GtkSignalFunc) do_save_fast, &db_save_mix);	
	gtk_signal_connect (GTK_OBJECT(save_as), "clicked", (GtkSignalFunc) do_save_as, &db_save_raw);	
	gtk_signal_connect (GTK_OBJECT(save_as_mix), "clicked", (GtkSignalFunc) do_save_as, &db_save_mix);	
	gtk_signal_connect (GTK_OBJECT(about_btn), "clicked", (GtkSignalFunc) mplcfitx, NULL);	
	gtk_signal_connect (GTK_OBJECT(scratch_btn), "drag-drop", (GtkSignalFunc) scratch_drop, NULL);
	gtk_signal_connect (GTK_OBJECT(lowpass_toggle_btn), "clicked", (GtkSignalFunc) lowpass_toggled, NULL);
	gtk_signal_connect (GTK_OBJECT(lowpass_freq), "value_changed", (GtkSignalFunc) lowpass_reso_changed, NULL);
}

void create_gui(int x, int y)
{
	GtkWidget *label;
	GtkWidget *separator;
	GtkWidget *top_hbox;
	GtkWidget *sub_vbox1;
	GtkWidget *sub_vbox2;
	GtkWidget *sub_sub_hbox;
	GtkWidget *sub_sub_vbox;
	char *fn;
	
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), VERSIONSTRING);
	gtk_window_set_default_size(GTK_WINDOW(window), x, y);
	
	tooltips=gtk_tooltips_new();
	
	gtk_container_set_border_width(GTK_CONTAINER(window), 5);

	mix_adjustment = (GtkAdjustment*) gtk_adjustment_new (1.0-globals.scratch_vol, 0, 1, 0.001, 0.01, 0.01);
	
	top_hbox = gtk_hbox_new(FALSE, 5);
	
	sub_vbox1 = gtk_vbox_new(FALSE, 5);

	/* begin upper box */
	
	sub_sub_hbox = gtk_hbox_new(FALSE, 5);	
	
	label = gtk_label_new ("Scratch: ");
	gtk_misc_set_alignment (GTK_MISC(label), 0.5 ,0.5);
	gtk_box_pack_start (GTK_BOX(sub_sub_hbox), label, WID_FIX);
	gtk_widget_show(label);
	
	if (strlen(globals.scratch_name))
	{
		fn=strrchr(globals.scratch_name, '/');
		if (fn) fn++;
		else fn=globals.scratch_name;
		scratch_btn = gtk_button_new_with_label (fn);
	}
	else
	{
		scratch_btn = gtk_button_new_with_label ("<NONE>");	
	}
	gtk_box_pack_start (GTK_BOX(sub_sub_hbox), scratch_btn, WID_DYN);
	gtk_tooltips_set_tip(tooltips, scratch_btn, "Click here to load a new wave-file to scratch on.", NULL);
	gtk_widget_show(scratch_btn);	
	
	mix_toggle_btn = gtk_check_button_new_with_label ("Mix");
	gtk_box_pack_start (GTK_BOX(sub_sub_hbox), mix_toggle_btn, WID_FIX);
	gtk_tooltips_set_tip(tooltips, mix_toggle_btn, "Click here to en-/disable mixing of loop wave-file.", NULL);	
	gtk_widget_show(mix_toggle_btn);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mix_toggle_btn), globals.do_mix);
	
	mix_slider = gtk_hscale_new(mix_adjustment);
	gtk_widget_set_usize(mix_slider, 100, 0);
	gtk_scale_set_draw_value(GTK_SCALE(mix_slider), 0);
	gtk_box_pack_start (GTK_BOX(sub_sub_hbox), mix_slider, WID_DYN);
	gtk_tooltips_set_tip(tooltips, mix_slider, "Select mixing ratio scratch/loop.", NULL);	
	gtk_widget_show(mix_slider);	
			
	label = gtk_label_new ("Loop: ");
	gtk_misc_set_alignment (GTK_MISC(label), 0.5 ,0.5);
	gtk_box_pack_start (GTK_BOX(sub_sub_hbox), label, WID_FIX);
	gtk_widget_show(label);
	
	if (strlen(globals.loop_name))
	{
		fn=strrchr(globals.loop_name, '/');
		if (fn) fn++;
		else fn=globals.loop_name;
		loop_btn = gtk_button_new_with_label (fn);
	}
	else
	{
		loop_btn = gtk_button_new_with_label ("<NONE>");	
	}
	gtk_box_pack_start (GTK_BOX(sub_sub_hbox), loop_btn, WID_DYN);
	gtk_tooltips_set_tip(tooltips, loop_btn, "Click here to load a new wave-file as loop.", NULL);
	gtk_widget_show(loop_btn);	
	
	gtk_box_pack_start(GTK_BOX(sub_vbox1), sub_sub_hbox, WID_FIX);
	gtk_widget_show(sub_sub_hbox);
	
	/* end upper box */	
	
	separator = gtk_hseparator_new ();
	gtk_box_pack_start(GTK_BOX(sub_vbox1), separator, WID_FIX);
	gtk_widget_show(separator);
	
	/* begin lower left box */
	
	sub_sub_vbox = gtk_vbox_new(FALSE, 5);
	
	wav_display = gtk_tx_new (globals.scratch_data, globals.scratch_size/2);
	gtk_box_pack_start (GTK_BOX(sub_sub_vbox), wav_display, WID_DYN);
	gtk_widget_show(wav_display);	

	wav_progress = gtk_progress_bar_new();
	gtk_box_pack_start (GTK_BOX(sub_sub_vbox), wav_progress, WID_FIX);
	gtk_widget_show(wav_progress);

	/* end lower left box */

	gtk_box_pack_start(GTK_BOX(sub_vbox1), sub_sub_vbox, WID_DYN);
	gtk_widget_show(sub_sub_vbox);

	
	/* begin lower right box */

	sub_vbox2 = gtk_vbox_new(TRUE, 5);

	label = gtk_label_new ("Operation:");
	gtk_misc_set_alignment (GTK_MISC(label), 0.5 ,0.5);
	gtk_box_pack_start (GTK_BOX(sub_vbox2), label, WID_FIX);
	gtk_widget_show(label);	
	
	mode_normal = gtk_radio_button_new_with_label(NULL, "Free Scratch");
	gtk_tooltips_set_tip(tooltips, mode_normal, "Click here to enable \"normal\", unlimited scratching.", NULL);	
	gtk_box_pack_start (GTK_BOX(sub_vbox2), mode_normal, WID_FIX);
	gtk_widget_show(mode_normal);	
	
	mode_radios = gtk_radio_button_group( GTK_RADIO_BUTTON(mode_normal) );
	
	mode_record = gtk_radio_button_new_with_label(mode_radios, "Record Scratch");
	gtk_box_pack_start (GTK_BOX(sub_vbox2), mode_record, WID_FIX);
	gtk_tooltips_set_tip(tooltips, mode_record, "Click here to enable recording to record buffer.", NULL);	
	gtk_widget_show(mode_record);	

	mode_radios = gtk_radio_button_group( GTK_RADIO_BUTTON(mode_record) );
		
	mode_playback = gtk_radio_button_new_with_label(mode_radios, "Playback Scratch");
	gtk_box_pack_start (GTK_BOX(sub_vbox2), mode_playback, WID_FIX);	
	gtk_tooltips_set_tip(tooltips, mode_playback, "Click here to enable playback of your recorded scratches.", NULL);		
	gtk_widget_show(mode_playback);	
	
	action_btn = gtk_button_new_with_label ("Start");
	gtk_box_pack_start (GTK_BOX(sub_vbox2), action_btn, WID_FIX);	
	gtk_tooltips_set_tip(tooltips, action_btn, "Click here to activate audio.", NULL);		
	gtk_widget_show(action_btn);	

	separator = gtk_hseparator_new ();
	gtk_box_pack_start(GTK_BOX(sub_vbox2), separator, WID_FIX);
	gtk_widget_show(separator);
	
	lowpass_toggle_btn = gtk_check_button_new_with_label ("LP Filter");
	gtk_box_pack_start (GTK_BOX(sub_vbox2), lowpass_toggle_btn, WID_FIX);
	gtk_tooltips_set_tip(tooltips, mix_toggle_btn, "Click here to en-/disable the lowpass filter.", NULL);	
	gtk_widget_show(lowpass_toggle_btn);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lowpass_toggle_btn), globals.lowpass_enable);

	lowpass_freq = (GtkAdjustment*) gtk_adjustment_new (globals.lowpass_reso, 0, 0.95, 0.001, 0.001, 0.01);
	
	lowpass_slider = gtk_hscale_new(lowpass_freq);
	gtk_scale_set_draw_value(GTK_SCALE(lowpass_slider), 0);
	gtk_box_pack_start (GTK_BOX(sub_vbox2), lowpass_slider, WID_FIX);
	gtk_tooltips_set_tip(tooltips, lowpass_slider, "This sets the resonance parameter of the lowpass filter.", NULL);	
	gtk_widget_show(lowpass_slider);	
		
	separator = gtk_hseparator_new ();
	gtk_box_pack_start(GTK_BOX(sub_vbox2), separator, WID_FIX);
	gtk_widget_show(separator);
	
	save_fast = gtk_button_new_with_label ("Save Fast");
	gtk_box_pack_start (GTK_BOX(sub_vbox2), save_fast, WID_FIX);	
	gtk_tooltips_set_tip(tooltips, save_fast, "Click here to save your recorded scratches without giving a name.", NULL);		
	gtk_widget_show(save_fast);	

	save_fast_mix = gtk_button_new_with_label ("Save Fast (mix)");
	gtk_tooltips_set_tip(tooltips, save_fast_mix, "Click here to save your recorded scratches (mixed with the loop) without giving a name.", NULL);		
	gtk_box_pack_start (GTK_BOX(sub_vbox2), save_fast_mix, WID_FIX);	
	gtk_widget_show(save_fast_mix);	

	save_as = gtk_button_new_with_label ("Save As");
	gtk_box_pack_start (GTK_BOX(sub_vbox2), save_as, WID_FIX);	
	gtk_tooltips_set_tip(tooltips, save_as, "Click here to save your recorded scratches after selecting a filename.", NULL);			
	gtk_widget_show(save_as);	

	save_as_mix = gtk_button_new_with_label ("Save As (mix)");
	gtk_box_pack_start (GTK_BOX(sub_vbox2), save_as_mix, WID_FIX);	
	gtk_tooltips_set_tip(tooltips, save_as_mix, "Click here to save your recorded scratches (mixed with the loop) after selecting a filename.", NULL);			
	gtk_widget_show(save_as_mix);	

	separator = gtk_hseparator_new ();
	gtk_box_pack_start(GTK_BOX(sub_vbox2), separator, WID_FIX);
	gtk_widget_show(separator);

	options_btn = gtk_button_new_with_label ("Options");
	gtk_box_pack_start (GTK_BOX(sub_vbox2), options_btn, WID_FIX);	
	gtk_tooltips_set_tip(tooltips, options_btn, "Click here to setup options. (And to disable tooptips ;)", NULL);				
	gtk_widget_show(options_btn);	
	
	about_btn = gtk_button_new_with_label ("About / License");
	gtk_box_pack_start (GTK_BOX(sub_vbox2), about_btn, WID_FIX);	
	gtk_tooltips_set_tip(tooltips, about_btn, "Click here to learn more about terminatorX.", NULL);				
	gtk_widget_show(about_btn);	

	separator = gtk_hseparator_new ();
	gtk_box_pack_start(GTK_BOX(sub_vbox2), separator, WID_FIX);
	gtk_widget_show(separator);
	
	pos_time = gtk_button_new_with_label ("00:00.00");
//	gtk_misc_set_alignment (GTK_MISC(pos_time), 0.5 ,0.5);
	gtk_box_pack_start (GTK_BOX(sub_vbox2), pos_time, WID_FIX);
	gtk_widget_show(pos_time);	
	
	/* end lower right box */
	
	gtk_box_pack_start(GTK_BOX(top_hbox), sub_vbox1, WID_DYN);
	gtk_widget_show(sub_vbox1);
	
	separator = gtk_vseparator_new ();
	
	gtk_box_pack_start(GTK_BOX(top_hbox), separator, WID_FIX);
	gtk_widget_show(separator);

	gtk_box_pack_start(GTK_BOX(top_hbox), sub_vbox2, WID_FIX);
	gtk_widget_show(sub_vbox2);
		
	gtk_container_add(GTK_CONTAINER(window), top_hbox);
	
	gtk_widget_show(top_hbox);

	setup_signals();

}

void display_gui()
{
	if (globals.tooltips) gtk_tooltips_enable(tooltips);
	else gtk_tooltips_disable(tooltips);

	gtk_widget_show(window);
	/* ARRGH!! 
	*/
}

void note_destroy(GtkWidget *widget, GtkWidget *mbox)
{
	gtk_widget_destroy(mbox);
}

void tx_note(char *message)
{
	char buffer[4096]="\n     [ terminatorX Message: ]     \n\n";
	
	GtkWidget *mbox;
	GtkWidget *label;
	GtkWidget *btn;
	
	mbox=gtk_dialog_new();

	strcat(buffer, "   ");	
	strcat(buffer, message);
	strcat(buffer, "   ");
	label=gtk_label_new(buffer);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(mbox)->vbox), label, TRUE, TRUE, 0);
	gtk_widget_show(label);
	
	btn = gtk_button_new_with_label("Ok");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(mbox)->action_area), btn, TRUE, TRUE, 0);
	gtk_widget_show(btn);
	
	gtk_signal_connect(GTK_OBJECT(btn), "clicked", note_destroy, mbox);
	
	gtk_widget_show(mbox);
}

gfloat old_percent=-1;

void wav_progress_update(gfloat percent)
{
	percent=floor(percent*10.0)/10.0; //Updateing statusbars with gtk-themes eats up hell of a lot CPU-time
					  // which is why we update every 10% only.
	
	if (wav_progress)
	{
		if (old_percent != percent)
		{
			old_percent = percent;
			gtk_progress_bar_update(GTK_PROGRESS_BAR(wav_progress), percent);
			while (gtk_events_pending()) gtk_main_iteration();	
		}
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
