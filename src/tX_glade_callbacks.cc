#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "tX_glade_callbacks.h"
#include "tX_glade_interface.h"
#include "tX_glade_support.h"
#include "tX_dialog.h"

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

