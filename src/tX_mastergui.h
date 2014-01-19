/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2011  Alexander König
 
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
 
    File: tX_mastergui.h
 
    Description: Header to tX_mastergui.cc
*/    

#ifndef _h_tx_mastergui
#define _h_tx_mastergui 1

#include <gtk/gtk.h>
#include <X11/Xlib.h>

#include "tX_seqpar.h"

extern int audioon;
extern Window x_window;
extern GdkWindow* top_window;

extern GtkWidget *main_window;

extern GtkAdjustment *volume_adj;
extern GtkAdjustment *pitch_adj;
extern GtkWidget *main_flash;

extern tX_seqpar_master_volume sp_master_volume;
extern tX_seqpar_master_pitch sp_master_pitch;

extern void create_mastergui(int x, int y);
extern void wav_progress_update(gfloat percent);
extern void note_destroy(GtkWidget *widget, GtkWidget *mbox);
extern void tx_note(const char *message, bool isError=false, GtkWindow *window=NULL);
extern void tx_l_note(const char *message);
extern void display_mastergui();
extern void grab_off();
extern void mg_update_status();
extern void load_tt_part(char *);
extern void seq_update();

extern GtkWidget *control_parent;
extern GtkWidget *audio_parent;

extern void gui_set_tooltip(GtkWidget *wid, const char *tip);

extern GtkWidget *panel_bar;

void add_to_panel_bar(GtkWidget *);
void remove_from_panel_bar(GtkWidget *);

typedef enum {
	ALL_EVENTS_ALL_TURNTABLES,
	ALL_EVENTS_FOR_TURNTABLE,
	ALL_EVENTS_FOR_SP
} tx_menu_del_mode;

extern GtkWidget *del_dialog;
extern tx_menu_del_mode menu_del_mode;
extern tX_seqpar *del_sp;
extern vtt_class *del_vtt;

class tX_cursor {
	public:
	enum cursor_shape {
		DEFAULT_CURSOR,
		WAIT_CURSOR,
		WAIT_A_SECOND_CURSOR,
		MAX_CURSOR
	};
	
	private:
	static GdkCursor *cursors[MAX_CURSOR];
	static cursor_shape current_shape;
	
	public:
	static void set_cursor(cursor_shape shape);
	static void reset_cursor();
	static GdkCursor *get_cursor();
};

extern GCallback menu_delete_all_events_for_sp(GtkWidget *, tX_seqpar *sp);
extern bool tX_shutdown;
#endif
