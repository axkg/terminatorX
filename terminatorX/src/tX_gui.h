/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999  Alexander König
 
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
 
    File: tX_gui.h
 
    Description: Header to tX_gui.c
*/    

#ifndef _TX_GUI_H_
#define _TX_GUI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtk/gtk.h>
#include <X11/Xlib.h>

extern Window xwindow;
extern GtkWidget* window;
extern GtkWidget *action_btn;
extern GtkWidget *wav_display;

extern void create_gui(int x, int y);
extern void display_gui();
extern void tx_note(char *message);
extern void wav_progress_update(gfloat);

#define DISK_IDLE 0
#define DISK_WRITING 1
#define DISK_READING 2

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
