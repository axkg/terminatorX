/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2002  Alexander König
 
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

#define TX_ICON_AUDIOENGINE 0
#define TX_ICON_POWER 1
#define TX_ICON_GRAB 2
#define TX_ICON_LOGO 3
#define TX_ICON_SEQUENCER 4
#define TX_ICON_PLAY 5
#define TX_ICON_STOP 6
#define TX_ICON_RECORD 7
#define TX_ICON_EDIT 8
#define TX_ICON_MIN_AUDIO 8
#define TX_ICON_RELOAD 9
#define TX_ICON_MINIMIZE 10
#define TX_ICON_FX_UP 11
#define TX_ICON_FX_DOWN 12
#define TX_ICON_FX_CLOSE 13
#define TX_ICON_MINIMIZE_PANEL 14
#define TX_ICON_MIN_CONTROL 15

extern GtkWidget *tx_pixmap_widget(int icon_id);
extern GtkWidget *tx_xpm_label_box(int icon_id, gchar *label_text, GtkWidget **labelwidget=NULL);
extern GtkWidget *tx_xpm_button_new(int icon_id, char *label, int toggle, GtkWidget **labelwidget=NULL);
#endif
