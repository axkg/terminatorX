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
 
    File: tX_dialog.h
 
    Description: Header to tX_dialog.c
*/    

#ifndef _H_TX_DIALOG_
#define _H_TX_DIALOG_
//#include <gdk/gdk.h>
#include <gtk/gtk.h>
extern GtkWidget *opt_dialog;

extern void display_options();
extern void show_about(int nag);
extern void destroy_about();
extern void init_tx_options(GtkWidget *dialog);
extern void tX_set_icon(GtkWidget *widget);
extern void apply_options(GtkWidget *);
#endif
