/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2021  Alexander König

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

#ifndef _h_tX_panel_
#define _h_tX_panel_

#include <gtk/gtk.h>

class vtt_fx;

class tX_panel
{
	GtkWidget *list_box_row;
	GtkWidget *drag_handle;
	GtkWidget *mainbox;
	GtkWidget *topbox;
	GtkWidget *clientbox;
	GtkWidget *clientframe;
	GtkWidget *labelbutton;
	GtkWidget *minimize_button;
	GtkWidget *maximize_button;
	GtkWidget *close_button;
	GtkWidget *button_box;
	GtkWidget *controlbox;
	GtkWidget *add_drywet_button;
	GtkWidget *remove_drywet_button;
	int client_hidden;

	public:
	tX_panel(const char *name, GtkWidget *controlbox, GCallback close_callback = NULL, vtt_fx* effect = NULL);
	~tX_panel();

	GtkWidget *get_widget() {return mainbox;};
	GtkWidget *get_list_box_row() {return list_box_row;};
	GtkWidget *get_labelbutton() {return labelbutton;}
	GtkWidget *get_add_drywet_button() { return add_drywet_button; }
	GtkWidget *get_remove_drywet_button() { return remove_drywet_button; }

	void add_client_widget(GtkWidget *w);
	int is_hidden() { return client_hidden; }
	void hide(int i) { client_hidden=i; tX_panel::minimize(NULL, this); }

	static void minimize(GtkWidget *w, tX_panel *p);
};

extern void tX_panel_make_label_bold(GtkWidget *widget);
#endif
