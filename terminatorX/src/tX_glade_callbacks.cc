/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2011  Alexander König
 
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

*/    

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "tX_glade_callbacks.h"
#include "tX_glade_interface.h"
#include "tX_glade_support.h"
#include "tX_dialog.h"
#include "tX_global.h"
#include "tX_mastergui.h"
#include "tX_sequencer.h"

void
on_pref_cancel_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_destroy(opt_dialog);
}


void
on_pref_apply_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	apply_options(opt_dialog);	
}


void
on_pref_ok_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	apply_options(opt_dialog);
	gtk_widget_destroy(opt_dialog);
}


void
on_tx_options_destroy                  (GtkObject       *object,
                                        gpointer         user_data)
{
	opt_dialog=NULL;
}


void
on_alsa_buffer_time_value_changed      (GtkRange        *range,
                                        gpointer         user_data)
{
	GtkAdjustment *buffer_time=gtk_range_get_adjustment(GTK_RANGE(user_data));
	GtkAdjustment *period_time=gtk_range_get_adjustment(GTK_RANGE(range));

	period_time->upper=buffer_time->value;
	gtk_adjustment_changed(period_time);
}

void
on_pref_reset_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(opt_dialog->window), 
		GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		"Loose all your settings and set default values?");
	
	int res=gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
		
	if (res!=GTK_RESPONSE_YES) {
		return;
	}
	
	set_global_defaults();
	init_tx_options(opt_dialog);
}

void
on_del_mode_cancel_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_destroy(del_dialog);
	del_dialog=NULL;
}


void
on_del_mode_ok_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	tX_sequencer::del_mode mode;
	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(del_dialog, "all_events")))) {
		mode=tX_sequencer::DELETE_ALL;
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(del_dialog, "upto_current")))) {
		mode=tX_sequencer::DELETE_UPTO_CURRENT;		
	} else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(del_dialog, "from_current")))) {
		mode=tX_sequencer::DELETE_FROM_CURRENT;		
	} else {
		tX_error("Invalid tX_sequencer::del_mode selected.");
		return;
	}
	
	switch(menu_del_mode) {
		case ALL_EVENTS_ALL_TURNTABLES:
			sequencer.delete_all_events(mode);
			break;
		case ALL_EVENTS_FOR_TURNTABLE:
			sequencer.delete_all_events_for_vtt(del_vtt, mode);
			break;
		case ALL_EVENTS_FOR_SP:
			sequencer.delete_all_events_for_sp(del_sp, mode);
			break;
		default:
			tX_error("Invalid del_mode");
	}
	
	gtk_widget_destroy(del_dialog);
	del_dialog=NULL;
}

void
color_clicked                          (GtkButton       *button,
                                        gpointer         user_data)
{
	GdkColor p;
	GtkWidget *dialog=create_tX_color_selection();
	GtkWidget *sel=lookup_widget(dialog, "color_selection");
	g_object_set_data(G_OBJECT(dialog), "Button", button);
	
	gdk_color_parse((const char *) g_object_get_data(G_OBJECT(button), "Color"), &p);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(sel), &p);
	gtk_widget_show(dialog);
}


void
on_color_selection_ok_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	char tmp[56];
	char *col;
	GdkColor p;
	
	GtkWidget *dialog=gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(button))));
	GtkWidget *c_but=(GtkWidget *) g_object_get_data(G_OBJECT(dialog), "Button");
	
	GtkWidget *sel=lookup_widget(dialog, "color_selection");
	gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(sel), &p);

	col=(char *) g_object_get_data(G_OBJECT(c_but), "Color");
	sprintf(col, "#%02X%02X%02X", p.red >> 8, p.green >> 8, p.blue >> 8);	
	sprintf(tmp, "<span foreground=\"%s\"><b>%s</b></span>", col, col);
	gtk_label_set_markup(GTK_LABEL(gtk_container_get_children(GTK_CONTAINER(c_but))->data), tmp);
	
	gtk_widget_destroy(dialog);
}


void
on_color_selection_cancel_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_destroy(
		gtk_widget_get_parent(
			gtk_widget_get_parent(
				gtk_widget_get_parent(GTK_WIDGET(button))
			)
		)
	);
}
