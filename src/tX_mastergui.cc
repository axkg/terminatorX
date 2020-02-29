/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2020  Alexander König
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
    File: tX_mastergui.cc
 
    Description: This implements the main (aka master) gtk+ GUI of terminatorX
    		 It serves as a container for the vtt-guis.
*/    

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <pango/pango.h>
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
#include "tX_ui_interface.h"
#include "tX_ui_support.h"
#include <sys/time.h>
#include <sys/wait.h>
#include "tX_midiin.h"
#include "tX_mouse.h"

#ifdef USE_SCHEDULER
#include <sys/resource.h>
#endif

#ifdef USE_X11
#include <X11/Xlib.h>
#endif

#define TX_SET_ID_10 "terminatorX turntable set file - version 1.0 - data:"
#define TX_SET_ID_11 "terminatorX turntable set file - version 1.1 - data:"
#define TX_SET_ID_12 "terminatorX turntable set file - version 1.2 - data:"
#define TX_SET_ID_13 "terminatorX turntable set file - version 1.3 - data:"
#define TX_SET_ID_14 "terminatorX turntable set file - version 1.4 - data:"

#define BROWSER1 "xdg-open"
#define BROWSER2 "x-www-browser"
#define BROWSER3 "firefox"

int audioon=0;
int sequencer_ready=1;
int fontHeight=16;

bool tX_shutdown=false;
tx_mouse mouse;

GtkWidget *tt_parent;
GtkWidget *control_parent;
GtkWidget *audio_parent;
GtkWidget *main_window;
GtkWidget *grab_button;
GtkWidget *main_flash;

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

int rec_dont_care=0;
gint update_tag;

#define connect_entry(wid, func, ptr); g_signal_connect(G_OBJECT(wid), "activate", (GCallback) func, (void *) ptr);
#define connect_adj(wid, func, ptr); g_signal_connect(G_OBJECT(wid), "value_changed", (GCallback) func, (void *) ptr);
#define connect_button(wid, func, ptr); g_signal_connect(G_OBJECT(wid), "clicked", (GCallback) func, (void *) ptr);

#ifdef USE_X11
Window x_window;
GtkWidget *fullscreen_item;
#endif

GdkWindow* top_window;
#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0
#define WID_FIX_BREATHE FALSE, FALSE, 5
extern void add_vtt(GtkWidget *ctrl, GtkWidget *audio, char *fn);
extern void destroy_gui(vtt_class *vtt);
extern void gui_show_focus(vtt_class *vtt, int show);

GdkWindow *rec_dialog_win=NULL;
GtkWidget *rec_dialog=NULL;

//GtkWidget *no_of_vtts=NULL;
GtkWidget *used_mem=NULL;

int stop_update=0;
int update_delay;

vtt_class *old_focus=NULL;

int grab_status=0;
int last_grab_status=0;

GtkWidget *delete_all_item=NULL;
GtkWidget *delete_all_vtt_item=NULL;

void gui_set_tooltip(GtkWidget *wid, const char *tip)
{
	gtk_widget_set_tooltip_text(wid, tip);
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
	f_prec temp,temp2;

	if (stop_update) {
		cleanup_all_vtts();
		tX_seqpar :: update_all_graphics();
		if (old_focus) gui_show_focus(old_focus, 0);
		old_focus=NULL;
		gtk_tx_flash_clear(main_flash);
		gdk_display_flush(gdk_display_get_default());
		update_tag=0;
		return FALSE;
	} else {
		update_all_vtts();

		/*main vu meter */
		temp=vtt_class::mix_max_l;
		vtt_class::mix_max_l=0;
		temp2=vtt_class::mix_max_r;
		vtt_class::mix_max_r=0;
		gtk_tx_flash_set_level(main_flash, temp/FL_SHRT_MAX, temp2/FL_SHRT_MAX);

		if (vtt_class::focused_vtt!=old_focus) {
			if (old_focus) gui_show_focus(old_focus, 0);
			old_focus=vtt_class::focused_vtt;
			if (old_focus) gui_show_focus(old_focus, 1);
		}

		grab_status = mouse.is_grabbed();

		if (grab_status!=last_grab_status) {
			last_grab_status=grab_status;
			if (!grab_status) {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(grab_button), 0);
			}
		}
		gdk_display_flush(gdk_display_get_default());
		update_delay--;

		if (update_delay < 0) {
			seq_update();
			tX_seqpar::update_all_graphics();
			update_delay=globals.update_delay;
		}

		if (tX_engine::get_instance()->check_error()) {
			tX_error("ouch - error while playback...");
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(engine_btn), 0);
			return FALSE;
		}

		// let the audio engine we got the chance to do something
		tX_engine::get_instance()->reset_cycles_ctr();

		return TRUE;
	}
}

#define TX_MEMTAG "VmData:"

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
			if (fgets(buffer, 256, procfs) && (strncmp(TX_MEMTAG, buffer, sizeof(TX_MEMTAG)-1)==0)) {
				found=1;
				sscanf(buffer, TX_MEMTAG" %i kB", &mem);
				sprintf(buffer, "%.1lf M", ((double) mem)/1024.0);
				gtk_label_set_text(GTK_LABEL(used_mem), buffer);
			}
		}
		fclose(procfs);	
	} else {
		gtk_label_set_text(GTK_LABEL(used_mem), "");
	}
	
	/*sprintf(buffer, "%i", vtt_class::vtt_amount);
	gtk_label_set_text(GTK_LABEL(no_of_vtts), buffer);*/
}

GCallback new_table(GtkWidget *, char *fn)
{
	//turn_audio_off();
		
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

GCallback new_tables() {
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(main_window), 
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		"Are you sure you want to lose all turntables and events?");
	
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
void load_tt_part(char * buffer)
{
	char wbuf[PATH_MAX];
	xmlDocPtr doc;
#ifdef ENABLE_TX_LEGACY
	FILE *in;
#endif	
	turn_audio_off();
	
	strncpy(globals.tables_filename, buffer, sizeof(globals.tables_filename));
	
	doc = xmlParseFile(buffer);
	if (doc) {
		vtt_class::load_all(doc, buffer);
		xmlFreeDoc(doc);
	} 
	
#ifdef ENABLE_TX_LEGACY
	else {	
		in=fopen(buffer, "r");	
	
		if (in) {
			char idbuff[256];
			
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
			char message[PATH_MAX+256];
			sprintf(message, "Failed to acesss file: \"%s\"", globals.tables_filename);
			tx_note(message, true);
			
			return;
		}
	}
#else
	else {
		char message[PATH_MAX+256];
		sprintf(message, "Failed to acesss file: \"%s\"", globals.tables_filename);
		tx_note(message, true);
		
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

GCallback load_tables()
{
	GtkWidget * dialog = gtk_file_chooser_dialog_new ("Open Set File",
		GTK_WINDOW(main_window), GTK_FILE_CHOOSER_ACTION_OPEN,
		"_Cancel", GTK_RESPONSE_CANCEL,  
	    "_Open", GTK_RESPONSE_ACCEPT, NULL);
				      

	GtkFileFilter *filter=gtk_file_filter_new();
	gtk_file_filter_add_pattern (filter, "*.tX");
	gtk_file_filter_add_pattern (filter, "*.tx");
	gtk_file_filter_set_name(filter, "terminatorX Set Files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter); 

	filter=gtk_file_filter_new();
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_filter_set_name(filter, "All Files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter); 
	
	if (strlen(globals.tables_filename)) {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), globals.tables_filename);
	}	
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
    	char * filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_widget_hide(dialog);
		tX_cursor::set_cursor(tX_cursor::WAIT_CURSOR);
		load_tt_part(filename);
		tX_cursor::reset_cursor();		
	}	
	
	gtk_widget_destroy(dialog);

	return NULL;
}

vtt_class* choose_vtt() {
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Select Turntable",
		GTK_WINDOW(main_window), GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_REJECT,
		"_OK", GTK_RESPONSE_ACCEPT, NULL);	

	GtkWidget *label = gtk_label_new ("Select turntable to load audio file to:");
	gtk_widget_show(label);

	gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label);
	gtk_container_set_border_width(GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 10);
	
	list <GtkWidget *> radio_buttons;
	list <vtt_class *> :: iterator iter;
	bool first = true;
	GtkWidget *radio;
	vtt_class *res_vtt = NULL;
	
	pthread_mutex_lock(&vtt_class::render_lock);
	
	for (iter = vtt_class::main_list.begin(); iter!=vtt_class::main_list.end(); iter++)  {
		if (first) {
			first = false;
			radio = gtk_radio_button_new_with_label(NULL, (*iter)->name);
		} else {
			radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio), (*iter)->name);
		}
		g_object_set_data(G_OBJECT(radio), "tX_vtt", (*iter));
		gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(dialog))), radio);
		gtk_widget_show(radio);
		radio_buttons.push_back(radio);
	}	
	
	pthread_mutex_unlock(&vtt_class::render_lock);
	
	// Giving up the lock here is necessary if we want audio to keep running
	// however it is also wrong. Anyway as it is a modal dialog not too many
	// evil things can happen. This sounds like some famous last words...
	
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	
	if (result == GTK_RESPONSE_ACCEPT) {
		list <GtkWidget *> :: iterator radio;
		
		for (radio = radio_buttons.begin(); radio!=radio_buttons.end(); radio++) {
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON((*radio)))) {
				res_vtt = (vtt_class*) g_object_get_data(G_OBJECT(*radio), "tX_vtt");
			}
		}
	} 
	
	gtk_widget_destroy(dialog);
	
	return res_vtt;
}


GCallback load_audio() {
	vtt_class *vtt = NULL;
	
	pthread_mutex_lock(&vtt_class::render_lock);
	if (vtt_class::main_list.size()==1) {
		vtt=(* vtt_class::main_list.begin());
	}
	pthread_mutex_unlock(&vtt_class::render_lock);
	
	if (vtt==NULL) {
		vtt = choose_vtt();
	}
	
	if (vtt!=NULL) load_file(NULL, vtt);
	
	return NULL;
}

GCallback drop_set(GtkWidget *widget, GdkDragContext *context,
		gint x, gint y, GtkSelectionData *selection_data,
		guint info, guint time, vtt_class *vtt)
{
	char filename[PATH_MAX];
	char *fn;
	
	strncpy(filename, (char *) gtk_selection_data_get_data(selection_data), (size_t) gtk_selection_data_get_length(selection_data));
	filename[gtk_selection_data_get_length(selection_data)]=0;

	fn = strchr (filename, '\r');
	*fn=0;	
	
	fn = strchr (filename, ':');
	if (fn) fn++; else fn=(char *) gtk_selection_data_get_data(selection_data);
	
	load_tt_part(fn);

	strcpy (filename, "dont segfault workaround ;)");
	return NULL;
}

/* save tables */
void do_save_tables(char *buffer)
{
	FILE *out;
	gzFile zout;
	char wbuf[PATH_MAX];
	char *ext;
	
	ext=strrchr(buffer, '.');
	
	if (ext) {
		if (strcmp(ext, ".tX")) strcat(buffer, ".tX");
	} else {
		strcat(buffer, ".tX");
	}

	tx_mg_have_setname=true;
	strcpy(tx_mg_current_setname, buffer);
	strcpy(globals.tables_filename, buffer);
	
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
}

GCallback save_tables_as()
{
	GtkWidget * dialog = gtk_file_chooser_dialog_new ("Save Set",
		GTK_WINDOW(main_window), GTK_FILE_CHOOSER_ACTION_SAVE, 
		"_Cancel", GTK_RESPONSE_CANCEL, 
		"_Save", GTK_RESPONSE_ACCEPT,NULL);
	
	if (tx_mg_have_setname) {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER (dialog), tx_mg_current_setname);
	}
				      
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
    	char buffer[PATH_MAX];
		char *filename=gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		strcpy(buffer, filename);
		g_free(filename);
		do_save_tables(buffer);
	}	
	
	gtk_widget_destroy(dialog);	

	return NULL;
}

GCallback save_tables()
{
	if (!tx_mg_have_setname) {
		save_tables_as();
	} else {
		do_save_tables(tx_mg_current_setname);
	}
	
	return NULL;
}

GCallback master_volume_changed (GtkWidget *wid, void *d)
{
	sp_master_volume.receive_gui_value((float) gtk_adjustment_get_value(GTK_ADJUSTMENT(wid)));
	return NULL;	
}

GCallback master_pitch_changed(GtkWidget *wid, void *d)
{
	sp_master_pitch.receive_gui_value((float) gtk_adjustment_get_value(GTK_ADJUSTMENT(wid)));	
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

GCallback seq_stop(GtkWidget *w, void *);

static bool stop_override=false;

GCallback audio_on(GtkWidget *w, void *d)
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
				case ERROR_BACKEND:
				char audio_backend[256];
				char message[4096];

				switch (globals.audiodevice_type) {
				    case OSS:
				    	strcpy(audio_backend, "the Open Sound System (OSS)");
				    	break;
				    case ALSA:
				    	strcpy(audio_backend, "the Advanced Linux Sound Architecture (ALSA)");
				    	break;
				    case JACK:
				    	strcpy(audio_backend, "the JACK Audio Connection Kit");
				    	break;
				    case PULSE:
				    	strcpy(audio_backend, "PulseAudio");
				    	break;
				    default:
					strcpy(audio_backend, "unkown");
				}

				sprintf(message,"Error starting engine: couldn't start audio driver.\nThis terminatorX binary was compiled without support for %s.\nPlease check the audio device settings in the \"Preferences\" dialog.", audio_backend);
				tx_note(message, true);
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
		update_tag=g_timeout_add(globals.update_idle, (GSourceFunc) pos_update, NULL);
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
		
		if (tX_engine::get_instance()->get_runtime_error()) {
			tx_note("Fatal: The audio device broke down while playing\nback audio. Note that that some audio devices can not\nrecover from such a breakdown.", true);
		}
		if (tX_engine::get_instance()->get_overload_error()) {
			tx_note("Fatal: The audio engine was stopped due to an overload\ncondition. Try reducing the amount of plugins or\nturntables.", true);
		}
	}
	
	return NULL;
}

GCallback cancel_rec(GtkWidget *wid)
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
	
	strcpy(buffer, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(rec_dialog)));

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

GCallback select_rec_file()
{
	GtkWidget * dialog = gtk_file_chooser_dialog_new ("Record To Disk",
		GTK_WINDOW(main_window), GTK_FILE_CHOOSER_ACTION_SAVE, 
		"_Cancel", GTK_RESPONSE_CANCEL, 
		"_Save", GTK_RESPONSE_ACCEPT,NULL);
	
	if (strlen(globals.record_filename)) {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER (dialog), globals.record_filename);
	}
				      
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename=gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		strncpy(globals.record_filename, filename, sizeof(globals.record_filename)-1);
		g_free(filename);
		
		tX_engine::get_instance()->set_recording_request(true);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec_menu_item), 1);
	} else {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec_menu_item), 0);
	}
	
	rec_dont_care = 0;
	
	gtk_widget_destroy(dialog);

	return NULL;
}

GCallback tape_on(GtkWidget *w, void *d)
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
		if (mouse.grab() != 0) {
			//tX_engine::get_instance()->set_grab_request();
			// TODO: handle error
		}
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
		"Exit terminatorX and lose all unsaved data?");
		
		int res=gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
			
		if (res!=GTK_RESPONSE_YES) {
			return TRUE;
		}
	}
	
	tX_shutdown=true;
	
	turn_audio_off();
	vtt_class::delete_all();

	if (update_tag) {
		g_source_remove(update_tag);
	}
	GtkAllocation allocation;
	gtk_widget_get_allocation(GTK_WIDGET(main_window), &allocation);
	globals.width=allocation.width;
	globals.height=allocation.height;

	gtk_main_quit();
	
	return true;
}

void mplcfitx()
/* Most Probably Least Called Function In terminatorX :) */
{
	show_about(0);
}

GCallback seq_play(GtkWidget *w, void *)
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

GCallback seq_stop(GtkWidget *w, void *)
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

GCallback seq_rec(GtkWidget *w, void *)
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
		pos=sequencer.set_start_timestamp((float) gtk_adjustment_get_value(GTK_ADJUSTMENT(wid)));
		seq_update_entry(pos);	
	}
}

#define add_sep(); 	dummy=gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);\
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);\
	gtk_widget_show(dummy);\

#define add_sep2(); 	dummy=gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);\
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);\
	gtk_widget_show(dummy);\

#ifdef USE_X11
void fullscreen_toggle(GtkCheckMenuItem *item, gpointer data);
#endif

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
	sprintf(label_str, "Delete all <b>%s</b> events for turntable '%s'.", sp->get_name(), sp->get_vtt_name());
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
		
		for (sp=tX_seqpar::all->begin(); sp!=tX_seqpar::all->end(); sp++) {
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

	menu_item = gtk_menu_item_new_with_mnemonic("Load _Audio File");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) load_audio, NULL);

	menu_item = gtk_separator_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);

	menu_item = gtk_menu_item_new_with_mnemonic("_New Set");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) new_tables, NULL);

	menu_item = gtk_menu_item_new_with_mnemonic("_Open Set");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) load_tables, NULL);

	menu_item = gtk_menu_item_new_with_mnemonic("_Save Set");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) save_tables, NULL);

	menu_item = gtk_menu_item_new_with_label("Save Set As");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) save_tables_as, NULL);

	menu_item = gtk_separator_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);

	menu_item = gtk_menu_item_new_with_mnemonic("_Quit");
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
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_KEY_A, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);	
	g_signal_connect(menu_item, "activate", (GCallback) new_table, NULL);

	menu_item = gtk_separator_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);

	menu_item = gtk_menu_item_new_with_mnemonic("Assign _Default MIDI Mappings");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_KEY_M, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

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
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_KEY_C, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

#ifdef USE_ALSA_MIDI_IN
	g_signal_connect(menu_item, "activate", G_CALLBACK(tX_midiin::clear_midi_mappings), (void *) true);
#else
	gtk_widget_set_sensitive(menu_item, FALSE);
#endif

	menu_item = gtk_separator_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);

	menu_item = gtk_check_menu_item_new_with_mnemonic("_Record Audio To Disk");
	rec_menu_item = menu_item;
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_KEY_R, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
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

	menu_item = gtk_separator_menu_item_new ();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);

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
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_KEY_F11, (GdkModifierType) 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(menu_item, "activate", (GCallback) fullscreen_toggle, NULL);

#ifndef USE_X11
	gtk_widget_set_sensitive(menu_item, False);
#endif
	menu_item = gtk_separator_menu_item_new();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);

	menu_item = gtk_menu_item_new_with_mnemonic("_Preferences");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) display_options, NULL);

	/* HELP */ 
	menu_item = gtk_menu_item_new_with_mnemonic ("_Help");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (main_menubar), menu_item);
	
	sub_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);

	menu_item = gtk_menu_item_new_with_mnemonic ("_Contents");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) display_help, NULL);
	gtk_widget_add_accelerator (menu_item, "activate", accel_group, GDK_KEY_F1, (GdkModifierType) 0, GTK_ACCEL_VISIBLE);

	menu_item = gtk_menu_item_new_with_mnemonic ("_About");
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);
	g_signal_connect(menu_item, "activate", (GCallback) mplcfitx, NULL);
	
	menu_item = gtk_separator_menu_item_new();
	gtk_widget_show (menu_item);
	gtk_container_add (GTK_CONTAINER (sub_menu), menu_item);

	menu_item = gtk_menu_item_new_with_mnemonic ("_Visit terminatorX.org");
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
	GtkWidget *wrapbox;
	
	main_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_title(GTK_WINDOW(main_window), "terminatorX");

	gtk_widget_realize(main_window);
	
	GtkWidget *posLabel=gtk_label_new("Pos:");
	PangoRectangle ink_rect;
	PangoRectangle logical_rect;
	pango_layout_get_pixel_extents(gtk_label_get_layout(GTK_LABEL(posLabel)), &ink_rect, &logical_rect);
	tX_debug("ink extent: x: %i, y: %i, width: %i, height %i ",  ink_rect.x, ink_rect.y, ink_rect.width, ink_rect.height);
	tX_debug("logical extent: x: %i, y: %i, width: %i, height %i ",  logical_rect.x, logical_rect.y, logical_rect.width, logical_rect.height);
	fontHeight = logical_rect.height - logical_rect.y;

	tx_icons_init(fontHeight);

	wrapbox=gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_add(GTK_CONTAINER(main_window), wrapbox);
	gtk_widget_show(wrapbox);

	main_menubar=gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(wrapbox), main_menubar, WID_FIX);
	gtk_widget_show(main_menubar);

	mother_of_all_boxen=gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_set_border_width(GTK_CONTAINER(mother_of_all_boxen), 5);
	gtk_widget_set_hexpand(mother_of_all_boxen, TRUE);
	gtk_widget_set_vexpand(mother_of_all_boxen, TRUE);
	gtk_container_add(GTK_CONTAINER(wrapbox), mother_of_all_boxen);
	gtk_widget_show(mother_of_all_boxen);	

	create_master_menu();

	g_signal_connect(G_OBJECT(main_window), "motion_notify_event", G_CALLBACK(tx_mouse::motion_notify_wrap), &mouse);
	g_signal_connect(G_OBJECT(main_window), "button_press_event", G_CALLBACK(tx_mouse::button_press_wrap), &mouse);
	g_signal_connect(G_OBJECT(main_window), "button_release_event", G_CALLBACK(tx_mouse::button_release_wrap), &mouse);
	g_signal_connect(G_OBJECT(main_window), "key_press_event", G_CALLBACK(tx_mouse::key_press_wrap), &mouse);
	g_signal_connect(G_OBJECT(main_window), "key_release_event", G_CALLBACK(tx_mouse::key_release_wrap), &mouse);
	
	main_vbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_box_pack_start(GTK_BOX(mother_of_all_boxen), main_vbox, WID_DYN);
	gtk_widget_show(main_vbox);
	
	left_hbox=gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), left_hbox, WID_DYN);
	gtk_widget_show(left_hbox);
	
	control_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_box_pack_start(GTK_BOX(left_hbox), control_box, WID_FIX);
	gtk_widget_show(control_box);
	
	dummy=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(left_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);

    /* control_box contents */

	dummy=tx_xpm_label_box(AUDIOENGINE, "Audio");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX_BREATHE);
	gtk_widget_show(dummy);
	
	dummy=tx_xpm_button_new(POWER,"Power ", 1);
	connect_button(dummy,audio_on, NULL);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Turn the audio engine on/off.");
	gtk_widget_show(dummy);
	engine_btn=dummy;
	
	grab_button=tx_xpm_button_new(GRAB, "Mouse Grab ", 1);
	gtk_box_pack_start(GTK_BOX(control_box), grab_button, WID_FIX);
	connect_button(grab_button, grab_on, NULL);
	gui_set_tooltip(grab_button, "Enter the mouse grab mode operation. Press <ESCAPE> to exit grab mode.");
	gtk_widget_show(grab_button);

	dummy=gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
    
	dummy=tx_xpm_label_box(SEQUENCER, "Seq.");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX_BREATHE);
	gtk_widget_show(dummy);

	dummy=tx_xpm_button_new(PLAY,"Play ", 1);
	connect_button(dummy, seq_play, NULL);
	seq_play_btn=dummy;
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Playback previously recorded events from the sequencer. This will turn on the audio engine automagically.");
	gtk_widget_show(dummy);

	dummy=tx_xpm_button_new(STOP,"Stop ", 0);
	seq_stop_btn=dummy;
	connect_button(dummy, seq_stop, NULL);	
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Stop the playback of sequencer events.");
	gtk_widget_show(dummy);

	dummy=tx_xpm_button_new(RECORD,"Record ", 1);
	connect_button(dummy, seq_rec, NULL);
	seq_rec_btn=dummy;
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gui_set_tooltip(dummy, "Enable recording of *events* into the sequencer. All touched controls will be recorded. Existing events for the song-time recording will be overwritten for touched controls.");
	gtk_widget_show(dummy);

	gtk_box_pack_start(GTK_BOX(control_box), posLabel, WID_FIX_BREATHE);
	gtk_widget_show(posLabel);
	
	dummy=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(dummy), 12);
	seq_entry=dummy;
	//gtk_widget_set_usize(dummy, 65, 20);
	gtk_entry_set_text(GTK_ENTRY(dummy), "00:00.00");
#if GTK_CHECK_VERSION(2,4,0)
	gtk_entry_set_alignment(GTK_ENTRY(dummy), 0.5);
#endif	
	gtk_entry_set_width_chars(GTK_ENTRY(dummy), 9);
	gtk_entry_set_max_width_chars(GTK_ENTRY(dummy), 9);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dumadj=(GtkAdjustment*) gtk_adjustment_new(0, 0, 100, 0.1, 1, 1);
	seq_adj=dumadj;
	connect_adj(dumadj, sequencer_move, NULL);	
	dummy=gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, dumadj);
	gtk_widget_set_size_request(dummy, 65, 20);
	seq_slider=dummy;
	g_signal_connect(G_OBJECT(seq_slider), "button-release-event", (GCallback) seq_slider_released, NULL);
	gtk_scale_set_draw_value(GTK_SCALE(dummy), FALSE);
	
	gui_set_tooltip(dummy, "Select the start position for the sequencer in song-time.");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_DYN);
	gtk_widget_show(dummy);
	
	dummy=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2); //gtk_hpaned_new ();
	gtk_box_pack_start(GTK_BOX(left_hbox), dummy, WID_DYN);
	gtk_widget_show(dummy);
	
	tt_parent=dummy;

    panel_bar=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_box_set_homogeneous(GTK_BOX(panel_bar), TRUE);
	gtk_box_pack_start(GTK_BOX(left_hbox), panel_bar, WID_FIX);

	control_parent=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,4);
	gtk_box_pack_start(GTK_BOX(tt_parent), control_parent, WID_FIX);
	gtk_widget_show(control_parent);

	dummy=gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	gtk_box_pack_start(GTK_BOX(tt_parent), dummy, WID_FIX);
	gtk_widget_show(dummy);

	audio_parent=gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
	gtk_box_pack_start(GTK_BOX(tt_parent), audio_parent, WID_DYN);
	gtk_widget_show(audio_parent);
	
	dummy=gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	gtk_box_pack_start(GTK_BOX(main_vbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
		
	right_hbox=gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), right_hbox, WID_FIX);
	gtk_widget_show(right_hbox);

	/* Master */
	
	dummy=gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(dummy),"<b>Master</b>");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);	

	dummy=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
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
	
	dummy=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	/* Volume */
	master_vol_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_box_pack_start(GTK_BOX(right_hbox), master_vol_box, WID_DYN);
	gtk_widget_show(master_vol_box);
	
	dumadj=(GtkAdjustment*) gtk_adjustment_new(globals.volume, 0, 2, 0.01, 0.05, 0.000);
	volume_adj=dumadj;

	connect_adj(dumadj, master_volume_changed, NULL);
	dummy=gtk_scale_new(GTK_ORIENTATION_VERTICAL, dumadj);
	gtk_range_set_inverted(GTK_RANGE(dummy), TRUE);
	gtk_scale_set_draw_value(GTK_SCALE(dummy), FALSE);
	g_signal_connect(G_OBJECT(dummy), "button_press_event", (GCallback) tX_seqpar::tX_seqpar_press, &sp_master_volume);	
	
	gtk_box_pack_end(GTK_BOX(master_vol_box), dummy, WID_FIX);
	gtk_widget_show(dummy);	
	gui_set_tooltip(dummy, "Adjust the master volume. This parameter will effect *all* turntables in the set.");
	
	main_flash=gtk_tx_flash_new();
	gtk_box_pack_end(GTK_BOX(master_vol_box), main_flash, WID_DYN);
	gtk_widget_show(main_flash);

	dummy=gtk_label_new("Volume");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);

	/* STATUS BOX */ 
	dummy=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	status_box=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(right_hbox), status_box, WID_FIX);
	gtk_widget_show(status_box);
	
	dummy=gtk_label_new("0");
	used_mem=dummy;
	gtk_widget_set_halign(dummy, GTK_ALIGN_END);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

//	dummy=gtk_label_new("Mem/MB:");
//	gtk_widget_set_halign(dummy, GTK_ALIGN_START);
//	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
//	gtk_widget_show(dummy);
	
	/*add_sep2();

	dummy=gtk_label_new("1");
	no_of_vtts=dummy;
	gtk_widget_set_halign(dummy, GTK_ALIGN_END);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Vtts:");
	gtk_widget_set_halign(dummy, GTK_ALIGN_START);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);*/

	add_sep2();

	dummy=gtk_label_new("v" VERSION);
	gtk_widget_set_halign(dummy, GTK_ALIGN_END);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	/*dummy=gtk_label_new("Release:");
	gtk_widget_set_halign(dummy, GTK_ALIGN_START);
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);*/
	
	add_sep2();

	dummy=gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(dummy), "<b>Status</b>");
	gtk_box_pack_end(GTK_BOX(status_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	/* END GUI */
	
	gtk_window_set_default_size(GTK_WINDOW(main_window), x, y);	
	gtk_widget_set_sensitive(grab_button, 0);

	new_table(NULL, NULL); // to give the user something to start with ;)

	g_signal_connect (G_OBJECT(main_window), "delete-event", (GCallback) quit, NULL);
	
//	if (globals.tooltips) gtk_tooltips_enable(gui_tooltips);
//	else gtk_tooltips_disable(gui_tooltips);
//  TODO: Check for global enable/disable of tooltips
}

gfloat old_percent=-1;

void note_destroy(GtkWidget *widget, GtkWidget *mbox)
{
	gtk_widget_destroy(GTK_WIDGET(mbox));
}

void tx_note(const char *message, bool isError, GtkWindow *window)
{
	if (!window) window=GTK_WINDOW(main_window);
	
	GtkWidget *dialog=gtk_message_dialog_new_with_markup(window,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		isError ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "%s", "");
	gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), message);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);	
}


void tx_l_note(const char *message)
{
	char buffer[4096]="Plugin info:\n\n";
	strcat(buffer, message);
	
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(main_window),
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "%s", message);
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

#ifdef USE_X11
/* Fullscreen code... */
#define _WIN_LAYER_TOP 		-1
#define _WIN_LAYER_NORMAL	4
#define _NET_WM_STATE_REMOVE	0
#define _NET_WM_STATE_ADD	1
#define _NET_WM_STATE_TOGGLE	2

void fullscreen_toggle(GtkCheckMenuItem *item, gpointer data) {
	XEvent xev;
	Window win = GDK_WINDOW_XID(gtk_widget_get_window(main_window));
	Display *disp=GDK_WINDOW_XDISPLAY(gtk_widget_get_window(main_window));
	
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
	XSendEvent(gdk_x11_get_default_xdisplay(), GDK_WINDOW_XID (gdk_get_default_root_window ()),
		False, SubstructureRedirectMask | SubstructureNotifyMask,
		&xev);	
}

void fullscreen_setup() {
	if (globals.fullscreen_enabled) {
		fullscreen_toggle(NULL, NULL);
	}
}
#endif

void display_mastergui()
{
	GtkWidget *top;
	gtk_widget_realize(main_window);
	tX_set_icon(main_window);
	load_knob_pixs(fontHeight, gdk_window_get_scale_factor(gtk_widget_get_window(GTK_WIDGET(main_window))));
	gtk_widget_show(main_window);
	top=gtk_widget_get_toplevel(main_window);
	top_window=GDK_WINDOW(gtk_widget_get_window(top));

#ifdef USE_X11	
	fullscreen_setup();	
	x_window=gdk_x11_window_get_xid(gtk_widget_get_window(top));
#endif
}

pid_t help_child=0;
GTimer *help_timer=NULL;
int help_tag=-1;

gboolean help_checker()
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
			g_source_remove(help_tag);
			help_tag=-1;
		}
	}  else {
		//yelp waitpid status does not allow determining success
		//printf("%i %i\n", WIFEXITED(status), WEXITSTATUS(status));
		///* We are still here and the child exited - that could mean trouble. */
		//tx_note("Couldn't run the gnome-help command (alias \"yelp\") to display the terminatorX manual. Please ensure that \"yelp\" is installed.", true);		
		
		g_source_remove(help_tag);
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
		g_source_remove(help_tag);
		if (help_timer) g_timer_destroy(help_timer);
		help_child=0;
		help_tag=-1;
		help_timer=NULL;
	}
	
	if (help_child==0) {
		// child
		execlp("gnome-help","gnome-help","help:terminatorX-manual", NULL);
		_exit(-1);
	} else if (help_child==-1) {
		tx_note("System error: couldn't fork() to run the help process.", true);
	} else {
		help_timer=g_timer_new();
		g_timer_start(help_timer);
	
		help_tag=g_idle_add((GSourceFunc) help_checker, NULL);
	}
}

pid_t browser_child=0;
GTimer *browser_timer=NULL;
int browser_tag=-1;

void display_browser()
{	
	browser_child=fork();
	
	if (browser_child==0) {
		// child
		execlp(BROWSER1, BROWSER1, "http://terminatorX.org", NULL);
		execlp(BROWSER2, BROWSER2, "http://terminatorX.org", NULL);
		execlp(BROWSER3, BROWSER3, "http://terminatorX.org", NULL);
		_exit(-1);
	} else if (browser_child==-1) {
		tx_note("System error: couldn't fork() to run the browser process.", true);
	} 
}



GdkCursor *tX_cursor::cursors[MAX_CURSOR]={NULL, NULL, NULL};
tX_cursor::cursor_shape tX_cursor::current_shape=tX_cursor::DEFAULT_CURSOR;

void tX_cursor::set_cursor(cursor_shape shape)
{
	GdkDisplay *display = gdk_window_get_display(gtk_widget_get_window(main_window));
	switch (shape) {
		case DEFAULT_CURSOR:
			cursors[shape]=NULL;
			break;
		
		case WAIT_CURSOR:
			if (!cursors[shape]) cursors[shape]=gdk_cursor_new_for_display(display, GDK_WATCH);
			break;
		
		case WAIT_A_SECOND_CURSOR:
			/* FIXME: What's that short-time wait cursor's id? */
			if (!cursors[shape]) cursors[shape]=gdk_cursor_new_for_display(display, GDK_WATCH);
			break;
		
		default:
			tX_debug("No such cursor shape.");
			return;
	}
	
	/* Still here? Ok... */
	current_shape=shape;
	
	gdk_window_set_cursor(gtk_widget_get_window(main_window), cursors[shape]);
}

GdkCursor *tX_cursor::get_cursor()
{
	return cursors[current_shape];
}

void tX_cursor::reset_cursor()
{
	current_shape=DEFAULT_CURSOR;
	gdk_window_set_cursor(gtk_widget_get_window(main_window), cursors[current_shape]);
}
