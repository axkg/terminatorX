/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2004  Alexander König
 
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
    
#include "tX_panel.h"
#include "tX_pbutton.h"
#include <string.h>
#include <stdio.h>

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0

void tX_panel :: minimize(GtkWidget *w, tX_panel *p)
{
	p->client_hidden=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->minbutton));
	
	if (p->client_hidden)
		gtk_widget_hide(p->clientframe);
	else
		gtk_widget_show(p->clientframe);
		
	gboolean expand;
	gboolean fill;
	guint padding;
	GtkPackType pack_type;
		
	if (p->container) {
		gtk_box_query_child_packing(GTK_BOX(p->container), p->mainbox,
									&expand, &fill, &padding, &pack_type);
		gtk_box_set_child_packing(GTK_BOX(p->container), p->mainbox,
									expand, fill, padding, pack_type);
		gtk_container_check_resize(GTK_CONTAINER(p->container));			    
	}
}

void tX_panel_make_label_bold(GtkWidget *widget) {
	char label[128];	
	sprintf(label, "<b>%s</b>", gtk_label_get_text(GTK_LABEL(widget)));
	gtk_label_set_markup(GTK_LABEL (widget), label);
}

tX_panel :: tX_panel (const char *name, GtkWidget *par)
{
	GtkWidget *pixmap;
	client_hidden=0;
	
	container=par;
	minbutton=gtk_toggle_button_new();
	pixmap=tx_pixmap_widget(TX_ICON_MINIMIZE);
	gtk_container_add (GTK_CONTAINER (minbutton), pixmap);
	labelbutton=gtk_button_new_with_label(name);
	gtk_container_foreach(GTK_CONTAINER(labelbutton), (GtkCallback) tX_panel_make_label_bold, NULL);
	mainbox=gtk_vbox_new(FALSE, 0);
	
	topbox=gtk_hbox_new(FALSE, 0);
	clientbox=gtk_vbox_new(FALSE, 0);
	clientframe=gtk_frame_new((char *) NULL);
	gtk_container_set_border_width( GTK_CONTAINER(clientframe), 0);
	gtk_container_add(GTK_CONTAINER(clientframe), clientbox);
	
	gtk_box_pack_start(GTK_BOX(mainbox), topbox, WID_FIX);
	gtk_box_pack_start(GTK_BOX(mainbox), clientframe, WID_FIX);
	
	gtk_box_pack_start(GTK_BOX(topbox), labelbutton, WID_DYN);
	gtk_box_pack_start(GTK_BOX(topbox), minbutton, WID_FIX);
	
	gtk_widget_show(pixmap);
	gtk_widget_show(labelbutton);
	gtk_widget_show(minbutton);
	gtk_widget_show(topbox);
	gtk_widget_show(clientbox);
	gtk_widget_show(clientframe);	
	gtk_widget_show(mainbox);
	
	g_signal_connect(G_OBJECT(minbutton), "clicked", (GtkSignalFunc) tX_panel::minimize, (void *) this);
}

void tX_panel :: add_client_widget(GtkWidget *w)
{
	gtk_box_pack_start(GTK_BOX(clientbox), w, WID_FIX);
	gtk_widget_show(w);
}


tX_panel :: ~tX_panel()
{
	gtk_widget_destroy(minbutton);
	gtk_widget_destroy(labelbutton);
	gtk_widget_destroy(clientbox);
	gtk_widget_destroy(clientframe);
	gtk_widget_destroy(topbox);
	gtk_widget_destroy(mainbox);
}
