#pragma once

#include <gtk/gtk.h>

void on_tx_options_destroy(GObject* object,
    gpointer user_data);

void on_alsa_buffer_time_value_changed(GtkRange* range,
    gpointer user_data);

void on_pref_reset_clicked(GtkButton* button,
    gpointer user_data);

void on_pref_cancel_clicked(GtkButton* button,
    gpointer user_data);

void on_pref_apply_clicked(GtkButton* button,
    gpointer user_data);

void on_pref_ok_clicked(GtkButton* button,
    gpointer user_data);

void on_del_mode_cancel_clicked(GtkButton* button,
    gpointer user_data);

void on_del_mode_ok_clicked(GtkButton* button,
    gpointer user_data);

void color_clicked(GtkButton* button,
    gpointer user_data);

void on_color_chooser_ok_clicked(GtkButton* button,
    gpointer user_data);

void color_response(GtkDialog* dialog, gint response_id, gpointer user_data);
