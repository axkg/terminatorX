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
 
    File: tX_mastergui.cc
 
    Description: This implements the main (aka master) gtk+ GUI of terminatorX
    		 It serves as a container for the vtt-guis.
*/    

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <math.h>
#include <string.h>
#include "version.h"
#include "tX_global.h"
#include "tX_engine.h"
#include "tX_vttgui.h"
#include "tX_vtt.h"
#include "tX_flash.h"
#include "tX_smlog.c"
#include "tX_dialog.h"
#define MAX_ROWS 5

GtkWidget *tt_box[MAX_ROWS];
GtkWidget *tt_parent;
GtkWidget *main_window;
GtkWidget *wav_progress;
GtkWidget *grab_button;
GtkWidget *main_flash;
GtkWidget *rec_btn;
GtkAdjustment *volume_adj;
GtkAdjustment *pitch_adj;
int rec_dont_care=0;

#define connect_entry(wid, func, ptr); gtk_signal_connect(GTK_OBJECT(wid), "activate", (GtkSignalFunc) func, (void *) ptr);
#define connect_adj(wid, func, ptr); gtk_signal_connect(GTK_OBJECT(wid), "value_changed", (GtkSignalFunc) func, (void *) ptr);
#define connect_button(wid, func, ptr); gtk_signal_connect(GTK_OBJECT(wid), "clicked", (GtkSignalFunc) func, (void *) ptr);

Window xwindow;
#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0
extern int add_vtt(GtkWidget *);
extern void recreate_gui(vtt_class *vtt, GtkWidget *daddy);
extern void destroy_gui(vtt_class *vtt);
extern void gui_show_frame(vtt_class *vtt, int show);

GdkWindow *save_dialog_win=NULL;
GdkWindow *load_dialog_win=NULL;
GtkWidget *save_dialog=NULL;
GtkWidget *load_dialog=NULL;

GdkWindow *rec_dialog_win=NULL;
GtkWidget *rec_dialog=NULL;

int stop_update=0;

vtt_class *old_focus=NULL;

int grab_status=0;
int last_grab_status=0;

void tx_note(char *message);

gint pos_update(gpointer data)
{
	f_prec temp;

	if (stop_update) 
	{		
		cleanup_all_vtts();
		if (old_focus) gui_show_frame(old_focus, 0);
		old_focus=NULL;
		gtk_tx_flash_clear(main_flash);
		gdk_flush();	
		return(FALSE);
	}
	else
	{
		update_all_vtts();
		temp=vtt_class::mix_max;
		vtt_class::mix_max=0;
		gtk_tx_flash_set_level(main_flash, temp);
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
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(grab_button), 0);
			//	gtk_widget_draw_default(grab_button);
		}
		gdk_flush();	
		return(TRUE);
	}
}

void rebuild_vtts(int need_delete)
{
	int i,box;
	list <vtt_class *> :: iterator vtt;
	
	int vtts, vtt_per_box, boxes, maxboxes;
	
	if (need_delete)
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		destroy_gui((*vtt));
	}
	
	
	for (i=0; i<MAX_ROWS; i++)
	{
		if (tt_box[i])
		{
			//gtk_container_remove(GTK_CONTAINER(tt_parent), tt_box[i]);
			gtk_widget_destroy(tt_box[i]);
			tt_box[i]=NULL;
		}
	}

	vtt_per_box=globals.gui_wrap;
	vtts=vtt_class::main_list.size();
	if (vtts>1)
	{
	maxboxes=vtts/globals.gui_wrap;	
	if (vtts%globals.gui_wrap) maxboxes++;
	boxes=maxboxes;
		
	while (boxes<=maxboxes)
	{
		vtt_per_box--;
		boxes=vtts/vtt_per_box;
		if (vtts%vtt_per_box) boxes++;
	}
	
	vtt_per_box++;
	}
	
	for (i=0, vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); i++, vtt++)
	{
		box=i/vtt_per_box;

		if (!tt_box[box])
		{	
			tt_box[box]=gtk_vbox_new(FALSE, 0);	
			gtk_box_pack_start(GTK_BOX(tt_parent), tt_box[box], WID_DYN);
			gtk_widget_show(tt_box[box]); 
		}
		recreate_gui((*vtt), tt_box[box]);
	}
}

GtkSignalFunc new_table()
{
	int i;
	
	i=vtt_class::main_list.size()/globals.gui_wrap;
	
	if (i<MAX_ROWS)
	{
		if (!tt_box[i])
		{
			tt_box[i]=gtk_vbox_new(FALSE, 0);	
			gtk_box_pack_start(GTK_BOX(tt_parent), tt_box[i], WID_DYN);
			gtk_widget_show(tt_box[i]); 
		}
		add_vtt(tt_box[i]);				
	}
	rebuild_vtts(1);
}

/* Loading saved setups */

GtkSignalFunc cancel_load_tables(GtkWidget *wid)
{
	gtk_widget_destroy(load_dialog);
	load_dialog=NULL;
	load_dialog_win=NULL;
	return(0);
}

void do_load_tables(GtkWidget *wid)
{
	FILE *in;
	char buffer[PATH_MAX];
	
	strcpy(buffer, gtk_file_selection_get_filename(GTK_FILE_SELECTION(load_dialog)));
	strcpy(globals.tables_filename, buffer);
	
	gtk_widget_destroy(load_dialog);
	
	load_dialog=NULL;
	load_dialog_win=NULL;
	
	in=fopen(buffer, "r");
	
	if (in)
	{
		if (vtt_class::load_all(in)) tx_note("Error while reading file.");
		fclose(in);
		gtk_adjustment_set_value(volume_adj, globals.volume);
		gtk_adjustment_set_value(pitch_adj, globals.pitch);
	}
	else
	{
		tx_note("Failed to access file.");
	}
	rebuild_vtts(0);
}

GtkSignalFunc load_tables()
{
	if (load_dialog_win) 
	{
		gdk_window_raise(load_dialog_win);
		return 0;
	}
	
	load_dialog=gtk_file_selection_new("Load Turntables");	
	
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
	char *ext;
	
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
		if (vtt_class::save_all(out)) tx_note("Error while writing file.");
		fclose(out);
	}
	else
	{
		tx_note("Failed to access file.");
	}
	rebuild_vtts(0);
}

GtkSignalFunc save_tables()
{
	if (save_dialog_win) 
	{
		gdk_window_raise(save_dialog_win);
		return 0;
	}
	
	save_dialog=gtk_file_selection_new("Save Turntables");	
	
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
	vtt_class::set_master_volume(GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc master_pitch_changed(GtkWidget *wid, void *d)
{
	vtt_class::set_master_pitch (GTK_ADJUSTMENT(wid)->value);
}

GtkSignalFunc saturate_changed(GtkWidget *w, void *d)
{
	vtt_class::enable_saturate (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)));
}

GtkSignalFunc audio_on(GtkWidget *w, void *d)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
	{
		stop_update=0;
		run_engine();
		gtk_timeout_add(globals.update_idle, (GtkFunction) pos_update, NULL);
	}
	else
	{
		stop_engine();
		stop_update=1;
		
		if ((want_recording) && (!globals.autoname))
		{
			want_recording=0;
			rec_dont_care=1;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rec_btn), 0);
			rec_dont_care=0;
		}
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
		if (globals.autoname)
		{		
			globals.filectr++;
			sprintf(globals.record_filename, "%s%04i.wav", globals.prefix, globals.filectr);
			want_recording=1;
		}
		else
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

GtkSignalFunc grab_on(GtkWidget *w, void *d)
{
	grab_mouse(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)));
	grab_status=1;
}

void grab_off()
{
	grab_status=0;
}

GtkSignalFunc hide_clicked(GtkWidget *w, void *d)
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
	{
		show_all_guis(0);
	}
	else
	{
		show_all_guis(1);
	}
}

void quit()
{
	globals.width=main_window->allocation.width;
	globals.height=main_window->allocation.height;

	gtk_main_quit();
}

void mplcfitx()
/* Most Proabably Least Called Function In Terminator X :) */
{
	show_about(0);
}



#define add_sep(); 	dummy=gtk_hseparator_new ();\
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);\
	gtk_widget_show(dummy);\

#define add_sep2(); 	dummy=gtk_hseparator_new ();\
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);\
	gtk_widget_show(dummy);\


int create_mastergui(int x, int y)
{
	GtkWidget *main_vbox;
	GtkWidget *right_hbox;
	GtkWidget *left_hbox;
	GtkWidget *control_box;
	GtkAdjustment *dumadj;
	GtkWidget *dummy;
	GtkWidget *small_box;
	GtkWidget *smaller_box;
	int i;

	main_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(main_window), VERSIONSTRING);
	
	gtk_container_set_border_width(GTK_CONTAINER(main_window), 5);
	
	main_vbox=gtk_hbox_new(FALSE, 5);
	
	gtk_container_add(GTK_CONTAINER(main_window), main_vbox);
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
	
	dummy=gtk_toggle_button_new_with_label(" Audio Engine ");
	connect_button(dummy,audio_on, NULL);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	grab_button=gtk_toggle_button_new_with_label(" Mouse Grab ");
	gtk_box_pack_start(GTK_BOX(control_box), grab_button, WID_FIX);
	connect_button(grab_button, grab_on, NULL);
	gtk_widget_show(grab_button);

	dummy=gtk_check_button_new_with_label("Record");
	rec_btn=dummy;
	connect_button(dummy,tape_on, NULL);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	
	dummy=gtk_label_new("Master Volume:");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	dumadj=(GtkAdjustment*) gtk_adjustment_new(globals.volume, 0, 1.5, 0.001, 0.001, 0.01);
	volume_adj=dumadj;
	connect_adj(dumadj, master_volume_changed, NULL);	
	dummy=gtk_hscale_new(dumadj);
	gtk_scale_set_digits(GTK_SCALE(dummy), 2);
	gtk_scale_set_value_pos(GTK_SCALE(dummy), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_DYN);
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Master Pitch:");
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dumadj=(GtkAdjustment*) gtk_adjustment_new(globals.pitch, -3, 3, 0.001, 0.001, 0.01);
	pitch_adj=dumadj;
	connect_adj(dumadj, master_pitch_changed, NULL);	
	dummy=gtk_hscale_new(dumadj);
	gtk_scale_set_digits(GTK_SCALE(dummy), 2);
	gtk_scale_set_value_pos(GTK_SCALE(dummy), GTK_POS_LEFT);
	gtk_box_pack_start(GTK_BOX(control_box), dummy, WID_DYN);
	gtk_widget_show(dummy);
	
	dummy=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(left_hbox), dummy, WID_DYN);
	gtk_widget_show(dummy);
	
	tt_parent=dummy;
	
	tt_box[0]=gtk_vbox_new(FALSE, 0);	
	gtk_box_pack_start(GTK_BOX(dummy), tt_box[0], WID_DYN);
	gtk_widget_show(tt_box[0]); 
		
	tt_box[1]=NULL;
	tt_box[2]=NULL;
	
	dummy=gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(left_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	wav_progress = gtk_progress_bar_new();
	gtk_box_pack_start (GTK_BOX(left_hbox), wav_progress, WID_FIX);
	gtk_widget_show(wav_progress);

	dummy=gtk_vseparator_new();
	gtk_box_pack_start(GTK_BOX(main_vbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
		
	right_hbox=gtk_vbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), right_hbox, WID_FIX);
	gtk_widget_show(right_hbox);
	
	dummy=gtk_button_new_with_label("Add Turntable");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	gtk_signal_connect(GTK_OBJECT(dummy), "clicked", GtkSignalFunc(new_table), NULL);	

	dummy=gtk_button_new_with_label("Load Turntables");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	gtk_signal_connect(GTK_OBJECT(dummy), "clicked", GtkSignalFunc(load_tables), NULL);	
	
	dummy=gtk_button_new_with_label("Save Turntables");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	gtk_signal_connect(GTK_OBJECT(dummy), "clicked", GtkSignalFunc(save_tables), NULL);	

	add_sep();

	dummy=gtk_check_button_new_with_label("Saturate");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	connect_button(dummy, saturate_changed, NULL);
	
	dummy=gtk_check_button_new_with_label("Hide Gui");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	connect_button(dummy, hide_clicked, NULL);
	
	add_sep();
	
	dummy=gtk_button_new_with_label("Options");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	gtk_signal_connect (GTK_OBJECT(dummy), "clicked", (GtkSignalFunc) display_options, NULL);

	dummy=gtk_button_new_with_label("About");
	gtk_box_pack_start(GTK_BOX(right_hbox), dummy, WID_FIX);
	gtk_widget_show(dummy);
	gtk_signal_connect (GTK_OBJECT(dummy), "clicked", (GtkSignalFunc) mplcfitx, NULL);	
	
	add_sep();		

	small_box=gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(right_hbox), small_box, WID_DYN);
	gtk_widget_show(small_box);
	
	smaller_box=gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(small_box), smaller_box, WID_FIX);
	gtk_widget_show(smaller_box);

	GdkBitmap *mask;
	GtkStyle *style;
	GdkPixmap *pmap=NULL;
	GtkWidget *pwid;
	
	gtk_widget_realize(main_window);
	style = gtk_widget_get_style( main_window );
	pmap=gdk_pixmap_create_from_xpm_d(main_window->window, &mask, &style->bg[GTK_STATE_NORMAL], (gchar **) tx_smlog_xpm );
  	pwid = gtk_pixmap_new( pmap, mask );
	gtk_box_pack_start(GTK_BOX(smaller_box), pwid, WID_FIX);
	gtk_widget_show( pwid );
	

	dummy=gtk_label_new("14628");
	gtk_misc_set_alignment(GTK_MISC(dummy), 1, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Memory/kB:");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);
	
	add_sep2();

	dummy=gtk_label_new("4");
	gtk_misc_set_alignment(GTK_MISC(dummy), 1, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	dummy=gtk_label_new("Vtts:");
	gtk_misc_set_alignment(GTK_MISC(dummy), 0, 0.5);
	gtk_box_pack_end(GTK_BOX(smaller_box), dummy, WID_FIX);
	gtk_widget_show(dummy);

	add_sep2();

	dummy=gtk_label_new("3.5");
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
	
	main_flash=gtk_tx_flash_new();
	gtk_box_pack_end(GTK_BOX(small_box), main_flash, WID_DYN);
	gtk_widget_show(main_flash);

	gtk_window_set_default_size(GTK_WINDOW(main_window), x, y);	

	gtk_signal_connect (GTK_OBJECT(main_window), "destroy", (GtkSignalFunc) quit, NULL);	
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

void note_destroy(GtkWidget *widget, GtkWidget *mbox)
{
	gtk_widget_destroy(GTK_WIDGET(mbox));
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
	
	gtk_signal_connect(GTK_OBJECT(btn), "clicked", GtkSignalFunc(note_destroy), mbox);
	
	gtk_widget_show(mbox);
}

void display_mastergui()
{
	GtkWidget *top;
	
	gtk_widget_show(main_window);
	top=gtk_widget_get_toplevel(main_window);
	xwindow=GDK_WINDOW_XWINDOW(top->window);
}
