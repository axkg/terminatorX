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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

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

	gboolean enable_auto_mnemonics;
	guint last_button_press;
	guint last_button_release;

	GdkDevice* pointer;
	GdkDevice* keyboard;

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
	
	int grabbed;

	public:
	int set_xinput();
	void reset_xinput();
	bool is_grabbed() { return grabbed != 0; }
		
	int grab();
	int check_event();
	void ungrab();
	tx_mouse();
	
	void motion_notify(GtkWidget *widget, GdkEventMotion *eventMotion);
	void button_press(GtkWidget *widget, GdkEventButton *eventButton);
	void button_release(GtkWidget *widget, GdkEventButton *eventButton);
	void key_press(GtkWidget *widget, GdkEventKey *eventKey);
	void key_release(GtkWidget *widget, GdkEventKey *eventKey);

	static gboolean motion_notify_wrap(GtkWidget *widget, GdkEventMotion *eventMotion, void *data);
	static gboolean button_press_wrap(GtkWidget *widget, GdkEventButton *eventButton, void *data);
	static gboolean button_release_wrap(GtkWidget *widget, GdkEventButton *eventButton, void *data);
	static gboolean key_press_wrap(GtkWidget *widget, GdkEventKey *eventKey, void *data);
	static gboolean key_release_wrap(GtkWidget *widget, GdkEventKey *eventKey, void *data);

	private:
	void set_x_pointer(char*);
};


#endif
