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
 
    File: tX_mastergui.cc
 
    Description: This implements the main (aka master) gtk+ GUI of terminatorX
    		 It serves as a container for the vtt-guis.
*/    

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "version.h"
#include "tX_global.h"
#include "tX_engine.h"
#include "tX_vttgui.h"
#include "tX_vtt.h"
#include "tX_flash.h"
#include "tX_dialog.h"
#include "tX_loaddlg.h"
#include "tX_seqpar.h"
#include "tX_pbutton.h"
#include "tX_sequencer.h"
#include "tX_mastergui.h"

/* I don't know a display that can handle that, but
   just to be on the safe side...
*/
#define MAX_ROWS 25

#ifdef USE_SCHEDULER
#include <sys/time.h>
#include <sys/resource.h>
#endif

#define TX_SET_ID_10 "terminatorX turntable set file - version 1.0 - data:"
#define TX_SET_ID_11 "terminatorX turntable set file - version 1.1 - data:"
#define TX_SET_ID_12 "terminatorX turntable set file - version 1.2 - data:"
#define TX_SET_ID_13 "terminatorX turntable set file - version 1.3 - data:"

int audioon=0;
int sequencer_ready=1;

GtkWidget *tt_parent;
GtkWidget *control_parent;
GtkWidget *audio_parent;
GtkWidget *main_window;
GtkWidget *wav_progress;
GtkWidget *grab_button;
GtkWidget *main_flash_l;
GtkWidget *main_flash_r;
GtkWidget *rec_btn;

GtkWidget *seq_rec_btn;
GtkWidget *seq_play_btn;
GtkWidget *seq_stop_btn;
GtkAdjustment *seq_adj;
GtkWidget *seq_slider;
GtkWidget *seq_entry;
int seq_adj_care=1;
int seq_stop_override=0;

GtkAdjustment *volume_adj;
GtkAdjustment *pitch_adj;

/* seq-pars */
tX_seqpar_master_volume sp_master_volume;
tX_seqpar_master_pitch sp_master_pitch;

GtkWidget *AddTable;
GtkWidget *LoadSet;
GtkWidget *SaveSet;

GtkWidget *engine_btn;

int rec_dont_care=0;
gint update_tag;

#define connect_entry(wid, func, ptr); gtk_signal_connect(GTK_OBJECT(wid), "activate", (GtkSignalFunc) func, (void *) ptr);
#define connect_adj(wid, func, ptr); gtk_signal_connect(GTK_OBJECT(wid), "value_changed", (GtkSignalFunc) func, (void *) ptr);
#define connect_button(wid, func, ptr); gtk_signal_connect(GTK_OBJECT(wid), "clicked", (GtkSignalFunc) func, (void *) ptr);

Window xwindow;
#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0
extern void add_vtt(GtkWidget *ctrl, GtkWidget *audio, char *fn);
extern void recreate_gui(vtt_class *vtt, GtkWidget *daddy);
extern void destroy_gui(vtt_class *vtt);
extern void gui_show_frame(vtt_class *vtt, int show);

GdkWindow *save_dialog_win=NULL;
GdkWindow *load_dialog_win=NULL;
GtkWidget *save_dialog=NULL;
GtkWidget *load_dialog=NULL;

GdkWindow *rec_dialog_win=NULL;
GtkWidget *rec_dialog=NULL;

GtkWidget *no_of_vtts=NULL;
GtkWidget *used_mem=NULL;

int stop_update=0;
int update_delay;

int mg_hide_gui=0;
vtt_class *old_focus=NULL;

int grab_status=0;
int last_grab_status=0;

void tx_note(const char *message);

GtkTooltips *gui_tooltips=NULL;

void gui_set_tooltip(GtkWidget *wid, char *tip)
{
	gtk_tooltips_set_tip(gui_tooltips, wid, tip, NULL);
}

void turn_audio_off(void)
{
	if (audioon) 
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(engine_btn), 0);
		while (gtk_events_pending()) gtk_main_iteration();		
	}
}


gint pos_update(gpointer data)
{
	f_prec temp;

	if (stop_update) 
	{		
		cleanup_all_vtts();
		tX_seqpar :: update_all_graphics();
		if (old_focus) gui_show_frame(old_focus, 0);
		old_focus=NULL;
		gtk_tx_flash_clear(main_flash_l);
		gtk_tx_flash_clear(main_flash_r);
		gdk_flush();	
		update_tag=0;
		return(FALSE);
	}
	else
	{
		update_all_vtts();
		
		/*left vu meter */
		temp=vtt_class::mix_max_l;
		vtt_class::mix_max_l=0;
		gtk_tx_flash_set_level(main_flash_l, temp);

		/*right vu meter */
		temp=vtt_class::mix_max_r;
		vtt_class::mix_max_r=0;
		gtk_tx_flash_set_level(main_flash_r, temp);
		
		if (vtt_class::focused_vtt!=old_focus)
		{
			if (old_focus) gui_show_frame(old_focus, 0);
			old_focus=vtt_class::focused_vtt;
			if (old_focus) gui_show_frame(old_focus, 1);			
		}
		if (grab_status!=last_grab_status)
		{
			last_grab_status=grab_status;
			if (!grab_status) 
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(grab_button), 0);
			}
		}
		gdk_flush();	
		update_delay--;
		
		if (update_delay < 0)
		{
			seq_update();
			tX_seqpar :: update_all_graphics();
			update_delay=globals.update_delay;
		}
		return(TRUE);
	}
}

void mg_update_status()
{
	FILE *procfs;
	pid_t mypid;
	char filename[PATH_MAX];
	char buffer[256];
	int found=0;	
	int mem;
	
	mypid=getpid();
	sprintf(filename, "/proc/%i/status", mypid);
	procfs=fopen(filename, "r");
	if (procfs)
	{
		while((!feof(procfs)) && !found)
		{
			fgets(buffer, 256, procfs);
			if (strncmp("VmSize:", buffer, 7)==0)
			{
				found=1;
				sscanf(buffer, "VmSize: %i kB", &mem);
				sprintf(buffer, "%i", mem);
				gtk_label_set(GTK_LABEL(used_mem), buffer);
			}
		}
	}
	fclose(procfs);	
	
	sprintf(buffer, "%i", vtt_class::vtt_amount);
	gtk_label_set(GTK_LABEL(no_of_vtts), buffer);
}

GtkSignalFunc new_table(GtkWidget *, char *fn)
{
	int i;
	
	turn_audio_off();
		
		if (fn) 
		{
			ld_create_loaddlg(TX_LOADDLG_MODE_SINGLE, 1);
			ld_set_filename(fn);
		}
		
		add_vtt(control_parent, audio_parent, fn);				
		
		if (fn) ld_destroy();		
	mg_update_status();
	return NULL;
}

GtkSignalFunc drop_new_table(GtkWidget *widget, GdkDragContext *context,
		gint x, gint y, GtkSelectionData *selection_data,
		guint info, guint time, vtt_class *vtt)
{
	char filename[PATH_MAX];
	char *fn;
	int s;
	void *prr;
	
	strncpy(filename, (char *) selection_data->data, (size_t) selection_data->length);
	filename[selection_data->length]=0;

	fn = strchr (filename, '\r');
	*fn=0;	
	
	fn = strchr (filename, ':');
	if (fn) fn++; else fn=(char *) selection_data->data;
	
	new_table(NULL, fn);

	strcpy (filename, "dont segfault workaround ;)");
	return NULL;
}


/* Loading saved setups */

GtkSignalFunc cancel_load_tables(GtkWidget *wid)
{
	gtk_widget_destroy(load_dialog);
	load_dialog=NULL;
	load_dialog_win=NULL;
	return(0);
}

void load_tt_part(char * buffer)
{
	FILE *in;
	char idbuff[256];
	char wbuf[PATH_MAX];

	turn_audio_off();
	
	strcpy(globals.tables_filename, buffer);

	in=fopen(buffer, "r");	
	
	if (in)
	{
		fread(idbuff, strlen(TX_SET_ID_10), 1, in);
		if (strncmp(idbuff, TX_SET_ID_10, strlen(TX_SET_ID_10))==0)
		{
			if (vtt_class::load_all_10(in, buffer)) tx_note("Error while reading set.");
		}
		else if (strncmp(idbuff, TX_SET_ID_11, strlen(TX_SET_ID_11))==0)
		{
			if (vtt_class::load_all_11(in, buffer)) tx_note("Error while reading set.");			
		}
		else if (strncmp(idbuff, TX_SET_ID_12, strlen(TX_SET_ID_12))==0)
		{
			if (vtt_class::load_all_12(in, buffer)) tx_note("Error while reading set.");			
		}
		else if (strncmp(idbuff, TX_SET_ID_13, strlen(TX_SET_ID_13))==0)
		{
			if (vtt_class::load_all_13(in, buffer)) tx_note("Error while reading set.");			
		}
		else
		{
			tx_note("Sorry, this file is not a terminatorX set-file.");
			fclose(in);
			return;
		}
		fclose(in);
		
		tX_seqpar :: init_all_graphics();
		vg_init_all_non_seqpars();
		
		gtk_adjustment_set_value(volume_adj, 2.0-globals.volume);
		gtk_adjustment_set_value(pitch_adj, globals.pitch);
		sprintf(wbuf,"terminatorX - %s", strip_path(buffer));
		gtk_window_set_title(GTK_WINDOW(main_window), wbuf);		
	}
	else
	{
		strcpy(idbuff, "Failed to access file: \"");	// I'm stealing the unrelated sting for a temp :)
		strcat(idbuff, globals.tables_filename);
		strcat(idbuff, "\"");
		tx_note(idbuff);
	}
}

void do_load_tables(GtkWidget *wid)
{
	char buffer[PATH_MAX];
	
	strcpy(buffer, gtk_file_selection_get_filename(GTK_FILE_SELECTION(load_dialog)));
	
	gtk_widget_destroy(load_dialog);
	
	load_dialog=NULL;
	load_dialog_win=NULL;

	load_tt_part(buffer);	
}

GtkSignalFunc load_tables()
{
	if (load_dialog_win) 
	{
		gdk_window_raise(load_dialog_win);
		return 0;
	}
	
	load_dialog=gtk_file_selection_new("Load Set");	
	
	gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(load_dialog));
	gtk_file_selection_complete(GTK_FILE_SELECTION(load_dialog), "*.tX");
	
	if (strlen(globals.tables_filename))
	{
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(load_dialog), globals.tables_filename);
	}
	
	gtk_widget_show(load_dialog);
	
	load_dialog_win=load_dialog->window;
	
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(load_dialog)->ok_button), "clicked", GTK_SIGNAL_FUNC(do_load_tables), NULL);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(load_dialog)->cancel_button), "clicked", GTK_SIGNAL_FUNC (cancel_load_tables), NULL);	
	gtk_signal_connect (GTK_OBJECT(load_dialog), "delete-event", GTK_SIGNAL_FUNC(cancel_load_tables), NULL);	
}

GtkSignalFunc drop_set(GtkWidget *widget, GdkDragContext *context,
		gint x, gint y, GtkSelectionData *selection_data,
		guint info, guint time, vtt_class *vtt)
{
	char filename[PATH_MAX];
	char *fn;
	int s;
	void *prr;
	
	strncpy(filename, (char *) selection_data->data, (size_t) selection_data->length);
	filename[selection_data->length]=0;

	fn = strchr (filename, '\r');
	*fn=0;	
	
	fn = strchr (filename, ':');
	if (fn) fn++; else fn=(char *) selection_data->data;
	
	load_tt_part(fn);

	strcpy (filename, "dont segfault workaround ;)");
	return NULL;
}


/* save tables */

GtkSignalFunc cancel_save_tables(GtkWidget *wid)
{
	gtk_widget_destroy(save_dialog);
	save_dialog=NULL;
	save_dialog_win=NULL;
	return(0);
}

void do_save_tables(GtkWidget *wid)
{
	FILE *out;
	char buffer[PATH_MAX];
	char wbuf[PATH_MAX];
	char *ext;
	char idbuffer[256];
	
	strcpy(buffer, gtk_file_selection_get_filename(GTK_FILE_SELECTION(save_dialog)));
	strcpy(globals.tables_filename, buffer);
	
	gtk_widget_destroy(save_dialog);
	
	save_dialog=NULL;
	save_dialog_win=NULL;
	
	ext=strrchr(buffer, '.');
	
	if (ext)
	{
		if (strcmp(ext, ".tX")) strcat(buffer, ".tX");
	}
	else
	{
		strcat(buffer, ".tX");
	}
	
	out=fopen(buffer, "w");
	
	if (out)
	{
		strcpy(idbuffer, TX_SET_ID_13);
		fwrite(idbuffer, strlen(idbuffer), 1, out);
		if (vtt_class::save_all(out)) tx_note("Error while saving set.");
		fclose(out);
		sprintf(wbuf,"terminatorX - %s", strip_path(buffer));
		gtk_window_set_title(GTK_WINDOW(main_window), wbuf);				
	}
	else
	{
		tx_note("Failed to access file.");
	}
}

GtkSignalFunc save_tables()
{
	if (save_dialog_win) 
	{
		gdk_window_raise(save_dialog_win);
		return 0;
	}
	
	save_dialog=gtk_file_selection_new("Save Set");	
	
	if (strlen(globals.tables_filename))
	{
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(save_dialog), globals.tables_filename);
	}
	
	gtk_widget_show(save_dialog);
	
	save_dialog_win=save_dialog->window;
	
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(save_dialog)->ok_button), "clicked", GTK_SIGNAL_FUNC(do_save_tables), NULL);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(save_dialog)->cancel_button), "clicked", GTK_SIGNAL_FUNC (cancel_save_tables), NULL);	
	gtk_signal_connect (GTK_OBJECT(save_dialog), "delete-event", GTK_SIGNAL_FUNC(cancel_save_tables), NULL);	
}

GtkSignalFunc master_volume_changed (GtkWidget *wid, void *d)
{
	sp_master_volume.receive_gui_value((float) 2.0-GTK_ADJUSTMENT(wid)->value);
	return NULL;	
}

GtkSignalFunc master_pitch_changed(GtkWidget *wid, void *d)
{
	sp_master_pitch.receive_gui_value((float) GTK_ADJUSTMENT(wid)->value);	
	return NULL;	
}

GtkSignalFunc saturate_changed(GtkWidget *w, void *d)
{
	vtt_class::enable_saturate (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)));
	return NULL;	
}

void mg_enable_critical_buttons(int enable)
{
	gtk_widget_set_sensitive(seq_rec_btn, enable);
	gtk_widget_set_sensitive(seq_play_btn, enable);
	gtk_widget_set_sensitive(seq_slider, enable);

	gtk_widget_set_sensitive(rec_btn, enable);
	vg_enable_critical_buttons(enable);
}


#ifdef USE_SCHEDULER
int mg_oldprio;
#endif

GtkSignalFunc seq_stop(GtkWidget *w, void *);

GtkSignalFunc audio_on(GtkWidget *w, void *d)
{
	int res;
	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
	{		
		sequencer_ready=0;
		mg_enable_critical_buttons(0);
		res=run_engine();
		sequencer_ready=1;

		if (res!=TX_ENG_OK)
		{
			mg_enable_critical_buttons(1);
			switch(res)
			{
				case TX_ENG_ERR_BUSY:
				tx_note("Error starting engine: engine is already running.");
				break;
				case TX_ENG_ERR_DEVICE:
				tx_note("Error starting engine: failed to access audiodevice.");
				break;
				case TX_ENG_ERR_TAPE:
				tx_note("Error starting engine: failed to open the recording file.");
				break;
				case TX_ENG_ERR_THREAD:
				tx_note("Error starting engine: failed to create the engine thread.");
				break;
				default:tx_note("Error starting engine: Unknown error.");
			}
			return 0;
		}

		stop_update=0;
		audioon=1;
		update_delay=globals.update_delay;
		update_tag=gtk_timeout_add(globals.update_idle, (GtkFunction) pos_update, NULL);
		gtk_widget_set_sensitive(grab_button, 1);
	}
	else
	{		
		if (!sequencer_ready) return NULL;
		gtk_widget_set_sensitive(grab_button, 0);
		stop_engine();
		stop_update=1;
		audioon=0;
		if ((want_recording) && (!globals.autoname))
		{
			want_recording=0;
			rec_dont_care=1;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rec_btn), 0);
			rec_dont_care=0;
		}
		seq_stop(NULL, NULL);
		mg_enable_critical_buttons(1);
	}
}

GtkSignalFunc cancel_rec(GtkWidget *wid)
{
	gtk_widget_destroy(rec_dialog);
	rec_dialog=NULL;
	rec_dialog_win=NULL;
	rec_dont_care=0;
	return(0);
}

void do_rec(GtkWidget *wid)
{
	char buffer[PATH_MAX];
	
	strcpy(buffer, gtk_file_selection_get_filename(GTK_FILE_SELECTION(rec_dialog)));

	if (strlen(buffer))
	{
		strcpy(globals.record_filename, buffer);		
		want_recording=1;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rec_btn), 1);
	}
	
	rec_dont_care=0;
	
	gtk_widget_destroy(rec_dialog);
	
	rec_dialog=NULL;
	rec_dialog_win=NULL;
}

GtkSignalFunc select_rec_file()
{
	if (rec_dialog_win) 
	{
		gdk_window_raise(rec_dialog_win);
		return 0;
	}
	
	rec_dialog=gtk_file_selection_new("Record To Disk");	
	
	if (strlen(globals.record_filename))
	{
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(rec_dialog), globals.record_filename);
	}
	
	gtk_widget_show(rec_dialog);
	
	rec_dialog_win=rec_dialog->window;
	
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(rec_dialog)->ok_button), "clicked", GTK_SIGNAL_FUNC(do_rec), NULL);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(rec_dialog)->cancel_button), "clicked", GTK_SIGNAL_FUNC (cancel_rec), NULL);	
	gtk_signal_connect (GTK_OBJECT(rec_dialog), "delete-event", GTK_SIGNAL_FUNC(cancel_rec), NULL);	
}

GtkSignalFunc tape_on(GtkWidget *w, void *d)
{
	if (rec_dont_care) return 0;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
	{	
		{
			rec_dont_care=1;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), 0);
			select_rec_file();
		}
	}
	else
	{
			want_recording=0;
	}
}

void grab_on(GtkWidget *w, void *d)
{
	grab_mouse(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)));
	grab_status=1;
}

void grab_off()
{
	grab_status=0;
}

void hide_clicked(GtkWidget *w, void *d)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
	{
		mg_hide_gui=1;
	}
	else
	{
		mg_hide_gui=0;
	}
}

void quit()
{
	turn_audio_off();
	if (update_tag)
	gtk_timeout_remove(update_tag);
	globals.width=main_window->allocation.width;
	globals.height=main_window->allocation.height;

	gtk_main_quit();
}

void mplcfitx()
/* Most Probably Least Called Function In Terminator X :) */
{
	show_about(0);
}

GtkSignalFunc seq_play(GtkWidget *w, void *)
{
	if (seq_stop_override) return NULL;
	seq_adj_care=0;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), 1);
	sequencer.trig_play();
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(engine_btn), 1);
}

GtkSignalFunc seq_stop(GtkWidget *w, void *)
{
	if (!sequencer_ready) return NULL;
	sequencer.trig_stop();
	seq_adj_care=1;
	seq_stop_override=1;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(seq_play_btn), 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(seq_rec_btn), 0);
	while (gtk_events_pending()) gtk_main_iteration();		
	seq_stop_override=0;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(engine_btn), 0);	
	gtk_widget_set_sensitive(seq_slider, 1);	
	gtk_widget_set_sensitive(engine_btn, 1);
	gtk_widget_set_sensitive(seq_rec_btn, 1);
}

GtkSignalFunc seq_rec(GtkWidget *w, void *)
{
	seq_adj_care=0;
	gtk_widget_set_sensitive(seq_slider, 0);

	if (seq_stop_override) return NULL;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), 1);
	gtk_widget_set_sensitive(engine_btn, 0);
	gtk_widget_set_sensitive(seq_rec_btn, 0);
	sequencer.trig_rec();
}

void seq_update_entry(const guint32 timestamp)
{
	char buffer[20];
	guint32 samples;
	guint32 minu,sec,hun;	
	
	samples=timestamp*globals.true_block_size;
	
	if (samples>0)
	{
		minu=samples/2646000;
		samples-=2646000*minu;
	
		sec=samples/44100;
		samples-=44100*sec;
	
		hun=samples/441;
	}
	else
	{
		minu=sec=hun=0;
	}
	
	sprintf(buffer, "%02li:%02li.%02li", minu, sec, hun);
	gtk_entry_set_text(GTK_ENTRY(seq_entry), buffer);
}

void seq_update()
{
	seq_update_entry(sequencer.get_timestamp());
	gtk_adjustment_set_value(seq_adj, sequencer.get_timestamp_as_float());
	
}
void seq_slider_released(GtkWidget *wid, void *d)
{
	seq_adj_care=0;
	gtk_widget_set_sensitive(seq_slider, 0);	
	sequencer.forward_to_start_timestamp(0);
	gtk_widget_set_sensitive(seq_slider, 1);	
	seq_adj_care=1;
}
void sequencer_move(GtkWidget *wid, void *d)
{
	guint32 pos;
	
	if (seq_adj_care)
	{
		pos=sequencer.set_start_timestamp((float) GTK_ADJUSTMENT(wid)->value);
		seq_update_entry(pos);	
	}
}

#define add_sep(); 	dummy=gtk_hseparator_new ();\
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);\
	gtk_widget_show(dummy);\

#define add_sep2(); 	dummy=gtk_hseparator_new ();\
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);\
	gtk_widget_show(dummy);\

void create_mastergui(int x, int y)
{
	GtkWidget *main_vbox;
	GtkWidget *right_hbox;
	GtkWidget *left_hbox;
	GtkWidget *control_box;
	GtkWidget *sequencer_box;
	GtkAdjustment *dumadj;
	GtkWidget *dummy;
	GtkWidget *small_box;
	GtkWidget *smaller_box;
	
	int i;
	static GtkTargetEntry drop_types [] = {
		{ "text/uri-list", 0, 0}
	};
	static gint n_drop_types = sizeof (drop_types) / sizeof(drop_types[0]);
	
	gui_tooltips=gtk_tooltips_new();

	main_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_wmclass(GTK_WINDOW(main_window), "terminatorX", "tX_mastergui");

	gtk_window_set_title(GTK_WINDOW(main_window), "terminatorX");
	
	gtk_container_set_border_width(GTK_CONTAINER(main_window), 5);

	gtk_widget_realize(main_window);
	
	main_vbox=gtk_hbox_new(FALSE, 5);
	
	gtk_container_add(GTK_CONTAINER(main_window), main_vbox);
	gtk_widget_show(main_vbox);
	
	left_hbox=gtk_vbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), left_hbox, WID_DYN);
	gtk_widget_show(left_hbox);
	
	control_box=gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(left_hbox), control_box, WID_FIX);
	gtk_widget_show(control_box);
	
/*	dummy=gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(left_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);*/

	sequencer_box=gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(left_hbox), sequencer_box, WID_FIX);
	gtk_widget_show(sequencer_box);

	dummy=gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(left_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=tx_xpm_label_box(TX_ICON_AUDIOENGINE, "Audio Eng.");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	dummy=tx_xpm_button_new(TX_ICON_POWER,"Power ", 1);
	connect_button(dummy,audio_on, NULL);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Turn the audio engine on/off.");
	gtk_widget_show(dummy);
	engine_btn=dummy;
	
	grab_button=tx_xpm_button_new(TX_ICON_GRAB, "Mouse Grab ", 1);
	gtk_box_pack_start(GTK_BOX(control_box), grab_button, WID_FIX);
	connect_button(grab_button, grab_on, NULL);
	gui_set_tooltip(grab_button, "Enter the mouse grab mode operation. Press <ESCAPE> to exit grab mode.");
	gtk_widget_show(grab_button);

	dummy=gtk_check_button_new_with_label("Record");
	rec_btn=dummy;
	connect_button(dummy,tape_on, NULL);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Record the audio the terminatorX' audio engine renders. You will be prompted to enter a name for the target wav-file.");
	gtk_widget_show(dummy);
	
	dummy=gtk_label_new("Pitch:");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dumadj=(GtkAdjustment*) gtk_adjustment_new(globals.pitch, -3, 3, 0.001, 0.001, 0.01);
	pitch_adj=dumadj;
	connect_adj(dumadj, master_pitch_changed, NULL);	
	dummy=gtk_hscale_new(dumadj);
	gtk_scale_set_digits(GTK_SCALE(dummy), 2);
	gtk_scale_set_value_pos(GTK_SCALE(dummy), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_DYN);
	gui_set_tooltip(dummy, "Use this scale to adjust the master pitch (affecting *all* turntables).");
	gtk_widget_show(dummy);

	dummy=tx_xpm_label_box(TX_ICON_SEQUENCER, "Sequencer");
	gtk_box_pack_start(GTK_BOX(sequencer_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=tx_xpm_button_new(TX_ICON_PLAY,"Play ", 1);
	connect_button(dummy, seq_play, NULL);
	seq_play_btn=dummy;
	gtk_box_pack_start(GTK_BOX(sequencer_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Playback previously recorded events from the sequencer. This will turn on the audio engine automagically.");
	gtk_widget_show(dummy);

	dummy=tx_xpm_button_new(TX_ICON_STOP,"Stop ", 0);
	seq_stop_btn=dummy;
	connect_button(dummy, seq_stop, NULL);	
	gtk_box_pack_start(GTK_BOX(sequencer_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Stop the playback of sequencer events.");
	gtk_widget_show(dummy);

	dummy=tx_xpm_button_new(TX_ICON_RECORD,"Record ", 1);
	connect_button(dummy, seq_rec, NULL);
	seq_rec_btn=dummy;
	gtk_box_pack_start(GTK_BOX(sequencer_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Enable recording of *events* into the sequencer. All touched controls will be recorded. Existing events for the song-time recording will be overwritten for touched controls.");
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Pos:");
	gtk_box_pack_start(GTK_BOX(sequencer_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	dummy=gtk_entry_new_with_max_length(12);
	seq_entry=dummy;
	gtk_widget_set_usize(dummy, 55, 20);
	gtk_entry_set_text(GTK_ENTRY(dummy), "00:00.00");
	gtk_box_pack_start(GTK_BOX(sequencer_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dumadj=(GtkAdjustment*) gtk_adjustment_new(0, 0, 100, 0.1, 1, 1);
	seq_adj=dumadj;
	connect_adj(dumadj, sequencer_move, NULL);	
	dummy=gtk_hscale_new(dumadj);
	seq_slider=dummy;
	gtk_signal_connect(GTK_OBJECT(seq_slider), "button-release-event", (GtkSignalFunc) seq_slider_released, NULL);
	gtk_scale_set_draw_value(GTK_SCALE(dummy), FALSE);
	
	gui_set_tooltip(dummy, "Select the start position for the sequencer in song-time.");
	gtk_box_pack_start(GTK_BOX(sequencer_box), dummy, WID_DYN);
	gtk_widget_show(dummy);
	
	dummy=gtk_hbox_new(FALSE,2);
	gtk_box_pack_start(GTK_BOX(left_hbox), dummy, WID_DYN);
	gtk_widget_show(dummy);
	
	tt_parent=dummy;
	
	control_parent=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(tt_parent), control_parent, WID_FIX);
	gtk_widget_show(control_parent);

	dummy=gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(tt_parent), dummy, WID_FIX);
	gtk_widget_show(dummy);

	audio_parent=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(tt_parent), audio_parent, WID_DYN);
	gtk_widget_show(audio_parent);
	
	dummy=gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(main_vbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
		
	right_hbox=gtk_vbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), right_hbox, WID_FIX);
	gtk_widget_show(right_hbox);
	
	dummy=gtk_button_new_with_label("Add Turntable");
	AddTable=dummy;	
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Click this button to add a new turntable to the current set.");
	gtk_widget_show(dummy);

	gtk_drag_dest_set (GTK_WIDGET (dummy), (GtkDestDefaults) (GTK_DEST_DEFAULT_MOTION |GTK_DEST_DEFAULT_HIGHLIGHT |GTK_DEST_DEFAULT_DROP),
			drop_types, n_drop_types,
			GDK_ACTION_COPY);
						
	gtk_signal_connect (GTK_OBJECT (dummy), "drag_data_received",
			GTK_SIGNAL_FUNC(drop_new_table), NULL);
	
	gtk_signal_connect(GTK_OBJECT(dummy), "clicked", GtkSignalFunc(new_table), NULL);	

	dummy=gtk_button_new_with_label("Load Set");
	LoadSet=dummy;
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	gui_set_tooltip(dummy, "Click to load a previously saved terminatorX-set-file. As an alternative you can drop a set file over this button.");
	gtk_signal_connect(GTK_OBJECT(dummy), "clicked", GtkSignalFunc(load_tables), NULL);	

	gtk_drag_dest_set (GTK_WIDGET (dummy), (GtkDestDefaults) (GTK_DEST_DEFAULT_MOTION |GTK_DEST_DEFAULT_HIGHLIGHT |GTK_DEST_DEFAULT_DROP),
			drop_types, n_drop_types,
			GDK_ACTION_COPY);
						
	gtk_signal_connect (GTK_OBJECT (dummy), "drag_data_received",
			GTK_SIGNAL_FUNC(drop_set), NULL);
	
	dummy=gtk_button_new_with_label("Save Set");
	SaveSet=dummy;
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	gui_set_tooltip(dummy, "Click here to save the current set.");
	gtk_signal_connect(GTK_OBJECT(dummy), "clicked", GtkSignalFunc(save_tables), NULL);	

	add_sep();
	
	dummy=gtk_button_new_with_label("Options");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	gui_set_tooltip(dummy, "Click here to configure terminatorX.");
	gtk_signal_connect (GTK_OBJECT(dummy), "clicked", (GtkSignalFunc) display_options, NULL);

	dummy=gtk_button_new_with_label("About/Legal");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	gui_set_tooltip(dummy, "Click here to read the license and to get some information about this binary.");
	gtk_signal_connect (GTK_OBJECT(dummy), "clicked", (GtkSignalFunc) mplcfitx, NULL);	

	dummy=gtk_button_new_with_label("Quit");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	gui_set_tooltip(dummy, "Click here to exit terminatorX.");
	gtk_signal_connect (GTK_OBJECT(dummy), "clicked", (GtkSignalFunc) quit, NULL);
	
	add_sep();		

	small_box=gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(right_hbox), small_box, WID_DYN);
	gtk_widget_show(small_box);
	
	smaller_box=gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(small_box), smaller_box, WID_FIX);
	gtk_widget_show(smaller_box);
	
	dummy = tx_pixmap_widget(TX_ICON_LOGO);
	gtk_box_pack_start(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show( dummy );

	dummy=gtk_label_new("0");
	used_mem=dummy;
	gtk_misc_set_alignment(GTK_MISC(dummy), 1, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Mem/kB:");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	add_sep2();

	dummy=gtk_label_new("1");
	no_of_vtts=dummy;
	gtk_misc_set_alignment(GTK_MISC(dummy), 1, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Vtts:");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	add_sep2();

	dummy=gtk_label_new(VERSION);
	gtk_misc_set_alignment(GTK_MISC(dummy), 1, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Release:");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	add_sep2();

	dummy=gtk_label_new("Status:");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0.5, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	dumadj=(GtkAdjustment*) gtk_adjustment_new(2.0-globals.volume, 0, 2, 0.01, 0.05, 0.005);
	volume_adj=dumadj;

	connect_adj(dumadj, master_volume_changed, NULL);	
	dummy=gtk_vscale_new(dumadj);
	gtk_scale_set_draw_value(GTK_SCALE(dummy), False);
	gtk_box_pack_end(GTK_BOX(small_box), dummy, WID_DYN);
	gtk_widget_show(dummy);	
	gui_set_tooltip(dummy, "Adjust the master volume. This parameter will effect *all* turntables in the set.");
	
#ifdef USE_FLASH	
	main_flash_r=gtk_tx_flash_new();
	gtk_box_pack_end(GTK_BOX(small_box), main_flash_r, WID_DYN);
	gtk_widget_show(main_flash_r);

	main_flash_l=gtk_tx_flash_new();
	gtk_box_pack_end(GTK_BOX(small_box), main_flash_l, WID_DYN);
	gtk_widget_show(main_flash_l);
#endif	
	gtk_window_set_default_size(GTK_WINDOW(main_window), x, y);	
	gtk_widget_set_sensitive(grab_button, 0);

	new_table(NULL, NULL); // to give the user something to start with ;)

	gtk_signal_connect (GTK_OBJECT(main_window), "delete-event", (GtkSignalFunc) quit, NULL);	
	
	if (globals.tooltips) gtk_tooltips_enable(gui_tooltips);
	else gtk_tooltips_disable(gui_tooltips);
}

gfloat old_percent=-1;

void wav_progress_update(gfloat percent)
{
	percent=floor(percent*10.0)/10.0; //Updating statusbars with gtk-themes eats up hell of a lot CPU-time
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

void note_destroy(GtkWidget *widget, GtkWidget *mbox)
{
	gtk_widget_destroy(GTK_WIDGET(mbox));
}

void tx_note(const char *message)
{
	char buffer[4096]="\n     [ terminatorX Message: ]     \n\n";
	
	GtkWidget *mbox;
	GtkWidget *label;
	GtkWidget *btn;
	GtkWindow *win;
	
	mbox=gtk_dialog_new();
	win=&(GTK_DIALOG(mbox)->window);
	strcat(buffer, "   ");	
	strcat(buffer, message);
	strcat(buffer, "   ");
	label=gtk_label_new(buffer);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(mbox)->vbox), label, TRUE, TRUE, 0);
	gtk_widget_show(label);
	
	btn = gtk_button_new_with_label("Ok");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(mbox)->action_area), btn, TRUE, TRUE, 0);
	
	gtk_signal_connect(GTK_OBJECT(btn), "clicked", GtkSignalFunc(note_destroy), mbox);

	gtk_window_set_default_size(win, 200, 100);
	gtk_window_set_position(win, GTK_WIN_POS_CENTER_ALWAYS);	

	GTK_WIDGET_SET_FLAGS (btn, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(btn);
	gtk_widget_show(btn);
	gtk_widget_show(mbox);
}


void tx_l_note(const char *message)
{
	char buffer[4096]="\n   Plugin Info:  \n   ------------  \n\n";
	
	GtkWidget *mbox;
	GtkWidget *label;
	GtkWidget *btn;
	GtkWindow *win;
	
	mbox=gtk_dialog_new();
	win=&(GTK_DIALOG(mbox)->window);
	strcat(buffer, message);
	strcat(buffer, "\n");
	label=gtk_label_new(buffer);
	gtk_label_set_justify (GTK_LABEL(label),  GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(mbox)->vbox), label, TRUE, TRUE, 0);
	gtk_widget_show(label);
	
	btn = gtk_button_new_with_label("Ok");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(mbox)->action_area), btn, TRUE, TRUE, 0);
	gtk_widget_show(btn);
	
	gtk_signal_connect(GTK_OBJECT(btn), "clicked", GtkSignalFunc(note_destroy), mbox);

	gtk_window_set_default_size(win, 200, 100);
	gtk_window_set_position(win, GTK_WIN_POS_CENTER_ALWAYS);	
	gtk_widget_show(mbox);
}

void display_mastergui()
{
	GtkWidget *top;
	gtk_widget_realize(main_window);
	tX_set_icon(main_window, "terminatorX");
	gtk_widget_show(main_window);
	top=gtk_widget_get_toplevel(main_window);
	xwindow=GDK_WINDOW_XWINDOW(top->window);
}
