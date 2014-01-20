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
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/    

#ifndef _h_tX_panel_
#define _h_tX_panel_

#include <gtk/gtk.h>

class tX_panel
{
	GtkWidget *container;
	GtkWidget *mainbox;
	GtkWidget *pixmap_min;
	GtkWidget *pixmap_max;
	GtkWidget *topbox;
	GtkWidget *clientbox;
	GtkWidget *clientframe;
	GtkWidget *labelbutton;
	GtkWidget *minbutton;
	GtkWidget *button_box;
	int client_hidden;
		
	public:
	tX_panel(const char *name, GtkWidget *par);
	~tX_panel();
	
	GtkWidget *get_widget() {return mainbox;};
	GtkWidget *get_labelbutton() {return minbutton;}
	void add_client_widget(GtkWidget *w);
	int is_hidden() { return client_hidden; }
	void hide(int i) { client_hidden=i; tX_panel::minimize(NULL, this); } 
	
	static void minimize(GtkWidget *w, tX_panel *p);
};

extern void tX_panel_make_label_bold(GtkWidget *widget);
#endif
