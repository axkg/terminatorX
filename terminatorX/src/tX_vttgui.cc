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
 
    File: tX_vttgui.cc
 
    Description: This implements the gtk+ GUI for the virtual turntable
    		 class implemented in tX_vtt.cc. This code is not in tX_vtt.cc
		 for mainly to keep the GUI code divided from the audio-rendering
		 code and as gtk+ callback to C++ method call wrapper.
*/    

#include <gtk/gtk.h>
#include "tX_vttgui.h"
#include "tX_vtt.h"
#include "tX_widget.h"
#include "tX_wavfunc.h"
#include "tX_mastergui.h"

#ifdef USE_FLASH
#include "tX_flash.h"
#endif
#include <stdio.h>

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0

#ifdef EXTRA_FRAME

#define begin_panel(s); dummy=gtk_frame_new(s); \
	gtk_container_set_border_width( GTK_CONTAINER (dummy), 5); \
	gtk_widget_show(dummy); \
	panel_hbox=gtk_vbox_new(FALSE, 0);\
	gtk_container_add (GTK_CONTAINER(dummy), panel_hbox); \
	gtk_widget_show(panel_hbox); \
	label= gtk_label_new(s); \
	gtk_notebook_append_page(GTK_NOTEBOOK(g->notebook), dummy, label	);

#else

#define begin_panel(s); \
	panel_hbox=gtk_vbox_new(FALSE, 0);\
	gtk_widget_show(panel_hbox); \
	label= gtk_label_new(s); \
	gtk_notebook_append_page(GTK_NOTEBOOK(g->notebook), panel_hbox, label	);

#endif	

	
#define panel_add_item(w); row_vbox=gtk_hbox_new(FALSE,5); \
	gtk_container_set_border_width(GTK_CONTAINER(row_vbox), 2); \
	gtk_box_pack_start(GTK_BOX(row_vbox), w, WID_DYN); \
	gtk_widget_show(w); \
	gtk_box_pack_start(GTK_BOX(panel_hbox), row_vbox, WID_FIX); gtk_widget_show(row_vbox);

#define wpanel_add_item(w); \
	gtk_box_pack_start(GTK_BOX(panel_hbox), w, WID_FIX); gtk_widget_show(row_vbox);

#define panel_add_label(s); dummy=gtk_label_new(s); \
	gtk_misc_set_alignment(GTK_MISC(dummy), 0, 0.5); \
	panel_add_item(dummy);

#define panel_add_hscale(a, d); dummy=gtk_hscale_new(GTK_ADJUSTMENT(a)); \
	gtk_scale_set_digits(GTK_SCALE(dummy), d); \
	gtk_scale_set_value_pos(GTK_SCALE(dummy), GTK_POS_RIGHT); \
	panel_add_item(dummy);

#define begin_row(); row_vbox=gtk_hbox_new(FALSE, 5); \
	gtk_container_set_border_width(GTK_CONTAINER(row_vbox), 5); \
	gtk_box_pack_start(GTK_BOX(panel_hbox), row_vbox, WID_FIX); gtk_widget_show(row_vbox);
	
#define row_add_item_fix(w); gtk_box_pack_start(GTK_BOX(row_vbox), w, WID_FIX); gtk_widget_show(w);
#define row_add_item_dyn(w); gtk_box_pack_start(GTK_BOX(row_vbox), w, WID_DYN); gtk_widget_show(w);

#define row_add_hscale(a, d); dummy=gtk_hscale_new(GTK_ADJUSTMENT(a)); \
	gtk_scale_set_digits(GTK_SCALE(dummy), d); \
	gtk_scale_set_value_pos(GTK_SCALE(dummy), GTK_POS_LEFT); \
	row_add_item_dyn(dummy);
	
#define row_add_label(s); dummy=gtk_label_new(s); \
	gtk_misc_set_alignment(GTK_MISC(dummy), 0.5, 0.5); \
	row_add_item_fix(dummy);
	
#define FILENAME_BUTTON_MAX 25

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
	vtt->set_volume(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc pitch_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_pitch(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc cancel_load_file(GtkWidget *wid, vtt_class *vtt)
{
	vtt->gui.file_dialog=NULL;
	if (vtt->gui.fs) gtk_widget_destroy(GTK_WIDGET(vtt->gui.fs));
	return(0);
}

GtkSignalFunc do_load_file(GtkWidget *wid, vtt_class *vtt)
{
	int ret;
	char newfile[PATH_MAX];
	char buffer[1024]="Couldn't open loop file: ";
	char fn[FILENAME_BUTTON_MAX];
	
	int16_t *newbuffer;
	unsigned int newsize;

	strcpy(newfile, gtk_file_selection_get_filename(GTK_FILE_SELECTION(vtt->gui.fs)));
	gtk_widget_destroy(GTK_WIDGET(vtt->gui.fs));

	ret = load_wav(newfile, &newbuffer, &newsize);
	
	if (ret)
	{
		puts("error");
	}
	else
	{
		vtt->set_file_data(newfile, newbuffer, newsize/sizeof(int16_t));

		nicer_filename(fn, newfile);
		
		gtk_label_set(GTK_LABEL(GTK_BUTTON(vtt->gui.file)->child), fn);
	}	
	vtt->gui.file_dialog=NULL;
}

GtkSignalFunc load_file(GtkWidget *wid, vtt_class *vtt)
{	
	char buffer[512];
	
	if (vtt->gui.file_dialog)
	{
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
	gtk_signal_connect (GTK_OBJECT(vtt->gui.fs), "delete-event", GTK_SIGNAL_FUNC(cancel_load_file), vtt);	
}


GtkSignalFunc delete_vtt(GtkWidget *wid, vtt_class *vtt)
{
	delete(vtt);
}

GtkSignalFunc clone_vtt(GtkWidget *wid, vtt_class *vtt)
{
	vtt->stop();
}

GtkSignalFunc trigger_vtt(GtkWidget *wid, vtt_class *vtt)
{
	vtt->trigger();
}

GtkSignalFunc stop_vtt(GtkWidget *wid, vtt_class *vtt)
{
	vtt->stop();
}

GtkSignalFunc autotrigger_toggled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_autotrigger(GTK_TOGGLE_BUTTON(wid)->active);
}

GtkSignalFunc loop_toggled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_loop(GTK_TOGGLE_BUTTON(wid)->active);
}

GtkSignalFunc lp_enabled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->lp_set_enable(GTK_TOGGLE_BUTTON(wid)->active);
}

GtkSignalFunc lp_gain_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->lp_set_gain(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc lp_reso_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->lp_set_reso(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc lp_freq_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->lp_set_freq(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc ec_enabled(GtkWidget *wid, vtt_class *vtt)
{
	vtt->ec_set_enable(GTK_TOGGLE_BUTTON(wid)->active);
}

GtkSignalFunc ec_length_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->ec_set_length(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc ec_feedback_changed(GtkWidget *wid, vtt_class *vtt)
{
	vtt->ec_set_feedback(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc master_setup(GtkWidget *wid, vtt_class *vtt)
{
	vtt->set_sync_master(GTK_TOGGLE_BUTTON(wid)->active);
}

GtkSignalFunc client_setup(GtkWidget *wid, vtt_class *vtt)
{
	int client;
	int cycles;
	
	client=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(vtt->gui.sync_client));	
	cycles=(int) GTK_ADJUSTMENT(vtt->gui.cycles)->value;	
	
	vtt->set_sync_client(client, cycles);
}

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

#define connect_entry(wid, func); gtk_signal_connect(GTK_OBJECT(g->wid), "activate", (GtkSignalFunc) func, (void *) vtt);
#define connect_adj(wid, func); gtk_signal_connect(GTK_OBJECT(g->wid), "value_changed", (GtkSignalFunc) func, (void *) vtt);
#define connect_button(wid, func); gtk_signal_connect(GTK_OBJECT(g->wid), "clicked", (GtkSignalFunc) func, (void *) vtt);

void gui_connect_signals(vtt_class *vtt)
{
	vtt_gui *g=&vtt->gui;

	connect_entry(name, name_changed);
	connect_adj(volume, volume_changed);
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
	connect_adj(cycles, client_setup);
	
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
	
	connect_button(lp_enable, lp_enabled);
	connect_adj(lp_gain, lp_gain_changed);
	connect_adj(lp_reso, lp_reso_changed);
	connect_adj(lp_freq, lp_freq_changed);
	
	connect_button(ec_enable, ec_enabled);
	connect_adj(ec_length, ec_length_changed);
	connect_adj(ec_feedback, ec_feedback_changed);
}
	
void build_vtt_gui(vtt_class *vtt)
{
	GtkWidget *dummy;
	GtkWidget *label;
	GtkWidget *main_vbox;
	GtkWidget *main_hbox;
	GtkWidget *panel_hbox;
	GtkWidget *row_vbox;
	GSList *radio_group;
	char nice_name[FILENAME_BUTTON_MAX];
	
	vtt_gui *g;
	
//	puts("Creating");
	
	g=&vtt->gui;
	vtt->have_gui=1;
	g->frame=gtk_frame_new(vtt->name);
	gtk_container_set_border_width( GTK_CONTAINER(g->frame), 5);
	gtk_widget_show(g->frame);
	
	main_vbox=gtk_hbox_new(FALSE, 5);
	gtk_container_set_border_width( GTK_CONTAINER(main_vbox), 5);

	g->notebook=gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(g->notebook), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(main_vbox), g->notebook, WID_FIX);
	gtk_widget_show(g->notebook);

	g->display=gtk_tx_new(vtt->buffer, vtt->samples_in_buffer);
	gtk_box_pack_start(GTK_BOX(main_vbox), g->display, WID_DYN);
	gtk_widget_show(g->display);	
	
#ifdef USE_FLASH
	g->flash=gtk_tx_flash_new();
	gtk_box_pack_start(GTK_BOX(main_vbox), g->flash, WID_FIX);
	gtk_widget_show(g->flash);		
#endif	
	
	gtk_container_add(GTK_CONTAINER (g->frame), main_vbox);
	gtk_widget_show(main_vbox);
		
	begin_panel("Main");
	
	begin_row();
	
	g->name = gtk_entry_new_with_max_length(256);	
	gtk_entry_set_text(GTK_ENTRY(g->name), vtt->name);
	row_add_item_dyn(g->name);
	gtk_widget_set_usize(g->name, 40, g->name->requisition.height);
	
	g->del=gtk_button_new_with_label("Delete");
	row_add_item_fix(g->del);
	
	nicer_filename(nice_name, vtt->filename);
	g->file = gtk_button_new_with_label(nice_name);
	panel_add_item(g->file);
		
	panel_add_label("Volume:");
	
	g->volume=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->rel_volume, 0, 2, 0.01, 0.01, 0.01));
	panel_add_hscale(g->volume, 3);
	
	panel_add_label("Pitch:");
	
	g->pitch=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->rel_pitch, -3, +3, 0.1, 0.01, 0.001));
	panel_add_hscale(g->pitch, 2);
		
/*	g->clone=gtk_button_new_with_label("Clone");
	row_add_item_dyn(g->clone);*/
	
	begin_panel("Trigger");
	
	begin_row();
	
	g->trigger=gtk_button_new_with_label("Trigger!");
	row_add_item_dyn(g->trigger);
	
	g->stop=gtk_button_new_with_label("Stop.");
	row_add_item_dyn(g->stop);
	
	g->autotrigger=gtk_check_button_new_with_label("Autotrigger");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->autotrigger), vtt->autotrigger);
	panel_add_item(g->autotrigger);
	
	g->loop=gtk_check_button_new_with_label("Loop");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->loop), vtt->loop);
	panel_add_item(g->loop);
	
	g->sync_master=gtk_check_button_new_with_label("Sync Master");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->sync_master), vtt->is_sync_master);
	panel_add_item(g->sync_master);
	
	begin_row();
	
	g->sync_client=gtk_check_button_new_with_label("Sync Client");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->sync_client), vtt->is_sync_client);
	row_add_item_fix(g->sync_client);
	
	g->cycles=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->sync_cycles, 0, 10.0, 1,1,1));
	dummy=gtk_spin_button_new(g->cycles, 1.0, 0);
	row_add_item_dyn(dummy);
	
	begin_panel("X-Control");
		
	g->x_scratch=gtk_radio_button_new_with_label(NULL, "Scratching");
	radio_group=gtk_radio_button_group( GTK_RADIO_BUTTON(g->x_scratch));
	if (vtt->x_control==CONTROL_SCRATCH) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->x_scratch), 1);	
	panel_add_item(g->x_scratch);

	g->x_volume=gtk_radio_button_new_with_label(radio_group, "Volume");
	radio_group=gtk_radio_button_group( GTK_RADIO_BUTTON(g->x_volume));
	if (vtt->x_control==CONTROL_VOLUME) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->x_volume), 1);	
	panel_add_item(g->x_volume);
	
	g->x_lp_cutoff=gtk_radio_button_new_with_label(radio_group, "LP Cutoff Freq.");
	radio_group=gtk_radio_button_group( GTK_RADIO_BUTTON(g->x_lp_cutoff));
	if (vtt->x_control==CONTROL_CUTOFF) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->x_lp_cutoff), 1);	
	panel_add_item(g->x_lp_cutoff);
	
	g->x_ec_feedback=gtk_radio_button_new_with_label(radio_group, "Echo Feedback");
	radio_group=gtk_radio_button_group( GTK_RADIO_BUTTON(g->x_ec_feedback));
	if (vtt->x_control==CONTROL_FEEDBACK) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->x_ec_feedback), 1);	
	panel_add_item(g->x_ec_feedback);
	
	g->x_nothing=gtk_radio_button_new_with_label(radio_group, "Nothing");
	radio_group=gtk_radio_button_group( GTK_RADIO_BUTTON(g->x_nothing));
	if (vtt->x_control==CONTROL_NOTHING) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->x_nothing), 1);	
	panel_add_item(g->x_nothing);
		
	begin_panel("Y-Control");
	
	g->y_scratch=gtk_radio_button_new_with_label(NULL, "Scratching");
	radio_group=gtk_radio_button_group( GTK_RADIO_BUTTON(g->y_scratch));
	if (vtt->y_control==CONTROL_SCRATCH) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->y_scratch), 1);	
	panel_add_item(g->y_scratch);

	g->y_volume=gtk_radio_button_new_with_label(radio_group, "Volume");
	radio_group=gtk_radio_button_group( GTK_RADIO_BUTTON(g->y_volume));
	if (vtt->y_control==CONTROL_VOLUME) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->y_volume), 1);	
	panel_add_item(g->y_volume);
	
	g->y_lp_cutoff=gtk_radio_button_new_with_label(radio_group, "LP Cutoff Freq.");
	radio_group=gtk_radio_button_group( GTK_RADIO_BUTTON(g->y_lp_cutoff));
	if (vtt->y_control==CONTROL_CUTOFF) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->y_lp_cutoff), 1);	
	panel_add_item(g->y_lp_cutoff);
	
	g->y_ec_feedback=gtk_radio_button_new_with_label(radio_group, "Echo Feedback");
	radio_group=gtk_radio_button_group( GTK_RADIO_BUTTON(g->y_ec_feedback));
	if (vtt->y_control==CONTROL_FEEDBACK) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->y_ec_feedback), 1);	
	panel_add_item(g->y_ec_feedback);
	
	g->y_nothing=gtk_radio_button_new_with_label(radio_group, "Nothing");
	radio_group=gtk_radio_button_group( GTK_RADIO_BUTTON(g->y_nothing));
	if (vtt->y_control==CONTROL_NOTHING) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->y_nothing), 1);	
	panel_add_item(g->y_nothing);
		
	begin_panel("Lowpass");
	
	g->lp_enable=gtk_check_button_new_with_label("Enable");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->lp_enable), vtt->lp_enable);
	panel_add_item(g->lp_enable);
		
	panel_add_label("Input Gain:");
	
	g->lp_gain=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->lp_gain, 0, 2, 0.1, 0.01, 0.01));
	panel_add_hscale(g->lp_gain, 2);

	panel_add_label("Resonance:");
	
	g->lp_reso=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->lp_reso, 0, 0.99, 0.1, 0.01, 0.01));
	panel_add_hscale(g->lp_reso, 2);
	
	panel_add_label("Frequency:");
	
	g->lp_freq=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->lp_freq, 0, 1, 0.1, 0.01, 0.01));
	panel_add_hscale(g->lp_freq, 2);

	begin_panel("Echo");

	g->ec_enable=gtk_check_button_new_with_label("Enable");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g->ec_enable), vtt->ec_enable);
	panel_add_item(g->ec_enable);
	
	panel_add_label("Duration:");
	
	g->ec_length=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->ec_length, 0, 2, 0.1, 0.01, 0.001));
	panel_add_hscale(g->ec_length, 2);
	
	panel_add_label("Feedback:");
		
	g->ec_feedback=GTK_ADJUSTMENT(gtk_adjustment_new(vtt->ec_feedback, 0, 1, 0.1, 0.01, 0.001));
	panel_add_hscale(g->ec_feedback, 2);
	
	g->file_dialog=NULL;
	
	gui_connect_signals(vtt);
}

void gui_set_name(vtt_class *vtt, char *newname)
{
	gtk_frame_set_label(GTK_FRAME(vtt->gui.frame), newname);
	gtk_entry_set_text(GTK_ENTRY(vtt->gui.name), newname);
}

void gui_set_filename (vtt_class *vtt, char *newname)
{
	gtk_label_set(GTK_LABEL(GTK_BUTTON(vtt->gui.file)->child), newname);
}

void gui_update_display(vtt_class *vtt)
{
	if (vtt->buffer) gtk_tx_set_data(GTK_TX(vtt->gui.display), vtt->buffer, vtt->samples_in_buffer);
}

void destroy_gui(vtt_class *vtt)
{
	gtk_widget_destroy(vtt->gui.frame);
}

void recreate_gui(vtt_class *vtt, GtkWidget *daddy)
{
	build_vtt_gui(vtt);
	gtk_box_pack_start(GTK_BOX(daddy), vtt->gui.frame, TRUE, TRUE, 0);
	gtk_widget_show(vtt->gui.frame);
}

void delete_gui(vtt_class *vtt)
{
	gtk_widget_destroy(vtt->gui.frame);
	rebuild_vtts(1);
}

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
}
