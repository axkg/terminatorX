/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999, 2000  Alexander K�nig
 
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

#ifdef USE_DIAL
#include "tX_dial.h"
#endif

#ifdef USE_FLASH
#include "tX_flash.h"
#endif
#include <stdio.h>

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0

#define FILENAME_BUTTON_MAX 20

void nicer_filename(char *dest, char *source)
{
		char *fn;
		char temp[PATH_MAX];
		int i;
		
		fn=strrchr(source, '/');
		if (fn) fn++;
		else fn=source;
		
		strcpy (temp, fn);
		
		fn=strrchr(temp, '.');
		if (fn) *fn=0;
		
		if (strlen(temp) > FILENAME_BUTTON_MAX)
		{
			temp[FILENAME_BUTTON_MAX-3]='.';
			temp[FILENAME_BUTTON_MAX-2]='.';
			temp[FILENAME_BUTTON_MAX-1]='.';
			temp[FILENAME_BUTTON_MAX]=0;
		} 
		strcpy (dest, temp);
}

GtkSignalFunc name_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_name(gtk_entry_get_text(GTK_ENTRY(wid)));
}

GtkSignalFunc volume_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_volume.receive_gui_value(2.0-GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc pitch_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_pitch.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc trigger_prelis(GtkWidget *w)
{
	GtkFileSelection *fs;
	
	fs=GTK_FILE_SELECTION(gtk_widget_get_toplevel(w));
	
	prelis_start(gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)));
	return(0);
}

int cancel_load_file(GtkWidget *wid, vtt_class *vtt)
{
	prelis_stop();
	
	vtt->gui.file_dialog=NULL;
	if (vtt->gui.fs) gtk_widget_destroy(GTK_WIDGET(vtt->gui.fs));
}

int quit_load_file(GtkWidget *wid, vtt_class *vtt)
{
	//vtt->gui.file_dialog=NULL;
	//prelis_stop();
	
	
	return 1;
}

char global_filename_buffer[PATH_MAX];

void load_part(char *newfile, vtt_class *vtt)
{
	int ret=0;
	char *fn;

	ld_create_loaddlg(TX_LOADDLG_MODE_SINGLE, 1);
	ld_set_filename(newfile);

	ret = vtt->load_file(newfile);
	
	ld_destroy();
	if (ret)
	{
		switch(ret)
		{
			case TX_AUDIO_ERR_ALLOC:
			tx_note("Error loading file: failed to allocate memory");
			break;
			case TX_AUDIO_ERR_PIPE_READ:
			tx_note("Error loading file: broken pipe (File not supported/corrupt?)");
			break;
			case TX_AUDIO_ERR_SOX:
			tx_note("Error loading file: couldn't execute sox");
			break;
			case TX_AUDIO_ERR_MPG123:
			tx_note("Error loading file: couldn't execute mpg123");
			break;
			case TX_AUDIO_ERR_WAV_NOTFOUND:
			tx_note("Error loading file: file not found");
			break;
			case TX_AUDIO_ERR_NOT_16BIT:
			tx_note("Error loading file: RIFF/WAV is not 16 Bit.");
			break;
			case TX_AUDIO_ERR_NOT_MONO:
			tx_note("Error loading file: RIFF/WAV is not mono");
			break;
			case TX_AUDIO_ERR_WAV_READ:
			tx_note("Error loading file: RIFF/WAV corrupt?");
			break;
			case TX_AUDIO_ERR_NOT_SUPPORTED:
			tx_note("Error loading file: filetype not supported.");
			break;
			default:					
			tx_note("OOPS: An unknown error occured - This shouldn't happen :(");	
		}
	}
	else
	{
		nicer_filename(global_filename_buffer, newfile);
//		strcpy(global_filename_buffer, fn);
		
		gtk_label_set(GTK_LABEL(GTK_BUTTON(vtt->gui.file)->child), global_filename_buffer);
	}	
}

GtkSignalFunc do_load_file(GtkWidget *wid, vtt_class *vtt)
{
	int ret;
	char newfile[PATH_MAX];
	char buffer[1024]="Couldn't open loop file: ";
	char fn[FILENAME_BUTTON_MAX];
	
	int16_t *newbuffer;
	unsigned int newsize;
	
	prelis_stop();

	strcpy(newfile, gtk_file_selection_get_filename(GTK_FILE_SELECTION(vtt->gui.fs)));
	gtk_widget_destroy(GTK_WIDGET(vtt->gui.fs));
	
	load_part(newfile, vtt);

	vtt->gui.file_dialog=NULL;
}

GtkSignalFunc drop_file(GtkWidget *widget, GdkDragContext *context,
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
	
	load_part(fn, vtt);
}


GtkSignalFunc load_file(GtkWidget *wid, vtt_class *vtt)
{	
	char buffer[512];
	
	if (vtt->gui.file_dialog)
	{
		//puts("praise");
		gdk_window_raise(vtt->gui.file_dialog);
		return(0);
	}
	
	sprintf(buffer, "Select Audio File for %s", vtt->name);
	vtt->gui.fs=gtk_file_selection_new(buffer);
	
	if (strlen(vtt->filename) > 0)
	{
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(vtt->gui.fs), vtt->filename);	
	}
	
	gtk_widget_show(GTK_WIDGET(vtt->gui.fs));
	
	vtt->gui.file_dialog=vtt->gui.fs->window;
	
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(vtt->gui.fs)->ok_button), "clicked", GTK_SIGNAL_FUNC(do_load_file), vtt);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(vtt->gui.fs)->cancel_button), "clicked", GTK_SIGNAL_FUNC (cancel_load_file), vtt);	
	gtk_signal_connect (GTK_OBJECT(vtt->gui.fs), "delete-event", GTK_SIGNAL_FUNC(quit_load_file), vtt);	
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(vtt->gui.fs)->file_list), "select_row", GTK_SIGNAL_FUNC(trigger_prelis), vtt->gui.fs);
}


GtkSignalFunc delete_vtt(GtkWidget *wid, vtt_class *vtt)
{
	if (audioon) tx_note("Sorry, you'll have to stop playback first.");
	else
	delete(vtt);
}

GtkSignalFunc edit_vtt_buffer(GtkWidget *wid, vtt_class *vtt)
{
	char command[2*PATH_MAX];

	if (vtt->samples_in_buffer == 0)
	{
		tx_note("Nothing to edit.");
	}
	else
	if (strlen(globals.file_editor)>0)
	{
		sprintf(command, "%s \"%s\" &", globals.file_editor, vtt->filename);
		system(command); /*) tx_note("Failed to run the soundfile editor."); */
	}
	else
	{
		tx_note("No soundfile editor configured.");
	}
}

GtkSignalFunc reload_vtt_buffer(GtkWidget *wid, vtt_class *vtt)
{
	char reload_buffer[PATH_MAX];
	
	while (gtk_events_pending()) gtk_main_iteration();
	
//	puts(vtt->filename);
	if (vtt->samples_in_buffer > 0)
	{
		strcpy(reload_buffer, vtt->filename);
		load_part(reload_buffer, vtt);
	}
	else tx_note("Nothing to reload.");
}

GtkSignalFunc clone_vtt(GtkWidget *wid, vtt_class *vtt)
{
	vtt->stop();
}

GtkSignalFunc trigger_vtt(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_trigger.receive_gui_value((float) 1.0);
}

GtkSignalFunc stop_vtt(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_trigger.receive_gui_value((float) 0.0);
}

GtkSignalFunc autotrigger_toggled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_autotrigger(GTK_TOGGLE_BUTTON(wid)->active);
}

GtkSignalFunc loop_toggled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_loop.receive_gui_value(GTK_TOGGLE_BUTTON(wid)->active);
}

GtkSignalFunc lp_enabled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_lp_enable.receive_gui_value(GTK_TOGGLE_BUTTON(wid)->active);
}

GtkSignalFunc lp_gain_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_lp_gain.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc lp_reso_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_lp_reso.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc lp_freq_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_lp_freq.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc ec_enabled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_ec_enable.receive_gui_value(GTK_TOGGLE_BUTTON(wid)->active);
}

GtkSignalFunc ec_length_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_ec_length.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc ec_feedback_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->sp_ec_feedback.receive_gui_value(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc master_setup(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_sync_master(GTK_TOGGLE_BUTTON(wid)->active);
}

GtkSignalFunc client_setup(GtkWidget *wid, vtt_class *vtt)
{
	int client;
	
	client=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(vtt->gui.sync_client));	
	vtt->sp_sync_client.receive_gui_value(client);
}

GtkSignalFunc client_setup_number(GtkWidget *wid, vtt_class *vtt)
{
	int cycles;
	
	cycles=(int) GTK_ADJUSTMENT(vtt->gui.cycles)->value;	
	
	vtt->sp_sync_cycles.receive_gui_value(cycles);
}

/*
GtkSignalFunc control_changed(GtkWidget *wid, vtt_class *vtt)
{
	int x,y;
	vtt_gui *g=&vtt->gui;
	
	if (GTK_TOGGLE_BUTTON(g->x_scratch)->active) x=CONTROL_SCRATCH;
	else if (GTK_TOGGLE_BUTTON(g->x_volume)->active) x=CONTROL_VOLUME;
	else if (GTK_TOGGLE_BUTTON(g->x_lp_cutoff)->active) x=CONTROL_CUTOFF;
	else if (GTK_TOGGLE_BUTTON(g->x_ec_feedback)->active) x=CONTROL_FEEDBACK;
	else if (GTK_TOGGLE_BUTTON(g->x_nothing)->active) x=CONTROL_NOTHING;

	if (GTK_TOGGLE_BUTTON(g->y_scratch)->active) y=CONTROL_SCRATCH;
	else if (GTK_TOGGLE_BUTTON(g->y_volume)->active) y=CONTROL_VOLUME;
	else if (GTK_TOGGLE_BUTTON(g->y_lp_cutoff)->active) y=CONTROL_CUTOFF;
	else if (GTK_TOGGLE_BUTTON(g->y_ec_feedback)->active) y=CONTROL_FEEDBACK;
	else if (GTK_TOGGLE_BUTTON(g->y_nothing)->active) y=CONTROL_NOTHING;
	
	vtt->set_controls(x,y);
}
*/

void vg_display_xcontrol(vtt_class *vtt)
{
	char buffer[2048];
	
	if (vtt->x_par)
	{
		strcpy(buffer, vtt->x_par->get_name());
		if (strlen(buffer)>35) 
		{
			buffer[34] = '.';
			buffer[35] = '.';
			buffer[36] = '.';
			buffer[37] = 0;			
		}
		gtk_label_set(GTK_LABEL(GTK_BUTTON(vtt->gui.x_control)->child), buffer);		
	}
	else
	{
		gtk_label_set(GTK_LABEL(GTK_BUTTON(vtt->gui.x_control)->child), "Nothing");
	}
}

void vg_display_ycontrol(vtt_class *vtt)
{
	char buffer[2048];
	
	if (vtt->y_par)
	{
		strcpy(buffer, vtt->y_par->get_name());
		if (strlen(buffer)>26) 
		{
			buffer[24] = '.';
			buffer[25] = '.';
			buffer[26] = '.';
			buffer[27] = 0;			
		}
		gtk_label_set(GTK_LABEL(GTK_BUTTON(vtt->gui.y_control)->child), buffer);		
	}
	else
	{
		gtk_label_set(GTK_LABEL(GTK_BUTTON(vtt->gui.y_control)->child), "Nothing");
	}
}

GtkSignalFunc vg_xcontrol_dis(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_x_input_parameter(NULL);
}

GtkSignalFunc vg_ycontrol_dis(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_y_input_parameter(NULL);
}

GtkSignalFunc vg_xcontrol_set(GtkWidget *wid, tX_seqpar *sp)
{
	vtt_class *vtt=(vtt_class *) sp->vtt;
	vtt->set_x_input_parameter(sp);
}

GtkSignalFunc vg_ycontrol_set(GtkWidget *wid, tX_seqpar *sp)
{
	vtt_class *vtt=(vtt_class *) sp->vtt;
	vtt->set_y_input_parameter(sp);
}

void vg_control_menu_popup(vtt_class *vtt, int axis)
{
	list <tX_seqpar *> :: iterator sp;
	vtt_gui *g=&vtt->gui;
	GtkWidget *item;

	if (g->par_menu) gtk_object_destroy(GTK_OBJECT(g->par_menu));
	g->par_menu=gtk_menu_new();
	
	item = gtk_menu_item_new_with_label("Nothing");
	gtk_menu_append(GTK_MENU(g->par_menu), item);
	gtk_widget_show(item);
	if (axis) gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(vg_xcontrol_dis), vtt);
	else gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(vg_ycontrol_dis), vtt);
	
	
	for (sp=tX_seqpar::all.begin(); sp!=tX_seqpar::all.end(); sp++)
	{
		if (((*sp)->is_mappable) && ((*sp)->vtt) == (void*) vtt)
		{
			item = gtk_menu_item_new_with_label((*sp)->get_name());
			gtk_menu_append(GTK_MENU(g->par_menu), item);
			gtk_widget_show(item);
			if (axis) gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(vg_xcontrol_set), (void*) (*sp));
			else gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(vg_ycontrol_set), (void*) (*sp));
		}
	}
	gtk_menu_popup (GTK_MENU(g->par_menu), NULL, NULL, NULL, NULL, 0, 0);
}

GtkSignalFunc vg_xcontrol_popup(GtkWidget *wid, vtt_class *vtt) 
{
	vg_control_menu_popup(vtt, 1);
}

GtkSignalFunc vg_ycontrol_popup(GtkWidget *wid, vtt_class *vtt)
{
	vg_control_menu_popup(vtt, 0);
}

static vtt_class * fx_vtt;

GtkSignalFunc new_effect(GtkWidget *wid, LADSPA_Plugin *plugin)
{
	fx_vtt->add_effect(plugin);
}

GtkSignalFunc fx_button_pressed(GtkWidget *wid, vtt_class *vtt)
{
	vtt_gui *g=&vtt->gui;
	GtkWidget *item;
	int i;
	LADSPA_Plugin *plugin;
	char buffer[1024];
	char oldfile[1024]="";
	GtkWidget *submenu;

	fx_vtt=vtt; /* AAAAARGH - Long live ugly code */

	if (g->ladspa_menu) gtk_object_destroy(GTK_OBJECT(g->ladspa_menu));
	g->ladspa_menu=gtk_menu_new();
	
	for (i=0; i<LADSPA_Plugin::getPluginCount(); i++)
	{
		plugin=LADSPA_Plugin::getPluginByIndex(i);
		if (strcmp(plugin->get_file_name(), oldfile))
		{
			strcpy(oldfile, plugin->get_file_name());
			item = gtk_menu_item_new_with_label(oldfile);
			submenu=gtk_menu_new();
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);
			gtk_menu_append(GTK_MENU(g->ladspa_menu), item);
			gtk_widget_show(item);
		}
		sprintf(buffer, "%s - [%i, %s]", plugin->getName(), plugin->getUniqueID(), plugin->getLabel());
		item=gtk_menu_item_new_with_label(buffer);
		gtk_menu_append(GTK_MENU(submenu), item);
		gtk_widget_show(item);
		gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(new_effect), plugin);
	}
	
	gtk_menu_popup (GTK_MENU(g->ladspa_menu), NULL, NULL, NULL, NULL, 0, 0);
}

#define connect_entry(wid, func); gtk_signal_connect(GTK_OBJECT(g->wid), "activate", (GtkSignalFunc) func, (void *) vtt);
#define connect_adj(wid, func); gtk_signal_connect(GTK_OBJECT(g->wid), "value_changed", (GtkSignalFunc) func, (void *) vtt);
#define connect_button(wid, func); gtk_signal_connect(GTK_OBJECT(g->wid), "clicked", (GtkSignalFunc) func, (void *) vtt);
#define connect_press_button(wid, func); gtk_signal_connect(GTK_OBJECT(g->wid), "pressed", (GtkSignalFunc) func, (void *) vtt);
#define connect_rel_button(wid, func); gtk_signal_connect(GTK_OBJECT(g->wid), "released", (GtkSignalFunc) func, (void *) vtt);

GtkWidget *vg_create_fx_bar(vtt_class *vtt, vtt_fx *effect, int showdel);

void gui_connect_signals(vtt_class *vtt)
{
	vtt_gui *g=&vtt->gui;

	connect_entry(name, name_changed);
	connect_adj(volume, volume_changed);
	connect_button(edit, edit_vtt_buffer);
	connect_button(reload, reload_vtt_buffer);
	connect_adj(pitch, pitch_changed);
	connect_button(file, load_file);
	
	connect_button(del, delete_vtt);
//	connect_button(clone, clone_vtt);
	connect_button(trigger, trigger_vtt);
	connect_button(stop, stop_vtt);
	connect_button(autotrigger, autotrigger_toggled);
	connect_button(loop, loop_toggled);
	connect_button(sync_master, master_setup);
	connect_button(sync_client, client_setup);
	connect_adj(cycles, client_setup_number);
/*	
	connect_button(x_scratch, control_changed);
	connect_button(x_volume, control_changed);
	connect_button(x_lp_cutoff, control_changed);
	connect_button(x_ec_feedback, control_changed);
	connect_button(x_nothing, control_changed);

	connect_button(y_scratch, control_changed);
	connect_button(y_volume, control_changed);
	connect_button(y_lp_cutoff, control_changed);
	connect_button(y_ec_feedback, control_changed);
	connect_button(y_nothing, control_changed);
*/	
	connect_button(fx_button, fx_button_pressed);
	
	connect_button(lp_enable, lp_enabled);
	connect_adj(lp_gain, lp_gain_changed);
	connect_adj(lp_reso, lp_reso_changed);
	connect_adj(lp_freq, lp_freq_changed);
	
	connect_button(ec_enable, ec_enabled);
	connect_adj(ec_length, ec_length_changed);
	connect_adj(ec_feedback, ec_feedback_changed);
	connect_button(x_control, vg_xcontrol_popup);
	connect_button(y_control, vg_ycontrol_popup);

	static GtkTargetEntry drop_types [] = {
		{ "text/uri-list", 0, 0}
	};
	static gint n_drop_types = sizeof (drop_types) / sizeof(drop_types[0]);
	
	gtk_drag_dest_set (GTK_WIDGET (g->file), (GtkDestDefaults) (GTK_DEST_DEFAULT_MOTION |GTK_DEST_DEFAULT_HIGHLIGHT |GTK_DEST_DEFAULT_DROP),
			drop_types, n_drop_types,
			GDK_ACTION_COPY);
						
	gtk_signal_connect (GTK_OBJECT (g->file), "drag_data_received",
			GTK_SIGNAL_FUNC(drop_file), (void *) vtt);

	gtk_drag_dest_set (GTK_WIDGET (g->display), (GtkDestDefaults) (GTK_DEST_DEFAULT_MOTION |GTK_DEST_DEFAULT_HIGHLIGHT |GTK_DEST_DEFAULT_DROP),
			drop_types, n_drop_types,
			GDK_ACTION_COPY);
						
	gtk_signal_connect (GTK_OBJECT (g->display), "drag_data_received",
			GTK_SIGNAL_FUNC(drop_file), (void *) vtt);
	
}
	
void build_vtt_gui(vtt_class *vtt)
{
	GtkWidget *tempbox;
	GtkWidget *tempbox2;
	GtkWidget *dummy;
	char nice_name[FILENAME_BUTTON_MAX];
	
	vtt_gui *g;
	
	g=&vtt->gui;
	vtt->have_gui=1;
	g->par_menu=NULL;
	g->ladspa_menu=NULL;

	/* Building Audio Box */
	g->audio_box=gtk_vbox_new(FALSE,2);
	gtk_widget_show(g->audio_box);
	
	tempbox=gtk_hbox_new(FALSE,2);
	gtk_widget_show(tempbox);
	gtk_box_pack_start(GTK_BOX(g->audio_box), tempbox, WID_FIX);
	
	g->audio_label=gtk_label_new(vtt->name);
	gtk_widget_show(g->audio_label);
	gtk_box_pack_start(GTK_BOX(tempbox), g->audio_label, WID_DYN);

	dummy=gtk_vseparator_new();
	gtk_widget_show(dummy);
	gtk_box_pack_start(GTK_BOX(tempbox), dummy, WID_FIX);

	nicer_filename(nice_name, vtt->filename);
	g->file = gtk_button_new_with_label(nice_name);
	gtk_widget_show(g->file);
	gtk_box_pack_start(GTK_BOX(tempbox), g->file, WID_DYN);

	g->edit=tx_xpm_button_new(TX_ICON_EDIT, "Edit", 0);
	gtk_widget_show(g->edit);
	gtk_box_pack_start(GTK_BOX(tempbox), g->edit, WID_FIX);
	
	g->reload=tx_xpm_button_new(TX_ICON_RELOAD, "Reload", 0);
	gtk_widget_show(g->reload);
	gtk_box_pack_start(GTK_BOX(tempbox), g->reload, WID_FIX);
	
	dummy=gtk_vseparator_new();
	gtk_widget_show(dummy);
	gtk_box_pack_start(GTK_BOX(tempbox), dummy, WID_FIX);

	dummy=gtk_label_new("X:");
	gtk_widget_show(dummy);
	gtk_box_pack_start(GTK_BOX(tempbox), dummy, WID_FIX);

	g->x_control=gtk_button_new_with_label(vtt->x_par->get_name());
	gtk_widget_show(g->x_control);
	gtk_box_pack_start(GTK_BOX(tempbox), g->x_control, WID_DYN);

	dummy=gtk_label_new("Y:");
	gtk_widget_show(dummy);
	gtk_box_pack_start(GTK_BOX(tempbox), dummy, WID_FIX);
	
	g->y_control=gtk_button_new_with_label(vtt->y_par->get_name());
	gtk_widget_show(g->y_control);
	gtk_box_pack_start(GTK_BOX(tempbox), g->y_control, WID_DYN);

	g->display=gtk_tx_new(vtt->buffer, vtt->samples_in_buffer);
	gtk_box_pack_start(GTK_BOX(g->audio_box), g->display, WID_DYN);
	gtk_widget_show(g->display);	
	
	/* Building Control Box */
	
	g->control_box=gtk_vbox_new(FALSE,2);
	gtk_widget_show(g->control_box);
	
	g->control_label=gtk_label_new(vtt->name);
	gtk_widget_show(g->control_label);
	gtk_box_pack_start(GTK_BOX(g->control_box), g->control_label, WID_FIX);
	
	g->scrolled_win=gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (g->scrolled_win), 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (g->scrolled_win),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show(g->scrolled_win);
	gtk_box_pack_start(GTK_BOX(g->control_box), g->scrolled_win, WID_DYN);
				    
	g->control_subbox=gtk_vbox_new(FALSE,0);
	gtk_scrolled_window_add_with_viewport (
                   GTK_SCROLLED_WINDOW (g->scrolled_win), g->control_subbox);
	gtk_widget_show(g->control_subbox);
		   

	/* Main panel */
	
	tX_panel *p=new tX_panel("Main", g->control_subbox);
	g->main_panel=p;
			
	g->name = gtk_entry_new_with_max_length(256);	
	gtk_entry_set_text(GTK_ENTRY(g->name), vtt->name);
	p->add_client_widget(g->name);
	gtk_widget_set_usize(g->name, 40, g->name->requisition.height);
	
	g->del=gtk_button_new_with_label("Delete");
	p->add_client_widget(g->del);
	
	g->show_audio=gtk_toggle_button_new_with_label("Show Audio");
	p->add_client_widget(g->show_audio);
		
	gtk_box_pack_start(GTK_BOX(g->control_subbox), p->get_widget(), WID_FIX);
				
	p=new tX_panel("Trigger", g->control_subbox);
	g->trigger_panel=p;
	
	g->trigger=gtk_button_new_with_label("Trigger!");
	p->add_client_widget(g->trigger);
	
	g->stop=gtk_button_new_with_label("Stop.");
	p->add_client_widget(g->stop);
	
	g->autotrigger=gtk_check_button_new_with_label("Auto");
	p->add_client_widget(g->autotrigger);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->autotrigger), vtt->autotrigger);
	
	g->loop=gtk_check_button_new_with_label("Loop");
	p->add_client_widget(g->loop);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->loop), vtt->loop);
	
	g->sync_master=gtk_check_button_new_with_label("Master");
	p->add_client_widget(g->sync_master);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->sync_master), vtt->is_sync_master);
	
	g->sync_client=gtk_check_button_new_with_label("Client");
	p->add_client_widget(g->sync_client);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->sync_client), vtt->is_sync_client);
	
	g->cycles=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->sync_cycles, 0, 10.0, 1,1,1));
	dummy=gtk_spin_button_new(g->cycles, 1.0, 0);
	p->add_client_widget(dummy);

	gtk_box_pack_start(GTK_BOX(g->control_subbox), p->get_widget(), WID_FIX);

	dummy=gtk_button_new_with_label("FX");
	gtk_widget_show(dummy);
	g->fx_button=dummy;
	gtk_box_pack_start(GTK_BOX(g->control_subbox), dummy, WID_FIX);
	
	/* Lowpass Panel */

	p=new tX_panel("Lowpass", g->control_subbox);
	g->lp_panel=p;
		
	g->lp_enable=gtk_check_button_new_with_label("Enable");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->lp_enable), vtt->lp_enable);
	p->add_client_widget(vg_create_fx_bar(vtt, vtt->lp_fx, 0));

	p->add_client_widget(g->lp_enable);

	g->lp_gain=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->lp_gain, 0, 2, 0.1, 0.01, 0.01));
	g->lp_reso=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->lp_reso, 0, 0.99, 0.1, 0.01, 0.01));
	g->lp_freq=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->lp_freq, 0, 1, 0.1, 0.01, 0.01));

	g->lp_gaind=new tX_extdial("Input Gain", g->lp_gain);
	p->add_client_widget(g->lp_gaind->get_widget());

	g->lp_freqd=new tX_extdial("Frequency", g->lp_freq);
	p->add_client_widget(g->lp_freqd->get_widget());

	g->lp_resod=new tX_extdial("Resonance", g->lp_reso);
	p->add_client_widget(g->lp_resod->get_widget());

	gtk_box_pack_start(GTK_BOX(g->control_subbox), p->get_widget(), WID_FIX);

	/* Echo Panel */

	p=new tX_panel("Echo", g->control_subbox);
	g->ec_panel=p;

	p->add_client_widget(vg_create_fx_bar(vtt, vtt->ec_fx, 0));
	
	g->ec_enable=gtk_check_button_new_with_label("Enable");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->ec_enable), vtt->ec_enable);
	p->add_client_widget(g->ec_enable);

	g->ec_length=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->ec_length, 0, 1, 0.1, 0.01, 0.001));
	g->ec_feedback=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->ec_feedback, 0, 1, 0.1, 0.01, 0.001));

	g->ec_lengthd=new tX_extdial("Duration", g->ec_length);
	p->add_client_widget(g->ec_lengthd->get_widget());

	g->ec_feedbackd=new tX_extdial("Feedback", g->ec_feedback);
	p->add_client_widget(g->ec_feedbackd->get_widget());

	gtk_box_pack_start(GTK_BOX(g->control_subbox), p->get_widget(), WID_FIX);
	
	/* Output */
	
	tempbox=gtk_hbox_new(FALSE,2);
	gtk_widget_show(tempbox);
	gtk_box_pack_end(GTK_BOX(g->control_box), tempbox, WID_FIX);
	
	tempbox2=gtk_vbox_new(FALSE,0);
	gtk_widget_show(tempbox2);
	gtk_box_pack_start(GTK_BOX(tempbox), tempbox2, WID_FIX);
	
	g->pitch=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->rel_pitch, -3, +3, 0.1, 0.01, 0.001));
	g->pan=GTK_ADJUSTMENT(gtk_adjustment_new(0, -1, 1, 0.1, 0.01, 0.001));

	g->pitchd=new tX_extdial("Pitch", g->pitch);
	gtk_box_pack_start(GTK_BOX(tempbox2), g->pitchd->get_widget(), WID_FIX);

	g->pand=new tX_extdial("Pan", g->pan);
	gtk_box_pack_start(GTK_BOX(tempbox2), g->pand->get_widget(), WID_FIX);

	tempbox2=gtk_hbox_new(FALSE,0);
	gtk_widget_show(tempbox2);
	gtk_box_pack_start(GTK_BOX(tempbox), tempbox2, WID_FIX);
	
	g->volume=GTK_ADJUSTMENT(gtk_adjustment_new(2.0-vtt->rel_volume, 0, 2, 0.01, 0.01, 0.01));
	dummy=gtk_vscale_new(GTK_ADJUSTMENT(g->volume)); 
	gtk_scale_set_draw_value(GTK_SCALE(dummy), False);
	gtk_box_pack_start(GTK_BOX(tempbox2), dummy, WID_FIX);
	gtk_widget_show(dummy);

	g->flash=gtk_tx_flash_new();
	gtk_box_pack_start(GTK_BOX(tempbox2), g->flash, WID_FIX);
	gtk_widget_show(g->flash);		

	g->file_dialog=NULL;

//	gtk_notebook_set_page(GTK_NOTEBOOK(vtt->gui.notebook), g->current_gui);
	
	gui_connect_signals(vtt);
}

GtkSignalFunc fx_up(GtkWidget *wid, vtt_fx *effect)
{
	vtt_class *vtt;
	
	vtt=(vtt_class*)effect->get_vtt();
	vtt->effect_up(effect);
}

GtkSignalFunc fx_down(GtkWidget *wid, vtt_fx *effect)
{
	vtt_class *vtt;
	
	vtt=(vtt_class*)effect->get_vtt();
	vtt->effect_down(effect);
}


GtkSignalFunc fx_kill(GtkWidget *wid, vtt_fx_ladspa *effect)
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

	if (showdel)
	{
		button=gtk_button_new();
		pixmap=tx_pixmap_widget(TX_ICON_FX_CLOSE);
		gtk_container_add (GTK_CONTAINER (button), pixmap);	
		gtk_box_pack_end(GTK_BOX(box), button, WID_FIX);
		gtk_widget_show(pixmap);
		gtk_widget_show(button);
		gtk_signal_connect(GTK_OBJECT(button), "clicked", (GtkSignalFunc) fx_kill, (void *) effect);
	}

	button=gtk_button_new();
	pixmap=tx_pixmap_widget(TX_ICON_FX_DOWN);
	gtk_container_add (GTK_CONTAINER (button), pixmap);	
	gtk_box_pack_end(GTK_BOX(box), button, WID_FIX);
	gtk_widget_show(pixmap);
	gtk_widget_show(button);
	gtk_signal_connect(GTK_OBJECT(button), "clicked", (GtkSignalFunc) fx_down, (void *) effect);

	button=gtk_button_new();
	pixmap=tx_pixmap_widget(TX_ICON_FX_UP);
	gtk_container_add (GTK_CONTAINER (button), pixmap);	
	gtk_box_pack_end(GTK_BOX(box), button, WID_FIX);
	gtk_widget_show(pixmap);
	gtk_widget_show(button);
	gtk_signal_connect(GTK_OBJECT(button), "clicked", (GtkSignalFunc) fx_up, (void *) effect);
	
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

void vg_move_fx_panel_up(GtkWidget *wid, vtt_class *vtt)
{
	int pos=gtk_box_get_widget_pos(GTK_BOX(vtt->gui.control_subbox), wid);
	gtk_box_reorder_child(GTK_BOX(vtt->gui.control_subbox), wid, pos-1);
}

void vg_move_fx_panel_down(GtkWidget *wid, vtt_class *vtt)
{
	int pos=gtk_box_get_widget_pos(GTK_BOX(vtt->gui.control_subbox), wid);
	gtk_box_reorder_child(GTK_BOX(vtt->gui.control_subbox), wid, pos+1);
}

GtkSignalFunc vg_show_fx_info(GtkWidget *wid, vtt_fx *effect)
{
	tx_l_note(effect->get_info_string());
}

void vg_create_fx_gui(vtt_class *vtt, vtt_fx_ladspa *effect, LADSPA_Plugin *plugin)
{
	char buffer[1024];
	
	vtt_gui *g;
	g=&vtt->gui;	
	tX_panel *p;
	list <tX_seqpar_vttfx *> :: iterator sp;
	
	strcpy(buffer, plugin->getLabel());
	if (strlen(buffer) > 8)
	{
		buffer[7]='.';
		buffer[8]='.';
		buffer[9]='.';
		buffer[10]=0;
	}

	p=new tX_panel(buffer, g->control_subbox);
	
	p->add_client_widget(vg_create_fx_bar(vtt, effect, 1));
	
	for (sp = effect->controls.begin(); sp != effect->controls.end(); sp++)
	{
			p->add_client_widget((*sp)->get_widget());
	}

	gtk_signal_connect(GTK_OBJECT(p->get_labelbutton()), "clicked", (GtkSignalFunc) vg_show_fx_info, (void *) effect);
	effect->set_panel_widget(p->get_widget());
	effect->set_panel(p);

	gtk_box_pack_start(GTK_BOX(g->control_subbox), p->get_widget(), WID_FIX);
}

void gui_set_name(vtt_class *vtt, char *newname)
{
	gtk_label_set_text(GTK_LABEL(vtt->gui.audio_label), newname);
	gtk_label_set_text(GTK_LABEL(vtt->gui.control_label), newname);
	gtk_entry_set_text(GTK_ENTRY(vtt->gui.name), newname);
}

void gui_set_filename (vtt_class *vtt, char *newname)
{
	gtk_label_set(GTK_LABEL(GTK_BUTTON(vtt->gui.file)->child), newname);
}

void gui_update_display(vtt_class *vtt)
{
	nicer_filename(global_filename_buffer, vtt->filename);
	gtk_label_set(GTK_LABEL(GTK_BUTTON(vtt->gui.file)->child), global_filename_buffer);
	gtk_tx_set_data(GTK_TX(vtt->gui.display), vtt->buffer, vtt->samples_in_buffer);
}

void delete_gui(vtt_class *vtt)
{
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
	delete vtt->gui.ec_panel;
	
	gtk_widget_destroy(vtt->gui.control_box);
	gtk_widget_destroy(vtt->gui.audio_box);
}

/*
void recreate_gui(vtt_class *vtt, GtkWidget *ctrl, GtkWidget *audio)
{
	build_vtt_gui(vtt);
//	gtk_notebook_set_page(GTK_NOTEBOOK(vtt->gui.notebook), g->current_gui);
	gtk_box_pack_start(GTK_BOX(daddy), vtt->gui.frame, TRUE, TRUE, 0);
	gtk_widget_show(vtt->gui.frame);
}

void delete_gui(vtt_class *vtt)
{
	gtk_widget_destroy(vtt->gui.frame);
}*/

void update_all_vtts()
{
	list <vtt_class *> :: iterator vtt;
	f_prec temp;
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		if ((*vtt)->is_playing)
		{
			gtk_tx_update_pos_display(GTK_TX((*vtt)->gui.display), (*vtt)->pos_i, (*vtt)->mute);
#ifdef USE_FLASH
			temp=(*vtt)->max_value*(*vtt)->res_volume*vtt_class::vol_channel_adjust;
			(*vtt)->max_value=0;
			gtk_tx_flash_set_level((*vtt)->gui.flash, temp);
#endif		
		}
	}
}

void cleanup_vtt(vtt_class *vtt)
{
		gtk_tx_cleanup_pos_display(GTK_TX(vtt->gui.display));	
#ifdef USE_FLASH
		gtk_tx_flash_set_level(vtt->gui.flash, 0.0);
		gtk_tx_flash_clear(vtt->gui.flash);
#endif		
}

void cleanup_all_vtts()
{
	list <vtt_class *> :: iterator vtt;
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		if ((*vtt)->buffer) gtk_tx_cleanup_pos_display(GTK_TX((*vtt)->gui.display));
#ifdef USE_FLASH
		gtk_tx_flash_set_level((*vtt)->gui.flash, 0.0);
		gtk_tx_flash_clear((*vtt)->gui.flash);
#endif		
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

void show_all_guis(int show)
{
	list <vtt_class *> :: iterator vtt;
/*	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		if (show)
		{
			gtk_widget_show((*vtt)->gui.notebook);
		}
		else
		{
			gtk_widget_hide((*vtt)->gui.notebook);		
		}
	}
	*/
}
/*
void vg_update_sync(void *p)
{
	vtt_class *vtt=(vtt_class*) p;
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(vtt->gui.sync_client), vtt->is_sync_client);
//	gtk_adjustment_set_value(vtt->gui.cycles
}
*/
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


int vg_get_current_page(vtt_class *vtt)
{
	return (0);
}

void vg_set_current_page(vtt_class *vtt, int page)
{
	vtt->gui.current_gui=page;
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
