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

*/
    
#include "tX_panel.h"
#include "tX_pbutton.h"
#include <string.h>
#include <stdio.h>

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0

void tX_panel :: minimize(GtkWidget *w, tX_panel *p)
{
	if (!p->client_hidden) {
		gtk_widget_hide(p->pixmap_min);
		gtk_widget_show(p->pixmap_max);
		gtk_widget_hide(p->clientframe);
		p->client_hidden=1;
	} else {
		gtk_widget_hide(p->pixmap_max);
		gtk_widget_show(p->pixmap_min);
		gtk_widget_show(p->clientframe);
		p->client_hidden=0;
	}
		
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
	client_hidden=0;
	
  	container=par;
	minbutton=gtk_button_new();
	pixmap_min=tx_pixmap_widget(MINIMIZE);
	pixmap_max=tx_pixmap_widget(MAXIMIZE);
	labelbutton=gtk_label_new(name);
	gtk_misc_set_alignment(GTK_MISC(labelbutton), 0, 0.5);
	tX_panel_make_label_bold(labelbutton);
 
	button_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	       
	gtk_box_pack_start(GTK_BOX(button_box), pixmap_min, WID_FIX);
	gtk_box_pack_start(GTK_BOX(button_box), pixmap_max, WID_FIX);
	gtk_box_pack_start(GTK_BOX(button_box), labelbutton, WID_DYN);

	gtk_container_set_border_width(GTK_CONTAINER(button_box), 2);
	
	gtk_container_add (GTK_CONTAINER (minbutton), button_box);
	mainbox=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	
	topbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	clientbox=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	clientframe=gtk_frame_new((char *) NULL);
	gtk_container_set_border_width( GTK_CONTAINER(clientframe), 0);
	gtk_container_add(GTK_CONTAINER(clientframe), clientbox);
	
	gtk_box_pack_start(GTK_BOX(mainbox), topbox, WID_FIX);
	gtk_box_pack_start(GTK_BOX(mainbox), clientframe, WID_FIX);
	
	gtk_box_pack_start(GTK_BOX(topbox), minbutton, WID_DYN);
	
	gtk_widget_show(pixmap_min);
	gtk_widget_show(button_box);
	gtk_widget_show(labelbutton);
	gtk_widget_show(minbutton);
	gtk_widget_show(topbox);
	gtk_widget_show(clientbox);
	gtk_widget_show(clientframe);	
	gtk_widget_show(mainbox);
	
	g_signal_connect(G_OBJECT(minbutton), "clicked", (GCallback) tX_panel::minimize, (void *) this);
}

void tX_panel :: add_client_widget(GtkWidget *w)
{
	gtk_box_pack_start(GTK_BOX(clientbox), w, WID_FIX);
	gtk_widget_show(w);
}


tX_panel :: ~tX_panel()
{
	gtk_widget_destroy(minbutton);
	gtk_widget_destroy(clientbox);
	gtk_widget_destroy(clientframe);
	gtk_widget_destroy(topbox);
	gtk_widget_destroy(mainbox);
}
