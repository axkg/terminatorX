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

    File: tX.h

    Description: Header to tX.cc

    For a closer description of the methods see implementation (tX.cc).
*/

#ifndef _H_TX
#define _H_TX

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>

class tX_Window;
class Virtual_TurnTable;

#include "turntable.h"
#include "tX_types.h"

class tX_Window
{
	XColor col_data;
	XColor col_frame;
	XColor col_pos;
	XColor col_activebtn;
	XColor col_inactivebtn;
	XColor col_marker;

	Font font;
	
	unsigned long black;
	unsigned long white;
	
	int verbose;
	
	Display *disp;	
	Window win;
	XSetWindowAttributes set_attr;
	GC gc;
	XGCValues xgcv;
	long event_mask;	
	XEvent xev;	
	XKeyEvent *xkeyev;
	XMotionEvent *xmotev;
	
	int modex, modey, bt_w, bt_h, bt_a;
	
	int samples_per_pixel;
	int mouse_border;
	f_prec mouse_speed;
	int sense_cycles;
	char s_name[256];
	char l_name[256];
	
	int width;
	int height;
	int medx;
	int medy;
	int warpx_max;
	int warpx_min;
	
	int data_disp_x;
	int data_disp_y;
	
	int data_disp_width;
	int data_disp_height;	
	
	unsigned int data_samples;
	int16_t *data_ptr;

	int pos_y;
	int pos_max_x;
	
	int motor_on;
	int first_mot_event;

#ifdef USE_STOPSENSE
	int mouse_busy;
#endif
	
	int y0;
	
	int ptrwarped;
	
	int quit;	
	
	Virtual_TurnTable *vtt;

	int mx,my;
	Time mtime;
	
	public:
	int playback_active;
	
	tX_Window(Virtual_TurnTable *, int, int, int, int, f_prec, int, char *, char *);
	~tX_Window();
	
	void prepare_data_display(int16_t *, unsigned int);
	void display_data();
	void display_mode();
	void display_action();
	void display_text();
	void set_pos(int pos);
	int get_samples_per_pixel();

	int event_wait();	
	int event_check();
	void handle_event();
	void handle_KeyPress();
	void handle_KeyRelease();
	void handle_Motion();
	void repaint();
	void loop();
};

#endif












