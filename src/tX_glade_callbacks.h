#include <gtk/gtk.h>

void
on_tx_options_destroy                  (GtkObject       *object,
                                        gpointer         user_data);

void
on_alsa_buffer_time_value_changed      (GtkRange        *range,
                                        gpointer         user_data);

void
on_pref_reset_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_pref_cancel_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_pref_apply_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_pref_ok_clicked                     (GtkButton       *button,
                                        gpointer         user_data);
