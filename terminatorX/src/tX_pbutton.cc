/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999, 2000  Alexander König
 
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
 
    File: tX_pbutton.cc
 
    Description: This implements the pixmaped buttons - based on
                 gtk+ tutorial.

*/

#include <gtk/gtk.h>
#include <stdio.h>
#include "tX_mastergui.h"
#include "tX_global.h"

#include "gui_icons/tx_audioengine.xpm"
#include "gui_icons/tx_power.xpm"
#include "gui_icons/tx_grab.xpm"
#include "gui_icons/tx_smaller_logo.xpm"
#include "gui_icons/tx_sequencer.xpm"
#include "gui_icons/tx_play.xpm"
#include "gui_icons/tx_stop.xpm"
#include "gui_icons/tx_record.xpm"
#include "gui_icons/tx_wave.xpm"
#include "gui_icons/tx_reload.xpm"
#include "gui_icons/tx_minimize.xpm"
#include "gui_icons/tX_fx_up.xpm"
#include "gui_icons/tX_fx_down.xpm"
#include "gui_icons/tX_fx_close.xpm"

gchar ** tx_icons[]={ tx_audioengine_xpm, tx_power_xpm, tx_grab_xpm, tx_smaller_logo_xpm,
		      tx_sequencer_xpm, tx_play_xpm, tx_stop_xpm, tx_record_xpm, 
		      tx_wave_xpm, tx_reload_xpm, tx_minimize_xpm,
		      tX_fx_up_xpm, tX_fx_down_xpm, tX_fx_close_xpm };

GtkWidget *tx_pixmap_widget(int icon_id)
{
    GtkWidget *pixmapwid;
    GdkPixmap *pixmap;
    GdkBitmap *mask;
    GtkStyle *style;

	// printf("id: %i, addr: %08x\n", icon_id, tx_icons[icon_id]);

    /* Get the style of the button to get the
     * background color. */
    style = gtk_widget_get_style(main_window);

    /* Now on to the xpm stuff */
    pixmap = gdk_pixmap_create_from_xpm_d(main_window->window, &mask,
					 &style->bg[GTK_STATE_NORMAL],
					 (gchar **) tx_icons[icon_id]);
    pixmapwid = gtk_pixmap_new (pixmap, mask);

    return pixmapwid;
}

GtkWidget *tx_xpm_label_box(int	icon_id, gchar *label_text )
{
    GtkWidget *box1;
    GtkWidget *label;
    GtkWidget *pixmapwid;

    /* Create box for xpm and label */
    box1 = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box1), 2);


    if (globals.button_type != BUTTON_TYPE_TEXT)
    {
	    pixmapwid=tx_pixmap_widget(icon_id);
	    gtk_box_pack_start (GTK_BOX (box1), pixmapwid, FALSE, FALSE, 3);
	    gtk_widget_show(pixmapwid);
    }

    if (globals.button_type != BUTTON_TYPE_ICON)
    {
	    label = gtk_label_new (label_text);
	    gtk_box_pack_start (GTK_BOX (box1), label, FALSE, FALSE, 3);
	    gtk_widget_show(label); 
    }

    return(box1);
}

GtkWidget *tx_xpm_button_new(int icon_id, char *label, int toggle)
{
	GtkWidget *box;
	GtkWidget *button;
	
	if (toggle) button=gtk_toggle_button_new();
	else button=gtk_button_new();
	
	box=tx_xpm_label_box(icon_id, label);
	gtk_widget_show(box);
	gtk_container_add (GTK_CONTAINER (button), box);		
	
	return(button);
}

