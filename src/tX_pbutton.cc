/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2014  Alexander König
 
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
 
    File: tX_pbutton.cc
 
    Description: This implements the pixmaped buttons - based on
                 gtk+ tutorial.

*/

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include "tX_mastergui.h"
#include "tX_pbutton.h"
#include "tX_global.h"

#include "icons/icons.pixbuf"

const guint8* tx_icons[ALL_ICONS];
long tx_icon_sizes[ALL_ICONS];

#define icon_init(id, data) { tx_icons[id]=data; tx_icon_sizes[id]=sizeof(data); }

void tx_icons_init() 
{
	icon_init(AUDIOENGINE, audioengine);
	icon_init(POWER, power);
	icon_init(GRAB, grab);
	icon_init(SEQUENCER, sequencer);
	icon_init(PLAY, play);
	icon_init(STOP, stop);
	icon_init(RECORD, record);
	icon_init(MIN_AUDIO, wave);
	icon_init(MIN_CONTROL, min_control);
	icon_init(MINIMIZE, minimize);
	icon_init(MAXIMIZE, maximize);
	icon_init(FX_UP, fx_up);
	icon_init(FX_DOWN, fx_down);
	icon_init(FX_CLOSE, fx_close);
	icon_init(MINIMIZE_PANEL, minimize_panel);
}

GtkWidget *tx_pixmap_widget(tX_icon id)
{
	GError *error;
	GdkPixbuf *pixbuf=gdk_pixbuf_new_from_inline(tx_icon_sizes[id], tx_icons[id], TRUE, &error);
	GtkWidget *widget=gtk_image_new();
	gtk_image_set_from_pixbuf(GTK_IMAGE(widget), pixbuf);

    return widget;
}

GtkWidget *tx_xpm_label_box(tX_icon id, const gchar *label_text, GtkWidget **labelwidget)
{
	GtkWidget *box1;
	GtkWidget *label;
	GtkWidget *pixmapwid;
	
	switch (globals.button_type) {
		case BUTTON_TYPE_TEXT:
			label = gtk_label_new(label_text);
			gtk_widget_show(label);
			if (labelwidget!=NULL) *labelwidget=label;
			return label;
			break;
		case BUTTON_TYPE_ICON:
			pixmapwid=tx_pixmap_widget(id);
			gtk_widget_show(pixmapwid);
			return pixmapwid;
			break;
		default:
			box1 = gtk_hbox_new (FALSE, 5);
			gtk_container_set_border_width (GTK_CONTAINER (box1), 2);
			pixmapwid=tx_pixmap_widget(id);
			gtk_box_pack_start (GTK_BOX (box1), pixmapwid, FALSE, FALSE, 0);
			gtk_widget_show(pixmapwid);
			label = gtk_label_new (label_text);
			gtk_box_pack_start (GTK_BOX (box1), label, FALSE, FALSE, 0);
			gtk_widget_show(label); 
			if (labelwidget!=NULL)  *labelwidget=label;
			return box1;
	}    
}

extern GtkWidget *tx_xpm_button_new(tX_icon id, const char *label, int toggle, GtkWidget **labelwidget)
{
	GtkWidget *box;
	GtkWidget *button;
	
	if (toggle) button=gtk_toggle_button_new();
	else button=gtk_button_new();
	
	box=tx_xpm_label_box(id, label, labelwidget);
	gtk_widget_show(box);
	gtk_container_add (GTK_CONTAINER (button), box);		
	
	return(button);
}
