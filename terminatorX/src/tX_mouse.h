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
 
    File: tX_mouse.h
 
    Description: Header to tX_mouse.cc
*/    

#ifndef _h_tx_mouse 
#define _h_tx_mouse

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tX_types.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/keysym.h>
#include <glib.h>
#include <gdk/gdk.h>

class tx_mouse
{
	XID OrgXPointer;
	char OrgXPointerName[256];
	XDevice *input_device;
	XEvent xev;
	long mask;
	XMotionEvent *xmot;
	XKeyEvent *xkey;
	XButtonEvent *xbut;
	bool warp_override;
	GdkEventMask savedEventMask;

#ifdef USE_DGA2	
	XEvent xev_copy;
	XDGAButtonEvent *xdgabut;
	XDGAKeyEvent *xdgakey;
	XDGAMotionEvent *xdgamot;
#endif	
	
	Time otime, ntime;
	f_prec dtime;
	Display *dpy;
	KeySym key;
	float warp;
	
	public:
	
	int grabbed;

	int set_xinput();
	void reset_xinput();
		
	int grab();
	int check_event();
	void ungrab();
	tx_mouse();
	
	void motion_notify(GtkWidget *widget, GdkEventMotion *eventMotion);
	void button_press(GtkWidget *widget, GdkEventButton *eventButton);
	void button_release(GtkWidget *widget, GdkEventButton *eventButton);
	void key_press(GtkWidget *widget, GdkEventKey *eventKey);
	void key_release(GtkWidget *widget, GdkEventKey *eventKey);

	static void motion_notify_wrap(GtkWidget *widget, GdkEventMotion *eventMotion, void *data);
	static void button_press_wrap(GtkWidget *widget, GdkEventButton *eventButton, void *data);
	static void button_release_wrap(GtkWidget *widget, GdkEventButton *eventButton, void *data);
	static void key_press_wrap(GtkWidget *widget, GdkEventKey *eventKey, void *data);
	static void key_release_wrap(GtkWidget *widget, GdkEventKey *eventKey, void *data);

	private:
	void set_x_pointer(char*);
};


#endif
