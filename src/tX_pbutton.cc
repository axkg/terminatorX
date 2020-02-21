/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2016  Alexander König
 
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

#include "icons/tX_icons_resources.c"

const char* tx_icons[ALL_ICONS];

int tx_icon_size=20;

void tx_icons_init(int size) 
{
	tx_icon_size=size;

	tx_icons[AUDIOENGINE] = "audio-speakers-symbolic";
	tx_icons[POWER] = "system-shutdown-symbolic";
	tx_icons[GRAB] = "input-mouse-symbolic";	

	tx_icons[SEQUENCER] = "emblem-music-symbolic";
	
	tx_icons[PLAY] = "media-playback-start-symbolic";
	tx_icons[STOP] = "media-playback-stop-symbolic";
	tx_icons[RECORD] = "media-record-symbolic";
	tx_icons[MIN_AUDIO] = "audio-x-generic-symbolic";
	tx_icons[MIN_CONTROL] = "multimedia-volume-control-symbolic";
	
	tx_icons[MINIMIZE] = "go-top-symbolic";
	tx_icons[MAXIMIZE] = "go-bottom-symbolic";
	tx_icons[FX_UP] = "go-up-symbolic";
	tx_icons[FX_DOWN] = "go-down-symbolic";
	tx_icons[FX_CLOSE] = "window-close-symbolic";
	tx_icons[MINIMIZE_PANEL] = "window-minimize-symbolic";
}

GtkWidget *tx_pixmap_widget(tX_icon id)
{
	return gtk_image_new_from_icon_name (tx_icons[id], GTK_ICON_SIZE_SMALL_TOOLBAR);
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
			box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
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
