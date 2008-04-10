/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2008  Alexander König
 
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
 
    File: tX_pbutton.h
 
    Description: Header to tX_pbutton.cc
*/ 

#ifndef _tx_pbutton_h_
#define _tx_pbutton_h_ 1

typedef enum {
	AUDIOENGINE,
	POWER,
	GRAB,
	SEQUENCER,
	PLAY,
	STOP,
	RECORD,
	MIN_AUDIO,
	MIN_CONTROL,
	MINIMIZE,
	MAXIMIZE,
	FX_UP,
	FX_DOWN,
	FX_CLOSE,
	MINIMIZE_PANEL,
	ALL_ICONS
} tX_icon;

extern GtkWidget *tx_pixmap_widget(tX_icon id);
extern void tx_icons_init();
extern GtkWidget *tx_xpm_label_box(tX_icon id, gchar *label_text, GtkWidget **labelwidget=(GtkWidget **) NULL);
extern GtkWidget *tx_xpm_button_new(tX_icon id, char *label, int toggle, GtkWidget **labelwidget=(GtkWidget **) NULL);
#endif
