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
