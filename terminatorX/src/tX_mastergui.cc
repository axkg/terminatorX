/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2003  Alexander König
 
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
#include <gdk/gdkkeysyms.h>
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
#include "tX_knobloader.h"
#include "tX_glade_interface.h"
#include "tX_glade_support.h"
#include <sys/time.h>
#include <sys/wait.h>
#include "tX_midiin.h"

#ifdef USE_SCHEDULER
#include <sys/resource.h>
#endif

#define TX_SET_ID_10 "terminatorX turntable set file - version 1.0 - data:"
#define TX_SET_ID_11 "terminatorX turntable set file - version 1.1 - data:"
#define TX_SET_ID_12 "terminatorX turntable set file - version 1.2 - data:"
#define TX_SET_ID_13 "terminatorX turntable set file - version 1.3 - data:"
#define TX_SET_ID_14 "terminatorX turntable set file - version 1.4 - data:"

int audioon=0;
int sequencer_ready=1;

bool tX_shutdown=false;

GtkWidget *tt_parent;
GtkWidget *control_parent;
GtkWidget *audio_parent;
GtkWidget *main_window;
GtkWidget *grab_button;
GtkWidget *main_flash_l;
GtkWidget *main_flash_r;

GtkWidget *seq_rec_btn;
GtkWidget *seq_play_btn;
GtkWidget *seq_stop_btn;
GtkAdjustment *seq_adj;
GtkWidget *seq_slider;
GtkWidget *seq_entry;
GtkWidget *panel_bar;

int buttons_on_panel_bar=0;

int seq_adj_care=1;
int seq_stop_override=0;

GtkAdjustment *volume_adj;
GtkAdjustment *pitch_adj;

/* seq-pars */
tX_seqpar_master_volume sp_master_volume;
tX_seqpar_master_pitch sp_master_pitch;

GtkWidget *engine_btn;

GtkWidget *main_menubar;
GtkWidget *rec_menu_item;
GtkWidget *fullscreen_item;

int rec_dont_care=0;
gint update_tag;

#define connect_entry(wid, func, ptr); g_signal_connect(G_OBJECT(wid), "activate", (GtkSignalFunc) func, (void *) ptr);
#define connect_adj(wid, func, ptr); g_signal_connect(G_OBJECT(wid), "value_changed", (GtkSignalFunc) func, (void *) ptr);
#define connect_button(wid, func, ptr); g_signal_connect(G_OBJECT(wid), "clicked", (GtkSignalFunc) func, (void *) ptr);

Window xwindow;
#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0
extern void add_vtt(GtkWidget *ctrl, GtkWidget *audio, char *fn);
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

vtt_class *old_focus=NULL;

int grab_status=0;
int last_grab_status=0;

GtkWidget *delete_all_item=NULL;
GtkWidget *delete_all_vtt_item=NULL;

GtkTooltips *gui_tooltips=NULL;

void gui_set_tooltip(GtkWidget *wid, char *tip)
{
	gtk_tooltips_set_tip(gui_tooltips, wid, tip, NULL);
}

void turn_audio_off(void)
{
	if (audioon) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(engine_btn), 0);
		while (gtk_events_pending()) gtk_main_iteration();		
	}
}


gint pos_update(gpointer data)
{
	f_prec temp;

	if (stop_update) {		
		cleanup_all_vtts();
		tX_seqpar :: update_all_graphics();
		if (old_focus) gui_show_frame(old_focus, 0);
		old_focus=NULL;
		gtk_tx_flash_clear(main_flash_l);
		gtk_tx_flash_clear(main_flash_r);
		gdk_flush();	
		update_tag=0;
		return(FALSE);
	} else {
		update_all_vtts();
		
		/*left vu meter */
		temp=vtt_class::mix_max_l;
		vtt_class::mix_max_l=0;
		gtk_tx_flash_set_level(main_flash_l, temp);

		/*right vu meter */
		temp=vtt_class::mix_max_r;
		vtt_class::mix_max_r=0;
		gtk_tx_flash_set_level(main_flash_r, temp);
		
		if (vtt_class::focused_vtt!=old_focus) {
			if (old_focus) gui_show_frame(old_focus, 0);
			old_focus=vtt_class::focused_vtt;
			if (old_focus) gui_show_frame(old_focus, 1);			
		}
		if (grab_status!=last_grab_status) {
			last_grab_status=grab_status;
			if (!grab_status) {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(grab_button), 0);
			}
		}
		gdk_flush();	
		update_delay--;
		
		if (update_delay < 0) {
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
	if (procfs) {
		while((!feof(procfs)) && !found) {
			fgets(buffer, 256, procfs);
			
			if (strncmp("VmSize:", buffer, 7)==0) {
				found=1;
				sscanf(buffer, "VmSize: %i kB", &mem);
				sprintf(buffer, "%i", mem);
				gtk_label_set_text(GTK_LABEL(used_mem), buffer);
			}
		}
	}
	fclose(procfs);	
	
	sprintf(buffer, "%i", vtt_class::vtt_amount);
	gtk_label_set_text(GTK_LABEL(no_of_vtts), buffer);
}

GtkSignalFunc new_table(GtkWidget *, char *fn)
{
	turn_audio_off();
		
	if (fn) {
		ld_create_loaddlg(TX_LOADDLG_MODE_SINGLE, 1);
		ld_set_filename(fn);
	}
	
	add_vtt(control_parent, audio_parent, fn);				
	
	if (fn) ld_destroy();		
	mg_update_status();
	
#ifdef USE_ALSA_MIDI_IN
	if (globals.auto_assign_midi) tX_midiin::auto_assign_midi_mappings(NULL, NULL);
#endif
	
	return NULL;
}

bool tx_mg_have_setname=false;
char tx_mg_current_setname[PATH_MAX]="";

GtkSignalFunc new_tables() {
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(main_window), 
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		"Are you sure you want to loose all turntables and events?");
	
	int res=gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
		
	if (res!=GTK_RESPONSE_YES) {
		return NULL;
	}

	vtt_class::delete_all();
	new_table(NULL, NULL);

#ifdef USE_ALSA_MIDI_IN
	if (globals.auto_assign_midi) tX_midiin::auto_assign_midi_mappings(NULL, NULL);
#endif
	
	gtk_window_set_title(GTK_WINDOW(main_window), "terminatorX");

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
	char idbuff[256];
	char wbuf[PATH_MAX];
	xmlDocPtr doc;
#ifdef ENABLE_TX_LEGACY
	FILE *in;
#endif	
	turn_audio_off();
	
	strcpy(globals.tables_filename, buffer);
	
	doc = xmlParseFile(buffer);
	if (doc) {
		vtt_class::load_all(doc, buffer);
		xmlFreeDoc(doc);
	} 
	
#ifdef ENABLE_TX_LEGACY
	else {	
		in=fopen(buffer, "r");	
	
		if (in) {
			fread(idbuff, strlen(TX_SET_ID_10), 1, in);
			if (strncmp(idbuff, TX_SET_ID_10, strlen(TX_SET_ID_10))==0) {
				if (vtt_class::load_all_10(in, buffer)) tx_note("Error while reading set.", true);
			} else if (strncmp(idbuff, TX_SET_ID_11, strlen(TX_SET_ID_11))==0)	{
				if (vtt_class::load_all_11(in, buffer)) tx_note("Error while reading set.", true);			
			} else if (strncmp(idbuff, TX_SET_ID_12, strlen(TX_SET_ID_12))==0) {
				if (vtt_class::load_all_12(in, buffer)) tx_note("Error while reading set.", true);			
			} else if (strncmp(idbuff, TX_SET_ID_13, strlen(TX_SET_ID_13))==0) {
				if (vtt_class::load_all_13(in, buffer)) tx_note("Error while reading set.", true);			
			} else if (strncmp(idbuff, TX_SET_ID_14, strlen(TX_SET_ID_14))==0) {
				if (vtt_class::load_all_14(in, buffer)) tx_note("Error while reading set.", true);			
			}	else {
				tx_note("This file is not a terminatorX set-file.", true);
				fclose(in);
				return;
			}
			fclose(in);	
		} else {
			strcpy(idbuff, "Failed to access file: \"");	// I'm stealing the unrelated sting for a temp :)
			strcat(idbuff, globals.tables_filename);
			strcat(idbuff, "\"");
			tx_note(idbuff, true);
			
			return;
		}
	}
#else
	else {
		strcpy(idbuff, "Failed to access file: \"");	// I'm stealing the unrelated sting for a temp :)
		strcat(idbuff, globals.tables_filename);
		strcat(idbuff, "\"");
		tx_note(idbuff, true);
		
		return;
	}
#endif	
	
	tx_mg_have_setname=true;
	strcpy(tx_mg_current_setname, buffer);
	
	tX_seqpar :: init_all_graphics();
	vg_init_all_non_seqpars();
		
	gtk_adjustment_set_value(volume_adj, globals.volume);
	gtk_adjustment_set_value(pitch_adj, globals.pitch);
	sprintf(wbuf,"terminatorX - %s", strip_path(buffer));
#ifdef USE_ALSA_MIDI_IN
	if (globals.auto_assign_midi) tX_midiin::auto_assign_midi_mappings(NULL, NULL);
#endif	
	gtk_window_set_title(GTK_WINDOW(main_window), wbuf);		
}

void do_load_tables(GtkWidget *wid)
{
	char buffer[PATH_MAX];
	
	strcpy(buffer, gtk_file_selection_get_filename(GTK_FILE_SELECTION(load_dialog)));
	
	gtk_widget_destroy(load_dialog);
	
	load_dialog=NULL;
	load_dialog_win=NULL;

	tX_cursor::set_cursor(tX_cursor::WAIT_CURSOR);
	load_tt_part(buffer);
	tX_cursor::reset_cursor();
}

GtkSignalFunc load_tables()
{
	if (load_dialog_win) {
		gdk_window_raise(load_dialog_win);
		return 0;
	}
	
	load_dialog=gtk_file_selection_new("Load Set");	
	
	gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(load_dialog));
	gtk_file_selection_complete(GTK_FILE_SELECTION(load_dialog), "*.tX");
	
	if (strlen(globals.tables_filename)) {
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(load_dialog), globals.tables_filename);
	}
	
	gtk_widget_show(load_dialog);
	
	load_dialog_win=load_dialog->window;
	
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(load_dialog)->ok_button), "clicked", G_CALLBACK(do_load_tables), NULL);
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(load_dialog)->cancel_button), "clicked", G_CALLBACK (cancel_load_tables), NULL);	
	g_signal_connect (G_OBJECT(load_dialog), "delete-event", G_CALLBACK(cancel_load_tables), NULL);	
	
	return NULL;
}

GtkSignalFunc drop_set(GtkWidget *widget, GdkDragContext *context,
		gint x, gint y, GtkSelectionData *selection_data,
		guint info, guint time, vtt_class *vtt)
{
	char filename[PATH_MAX];
	char *fn;
	
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

gboolean do_save_tables(GtkWidget *wid)
{
	FILE *out;
	gzFile zout;
	char buffer[PATH_MAX];
	char wbuf[PATH_MAX];
	char *ext;
	
	if (wid) {
		strcpy(buffer, gtk_file_selection_get_filename(GTK_FILE_SELECTION(save_dialog)));
		int len=strlen(buffer);
		if (!len || (buffer[len-1]=='/')) {			
			tx_note("Invalid filename for set file.", true);			
			return FALSE;
		}
		strcpy(globals.tables_filename, buffer);
		gtk_widget_destroy(save_dialog);
		save_dialog=NULL;
		save_dialog_win=NULL;
	} else {
		strcpy(buffer, tx_mg_current_setname);
	}
	
	ext=strrchr(buffer, '.');
	
	if (ext) {
		if (strcmp(ext, ".tX")) strcat(buffer, ".tX");
	} else {
		strcat(buffer, ".tX");
	}

	tx_mg_have_setname=true;
	strcpy(tx_mg_current_setname, buffer);
	
	if (globals.compress_set_files) {
		_store_compress_xml=1;
		out=NULL;
		zout=gzopen(buffer, "w");
	} else {
		_store_compress_xml=0;
		out=fopen(buffer, "w");
		zout=NULL;
	}
	
	if (out || zout) {
		if (vtt_class::save_all(out, zout)) tx_note("Error while saving set.", true);
		if (out) fclose(out); 
		else if (zout) gzclose(zout);
		sprintf(wbuf,"terminatorX - %s", strip_path(buffer));
		gtk_window_set_title(GTK_WINDOW(main_window), wbuf);				
	} else {
		tx_note("Failed to open file for write access.", true);
	}
	
	return FALSE;
}

GtkSignalFunc save_tables_as()
{
	if (save_dialog_win) {
		gtk_widget_destroy(save_dialog);
		save_dialog=NULL;
		save_dialog_win=NULL;
	}
	
	save_dialog=gtk_file_selection_new("Save Set");	

	if (tx_mg_have_setname) {
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(save_dialog), tx_mg_current_setname);
	}
	
	gtk_widget_show(save_dialog);
	
	save_dialog_win=save_dialog->window;
	
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(save_dialog)->ok_button), "clicked", G_CALLBACK(do_save_tables), NULL);
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(save_dialog)->cancel_button), "clicked", G_CALLBACK (cancel_save_tables), NULL);	
	g_signal_connect (G_OBJECT(save_dialog), "delete-event", G_CALLBACK(cancel_save_tables), NULL);	

	return NULL;
}

GtkSignalFunc save_tables()
{
	if (!tx_mg_have_setname) {
		save_tables_as();
	} else {
		do_save_tables(NULL);
	}
	
	return NULL;
}

GtkSignalFunc master_volume_changed (GtkWidget *wid, void *d)
{
	sp_master_volume.receive_gui_value((float) GTK_ADJUSTMENT(wid)->value);
	return NULL;	
}

GtkSignalFunc master_pitch_changed(GtkWidget *wid, void *d)
{
	sp_master_pitch.receive_gui_value((float) GTK_ADJUSTMENT(wid)->value);	
	return NULL;	
}

void mg_enable_critical_buttons(int enable)
{
	gtk_widget_set_sensitive(seq_rec_btn, enable);
	gtk_widget_set_sensitive(seq_play_btn, enable);
	gtk_widget_set_sensitive(seq_slider, enable);

	gtk_widget_set_sensitive(rec_menu_item, enable);
	gtk_widget_set_sensitive(delete_all_item, enable);
	gtk_widget_set_sensitive(delete_all_vtt_item, enable);
	
	vg_enable_critical_buttons(enable);
}


GtkSignalFunc seq_stop(GtkWidget *w, void *);

static bool stop_override=false;

GtkSignalFunc audio_on(GtkWidget *w, void *d)
{
	tX_engine_error res;
	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {		
		sequencer_ready=0;
		mg_enable_critical_buttons(0);
		res=tX_engine::get_instance()->run();

		if (res!=NO_ERROR) {
			mg_enable_critical_buttons(1);
			stop_override=true;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), 0);
			stop_override=false;			

			switch(res) {
				case ERROR_BUSY:
				tx_note("Error starting engine: engine is already running.", true);
				break;
				case ERROR_AUDIO:
				tx_note("Error starting engine: failed to access audiodevice.\nPlease check the audio device settings in the \"Preferences\" dialog.", true);
				break;
				case ERROR_TAPE:
				tx_note("Error starting engine: failed to open the recording file.", true);
				break;
				default:tx_note("Error starting engine: Unknown error.", true);
			}
			
			return 0;
		}
		
		sequencer_ready=1;
		stop_update=0;
		audioon=1;
		update_delay=globals.update_delay;
		update_tag=gtk_timeout_add(globals.update_idle, (GtkFunction) pos_update, NULL);
		gtk_widget_set_sensitive(grab_button, 1);
	} else {	
		if (stop_override) return NULL;
		if (!sequencer_ready) return NULL;
		gtk_widget_set_sensitive(grab_button, 0);
		tX_engine::get_instance()->stop();
		stop_update=1;
		audioon=0;
		if (tX_engine::get_instance()->get_recording_request()) {
			tX_engine::get_instance()->set_recording_request(false);
			rec_dont_care=1;
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec_menu_item), 0);
			rec_dont_care=0;
		}
		
		seq_stop(NULL, NULL);
		mg_enable_critical_buttons(1);
	}
	
	return NULL;
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

	if (strlen(buffer)) {
		strcpy(globals.record_filename, buffer);		
		tX_engine::get_instance()->set_recording_request(true);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec_menu_item), 1);
	}
	
	rec_dont_care=0;
	
	gtk_widget_destroy(rec_dialog);
	
	rec_dialog=NULL;
	rec_dialog_win=NULL;
}

GtkSignalFunc select_rec_file()
{
	if (rec_dialog_win) {
		gdk_window_raise(rec_dialog_win);
		return 0;
	}
	
	rec_dialog=gtk_file_selection_new("Record To Disk");	
	
	if (strlen(globals.record_filename)) {
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(rec_dialog), globals.record_filename);
	}
	
	gtk_widget_show(rec_dialog);
	
	rec_dialog_win=rec_dialog->window;
	
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(rec_dialog)->ok_button), "clicked", G_CALLBACK(do_rec), NULL);
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(rec_dialog)->cancel_button), "clicked", G_CALLBACK (cancel_rec), NULL);	
	g_signal_connect (G_OBJECT(rec_dialog), "delete-event", G_CALLBACK(cancel_rec), NULL);	
	
	return NULL;
}

GtkSignalFunc tape_on(GtkWidget *w, void *d)
{
	if (rec_dont_care) return 0;

	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {	
		rec_dont_care=1;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), 0);
		select_rec_file();
	} else {
			tX_engine::get_instance()->set_recording_request(false);
	}
	
	return NULL;
}

void grab_on(GtkWidget *w, void *d)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
		tX_engine::get_instance()->set_grab_request();
	}
	grab_status=1;
}

void grab_off()
{
	grab_status=0;
}

gboolean quit()
{	
	if (globals.quit_confirm) {
		GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(main_window), 
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		"Exit terminatorX and loose all unsaved data?");
		
		int res=gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
			
		if (res!=GTK_RESPONSE_YES) {
			return TRUE;
		}
	}
	
	tX_shutdown=true;
	
	turn_audio_off();
	vtt_class::delete_all();

	if (update_tag)
	gtk_timeout_remove(update_tag);
	globals.width=main_window->allocation.width;
	globals.height=main_window->allocation.height;

	gtk_main_quit();
	
	return true;
}

void mplcfitx()
/* Most Probably Least Called Function In terminatorX :) */
{
	show_about(0);
}

GtkSignalFunc seq_play(GtkWidget *w, void *)
{
	if ((sequencer.is_empty()) && 	(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(seq_rec_btn)))) {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		 {
			tx_note("Sequencer playback triggered - but no events recorded yet - nothing to playback!\n\nTo perform live with terminatorX just activate the audio engine with the \"Power\" button.");
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), 0);
		 }
	} else {
		if (seq_stop_override) return NULL;
			
		seq_adj_care=0;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), 1);
		sequencer.trig_play();
	
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(engine_btn), 1);
	}
	
	return NULL;
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

	return NULL;
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
	
	return NULL;
}

void seq_update_entry(const guint32 timestamp)
{
	char buffer[20];
	guint32 samples;
	guint32 minu,sec,hun;	
	guint32 sr;
	
	samples=timestamp*vtt_class::get_mix_buffer_size();
	sr=vtt_class::get_last_sample_rate();
	
	if (samples>0) {
		minu=samples/(sr*60);
		samples-=(sr*60)*minu;
	
		sec=samples/sr;
		samples-=sr*sec;
	
		hun=samples/(sr/100);
	} else {
		minu=sec=hun=0;
	}
	
	sprintf(buffer, "%02i:%02i.%02i", minu, sec, hun);
	gtk_entry_set_text(GTK_ENTRY(seq_entry), buffer);
}

void seq_update()
{
	seq_update_entry(sequencer.get_timestamp());
	gtk_adjustment_set_value(seq_adj, sequencer.get_timestamp_as_float());
	
}
gboolean seq_slider_released(GtkWidget *wid, void *d)
{
	seq_adj_care=0;
	gtk_widget_set_sensitive(seq_slider, 0);	
	sequencer.forward_to_start_timestamp(0);
	gtk_widget_set_sensitive(seq_slider, 1);	
	seq_adj_care=1;
	
	return FALSE;
}
void sequencer_move(GtkWidget *wid, void *d)
{
	guint32 pos;
	
	if (seq_adj_care) {
		pos=sequencer.set_start_timestamp((float) GTK_ADJUSTMENT(wid)->value);
		seq_update_entry(pos);	
	}
}

#define add_sep(); 	dummy=gtk_hseparator_new ();\
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);\
	gtk_widget_show(dummy);\

#define add_sep2(); 	dummy=gtk_hseparator_new ();\
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);\
	gtk_widget_show(dummy);\

void fullscreen_toggle(GtkCheckMenuItem *item, gpointer data);
void display_help();
void display_browser();

tX_seqpar *del_sp=NULL;
vtt_class *del_vtt=NULL;
tx_menu_del_mode menu_del_mode=ALL_EVENTS_ALL_TURNTABLES;

GtkWidget *del_dialog=NULL;

GCallback menu_delete_all_events(GtkWidget *, void *param)
{	
	del_dialog=create_tx_del_mode();
	tX_set_icon(del_dialog);
	
	GtkWidget *label=lookup_widget(del_dialog, "delmode_label");
	
	menu_del_mode=ALL_EVENTS_ALL_TURNTABLES;
	
	gtk_label_set_markup(GTK_LABEL(label), "Delete <b>all</b> events for <b>all</b> turntables.");
	gtk_widget_show(del_dialog);
	
	return NULL;
}

GCallback menu_delete_all_events_for_vtt(GtkWidget *, vtt_class *vtt)
{	
	if (!vtt) {
		tX_error("No vtt passed to menu_delete_all_events_for_vtt().");
		return FALSE;
	}
	
	char label_str[512];
	
	del_dialog=create_tx_del_mode();
	tX_set_icon(del_dialog);

	del_vtt=vtt;
	GtkWidget *label=lookup_widget(del_dialog, "delmode_label");
	
	menu_del_mode=ALL_EVENTS_FOR_TURNTABLE;
	
	sprintf(label_str, "Delete <b>all</b> events for turntable <b>%s</b>.", vtt->name);
	gtk_label_set_markup(GTK_LABEL(label), label_str);
	gtk_widget_show(del_dialog);
	
	return NULL;
}

GCallback menu_delete_all_events_for_sp(GtkWidget *, tX_seqpar *sp)
{	
	if (!sp) {
		tX_error("No sp passed to menu_delete_all_events_for_sp().");
		return FALSE;
	}
	
	char label_str[512];
	
	del_dialog=create_tx_del_mode();
	tX_set_icon(del_dialog);
	
	GtkWidget *label=lookup_widget(del_dialog, "delmode_label");
	
	menu_del_mode=ALL_EVENTS_FOR_SP;
	del_sp=sp;
	sprintf(label_str, "Delete all <b>%s</b> events for turntable <b>%s</b>.", sp->get_name(), ((vtt_class *) sp->vtt)->name);
	gtk_label_set_markup(GTK_LABEL(label), label_str);
	gtk_widget_show(del_dialog);

	return NULL;
}

static GtkWidget *table_menu=NULL;
static GtkWidget *table_menu_item=NULL;

GCallback create_table_sequencer_menu(GtkWidget *widget, void *param) 
{
	char label[328];
	table_menu=gtk_menu_new();
	
	list <vtt_class *> :: iterator vtt;

	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++) {
		GtkWidget *menu_item=gtk_menu_item_new_with_label((*vtt)->name);
		gtk_container_add (GTK_CONTAINER (table_menu), menu_item);
		gtk_widget_show(menu_item);
		
		GtkWidget *seqpar_menu=gtk_menu_new();
		list <tX_seqpar *> :: iterator sp;
		
		GtkWidget *all=gtk_menu_item_new_with_label("Delete All Events");
		gtk_container_add (GTK_CONTAINER (seqpar_menu), all);
		g_signal_connect(all, "activate", (GCallback) menu_delete_all_events_for_vtt, (*vtt));
		gtk_widget_show(all);

		GtkWidget *sep = gtk_menu_item_new ();
		gtk_widget_show(sep);
		gtk_container_add(GTK_CONTAINER (seqpar_menu), sep);
		gtk_widget_set_sensitive (sep, FALSE);
		
		for (sp=tX_seqpar::all.begin(); sp!=tX_seqpar::all.end(); sp++) {
			if ((*sp)->vtt==(*vtt)) {
				sprintf(label, "Delete '%s' Events", (*sp)->get_name());
				GtkWidget *menu_item=gtk_menu_item_new_with_label(label);
				g_signal_connect(menu_item, "activate", (GCallback) menu_delete_all_events_for_sp, (*sp));
				gtk_container_add(GTK_CONTAINER(seqpar_menu), menu_item);
				gtk_widget_show(menu_item);
			}
		}
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), seqpar_menu);
	}
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(table_menu_item), table_menu);
	
	return NULL;
}

GCallback toggle_confirm_events(GtkWidget *widget, void *dummy)
{	
	globals.confirm_events=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));	
	return NULL;	
}

GCallback toggle_auto_assign(GtkWidget *widget, void *dummy)
{	
	globals.auto_assign_midi=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
#ifdef USE_ALSA_MIDI_IN
	tX_midiin::auto_assign_midi_mappings(NULL, NULL);
#endif
/*	if (globals.auto_assign_midi) {
		GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(main_window), 
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
		"Note: Enabling \"Auto Assign Default MIDI Settings\" will constantly overwrite the\
MIDI mappings for the standard parameters. \
Are you sure you really want this?");
		
		int res=gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
			
		if (res!=GTK_RESPONSE_YES) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), 0);
		}			
	} */
	return NULL;	
}

void create_master_menu() 
{
	GtkWidget *menu_item;
	GtkWidget *sub_menu;
	GtkAccelGroup* accel_group=gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(main_window), accel_group);

	/* FILE */
	menu_item = gtk_menu_item_new_with_mnemonic ("_File");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (main_menubar), menu_item);

	sub_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);

	menu_item = gtk_image_menu_item_new_from_stock ("gtk-new", accel_group);
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) new_tables, NULL);

	menu_item = gtk_image_menu_item_new_from_stock ("gtk-open", accel_group);
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) load_tables, NULL);

	menu_item = gtk_image_menu_item_new_from_stock ("gtk-save", accel_group);
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) save_tables, NULL);

	menu_item = gtk_image_menu_item_new_from_stock ("gtk-save-as", accel_group);
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) save_tables_as, NULL);

	menu_item = gtk_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_widget_set_sensitive (menu_item, FALSE);

	menu_item = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) quit, NULL);

	/* Turntables */
	menu_item = gtk_menu_item_new_with_mnemonic ("_Turntables");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (main_menubar), menu_item);
	
	sub_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);

	menu_item = gtk_menu_item_new_with_mnemonic("_Add Turntable");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_A, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);	
	g_signal_connect(menu_item, "activate", (GCallback) new_table, NULL);

	menu_item = gtk_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_widget_set_sensitive (menu_item, FALSE);

	menu_item = gtk_menu_item_new_with_mnemonic("Assign _Default MIDI Mappings");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_M, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

#ifdef USE_ALSA_MIDI_IN
	g_signal_connect(menu_item, "activate", G_CALLBACK(tX_midiin::auto_assign_midi_mappings), (void *) true);
#else
	gtk_widget_set_sensitive(menu_item, FALSE);
#endif

	menu_item = gtk_check_menu_item_new_with_mnemonic("A_uto Assign Default MIDI Mappings");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), globals.auto_assign_midi);
#ifdef USE_ALSA_MIDI_IN	
	g_signal_connect(menu_item, "activate", (GCallback) toggle_auto_assign, NULL);
#else
	gtk_widget_set_sensitive(menu_item, FALSE);
#endif

	menu_item = gtk_menu_item_new_with_mnemonic("_Clear MIDI Mappings");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_C, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

#ifdef USE_ALSA_MIDI_IN
	g_signal_connect(menu_item, "activate", G_CALLBACK(tX_midiin::clear_midi_mappings), (void *) true);
#else
	gtk_widget_set_sensitive(menu_item, FALSE);
#endif

	menu_item = gtk_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_widget_set_sensitive (menu_item, FALSE);

	menu_item = gtk_check_menu_item_new_with_mnemonic("_Record Audio To Disk");
	rec_menu_item = menu_item;
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) tape_on, NULL);

	/* Sequencer */
	
	menu_item = gtk_menu_item_new_with_mnemonic("_Sequencer");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (main_menubar), menu_item);

	sub_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);
		
	table_menu = gtk_menu_new();
	menu_item = gtk_menu_item_new_with_mnemonic("Delete _Events");
	delete_all_vtt_item = menu_item;
	table_menu_item = menu_item;
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(table_menu_item), table_menu);
	
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect_swapped (G_OBJECT (menu_item), "select", G_CALLBACK (create_table_sequencer_menu), NULL);
	
	menu_item = gtk_menu_item_new_with_mnemonic("Delete _All Events");
	delete_all_item = menu_item;
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) menu_delete_all_events, NULL);

	menu_item = gtk_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_widget_set_sensitive (menu_item, FALSE);

	menu_item = gtk_check_menu_item_new_with_mnemonic("_Confirm Recorded Events");
	//rec_menu_item = menu_item;
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), globals.confirm_events);
	g_signal_connect(menu_item, "activate", (GCallback) toggle_confirm_events, NULL);

	/* Options */
	menu_item = gtk_menu_item_new_with_mnemonic ("_Options");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (main_menubar), menu_item);

	sub_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);
		
	menu_item = gtk_check_menu_item_new_with_mnemonic("_Fullscreen");
	fullscreen_item = menu_item;
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), globals.fullscreen_enabled);
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_F11, (GdkModifierType) 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(menu_item, "activate", (GCallback) fullscreen_toggle, NULL);
	
	menu_item = gtk_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_widget_set_sensitive (menu_item, FALSE);

	menu_item = gtk_image_menu_item_new_from_stock ("gtk-preferences", accel_group);
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) display_options, NULL);

	/* HELP */ 
	menu_item = gtk_menu_item_new_with_mnemonic ("_Help");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (main_menubar), menu_item);
	gtk_menu_item_set_right_justified(GTK_MENU_ITEM(menu_item), TRUE);
	
	sub_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);

	menu_item = gtk_menu_item_new_with_mnemonic ("_Contents");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) display_help, NULL);
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_F1, (GdkModifierType) 0, GTK_ACCEL_VISIBLE);

	menu_item = gtk_menu_item_new_with_mnemonic ("_About");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) mplcfitx, NULL);
	
	menu_item = gtk_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_widget_set_sensitive (menu_item, FALSE);

	menu_item = gtk_menu_item_new_with_mnemonic ("_Visit terminatorX.cx");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) display_browser, NULL);
}

void create_mastergui(int x, int y)
{
	GtkWidget *mother_of_all_boxen;
	GtkWidget *main_vbox;
	GtkWidget *right_hbox;
	GtkWidget *left_hbox;
	GtkWidget *control_box;
	//GtkWidget *sequencer_box;
	GtkAdjustment *dumadj;
	GtkWidget *dummy;
	GtkWidget *master_vol_box;
	GtkWidget *status_box;
	
	gui_tooltips=gtk_tooltips_new();

	main_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_wmclass(GTK_WINDOW(main_window), "terminatorX", "tX_mastergui");

	gtk_window_set_title(GTK_WINDOW(main_window), "terminatorX");
	
	gtk_container_set_border_width(GTK_CONTAINER(main_window), 5);

	gtk_widget_realize(main_window);
	
	mother_of_all_boxen=gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(main_window), mother_of_all_boxen);
	gtk_widget_show(mother_of_all_boxen);	
	
	main_menubar=gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(mother_of_all_boxen), main_menubar, WID_FIX);
	gtk_widget_show(main_menubar);	
	
	create_master_menu();
	
	main_vbox=gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(mother_of_all_boxen), main_vbox, WID_DYN);
	gtk_widget_show(main_vbox);
	
	left_hbox=gtk_vbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), left_hbox, WID_DYN);
	gtk_widget_show(left_hbox);
	
	control_box=gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(left_hbox), control_box, WID_FIX);
	gtk_widget_show(control_box);
	
	dummy=gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(left_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);

    /* control_box contents */

	dummy=tx_xpm_label_box(TX_ICON_AUDIOENGINE, "Audio");
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

	dummy=gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
    
	dummy=tx_xpm_label_box(TX_ICON_SEQUENCER, "Seq.");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=tx_xpm_button_new(TX_ICON_PLAY,"Play ", 1);
	connect_button(dummy, seq_play, NULL);
	seq_play_btn=dummy;
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Playback previously recorded events from the sequencer. This will turn on the audio engine automagically.");
	gtk_widget_show(dummy);

	dummy=tx_xpm_button_new(TX_ICON_STOP,"Stop ", 0);
	seq_stop_btn=dummy;
	connect_button(dummy, seq_stop, NULL);	
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Stop the playback of sequencer events.");
	gtk_widget_show(dummy);

	dummy=tx_xpm_button_new(TX_ICON_RECORD,"Record ", 1);
	connect_button(dummy, seq_rec, NULL);
	seq_rec_btn=dummy;
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Enable recording of *events* into the sequencer. All touched controls will be recorded. Existing events for the song-time recording will be overwritten for touched controls.");
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Pos:");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	dummy=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(dummy), 12);
	seq_entry=dummy;
	//gtk_widget_set_usize(dummy, 65, 20);
	gtk_entry_set_text(GTK_ENTRY(dummy), "00:00.00");
	gtk_entry_set_width_chars(GTK_ENTRY(dummy), 9);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dumadj=(GtkAdjustment*) gtk_adjustment_new(0, 0, 100, 0.1, 1, 1);
	seq_adj=dumadj;
	connect_adj(dumadj, sequencer_move, NULL);	
	dummy=gtk_hscale_new(dumadj);
	gtk_widget_set_size_request(dummy, 65, 20);
	seq_slider=dummy;
	g_signal_connect(G_OBJECT(seq_slider), "button-release-event", (GtkSignalFunc) seq_slider_released, NULL);
	gtk_scale_set_draw_value(GTK_SCALE(dummy), FALSE);
	
	gui_set_tooltip(dummy, "Select the start position for the sequencer in song-time.");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_DYN);
	gtk_widget_show(dummy);
	
	dummy=gtk_hbox_new(FALSE,2); //gtk_hpaned_new ();
	gtk_box_pack_start(GTK_BOX(left_hbox), dummy, WID_DYN);
	gtk_widget_show(dummy);
	
	tt_parent=dummy;

    panel_bar=gtk_hbox_new(TRUE,2);
	gtk_box_pack_start(GTK_BOX(left_hbox), panel_bar, WID_FIX);

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

	/* Master */
	
	dummy=gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(dummy),"<b>Master</b>");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0.5, 0.5);
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);	

	dummy=gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);

	 /* Pitch */
	 
	/*dummy=gtk_label_new("Pitch:");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);*/

	dumadj=(GtkAdjustment*) gtk_adjustment_new(globals.pitch, -3, 3, 0.001, 0.001, 0.01);
	pitch_adj=dumadj;
	connect_adj(dumadj, master_pitch_changed, NULL);
	
	tX_extdial *pdial=new tX_extdial("Pitch", pitch_adj, &sp_master_pitch, true);
	gtk_box_pack_start(GTK_BOX(right_hbox), pdial->get_widget(), WID_FIX);
	gui_set_tooltip(pdial->get_entry(), "Use this dial to adjust the master pitch (affecting *all* turntables).");
	
	dummy=gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	/* Volume */
	master_vol_box=gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(right_hbox), master_vol_box, WID_DYN);
	gtk_widget_show(master_vol_box);	
	
	dumadj=(GtkAdjustment*) gtk_adjustment_new(globals.volume, 0, 2, 0.01, 0.05, 0.005);
	volume_adj=dumadj;

	connect_adj(dumadj, master_volume_changed, NULL);	
	dummy=gtk_vscale_new(dumadj);
	gtk_range_set_inverted(GTK_RANGE(dummy), TRUE);
	gtk_scale_set_draw_value(GTK_SCALE(dummy), False);
	g_signal_connect(G_OBJECT(dummy), "button_press_event", (GtkSignalFunc) tX_seqpar::tX_seqpar_press, &sp_master_volume);	
	
	gtk_box_pack_end(GTK_BOX(master_vol_box), dummy, WID_FIX);
	gtk_widget_show(dummy);	
	gui_set_tooltip(dummy, "Adjust the master volume. This parameter will effect *all* turntables in the set.");
	
	main_flash_r=gtk_tx_flash_new();
	gtk_box_pack_end(GTK_BOX(master_vol_box), main_flash_r, WID_DYN);
	gtk_widget_show(main_flash_r);

	main_flash_l=gtk_tx_flash_new();
	gtk_box_pack_end(GTK_BOX(master_vol_box), main_flash_l, WID_DYN);
	gtk_widget_show(main_flash_l);

	dummy=gtk_label_new("Volume");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0.5, 0.5);
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);

	/* STATUS BOX */ 
	dummy=gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	status_box=gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_hbox), status_box, WID_FIX);
	gtk_widget_show(status_box);
	
	dummy=gtk_label_new("0");
	used_mem=dummy;
	gtk_misc_set_alignment(GTK_MISC(dummy), 1, 0.5);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Mem/kB:");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0, 0.5);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	add_sep2();

	dummy=gtk_label_new("1");
	no_of_vtts=dummy;
	gtk_misc_set_alignment(GTK_MISC(dummy), 1, 0.5);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Vtts:");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0, 0.5);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	add_sep2();

	dummy=gtk_label_new(VERSION);
	gtk_misc_set_alignment(GTK_MISC(dummy), 1, 0.5);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Release:");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0, 0.5);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	add_sep2();

	dummy=gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(dummy), "<b>Status</b>");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0.5, 0.5);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	/* END GUI */
	
	gtk_window_set_default_size(GTK_WINDOW(main_window), x, y);	
	gtk_widget_set_sensitive(grab_button, 0);

	new_table(NULL, NULL); // to give the user something to start with ;)

	g_signal_connect (G_OBJECT(main_window), "delete-event", (GtkSignalFunc) quit, NULL);	
	
	if (globals.tooltips) gtk_tooltips_enable(gui_tooltips);
	else gtk_tooltips_disable(gui_tooltips);
}

gfloat old_percent=-1;

void note_destroy(GtkWidget *widget, GtkWidget *mbox)
{
	gtk_widget_destroy(GTK_WIDGET(mbox));
}

void tx_note(const char *message, bool isError)
{
	char buffer[4096]="terminatorX ";
	if (isError) {
		strcat(buffer, "note:\n\n");
	} else {
		strcat(buffer, "error:\n\n");
	}
	
	strcat(buffer, message);
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(main_window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		isError ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, message);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);	
}


void tx_l_note(const char *message)
{
	char buffer[4096]="Plugin info:\n\n";
	strcat(buffer, message);
	
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(main_window),
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, message);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);	
}


void add_to_panel_bar(GtkWidget *button) 
{
	buttons_on_panel_bar++;
	gtk_box_pack_start(GTK_BOX(panel_bar), button, WID_DYN);
	gtk_widget_show(panel_bar);
}

void remove_from_panel_bar(GtkWidget *button) 
{
	buttons_on_panel_bar--;
	gtk_container_remove(GTK_CONTAINER(panel_bar), button);
	if (buttons_on_panel_bar==0) gtk_widget_hide(panel_bar);
}

/* Fullscreen code... */
#define _WIN_LAYER_TOP 		-1
#define _WIN_LAYER_NORMAL	4
#define _NET_WM_STATE_REMOVE	0
#define _NET_WM_STATE_ADD	1
#define _NET_WM_STATE_TOGGLE	2

void fullscreen_toggle(GtkCheckMenuItem *item, gpointer data) {
	XEvent xev;
	Window win=GDK_WINDOW_XID(main_window->window);
	Display *disp=GDK_WINDOW_XDISPLAY(main_window->window);
	
	globals.fullscreen_enabled=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(fullscreen_item));
	
	/* Top layer.. */
	xev.xclient.type = ClientMessage;
	xev.xclient.serial = 0;
	xev.xclient.send_event = True;
	xev.xclient.display = disp;
	xev.xclient.window = win;
	xev.xclient.message_type = gdk_x11_get_xatom_by_name ("_WIN_LAYER");
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = globals.fullscreen_enabled ? _WIN_LAYER_TOP : _WIN_LAYER_NORMAL ;
	XSendEvent(disp, GDK_WINDOW_XID (gdk_get_default_root_window ()),
		False, SubstructureRedirectMask | SubstructureNotifyMask,
		&xev);
	
	/* Fullscreen */
	xev.xclient.type = ClientMessage;
	xev.xclient.serial = 0;
	xev.xclient.send_event = True;
	xev.xclient.display = disp;
	xev.xclient.window = win;
	xev.xclient.message_type = gdk_x11_get_xatom_by_name ("_NET_WM_STATE");
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = globals.fullscreen_enabled ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
	xev.xclient.data.l[1] = gdk_x11_atom_to_xatom (gdk_atom_intern ("_NET_WM_STATE_FULLSCREEN", TRUE));
	xev.xclient.data.l[2] = gdk_x11_atom_to_xatom (GDK_NONE);
	XSendEvent(gdk_display, GDK_WINDOW_XID (gdk_get_default_root_window ()),
		False, SubstructureRedirectMask | SubstructureNotifyMask,
		&xev);	
}

void fullscreen_setup() {
	if (globals.fullscreen_enabled) {
		fullscreen_toggle(NULL, NULL);
	}
}

void display_mastergui()
{
	GtkWidget *top;
	gtk_widget_realize(main_window);
	tX_set_icon(main_window);
	load_knob_pixs();
	gtk_widget_show(main_window);
	fullscreen_setup();	
	top=gtk_widget_get_toplevel(main_window);
	xwindow=GDK_WINDOW_XWINDOW(top->window);
}

pid_t help_child=0;
GTimer *help_timer=NULL;
int help_tag=-1;

int help_checker()
{
	gdouble time;
	gulong ms;
	int status;
	int result=waitpid(help_child, &status, WNOHANG);
	
	if (result==0) {
		time=g_timer_elapsed(help_timer, &ms);
		if (time > 5) {
			/* 5 seconds and it's still running - so we assume everything's OK. */
			tX_debug("No longer waiting for gnome-help..");
			gtk_idle_remove(help_tag);
			help_tag=-1;
		}
	} else {
		/* We are still here and the child exited - that could mean trouble. */
		tx_note("Couldn't run the gnome-help command (alias \"yelp\") to display the terminatorX manual. Please ensure that \"yelp\" is installed.", true);		
		
		gtk_idle_remove(help_tag);
		help_tag=-1;
	}
	return TRUE;	
}

#ifndef INSTALL_PREFIX
#define INSTALL_PREFIX "/usr/local/share"
#endif

void display_help()
{	
	help_child=fork();

	if (help_tag!=-1) {
		gtk_idle_remove(help_tag);
		if (help_timer) g_timer_destroy(help_timer);
		help_child=0;
		help_tag=-1;
		help_timer=NULL;
	}
	
	if (help_child==0) {
		// child
		// execlp("gnome-help","gnome-help","ghelp:/" INSTALL_PREFIX "/terminatorX/doc/terminatorX-manual/C/terminatorX-manual.xml", NULL);
		execlp("gnome-help","gnome-help","ghelp:/" XML_MANUAL, NULL);		
		_exit(-1);
	} else if (help_child==-1) {
		tx_note("System error: couldn't fork() to run the help process.", true);
	} else {
		help_timer=g_timer_new();
		g_timer_start(help_timer);
	
		help_tag=gtk_idle_add((GtkFunction) help_checker, NULL);
	}
}

pid_t browser_child=0;
GTimer *browser_timer=NULL;
int browser_tag=-1;

int browser_checker()
{
	gdouble time;
	gulong ms;
	int status;
	int result=waitpid(browser_child, &status, WNOHANG);
	
	if (result==0) {
		time=g_timer_elapsed(browser_timer, &ms);
		if (time > 5) {
			/* 5 seconds and it's still running - so we assume everything's OK. */
			tX_debug("No longer waiting for a browser..");
			gtk_idle_remove(browser_tag);
			browser_tag=-1;
		}
	} else {
		/* We are still here and the child exited - that could mean trouble. */
		tx_note("Failed to run a suitable web browser - if there's one installed on this system, please run it and forward yourself to:\nhttp://terminatorX.cx", true);		
		
		gtk_idle_remove(browser_tag);
		browser_tag=-1;
	}
	return TRUE;	
}

void display_browser()
{	
	browser_child=fork();

	if (browser_tag!=-1) {
		gtk_idle_remove(browser_tag);
		if (browser_timer) g_timer_destroy(browser_timer);
		browser_child=0;
		browser_tag=-1;
		browser_timer=NULL;
	}
	
	if (browser_child==0) {
		// child
		execlp("mozilla","mozilla","http://terminatorX.cx", NULL);
		execlp("netscape","netscape","http://terminatorX.cx", NULL);
		execlp("galeon","galeon","http://terminatorX.cx", NULL);
		execlp("konqueror","konqueror","http://terminatorX.cx", NULL);		
		_exit(-1);
	} else if (browser_child==-1) {
		tx_note("System error: couldn't fork() to run the browser process.", true);
	} else {
		browser_timer=g_timer_new();
		g_timer_start(browser_timer);
	
		browser_tag=gtk_idle_add((GtkFunction) browser_checker, NULL);
	}
}



GdkCursor *tX_cursor::cursors[MAX_CURSOR]={NULL, NULL, NULL};
tX_cursor::cursor_shape tX_cursor::current_shape=tX_cursor::DEFAULT_CURSOR;

void tX_cursor::set_cursor(cursor_shape shape)
{
	switch (shape) {
		case DEFAULT_CURSOR:
			cursors[shape]=NULL;
			break;
		
		case WAIT_CURSOR:
			if (!cursors[shape]) cursors[shape]=gdk_cursor_new(GDK_WATCH);
			break;
		
		case WAIT_A_SECOND_CURSOR:
			/* FIXME: What's that short-time wait cursor's id? */
			if (!cursors[shape]) cursors[shape]=gdk_cursor_new(GDK_WATCH);
			break;
		
		default:
			tX_debug("No such cursor shape.");
			return;
	}
	
	/* Still here? Ok... */
	current_shape=shape;
	
	gdk_window_set_cursor(main_window->window, cursors[shape]);
}

GdkCursor *tX_cursor::get_cursor()
{
	return cursors[current_shape];
}

void tX_cursor::reset_cursor()
{
	current_shape=DEFAULT_CURSOR;
	gdk_window_set_cursor(main_window->window, cursors[current_shape]);
}
