/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2011  Alexander K�nig
 
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
 
    File: tX_vttgui.cc
 
    Description: This implements the gtk+ GUI for the virtual turntable
    		 class implemented in tX_vtt.cc. This code is not in tX_vtt.cc
		 for mainly to keep the GUI code divided from the audio-rendering
		 code and as gtk+ callback to C++ method call wrapper.
		 
    Changes:
    
    before 11-26-2001: too many changes.
    
    11-26-2001: applied Adrian's solo/mute patch - Alex
    
    11-27-2001: modified solo/mute to use the set_mix_mute/solo function
    		of the vtts. The previous approach messed up the sequencer
		and more. Removed some old unnecessary code, too.
*/    

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "tX_vttgui.h"
#include "tX_vtt.h"
#include "tX_widget.h"
#include "tX_mastergui.h"
#include "tX_loaddlg.h"
#include "tX_prelis.h"
#include "tX_pbutton.h"
#include "tX_global.h"
#include "tX_extdial.h"
#include "tX_panel.h"
#include "tX_ladspa.h"
#include "tX_ladspa_class.h"
#include "tX_engine.h"
#include "tX_glade_interface.h"
#include "tX_glade_support.h"
#include "tX_dialog.h"

#ifdef USE_DIAL
#include "tX_dial.h"
#endif

#include "tX_flash.h"
#include <stdio.h>
#include <ctype.h>

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0

static gint vg_show_fx_menu(GtkWidget *wid, GdkEventButton *event, vtt_fx *effect);

void nicer_filename(char *dest, char *source)
{
		char *fn;
		char temp[PATH_MAX];
		
		fn=strrchr(source, '/');
		if (fn) fn++;
		else fn=source;
		
		strcpy (temp, fn);
		
		fn=strrchr(temp, '.');
		if (fn) *fn=0;
		
		if (strlen(temp) > (unsigned int) globals.filename_length) {
			temp[globals.filename_length-3]='.';
			temp[globals.filename_length-2]='.';
			temp[globals.filename_length-1]='.';
			temp[globals.filename_length]=0;
		} 
		strcpy (dest, temp);
}

void name_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_name((char *) gtk_entry_get_text(GTK_ENTRY(wid)));
}

void volume_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_volume.receive_gui_value(2.0-GTK_ADJUSTMENT(wid)->value);
}

void pan_changed(GtkWidget *wid, vtt_class *vtt)
{
      vtt->sp_pan.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

void pitch_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_pitch.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

#ifdef USE_FILECHOOSER
GtkSignalFunc chooser_prelis(GtkWidget *w)
{
	GtkFileChooser *fc=GTK_FILE_CHOOSER(gtk_widget_get_toplevel(w));
	char *filename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));
	
	if (filename) {
		prelis_start(filename);
		g_free(filename);
	} else {
		prelis_stop();
	}
	return NULL;
}
#else
GtkSignalFunc trigger_prelis(GtkWidget *w)
{
	GtkFileSelection *fs;

	fs=GTK_FILE_SELECTION(gtk_widget_get_toplevel(w));
	
	prelis_start((char *) gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)));
	return NULL;
}

void cancel_load_file(GtkWidget *wid, vtt_class *vtt)
{
	prelis_stop();
	
	vtt->gui.file_dialog=NULL;
	if (vtt->gui.fs) gtk_widget_destroy(GTK_WIDGET(vtt->gui.fs));
}

int quit_load_file(GtkWidget *wid, vtt_class *vtt)
{
	return 1;
}
#endif

char global_filename_buffer[PATH_MAX];

void load_part(char *newfile, vtt_class *vtt)
{
	tX_audio_error ret=TX_AUDIO_SUCCESS;

	ld_create_loaddlg(TX_LOADDLG_MODE_SINGLE, 1);
	ld_set_filename(newfile);

	ret = vtt->load_file(newfile);
	
	ld_destroy();
	if (ret) {
		switch(ret) {
			case TX_AUDIO_ERR_ALLOC:
			tx_note("Failed to load audiofile - there's not enough memory available.", true);
			break;
			case TX_AUDIO_ERR_PIPE_READ:
			tx_note("An error occured on reading from the piped process - probably the file format of the audiofile is not supported by this configuration - please check terminatorX' INSTALL file on howto configure terminatorX for files of this format.", true);
			break;
			case TX_AUDIO_ERR_SOX:
			tx_note("Failed to run sox - to load the given audiofile please ensure that sox is installed correctly.", true);
			break;
			case TX_AUDIO_ERR_MPG123:
			tx_note("Failed to run mpg123 - to load the given mp3 file please ensure that mpg123 (or mpg321) is installed correctly.", true);
			break;
			case TX_AUDIO_ERR_WAV_NOTFOUND:
			tx_note("Couldn't acces the audiofile - file not found.", true);
			break;
			case TX_AUDIO_ERR_NOT_16BIT:
			tx_note("The wav file doesn't use 16 bit wide samples - please compile terminatorX with libaudiofile support to enable loading of such files.", true);
			break;
			case TX_AUDIO_ERR_NOT_MONO:
			tx_note("The wav file is not mono - please compile terminatorX with libaudiofile support to enable loading of such files.", true);
			break;
			case TX_AUDIO_ERR_WAV_READ:
			tx_note("The wav file seems to be corrupt.", true);
			break;
			case TX_AUDIO_ERR_NOT_SUPPORTED:
			tx_note("The file format of the audiofile is not supported - please check terminatorX' INSTALL file on howto configure terminatorX for files of this format.", true);
			break;
			case TX_AUDIO_ERR_MAD_OPEN:
			tx_note("Failed to open this mp3 file - please ensure that the file exists and is readable.", true);
			break;	
			case TX_AUDIO_ERR_MAD_STAT:
			tx_note("Failed to 'stat' this mp3 file - please ensure that the file exists and is readable.", true);
			break;
			case TX_AUDIO_ERR_MAD_DECODE:
			tx_note("Failed to decode the mp3 stream - file is corrupt.", true);
			break;
			case TX_AUDIO_ERR_MAD_MMAP:
			tx_note("Failed to map the audiofile to memory - please ensure the file is readable.", true);
			break;
			case TX_AUDIO_ERR_MAD_MUNMAP:
			tx_note("Failed to unmap audiofile.", true);
			break;
			case TX_AUDIO_ERR_VORBIS_OPEN:
			tx_note("Failed to open ogg file - please ensure the file is an ogg stream and that it is readable.", true);
			break;			
			case TX_AUDIO_ERR_VORBIS_NODATA:
			tx_note("The vorbis codec failed to decode any data - possibly this ogg stream is corrupt.", true);
			break;
			case TX_AUDIO_ERR_AF_OPEN:
			tx_note("Failed to open this file with libaudiofile - please check terminatorX' INSTALL file on howto configure terminatorX for files of this format.",true);
			break;
			case TX_AUDIO_ERR_AF_NODATA:
			tx_note("libaudiofile failed to decode any data - possilby the audiofile is corrupt.", true);
			break;
			default:					
			tx_note("An unknown error occured - if this bug is reproducible please report it, thanks.", true);	
		}
	} else {
		nicer_filename(global_filename_buffer, newfile);
        gtk_button_set_label(GTK_BUTTON(vtt->gui.file), global_filename_buffer);
	}	
}

#ifndef USE_FILECHOOSER
void do_load_file(GtkWidget *wid, vtt_class *vtt)
{
	char newfile[PATH_MAX];
	
	prelis_stop();

	strcpy(newfile, gtk_file_selection_get_filename(GTK_FILE_SELECTION(vtt->gui.fs)));
	gtk_widget_destroy(GTK_WIDGET(vtt->gui.fs));
	vtt->gui.file_dialog=NULL;
	
	tX_cursor::set_cursor(tX_cursor::WAIT_CURSOR);
	load_part(newfile, vtt);
	strcpy(globals.current_path, newfile);
	tX_cursor::reset_cursor();
}
#endif

void drop_file(GtkWidget *widget, GdkDragContext *context,
		gint x, gint y, GtkSelectionData *selection_data,
		guint info, guint time, vtt_class *vtt)
{
	char filename[PATH_MAX];
	char *fn;
	
	strncpy(filename, (char *) selection_data->data, (size_t) selection_data->length);
	gtk_drag_finish(context, TRUE, FALSE, time);
	filename[selection_data->length]=0;

	fn = strchr (filename, '\r');
	*fn=0;	
	
	char *realfn=NULL;
	char *host=NULL;
	
	realfn=g_filename_from_uri(filename, &host, NULL);
	if (realfn) {
		fn=realfn;
	} else {
		fn = strchr (filename, ':');
		if (fn) fn++; else fn=(char *) selection_data->data;
	}

	load_part(fn, vtt);

	if (realfn) g_free(realfn);
	if (host) g_free(host);
}

GtkSignalFunc load_file(GtkWidget *wid, vtt_class *vtt)
{	
#ifdef USE_FILECHOOSER
	const char *extensions[]={ "mp3", "wav", "ogg", "iff", "aiff", "voc", "au", NULL };
	char name_buf[512];
	sprintf(name_buf, "Select Audio File for %s", vtt->name);
	
	GtkWidget * dialog = gtk_file_chooser_dialog_new (name_buf,
		GTK_WINDOW(main_window), GTK_FILE_CHOOSER_ACTION_OPEN, 
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

		
	GtkFileFilter *filter=gtk_file_filter_new();
	for (int i=0; extensions[i]!=NULL; i++) {
		char buffer[32]="*.";
		
		gtk_file_filter_add_pattern(filter, strcat(buffer, extensions[i]));
		for (unsigned int c=0; c<strlen(buffer); c++) {
			buffer[c]=toupper(buffer[c]);
		}
		gtk_file_filter_add_pattern(filter, buffer);
	}
	
	gtk_file_filter_set_name(filter, "Audio Files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter); 

	filter=gtk_file_filter_new();
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_filter_set_name(filter, "All Files");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter); 
	
	if (vtt->audiofile) {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), vtt->filename);	
	} else if (strlen(globals.current_path)>0 ) {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), globals.current_path);
	}
	
	g_signal_connect (G_OBJECT(dialog), "selection-changed", G_CALLBACK(chooser_prelis), vtt);	
	
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
    	char * filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		gtk_widget_hide(dialog);
		tX_cursor::set_cursor(tX_cursor::WAIT_CURSOR);
		load_part(filename, vtt);
		strcpy(globals.current_path, filename);
		tX_cursor::reset_cursor();		
	}	
	
	prelis_stop();
	gtk_widget_destroy(dialog);	
#else
	char buffer[512];
	
	if (vtt->gui.file_dialog) {
		gdk_window_raise(vtt->gui.file_dialog);
		return(0);
	}
	
	sprintf(buffer, "Select Audio File for %s", vtt->name);
	vtt->gui.fs=gtk_file_selection_new(buffer);
	
	if (vtt->audiofile) {
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(vtt->gui.fs), vtt->filename);	
	} else if (strlen(globals.current_path)>0 ) {
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(vtt->gui.fs),globals.current_path);
	}
	
	gtk_widget_show(GTK_WIDGET(vtt->gui.fs));	
	vtt->gui.file_dialog=vtt->gui.fs->window;
	
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(vtt->gui.fs)->ok_button), "clicked", G_CALLBACK(do_load_file), vtt);
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(vtt->gui.fs)->cancel_button), "clicked", G_CALLBACK (cancel_load_file), vtt);	
	g_signal_connect (G_OBJECT(vtt->gui.fs), "delete-event", G_CALLBACK(quit_load_file), vtt);	
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(vtt->gui.fs)->file_list), "cursor_changed", G_CALLBACK(trigger_prelis), vtt->gui.fs);		
#endif	
	return NULL;
}

void delete_vtt(GtkWidget *wid, vtt_class *vtt)
{
	if (audioon) tx_note("Sorry, you'll have to stop playback first.");
	else delete(vtt);
		
	mg_update_status();
}

void edit_vtt_buffer(GtkWidget *wid, vtt_class *vtt)
{
	char command[2*PATH_MAX];

	if (vtt->samples_in_buffer == 0) {
		tx_note("No audiofile loaded - so there's nothing to edit.", true);
	} else if (strlen(globals.file_editor)>0) {
		sprintf(command, "%s \"%s\" &", globals.file_editor, vtt->filename);
		int res = system(command); /*) tx_note("Failed to run the soundfile editor."); */
	} else {
		tx_note("No soundfile editor has been configured - to do so enter the soundfile editor of your choice in the options dialog.", true);
	}
}

void reload_vtt_buffer(GtkWidget *wid, vtt_class *vtt)
{
	char reload_buffer[PATH_MAX];
	
	while (gtk_events_pending()) gtk_main_iteration();
	
	if (vtt->samples_in_buffer > 0) {
		strcpy(reload_buffer, vtt->filename);
		load_part(reload_buffer, vtt);
	}
	else tx_note("No audiofile loaded - so there's nothing to reload.", true);
}

void clone_vtt(GtkWidget *wid, vtt_class *vtt)
{
	vtt->stop();
}

void trigger_vtt(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_trigger.receive_gui_value((float) 1.0);
}

void stop_vtt(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_trigger.receive_gui_value((float) 0.0);
}

void autotrigger_toggled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_autotrigger(GTK_TOGGLE_BUTTON(wid)->active);
}

void loop_toggled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_loop.receive_gui_value(GTK_TOGGLE_BUTTON(wid)->active);
}

void lp_enabled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_lp_enable.receive_gui_value(GTK_TOGGLE_BUTTON(wid)->active);
}

void lp_gain_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_lp_gain.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

void lp_reso_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_lp_reso.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

void lp_freq_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_lp_freq.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

void ec_enabled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_ec_enable.receive_gui_value(GTK_TOGGLE_BUTTON(wid)->active);
}

#ifdef USE_ALSA_MIDI_IN
void midi_mapping_clicked(GtkWidget *wid, vtt_class *vtt)
{
	tX_engine::get_instance()->get_midi()->configure_bindings(vtt);
}
#endif

void ec_length_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_ec_length.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

void ec_feedback_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_ec_feedback.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

void ec_pan_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_ec_pan.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

void ec_volume_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_ec_volume.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

void master_setup(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_sync_master(GTK_TOGGLE_BUTTON(wid)->active);
}

void client_setup(GtkWidget *wid, vtt_class *vtt)
{
	int client;
	
	client=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(vtt->gui.sync_client));	
	vtt->sp_sync_client.receive_gui_value(client);
}

void client_setup_number(GtkWidget *wid, vtt_class *vtt)
{
	int cycles;
	
	cycles=(int) GTK_ADJUSTMENT(vtt->gui.cycles)->value;	
	
	vtt->sp_sync_cycles.receive_gui_value(cycles);
}

void mute_volume(GtkWidget *widget, vtt_class *vtt)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		vtt->set_mix_mute(1);
	} else {
		vtt->set_mix_mute(0);
	}
}

void solo_vtt(GtkWidget *widget, vtt_class *vtt)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
		vtt->set_mix_solo(1);
	} else {
		vtt->set_mix_solo(0);
	}
}      

void minimize_control_panel(GtkWidget *wid, vtt_class *vtt)
{
	vtt->hide_control(true);
}

void unminimize_control_panel(GtkWidget *wid, vtt_class *vtt)
{
	vtt->hide_control(false);
}

void minimize_audio_panel(GtkWidget *wid, vtt_class *vtt)
{
	vtt->hide_audio(true);
}

void unminimize_audio_panel(GtkWidget *wid, vtt_class *vtt)
{
	vtt->hide_audio(false);
}

void vg_xcontrol_dis(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_x_input_parameter(NULL);
}

void vg_ycontrol_dis(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_y_input_parameter(NULL);
}

void vg_xcontrol_set(GtkWidget *wid, tX_seqpar *sp)
{
	vtt_class *vtt=(vtt_class *) sp->vtt;
	vtt->set_x_input_parameter(sp);
}

void vg_ycontrol_set(GtkWidget *wid, tX_seqpar *sp)
{
	vtt_class *vtt=(vtt_class *) sp->vtt;
	vtt->set_y_input_parameter(sp);
}

gboolean vg_delete_pitch_adjust (GtkWidget *wid, vtt_class *vtt) {
	vtt->gui.adjust_dialog=NULL;
	return FALSE;
}

void vg_do_pitch_adjust (GtkWidget *wid, vtt_class *vtt) {
	int master_cycles=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(vtt->gui.adjust_dialog, "master_cycles")));
	int cycles=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(vtt->gui.adjust_dialog, "cycles")));
	bool create_event=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(vtt->gui.adjust_dialog, "create_event")));

	vtt->adjust_to_master_pitch(master_cycles, cycles, create_event);
	
	gtk_widget_destroy(vtt->gui.adjust_dialog);
}

void vg_cancel_pitch_adjust (GtkWidget *wid, vtt_class *vtt) {
	gtk_widget_destroy(vtt->gui.adjust_dialog);
}

void vg_adjust_pitch_vtt(GtkWidget *wid, vtt_class *vtt) {
	if (vtt->gui.adjust_dialog) {
		gtk_widget_destroy(vtt->gui.adjust_dialog);
		return;
	}
	
	if (!vtt_class::sync_master) {
		tx_note("No master turntable to adjust pitch to selected.", true);
		return;
	}
	
	if (vtt==vtt_class::sync_master) {
		tx_note("This is the master turntable - cannot adjust a turntable to itself.", true);
		return;
	}
	
	vtt->gui.adjust_dialog=create_tx_adjust();
	tX_set_icon(vtt->gui.adjust_dialog);
	gtk_widget_show(vtt->gui.adjust_dialog);
	
	GtkWidget *ok_button=lookup_widget(vtt->gui.adjust_dialog, "ok");
	GtkWidget *cancel_button=lookup_widget(vtt->gui.adjust_dialog, "cancel");
	
	g_signal_connect(G_OBJECT(ok_button), "clicked", G_CALLBACK(vg_do_pitch_adjust), vtt);
	g_signal_connect(G_OBJECT(vtt->gui.adjust_dialog), "destroy", G_CALLBACK(vg_delete_pitch_adjust), vtt);
	g_signal_connect(G_OBJECT(cancel_button), "clicked", G_CALLBACK(vg_cancel_pitch_adjust), vtt);
}

static gint vg_mouse_mapping_pressed(GtkWidget *wid, GdkEventButton *event, vtt_class *vtt) {
	if (vtt->gui.mouse_mapping_menu) {
		gtk_widget_destroy(vtt->gui.mouse_mapping_menu);
		vtt->gui.mouse_mapping_menu=NULL;
	}
	/* gtk+ seems to cleanup the submenus automatically */
	
	vtt->gui.mouse_mapping_menu=gtk_menu_new();
	GtkWidget *x_item;
	GtkWidget *y_item;
	
	x_item=gtk_menu_item_new_with_label("X-axis (Left <-> Right)");
	gtk_menu_shell_append(GTK_MENU_SHELL(vtt->gui.mouse_mapping_menu), x_item);
	gtk_widget_show(x_item);

	y_item=gtk_menu_item_new_with_label("Y-axis (Up <-> Down)");
	gtk_menu_shell_append(GTK_MENU_SHELL(vtt->gui.mouse_mapping_menu), y_item);
	gtk_widget_show(y_item);
	
	vtt->gui.mouse_mapping_menu_x=gtk_menu_new();
	vtt->gui.mouse_mapping_menu_y=gtk_menu_new();
	
	GtkWidget *item;
	GtkWidget *item_to_activate=NULL;
	
	/* Filling the X menu */
	item = gtk_check_menu_item_new_with_label("Disable");
	gtk_menu_shell_append(GTK_MENU_SHELL(vtt->gui.mouse_mapping_menu_x), item);
	gtk_widget_show(item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(vg_xcontrol_dis), vtt);
	if (vtt->x_par==NULL) item_to_activate=item;

	list <tX_seqpar *> :: iterator sp;

	for (sp=tX_seqpar::all.begin(); sp!=tX_seqpar::all.end(); sp++) {
		if (((*sp)->is_mappable) && ((*sp)->vtt) == (void*) vtt) {
			item=gtk_check_menu_item_new_with_label((*sp)->get_name());
			gtk_menu_shell_append(GTK_MENU_SHELL(vtt->gui.mouse_mapping_menu_x), item);
			gtk_widget_show(item);
			g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(vg_xcontrol_set), (void*) (*sp));
			
			if (vtt->x_par==(*sp)) item_to_activate=item;
		}
	}

	if (item_to_activate) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item_to_activate), TRUE);
	
	/* Filling the Y menu */
	item_to_activate=NULL;
	
	item = gtk_check_menu_item_new_with_label("Disable");
	gtk_menu_shell_append(GTK_MENU_SHELL(vtt->gui.mouse_mapping_menu_y), item);
	gtk_widget_show(item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(vg_ycontrol_dis), vtt);
	if (vtt->y_par==NULL) item_to_activate=item;

	for (sp=tX_seqpar::all.begin(); sp!=tX_seqpar::all.end(); sp++) {
		if (((*sp)->is_mappable) && ((*sp)->vtt) == (void*) vtt) {
			item=gtk_check_menu_item_new_with_label((*sp)->get_name());
			gtk_menu_shell_append(GTK_MENU_SHELL(vtt->gui.mouse_mapping_menu_y), item);
			gtk_widget_show(item);
			g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(vg_ycontrol_set), (void*) (*sp));
			
			if (vtt->y_par==(*sp)) item_to_activate=item;
		}
	}

	if (item_to_activate) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item_to_activate), TRUE);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(x_item), vtt->gui.mouse_mapping_menu_x);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(y_item), vtt->gui.mouse_mapping_menu_y);
	gtk_menu_popup (GTK_MENU(vtt->gui.mouse_mapping_menu), NULL, NULL, NULL, NULL, 0, 0);
	
	g_signal_emit_by_name(G_OBJECT(wid), "released", vtt);
	
	return TRUE;
}

static gint vg_file_button_pressed(GtkWidget *wid, GdkEventButton *event, vtt_class *vtt) {
	if (vtt->gui.file_menu==NULL) {
		GtkWidget *item;
		
		vtt->gui.file_menu=gtk_menu_new();
		item=gtk_menu_item_new_with_label("Load audio file");
		gtk_menu_shell_append(GTK_MENU_SHELL(vtt->gui.file_menu), item);
		gtk_widget_show(item);
		
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(load_file), vtt);
		
		item=gtk_menu_item_new_with_label("Edit audio file");
		gtk_menu_shell_append(GTK_MENU_SHELL(vtt->gui.file_menu), item);
		gtk_widget_show(item);

		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(edit_vtt_buffer), vtt);

		item=gtk_menu_item_new_with_label("Reload current audio file");
		gtk_menu_shell_append(GTK_MENU_SHELL(vtt->gui.file_menu), item);
		gtk_widget_show(item);

		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(reload_vtt_buffer), vtt);
	}
	
	gtk_menu_popup(GTK_MENU(vtt->gui.file_menu), NULL, NULL, NULL, NULL, 0,0);
	/* gtk+ is really waiting for this.. */
	g_signal_emit_by_name(G_OBJECT(wid), "released", vtt);
	
	return TRUE;
}

void vg_adjust_zoom(GtkWidget *wid, vtt_class *vtt) {	
	GtkAdjustment *adj=gtk_range_get_adjustment(GTK_RANGE(vtt->gui.zoom));
	gtk_tx_set_zoom(GTK_TX(vtt->gui.display), adj->value/100.0);
}

static gint fx_button_pressed(GtkWidget *wid, GdkEventButton *event, vtt_class *vtt)
{
	vtt_gui *g=&vtt->gui;

	LADSPA_Class::set_current_vtt(vtt);

	if (g->ladspa_menu) gtk_object_destroy(GTK_OBJECT(g->ladspa_menu));
	g->ladspa_menu=LADSPA_Class::get_ladspa_menu();
	gtk_menu_popup (GTK_MENU(g->ladspa_menu), NULL, NULL, NULL, NULL, 0, 0);

	/* gtk+ is really waiting for this.. */
	g_signal_emit_by_name(G_OBJECT(wid), "released", vtt);
	
	return TRUE;
}

static gint stereo_fx_button_pressed(GtkWidget *wid, GdkEventButton *event, vtt_class *vtt)
{
	vtt_gui *g=&vtt->gui;

	LADSPA_Class::set_current_vtt(vtt);

	if (g->ladspa_menu) gtk_object_destroy(GTK_OBJECT(g->ladspa_menu));
	g->ladspa_menu=LADSPA_Class::get_stereo_ladspa_menu();
	gtk_menu_popup (GTK_MENU(g->ladspa_menu), NULL, NULL, NULL, NULL, 0, 0);

	/* gtk+ is really waiting for this.. */
	g_signal_emit_by_name(G_OBJECT(wid), "released", vtt);
	
	return TRUE;
}

#define TOOLTIP_LENGTH 2048

void gui_set_name(vtt_class *vtt, char *newname)
{
	char bold_name[128];
	strcpy(bold_name, "<b>");
	strcat(bold_name, newname);
	strcat(bold_name, "</b>");
	
	gtk_label_set_markup(GTK_LABEL(vtt->gui.audio_label), bold_name);
	gtk_label_set_markup(GTK_LABEL(vtt->gui.control_label), bold_name);
	gtk_entry_set_text(GTK_ENTRY(vtt->gui.name), newname);
	char tooltip[2048];
	
	if (vtt->gui.audio_minimized_panel_bar_button!=NULL) {
		gtk_label_set_text(GTK_LABEL(vtt->gui.audio_minimized_panel_bar_label), newname);
		snprintf(tooltip, TOOLTIP_LENGTH, "Show \"%s\" audio panel.", newname);
		gui_set_tooltip(vtt->gui.audio_minimized_panel_bar_button, tooltip);
	}

	if (vtt->gui.control_minimized_panel_bar_button!=NULL) {
		gtk_label_set_text(GTK_LABEL(vtt->gui.control_minimized_panel_bar_label), newname);
		snprintf(tooltip, TOOLTIP_LENGTH, "Show \"%s\" control panel.", newname);
		gui_set_tooltip(vtt->gui.control_minimized_panel_bar_button, tooltip);
	}
}

#define connect_entry(wid, func); g_signal_connect(G_OBJECT(g->wid), "activate", G_CALLBACK(func), (void *) vtt);
#define connect_adj(wid, func); g_signal_connect(G_OBJECT(g->wid), "value_changed", G_CALLBACK(func), (void *) vtt);
#define connect_button(wid, func); g_signal_connect(G_OBJECT(g->wid), "clicked", G_CALLBACK(func), (void *) vtt);
#define connect_range(wid, func); g_signal_connect(G_OBJECT(gtk_range_get_adjustment(GTK_RANGE(g->wid))), "value_changed", G_CALLBACK(func), (void *) vtt);
#define connect_scale_format(wid, func); g_signal_connect(G_OBJECT(g->wid), "format-value", G_CALLBACK(func), (void *) vtt);
#define connect_press_button(wid, func); g_signal_connect(G_OBJECT(g->wid), "button_press_event", G_CALLBACK(func), (void *) vtt);
#define connect_rel_button(wid, func); g_signal_connect(G_OBJECT(g->wid), "released", G_CALLBACK(func), (void *) vtt);

GtkWidget *vg_create_fx_bar(vtt_class *vtt, vtt_fx *effect, int showdel);

gchar dnd_uri[128];

void gui_connect_signals(vtt_class *vtt)
{
	vtt_gui *g=&vtt->gui;
	strncpy(dnd_uri, "text/uri-list", 128);

	connect_entry(name, name_changed);
	connect_adj(volume, volume_changed);
	connect_adj(pitch, pitch_changed);
	connect_adj(pan, pan_changed);
	connect_press_button(file, vg_file_button_pressed);
	
	connect_button(del, delete_vtt);
	connect_button(trigger, trigger_vtt);
	connect_button(stop, stop_vtt);
	connect_button(autotrigger, autotrigger_toggled);
	connect_button(loop, loop_toggled);
	connect_button(sync_master, master_setup);
	connect_button(sync_client, client_setup);
	connect_button(adjust_button, vg_adjust_pitch_vtt);
	connect_adj(cycles, client_setup_number);
	connect_press_button(fx_button, fx_button_pressed);
	connect_press_button(stereo_fx_button, stereo_fx_button_pressed);
	
	connect_button(lp_enable, lp_enabled);
	connect_adj(lp_gain, lp_gain_changed);
	connect_adj(lp_reso, lp_reso_changed);
	connect_adj(lp_freq, lp_freq_changed);
	
	connect_button(ec_enable, ec_enabled);
#ifdef USE_ALSA_MIDI_IN	
	connect_button(midi_mapping, midi_mapping_clicked);
#endif	
	connect_adj(ec_length, ec_length_changed);
	connect_adj(ec_feedback, ec_feedback_changed);
	connect_adj(ec_pan, ec_pan_changed);
	connect_adj(ec_volume, ec_volume_changed);	
	connect_range(zoom, vg_adjust_zoom);
	//connect_scale_format(zoom, vg_format_zoom);
	connect_press_button(mouse_mapping, vg_mouse_mapping_pressed);
	connect_button(control_minimize, minimize_control_panel);
	connect_button(audio_minimize, minimize_audio_panel);

	static GtkTargetEntry drop_types [] = {
		{ dnd_uri, 0, 0}
	};
	static gint n_drop_types = sizeof (drop_types) / sizeof(drop_types[0]);
	
	gtk_drag_dest_set (GTK_WIDGET (g->file), (GtkDestDefaults) (GTK_DEST_DEFAULT_MOTION |GTK_DEST_DEFAULT_HIGHLIGHT |GTK_DEST_DEFAULT_DROP),
			drop_types, n_drop_types,
			GDK_ACTION_COPY);
						
	g_signal_connect (G_OBJECT (g->file), "drag_data_received",
			G_CALLBACK(drop_file), (void *) vtt);

	gtk_drag_dest_set (GTK_WIDGET (g->display), (GtkDestDefaults) (GTK_DEST_DEFAULT_MOTION |GTK_DEST_DEFAULT_HIGHLIGHT |GTK_DEST_DEFAULT_DROP),
			drop_types, n_drop_types,
			GDK_ACTION_COPY);
						
	g_signal_connect (G_OBJECT (g->display), "drag_data_received",
			G_CALLBACK(drop_file), (void *) vtt);
	
}

void build_vtt_gui(vtt_class *vtt)
{
	GtkWidget *tempbox;
	GtkWidget *tempbox2;
	GtkWidget *tempbox3;
	GtkWidget *dummy;
	char nice_name[256];
	
	vtt_gui *g;
	
	g=&vtt->gui;
	vtt->have_gui=1;
	g->par_menu=NULL;
	g->ladspa_menu=NULL;

	/* Building Audio Box */
	g->audio_box=gtk_vbox_new(FALSE,2);
	gtk_widget_show(g->audio_box);
	
	tempbox2=gtk_hbox_new(FALSE,2);
	gtk_widget_show(tempbox2);
	gtk_box_pack_start(GTK_BOX(g->audio_box), tempbox2, WID_FIX);
	
	tempbox=gtk_hbox_new(TRUE,2);
	gtk_widget_show(tempbox);
	gtk_box_pack_start(GTK_BOX(tempbox2), tempbox, WID_DYN);

	GtkWidget *pixmap;
	g->audio_minimize=gtk_button_new();
	pixmap=tx_pixmap_widget(MINIMIZE_PANEL);
	gtk_container_add (GTK_CONTAINER (g->audio_minimize), pixmap);	
	gtk_box_pack_end(GTK_BOX(tempbox2), g->audio_minimize, WID_FIX);
	gtk_widget_show(pixmap);
	gtk_widget_show(g->audio_minimize);


	g->audio_label=gtk_label_new(vtt->name);
	gtk_misc_set_alignment(GTK_MISC(g->audio_label), 0.025, 0.5);
	gtk_widget_show(g->audio_label);
	gtk_box_pack_start(GTK_BOX(tempbox), g->audio_label, WID_DYN);

	nicer_filename(nice_name, vtt->filename);
	g->file = gtk_button_new_with_label(nice_name);
	gtk_widget_show(g->file);
	gui_set_tooltip(g->file, "Click to Load/Edit/Reload a sample for this turntable. To load you can also drag a file and drop it over this button or the sound data display below.");
	gtk_box_pack_start(GTK_BOX(tempbox), g->file, WID_DYN);

	g->mouse_mapping=gtk_button_new_with_label("Mouse");
	gtk_widget_show(g->mouse_mapping);
	gui_set_tooltip(g->mouse_mapping, "Determines what parameters should be affected on mouse moition in mouse grab mode.");
	gtk_box_pack_start(GTK_BOX(tempbox2), g->mouse_mapping, WID_FIX);

#ifdef USE_ALSA_MIDI_IN
	g->midi_mapping=gtk_button_new_with_label("MIDI");
	gtk_widget_show(g->midi_mapping);
	gui_set_tooltip(g->midi_mapping, "Determines what parameters should be bound to what MIDI events.");
	gtk_box_pack_start(GTK_BOX(tempbox2), g->midi_mapping, WID_FIX);
	
	if (!tX_engine::get_instance()->get_midi()->get_is_open()) {
		gtk_widget_set_sensitive(g->midi_mapping, FALSE);
	}
#endif

	tempbox=gtk_hbox_new(FALSE, 2);

	g->display=gtk_tx_new(vtt->buffer, vtt->samples_in_buffer);
	gtk_box_pack_start(GTK_BOX(tempbox), g->display, WID_DYN);
	gtk_widget_show(g->display);	
	
	g->zoom=gtk_vscale_new_with_range(0,99.0,1.0);
	gtk_range_set_inverted(GTK_RANGE(g->zoom), TRUE);
	gtk_scale_set_draw_value(GTK_SCALE(g->zoom), TRUE);
	gtk_scale_set_digits(GTK_SCALE(g->zoom), 0);
	gtk_scale_set_value_pos(GTK_SCALE(g->zoom), GTK_POS_BOTTOM);
	gtk_adjustment_set_value(gtk_range_get_adjustment(GTK_RANGE(g->zoom)), 0);
	
	gui_set_tooltip(g->zoom, "Set the zoom-level for the audio data display.");
	gtk_box_pack_start(GTK_BOX(tempbox), g->zoom, WID_FIX);
	gtk_widget_show(g->zoom);
	
	gtk_box_pack_start(GTK_BOX(g->audio_box), tempbox, WID_DYN);
	gtk_widget_show(tempbox);
	
	/* Building Control Box */
	
	g->control_box=gtk_vbox_new(FALSE,2);
	gtk_widget_show(g->control_box);

	tempbox2=gtk_hbox_new(FALSE, 2);
	gtk_widget_show(tempbox2);
	gtk_box_pack_start(GTK_BOX(g->control_box), tempbox2, WID_FIX);

	g->control_label=gtk_label_new(vtt->name);
	gtk_widget_show(g->control_label);
	gtk_box_pack_start(GTK_BOX(tempbox2), g->control_label, WID_DYN);

	g->control_minimize=gtk_button_new();
	pixmap=tx_pixmap_widget(MINIMIZE_PANEL);
	gtk_container_add (GTK_CONTAINER (g->control_minimize), pixmap);	
	gtk_box_pack_end(GTK_BOX(tempbox2), g->control_minimize, WID_FIX);
	gtk_widget_show(pixmap);
	gtk_widget_show(g->control_minimize);

	g->scrolled_win=gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (g->scrolled_win), 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (g->scrolled_win),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_widget_show(g->scrolled_win);
	gtk_box_pack_start(GTK_BOX(g->control_box), g->scrolled_win, WID_DYN);
				    
	g->control_subbox=gtk_vbox_new(FALSE,0);
	gtk_scrolled_window_add_with_viewport (
                   GTK_SCROLLED_WINDOW (g->scrolled_win), g->control_subbox);
	gtk_widget_show(g->control_subbox);
		   

	/* Main panel */
	
	tX_panel *p=new tX_panel("Main", g->control_subbox);
	g->main_panel=p;
			
	g->name = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(g->name), 256);
	gtk_entry_set_text(GTK_ENTRY(g->name), vtt->name);
	p->add_client_widget(g->name);
	gui_set_tooltip(g->name, "Enter the turntable's name here.");
	gtk_widget_set_size_request(g->name, 40, -1);

	g->del=gtk_button_new_with_label("Delete");
	gui_set_tooltip(g->del, "Click here to annihilate this turntable. All events recorded for this turntable will be erased, too.");
	p->add_client_widget(g->del);
	
	g->adjust_button=gtk_button_new_with_label("Pitch Adj.");
	gui_set_tooltip(g->adjust_button, "Activate this button to adjust this turntable's speed to the master turntable's speed.");
	p->add_client_widget(g->adjust_button);

	gtk_box_pack_start(GTK_BOX(g->control_subbox), p->get_widget(), WID_FIX);
				
	p=new tX_panel("Playback", g->control_subbox);
	g->trigger_panel=p;
	
	g->trigger=gtk_button_new_with_label("Trigger!");
	gui_set_tooltip(g->trigger, "Click here to trigger this turntable right now. If the audio engine is disabled this turntable will be triggerd as soon as the engine is turned on.");
	p->add_client_widget(g->trigger);
	
	g->stop=gtk_button_new_with_label("Stop.");
	gui_set_tooltip(g->stop, "Stop this turntable's playback.");
	p->add_client_widget(g->stop);
	g_signal_connect(G_OBJECT(g->trigger), "button_press_event", (GtkSignalFunc) tX_seqpar::tX_seqpar_press, &vtt->sp_trigger);		
	g_signal_connect(G_OBJECT(g->stop), "button_press_event", (GtkSignalFunc) tX_seqpar::tX_seqpar_press, &vtt->sp_trigger);		
	
	g->autotrigger=gtk_check_button_new_with_label("Auto");
	p->add_client_widget(g->autotrigger);
	gui_set_tooltip(g->autotrigger, "If turned on, this turntable will be automagically triggered whenever the audio engine is turned on.");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->autotrigger), vtt->autotrigger);

	g->loop=gtk_check_button_new_with_label("Loop");
	p->add_client_widget(g->loop);
	gui_set_tooltip(g->loop, "Enable this option to make the turntable loop the audio data.");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->loop), vtt->loop);
	g_signal_connect(G_OBJECT(g->loop), "button_press_event", (GtkSignalFunc) tX_seqpar::tX_seqpar_press, &vtt->sp_loop);		
	
	g->sync_master=gtk_check_button_new_with_label("Master");
	p->add_client_widget(g->sync_master);
	gui_set_tooltip(g->sync_master, "Click here to make this turntable the sync-master. All turntables marked as sync-clients will be (re-)triggered in relation to the sync-master. Note that only *one* turntable can be the sync-master.");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->sync_master), vtt->is_sync_master);
	
	g->sync_client=gtk_check_button_new_with_label("Client");
	p->add_client_widget(g->sync_client);
	gui_set_tooltip(g->sync_client, "If enabled this turntable will be (re-)triggerd in relation to the sync-master turntable.");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->sync_client), vtt->is_sync_client);
	g_signal_connect(G_OBJECT(g->sync_client), "button_press_event", (GtkSignalFunc) tX_seqpar::tX_seqpar_press, &vtt->sp_sync_client);	
	
	g->cycles=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->sync_cycles, 0, 10.0, 1,1,0));
	dummy=gtk_spin_button_new(g->cycles, 1.0, 0);
	p->add_client_widget(dummy);
	gui_set_tooltip(dummy, "Determines how often a sync-client turntable gets triggered. 0 -> this turntable will be triggered with every trigger of the sync-master table, 1 -> the table will be triggered every 2nd master trigger and so on.");
	g_signal_connect(G_OBJECT(dummy), "button_press_event", (GtkSignalFunc) tX_seqpar::tX_seqpar_press, &vtt->sp_sync_cycles);	

	gtk_box_pack_start(GTK_BOX(g->control_subbox), p->get_widget(), WID_FIX);
	
	g->fx_box=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(g->control_subbox), g->fx_box, WID_FIX);
	gtk_widget_show(g->fx_box);
	
	dummy=gtk_button_new_with_label("FX");
	gtk_container_foreach(GTK_CONTAINER(dummy), (GtkCallback) tX_panel_make_label_bold, NULL);
	gtk_widget_show(dummy);
	g->fx_button=dummy;
	gui_set_tooltip(g->fx_button, "Click here to load a LADSPA plugin. You will get a menu from which you can choose which plugin to load.");
	gtk_box_pack_start(GTK_BOX(g->fx_box), dummy, WID_FIX);
	
	/* Lowpass Panel */

	p=new tX_panel("Lowpass", g->fx_box);
	g_signal_connect(G_OBJECT(p->get_labelbutton()), "button_press_event", G_CALLBACK(vg_show_fx_menu), vtt->lp_fx);
	g->lp_panel=p;
		
	g->lp_enable=gtk_check_button_new_with_label("Enable");
	gui_set_tooltip(g->lp_enable, "Click here to enable the built-in lowpass effect.");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->lp_enable), vtt->lp_enable);
	p->add_client_widget(vg_create_fx_bar(vtt, vtt->lp_fx, 0));
	g_signal_connect(G_OBJECT(g->lp_enable), "button_press_event", (GtkSignalFunc) tX_seqpar::tX_seqpar_press, &vtt->sp_lp_enable);	

	p->add_client_widget(g->lp_enable);

	g->lp_gain=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->lp_gain, 0, 2, 0.1, 0.01, 0.01));
	g->lp_reso=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->lp_reso, 0, 0.99, 0.1, 0.01, 0.01));
	g->lp_freq=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->lp_freq, 0, 1, 0.1, 0.01, 0.01));

	g->lp_gaind=new tX_extdial("Input Gain", g->lp_gain, &vtt->sp_lp_gain);
	p->add_client_widget(g->lp_gaind->get_widget());
	gui_set_tooltip(g->lp_gaind->get_entry(), "Adjust the input gain. with this parameter you can either amplify or damp the input-signal for the lowpass effect.");

	g->lp_freqd=new tX_extdial("Frequency", g->lp_freq, &vtt->sp_lp_freq);
	p->add_client_widget(g->lp_freqd->get_widget());
	gui_set_tooltip(g->lp_freqd->get_entry(), "Adjust the cutoff frequency of the lowpass filter. 0 is 0 Hz, 1 is 22.1 kHz.");

	g->lp_resod=new tX_extdial("Resonance", g->lp_reso, &vtt->sp_lp_reso);
	p->add_client_widget(g->lp_resod->get_widget());
	gui_set_tooltip(g->lp_resod->get_entry(), "Adjust the resonance of the lowpass filter. This value determines how much the signal at the cutoff frequency will be amplified.");

	gtk_box_pack_start(GTK_BOX(g->fx_box), p->get_widget(), WID_FIX);

	/* Echo Panel */

	p=new tX_panel("Echo", g->fx_box);
	g_signal_connect(G_OBJECT(p->get_labelbutton()), "button_press_event",  G_CALLBACK(vg_show_fx_menu), vtt->ec_fx);
	g->ec_panel=p;

	p->add_client_widget(vg_create_fx_bar(vtt, vtt->ec_fx, 0));
	
	g->ec_enable=gtk_check_button_new_with_label("Enable");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->ec_enable), vtt->ec_enable);
	p->add_client_widget(g->ec_enable);
	gui_set_tooltip(g->ec_enable, "Enable the built-in echo effect.");
	g_signal_connect(G_OBJECT(g->ec_enable), "button_press_event", (GtkSignalFunc) tX_seqpar::tX_seqpar_press, &vtt->sp_ec_enable);	

	g->ec_length=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->ec_length, 0, 1, 0.1, 0.01, 0.001));
	g->ec_feedback=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->ec_feedback, 0, 1, 0.1, 0.01, 0.001));
	g->ec_pan=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->ec_pan, -1.0, 1, 0.1, 0.01, 0.001));
	g->ec_volume=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->ec_volume, 0.0, 3.0, 0.1, 0.01, 0.001));

	g->ec_lengthd=new tX_extdial("Duration", g->ec_length, &vtt->sp_ec_length);
	p->add_client_widget(g->ec_lengthd->get_widget());
	gui_set_tooltip(g->ec_lengthd->get_entry(), "Adjust the length of the echo buffer.");

	g->ec_feedbackd=new tX_extdial("Feedback", g->ec_feedback, &vtt->sp_ec_feedback);
	p->add_client_widget(g->ec_feedbackd->get_widget());
	gui_set_tooltip(g->ec_feedbackd->get_entry(), "Adjust the feedback of the echo effect. Note that a value of 1 will result in a constant signal.");

	g->ec_volumed=new tX_extdial("Volume", g->ec_volume, &vtt->sp_ec_volume);
	p->add_client_widget(g->ec_volumed->get_widget());
	gui_set_tooltip(g->ec_volumed->get_entry(), "Adjust the volume of the echo effect.");

	g->ec_pand=new tX_extdial("Pan", g->ec_pan, &vtt->sp_ec_pan);
	p->add_client_widget(g->ec_pand->get_widget());
	gui_set_tooltip(g->ec_pand->get_entry(), "Adjust the panning of the echo effect.");

	gtk_box_pack_start(GTK_BOX(g->fx_box), p->get_widget(), WID_FIX);
	
	g->stereo_fx_box=gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(g->control_subbox), g->stereo_fx_box, WID_FIX);
	gtk_widget_show(g->stereo_fx_box);
	
	dummy=gtk_button_new_with_label("Stereo FX");
	gtk_container_foreach(GTK_CONTAINER(dummy), (GtkCallback) tX_panel_make_label_bold, NULL);
	gtk_widget_show(dummy);
	g->stereo_fx_button=dummy;
	gui_set_tooltip(g->stereo_fx_button, "Click here to load a stereo LADSPA plugin. You will get a menu from which you can choose which plugin to load.");
	gtk_box_pack_start(GTK_BOX(g->stereo_fx_box), dummy, WID_FIX);

	/* Output */
	
	tempbox=gtk_hbox_new(FALSE,2);
	gtk_widget_show(tempbox);
	gtk_box_pack_end(GTK_BOX(g->control_box), tempbox, WID_FIX);
	
	tempbox2=gtk_vbox_new(FALSE,0);
	gtk_widget_show(tempbox2);
	gtk_box_pack_start(GTK_BOX(tempbox), tempbox2, WID_FIX);
	
	g->pitch=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->rel_pitch, -3, +3, 0.1, 0.01, 0.001));
	g->pan=GTK_ADJUSTMENT(gtk_adjustment_new(0, -1, 1, 0.1, 0.01, 0.001));

	g->pitchd=new tX_extdial("Pitch", g->pitch, &vtt->sp_pitch);
	gui_set_tooltip(g->pitchd->get_entry(), "Adjust this turntable's pitch.");

	gtk_box_pack_start(GTK_BOX(tempbox2), g->pitchd->get_widget(), WID_FIX);

	g->pand=new tX_extdial("Pan", g->pan, &vtt->sp_pan);
	gtk_box_pack_start(GTK_BOX(tempbox2), g->pand->get_widget(), WID_FIX);
	gui_set_tooltip(g->pand->get_entry(), "Specifies the position of this turntable within the stereo spectrum: -1 -> left, 0-> center, 1->right.");

	tempbox3=gtk_hbox_new(FALSE,2);
	gtk_widget_show(tempbox3);

	g->mute=gtk_check_button_new_with_label("M");
	gtk_box_pack_start(GTK_BOX(tempbox3), g->mute, WID_FIX);
	g_signal_connect(G_OBJECT(g->mute),"clicked", (GtkSignalFunc) mute_volume, vtt);
	gtk_widget_show(g->mute);
	gui_set_tooltip(g->mute, "Mute this turntable's mixer output.");

	g->solo=gtk_check_button_new_with_label("S");
	gtk_box_pack_start(GTK_BOX(tempbox3), g->solo, WID_FIX);
	g_signal_connect(G_OBJECT(g->solo),"clicked", (GtkSignalFunc) solo_vtt, vtt);
	gtk_widget_show(g->solo);
	gui_set_tooltip(g->solo, "Allow only this and other solo-switched turntabels' signal to be routed to the mixer.");

	gtk_box_pack_start(GTK_BOX(tempbox2), tempbox3, WID_FIX);

	tempbox2=gtk_hbox_new(FALSE,0);
	gtk_widget_show(tempbox2);
	gtk_box_pack_start(GTK_BOX(tempbox), tempbox2, WID_FIX);
	
	g->volume=GTK_ADJUSTMENT(gtk_adjustment_new(2.0-vtt->rel_volume, 0, 2, 0.01, 0.01, 0.01));
	dummy=gtk_vscale_new(GTK_ADJUSTMENT(g->volume)); 
	gtk_scale_set_draw_value(GTK_SCALE(dummy), False);
	gui_set_tooltip(dummy, "Adjust this turntable's volume.");
	g_signal_connect(G_OBJECT(dummy), "button_press_event", (GtkSignalFunc) tX_seqpar::tX_seqpar_press, &vtt->sp_volume);	

	gtk_box_pack_start(GTK_BOX(tempbox2), dummy, WID_FIX);
	gtk_widget_show(dummy);

	g->flash=gtk_tx_flash_new();
	gtk_box_pack_start(GTK_BOX(tempbox2), g->flash, WID_FIX);
	gtk_widget_show(g->flash);		

#ifndef USE_FILECHOOSER
	g->file_dialog=NULL;
#endif
	gui_connect_signals(vtt);
	
	g->audio_minimized_panel_bar_button=NULL;
	g->control_minimized_panel_bar_button=NULL;
	g->file_menu=NULL;
	g->mouse_mapping_menu=NULL;
	g->mouse_mapping_menu_x=NULL;
	g->mouse_mapping_menu_y=NULL;
	
	g->adjust_dialog=NULL;
	
	gui_set_name(vtt, vtt->name);	
}

void fx_up(GtkWidget *wid, vtt_fx *effect)
{
	vtt_class *vtt;
	
	vtt=(vtt_class*)effect->get_vtt();
	vtt->effect_up(effect);
}

void fx_down(GtkWidget *wid, vtt_fx *effect)
{
	vtt_class *vtt;
	
	vtt=(vtt_class*)effect->get_vtt();
	vtt->effect_down(effect);
}

void fx_kill(GtkWidget *wid, vtt_fx_ladspa *effect)
{
	vtt_class *vtt;
	
	vtt=(vtt_class*)effect->get_vtt();
	vtt->effect_remove(effect);
}

GtkWidget *vg_create_fx_bar(vtt_class *vtt, vtt_fx *effect, int showdel)
{
	GtkWidget *box;
	GtkWidget *pixmap;
	GtkWidget *button;
	
	box=gtk_hbox_new(FALSE,0);

	if (showdel) {
		button=gtk_button_new();
		pixmap=tx_pixmap_widget(FX_CLOSE);
		gtk_container_add (GTK_CONTAINER (button), pixmap);	
		gtk_box_pack_end(GTK_BOX(box), button, WID_FIX);
		gtk_widget_show(pixmap);
		gtk_widget_show(button);
		g_signal_connect(G_OBJECT(button), "clicked", (GtkSignalFunc) fx_kill, (void *) effect);
	}

	button=gtk_button_new();
	pixmap=tx_pixmap_widget(FX_DOWN);
	gtk_container_add (GTK_CONTAINER (button), pixmap);	
	gtk_box_pack_end(GTK_BOX(box), button, WID_FIX);
	gtk_widget_show(pixmap);
	gtk_widget_show(button);
	g_signal_connect(G_OBJECT(button), "clicked", (GtkSignalFunc) fx_down, (void *) effect);

	button=gtk_button_new();
	pixmap=tx_pixmap_widget(FX_UP);
	gtk_container_add (GTK_CONTAINER (button), pixmap);	
	gtk_box_pack_end(GTK_BOX(box), button, WID_FIX);
	gtk_widget_show(pixmap);
	gtk_widget_show(button);
	g_signal_connect(G_OBJECT(button), "clicked", (GtkSignalFunc) fx_up, (void *) effect);
	
	gtk_widget_show(box);
	
	return box;
}

int gtk_box_get_widget_pos(GtkBox *box, GtkWidget *child)
{
	int i=0;
	GList *list;
 
	list = box->children;
	while (list)
	{
		GtkBoxChild *child_info;
		child_info = (GtkBoxChild *) list->data;
		if (child_info->widget == child)
		break;
		list = list->next; i++;
    	}
	return i;
}

void vg_move_fx_panel_up(GtkWidget *wid, vtt_class *vtt, bool stereo)
{
	GtkWidget *box=(stereo ? vtt->gui.stereo_fx_box : vtt->gui.fx_box);
	int pos=gtk_box_get_widget_pos(GTK_BOX(box), wid);
	gtk_box_reorder_child(GTK_BOX(box), wid, pos-1);
}

void vg_move_fx_panel_down(GtkWidget *wid, vtt_class *vtt, bool stereo)
{
	GtkWidget *box=(stereo ? vtt->gui.stereo_fx_box : vtt->gui.fx_box);
	int pos=gtk_box_get_widget_pos(GTK_BOX(box), wid);
	gtk_box_reorder_child(GTK_BOX(box), wid, pos+1);
}

static gint vg_show_fx_info(GtkWidget *wid, vtt_fx *effect)
{
	tx_l_note(effect->get_info_string());
	return TRUE;
}

void vg_toggle_drywet(GtkWidget *wid, vtt_fx *effect)
{
	effect->toggle_drywet();
}

static gint vg_show_fx_menu(GtkWidget *wid, GdkEventButton *event, vtt_fx *effect)
{
	if (event->button==3) {
		GtkWidget *menu=gtk_menu_new();
		GtkWidget *item=gtk_menu_item_new_with_label("View Plugin Details");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		gtk_widget_set_sensitive(item, (effect->has_drywet_feature()!=NOT_DRYWET_CAPABLE));
		gtk_widget_show(item);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(vg_show_fx_info), effect);
		
		switch (effect->has_drywet_feature()) {
			case (NOT_DRYWET_CAPABLE):
				item=gtk_menu_item_new_with_label("Add Dry/Wet Control");
				gtk_widget_set_sensitive(item, FALSE);
				break;
			case (DRYWET_ACTIVE):
				item=gtk_menu_item_new_with_label("Remove Dry/Wet Control");
				break;
			case (DRYWET_AVAILABLE):
				item=gtk_menu_item_new_with_label("Add Dry/Wet Control");
				break;
		}
		
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(vg_toggle_drywet), effect);	
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		gtk_widget_show(item);
		
		item = gtk_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		gtk_widget_set_sensitive(item, FALSE);
		gtk_widget_show(item);
	
		item=gtk_menu_item_new_with_label("Up");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		gtk_widget_show(item);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(fx_up), effect);
	
		item=gtk_menu_item_new_with_label("Down");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		gtk_widget_show(item);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(fx_down), effect);
	
		item=gtk_menu_item_new_with_label("Delete");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		gtk_widget_set_sensitive(item, (effect->has_drywet_feature()!=NOT_DRYWET_CAPABLE));
		gtk_widget_show(item);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(fx_kill), effect);
	
		gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, 0);
	
		/* gtk+ is really waiting for this.. */
		g_signal_emit_by_name(G_OBJECT(wid), "released", effect);
		return TRUE;
	}
	return FALSE;
}

void vg_create_fx_gui(vtt_class *vtt, vtt_fx_ladspa *effect, LADSPA_Plugin *plugin)
{
	char buffer[1024];
	
	vtt_gui *g;
	g=&vtt->gui;	
	tX_panel *p;
	list <tX_seqpar_vttfx *> :: iterator sp;
	
	strcpy(buffer, plugin->getLabel());
	if (strlen(buffer) > 6) {
		buffer[5]='.';
		buffer[6]='.';
		buffer[7]='.';
		buffer[8]=0;
	}

	p=new tX_panel(buffer, g->control_subbox);
	
	p->add_client_widget(vg_create_fx_bar(vtt, effect, 1));
	
	for (sp = effect->controls.begin(); sp != effect->controls.end(); sp++) {
			p->add_client_widget((*sp)->get_widget());
	}

	g_signal_connect(G_OBJECT(p->get_labelbutton()), "button_press_event", (GtkSignalFunc) vg_show_fx_menu, (void *) effect);
	gui_set_tooltip(p->get_labelbutton(), "Right-click to access menu.");
	effect->set_panel_widget(p->get_widget());
	effect->set_panel(p);

	gtk_box_pack_start(GTK_BOX((effect->is_stereo() ? g->stereo_fx_box : g->fx_box)), p->get_widget(), WID_FIX);
}

void gui_set_filename (vtt_class *vtt, char *newname)
{
        gtk_button_set_label(GTK_BUTTON(vtt->gui.file), newname);
}

void gui_update_display(vtt_class *vtt)
{
	nicer_filename(global_filename_buffer, vtt->filename);
	gtk_button_set_label(GTK_BUTTON(vtt->gui.file), global_filename_buffer);
	gtk_tx_set_data(GTK_TX(vtt->gui.display), vtt->buffer, vtt->samples_in_buffer);
}

void gui_hide_control_panel(vtt_class *vtt, bool hide) {
	char tooltip[2048];

	if (hide) {
		gtk_widget_hide(vtt->gui.control_box);
		vtt->gui.control_minimized_panel_bar_button=tx_xpm_button_new(MIN_CONTROL, vtt->name, 0, &vtt->gui.control_minimized_panel_bar_label);
		g_signal_connect(G_OBJECT(vtt->gui.control_minimized_panel_bar_button), "clicked", (GtkSignalFunc) unminimize_control_panel, vtt);
		gtk_widget_show(vtt->gui.control_minimized_panel_bar_button);
		snprintf(tooltip, TOOLTIP_LENGTH, "Show \"%s\" control panel.", vtt->name);
		gui_set_tooltip(vtt->gui.control_minimized_panel_bar_button, tooltip);
		add_to_panel_bar(vtt->gui.control_minimized_panel_bar_button);
	} else {
		gtk_widget_show(vtt->gui.control_box);
		remove_from_panel_bar(vtt->gui.control_minimized_panel_bar_button);
		if (!tX_shutdown) gtk_widget_destroy(vtt->gui.control_minimized_panel_bar_button);
		vtt->gui.control_minimized_panel_bar_button=NULL;
	}
}

void gui_hide_audio_panel(vtt_class *vtt, bool hide) {
	char tooltip[2048];

	if (hide) {
		gtk_widget_hide(vtt->gui.audio_box);
		vtt->gui.audio_minimized_panel_bar_button=tx_xpm_button_new(MIN_AUDIO, vtt->name, 0, &vtt->gui.audio_minimized_panel_bar_label);
		g_signal_connect(G_OBJECT(vtt->gui.audio_minimized_panel_bar_button), "clicked", (GtkSignalFunc) unminimize_audio_panel, vtt);		
		gtk_widget_show(vtt->gui.audio_minimized_panel_bar_button);
		snprintf(tooltip, TOOLTIP_LENGTH, "Show \"%s\" audio panel.", vtt->name);
		gui_set_tooltip(vtt->gui.audio_minimized_panel_bar_button, tooltip);
		add_to_panel_bar(vtt->gui.audio_minimized_panel_bar_button);
	} else {
		gtk_widget_show(vtt->gui.audio_box);
		remove_from_panel_bar(vtt->gui.audio_minimized_panel_bar_button);
		if (!tX_shutdown) gtk_widget_destroy(vtt->gui.audio_minimized_panel_bar_button);
		vtt->gui.audio_minimized_panel_bar_button=NULL;
	}
}

void delete_gui(vtt_class *vtt)
{
	if (vtt->gui.control_minimized_panel_bar_button!=NULL) gui_hide_control_panel(vtt, false);
	if (vtt->gui.audio_minimized_panel_bar_button!=NULL) gui_hide_audio_panel(vtt, false);

	delete vtt->gui.main_panel;
	delete vtt->gui.trigger_panel;
	
	delete vtt->gui.pitchd;
	delete vtt->gui.pand;
	
	delete vtt->gui.lp_gaind;
	delete vtt->gui.lp_resod;
	delete vtt->gui.lp_freqd;
	delete vtt->gui.lp_panel;
	
	delete vtt->gui.ec_lengthd;
	delete vtt->gui.ec_feedbackd;
	delete vtt->gui.ec_volumed;
	delete vtt->gui.ec_pand;
	delete vtt->gui.ec_panel;

	gtk_widget_destroy(vtt->gui.control_box);
	gtk_widget_destroy(vtt->gui.audio_box);
	if (vtt->gui.file_menu) gtk_widget_destroy(vtt->gui.file_menu);
	if (vtt->gui.mouse_mapping_menu) gtk_widget_destroy(vtt->gui.mouse_mapping_menu);
	if (vtt->gui.ladspa_menu) gtk_widget_destroy(vtt->gui.ladspa_menu);
}

void cleanup_vtt(vtt_class *vtt)
{
	gtk_tx_cleanup_pos_display(GTK_TX(vtt->gui.display));	
	gtk_tx_flash_set_level(vtt->gui.flash, 0.0, 0.0);
	gtk_tx_flash_clear(vtt->gui.flash);
	vtt->cleanup_required=false;
}

void update_all_vtts()
{
	list <vtt_class *> :: iterator vtt;
	f_prec temp,temp2;
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		if ((*vtt)->is_playing) {
			gtk_tx_update_pos_display(GTK_TX((*vtt)->gui.display), (*vtt)->pos_i, (*vtt)->mute);
			temp=(*vtt)->max_value*(*vtt)->res_volume*vtt_class::vol_channel_adjust;
			temp2=(*vtt)->max_value2*(*vtt)->res_volume*vtt_class::vol_channel_adjust;
//			tX_msg("Setting value: %f, %f -> %f; %f, %f -> %f (%f)\n",
//				(*vtt)->max_value, (*vtt)->res_volume, temp,
//				(*vtt)->max_value2, (*vtt)->res_volume, temp2,
//				vtt_class::vol_channel_adjust
//			);
			gtk_tx_flash_set_level((*vtt)->gui.flash, temp, temp2);
			
			(*vtt)->max_value=0;
			(*vtt)->max_value2=0;
		}
		
		if ((*vtt)->needs_cleaning_up()) {
			cleanup_vtt((*vtt));
		}
	}
}

void cleanup_all_vtts()
{
	list <vtt_class *> :: iterator vtt;
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		if ((*vtt)->buffer) gtk_tx_cleanup_pos_display(GTK_TX((*vtt)->gui.display));
		gtk_tx_flash_set_level((*vtt)->gui.flash, 0.0, 0.0);
		gtk_tx_flash_clear((*vtt)->gui.flash);
	}
}

void gui_clear_master_button(vtt_class *vtt)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(vtt->gui.sync_master), 0);
}

void gui_show_frame(vtt_class *vtt, int show)
{
	gtk_tx_show_frame(GTK_TX(vtt->gui.display), show);
}

#define vgui (*vtt)->gui
#define v (*vtt)

void vg_enable_critical_buttons(int enable)
{
	list <vtt_class *> :: iterator vtt;
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		gtk_widget_set_sensitive(vgui.del, enable);
		gtk_widget_set_sensitive(vgui.sync_master, enable);
	}
}

void vg_init_all_non_seqpars()
{
	list <vtt_class *> :: iterator vtt;
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON((*vtt)->gui.autotrigger), (*vtt)->autotrigger);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON((*vtt)->gui.sync_master), (*vtt)->is_sync_master);
	}	
}

f_prec gui_get_audio_x_zoom(vtt_class *vtt) {
	return gtk_tx_get_zoom(GTK_TX(vtt->gui.display));
}

/* Yes, this is yet another evil hack. Fix it :) */
int vttgui_zoom_depth=0;

extern void gui_set_audio_x_zoom(vtt_class *vtt, f_prec value) {
	if (vttgui_zoom_depth==0) {
		vttgui_zoom_depth=1;
		gtk_range_set_value(GTK_RANGE(vtt->gui.zoom), value*100.0);
		vttgui_zoom_depth=0;
	} else {
		gtk_tx_set_zoom(GTK_TX(vtt->gui.display), value);
	}
}
