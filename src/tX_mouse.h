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
#include <glib.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#define KEY_Alt_L 1
#define KEY_Alt_R 2
#define KEY_BackSpace 3
#define KEY_Control_L 4
#define KEY_Control_R 5
#define KEY_Escape 6
#define KEY_f 7
#define KEY_F1 8
#define KEY_F10 9
#define KEY_F11 10
#define KEY_F12 11
#define KEY_F2 12
#define KEY_F3 13
#define KEY_F4 14
#define KEY_F5 15
#define KEY_F6 16
#define KEY_F7 17
#define KEY_F8 18
#define KEY_F9 19
#define KEY_m  20
#define KEY_Return 21
#define KEY_s 22
#define KEY_space 23
#define KEY_Tab 24
#define KEY_w 25
#define MAX_KEY 26

typedef struct __attribute__((__packed__)) {
	uint8_t buttons;
	int8_t x_motion;
	int8_t y_motion;
} tx_input_event;

class tx_mouse
{
	bool warp_override;
	GdkEventMask savedEventMask;

	gboolean enable_auto_mnemonics;
	guint last_button_press;
	guint last_button_release;

	GdkDevice* pointer;
	GdkDevice* keyboard;
	GdkScreen* screen;
	GdkWindow* window;
	GdkCursor* blankCursor;
	GdkCursor* cursor;
	gdouble x_abs;
	gdouble y_abs;
	gint x_restore;
	gint y_restore;

	float warp;
	
	int grabbed;
	bool key_pressed[MAX_KEY];
	
	enum  {
		FALLBACK,
		LINUX_INPUT
	} grab_mode;
	
	GIOChannel *linux_input_channel;
	guint linux_input_watch;

	bool press_key(int keycode) {
		if (!key_pressed[keycode]) {
			key_pressed[keycode] = true;
			return true;
		}
		return false;
	}

	bool release_key(int keycode) {
		if (key_pressed[keycode]) {
			key_pressed[keycode] = false;
			return true;
		}
		return false;
	}

	public:
	int set_xinput();
	void reset_xinput();
	bool is_grabbed() { return grabbed != 0; }
		
	int grab();
	int grab_linux_input();
		
	int check_event();
	void ungrab();
	void ungrab_linux_input();
	tx_mouse();
	
	void motion_notify(GtkWidget *widget, GdkEventMotion *eventMotion);
	void button_press(GtkWidget *widget, GdkEventButton *eventButton);
	void button_release(GtkWidget *widget, GdkEventButton *eventButton);
	void key_press(GtkWidget *widget, GdkEventKey *eventKey);
	void key_release(GtkWidget *widget, GdkEventKey *eventKey);
	void linux_input(tx_input_event *event);

	static gboolean motion_notify_wrap(GtkWidget *widget, GdkEventMotion *eventMotion, void *data);
	static gboolean button_press_wrap(GtkWidget *widget, GdkEventButton *eventButton, void *data);
	static gboolean button_release_wrap(GtkWidget *widget, GdkEventButton *eventButton, void *data);
	static gboolean key_press_wrap(GtkWidget *widget, GdkEventKey *eventKey, void *data);
	static gboolean key_release_wrap(GtkWidget *widget, GdkEventKey *eventKey, void *data);
	static gboolean linux_input_wrap(GIOChannel *source, GIOCondition condition, gpointer data);

};


#endif
