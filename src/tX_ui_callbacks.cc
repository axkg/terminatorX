/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2022  Alexander König

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

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include "tX_dialog.h"
#include "tX_global.h"
#include "tX_maingui.h"
#include "tX_sequencer.h"
#include "tX_ui_callbacks.h"
#include "tX_ui_interface.h"
#include "tX_ui_support.h"

void on_pref_cancel_clicked(GtkButton* button,
    gpointer user_data) {
    gtk_widget_destroy(opt_dialog);
}

void on_pref_apply_clicked(GtkButton* button,
    gpointer user_data) {
    apply_options(opt_dialog);
}

void on_pref_ok_clicked(GtkButton* button,
    gpointer user_data) {
    apply_options(opt_dialog);
    gtk_widget_destroy(opt_dialog);
}

void on_tx_options_destroy(GObject* object,
    gpointer user_data) {
    opt_dialog = NULL;
}

void on_alsa_buffer_time_value_changed(GtkRange* range,
    gpointer user_data) {
    GtkAdjustment* buffer_time = gtk_range_get_adjustment(GTK_RANGE(user_data));
    GtkAdjustment* period_time = gtk_range_get_adjustment(GTK_RANGE(range));

    gtk_adjustment_set_upper(period_time, gtk_adjustment_get_value(buffer_time));
}

void on_pref_reset_clicked(GtkButton* button,
    gpointer user_data) {
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(opt_dialog)),
        GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "Loose all your settings and set default values?");

    int res = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (res != GTK_RESPONSE_YES) {
        return;
    }

    set_global_defaults();
    init_tx_options(opt_dialog);
}

void on_del_mode_cancel_clicked(GtkButton* button,
    gpointer user_data) {
    gtk_widget_destroy(del_dialog);
    del_dialog = NULL;
}

void on_del_mode_ok_clicked(GtkButton* button,
    gpointer user_data) {
    tX_sequencer::del_mode mode;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(del_dialog, "all_events")))) {
        mode = tX_sequencer::DELETE_ALL;
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(del_dialog, "upto_current")))) {
        mode = tX_sequencer::DELETE_UPTO_CURRENT;
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(del_dialog, "from_current")))) {
        mode = tX_sequencer::DELETE_FROM_CURRENT;
    } else {
        tX_error("Invalid tX_sequencer::del_mode selected.");
        return;
    }

    switch (menu_del_mode) {
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
    del_dialog = NULL;
}

void color_clicked(GtkButton* button,
    gpointer user_data) {
    GdkRGBA p;
    GtkWidget* dialog = create_tX_color_chooser(gtk_widget_get_toplevel(GTK_WIDGET(button)));
    GtkWidget* sel = lookup_widget(dialog, "tX_color_chooser");
    g_object_set_data(G_OBJECT(dialog), "Button", button);

    gdk_rgba_parse(&p, (const char*)g_object_get_data(G_OBJECT(button), "Color"));
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(sel), &p);
    gtk_widget_show(dialog);
}

void color_response(GtkDialog* dialog, gint response_id, gpointer user_data) {
    if (response_id == GTK_RESPONSE_OK) {
        char tmp[128];
        GtkWidget* c_but = (GtkWidget*)g_object_get_data(G_OBJECT(dialog), "Button");
        char* col = (char*)g_object_get_data(G_OBJECT(c_but), "Color");
        GdkRGBA p;

        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &p);
        sprintf(col, "#%02X%02X%02X", (int)(p.red * 255.0), (int)(p.green * 255.0), (int)(p.blue * 255.0));
        sprintf(tmp, "<span foreground=\"%s\"><b>%s</b></span>", col, col);
        gtk_label_set_markup(GTK_LABEL(gtk_container_get_children(GTK_CONTAINER(c_but))->data), tmp);
    }

    gtk_widget_hide(GTK_WIDGET(dialog));
    gtk_widget_destroy(GTK_WIDGET(dialog));
}
