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
 
    File: tX_mouse.cc
 
    Description: This implements the mouse AND keyboard Input handling in
    		 Grab-Mode.
*/

#include <sys/wait.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdint.h>

#include <config.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "tX_mouse.h"
#include "tX_mastergui.h"
#include "tX_global.h"
#include "tX_engine.h"
#include "tX_vtt.h"
#include <stdlib.h>
#include <string.h>

#define TX_MOUSE_SPEED_NORMAL 0.05
#define TX_MOUSE_SPEED_WARP 250000

tx_mouse :: tx_mouse()
{
	pointer = NULL;
	keyboard = NULL;
	linux_input_channel = NULL;
	grab_mode = LINUX_INPUT;
	
	grabbed=0;
	warp=TX_MOUSE_SPEED_NORMAL;
	enable_auto_mnemonics = FALSE;

	last_button_press = 0;
	last_button_release = 0;
}


int tx_mouse :: grab()
{	
	if (grabbed) return 0;

	// release all keys
	memset(key_pressed, 0, sizeof(key_pressed));

	warp_override=false;
	
	if (grab_linux_input()) {
		grab_mode = LINUX_INPUT;
	} else {
		grab_mode = FALLBACK;
	}
	
	window =  gtk_widget_get_window(main_window);
	GdkDisplay* gdk_dpy = gdk_window_get_display(window);
	GdkDeviceManager *device_manager = gdk_display_get_device_manager(gdk_dpy);
	
	if (!gdk_dpy) {
		fputs("GrabMode Error: couldn't access GDKDisplay.", stderr);
		return(ENG_ERR_XOPEN);
	}

	gtk_window_present(GTK_WINDOW(main_window));

	savedEventMask = gdk_window_get_events(window);
	GdkEventMask newEventMask = GdkEventMask ((int) savedEventMask | GDK_POINTER_MOTION_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gdk_window_set_events(top_window, newEventMask);

	g_object_get (gtk_widget_get_settings (main_window), "gtk-auto-mnemonics", &enable_auto_mnemonics, NULL);

	if (enable_auto_mnemonics) {
		gboolean off = FALSE;
		g_object_set (gtk_widget_get_settings (main_window), "gtk-auto-mnemonics", off, NULL);
	}

	pointer = gdk_device_manager_get_client_pointer(device_manager);
	GdkGrabStatus grab_status = gdk_device_grab(pointer, top_window, GDK_OWNERSHIP_APPLICATION, FALSE, GdkEventMask (GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK), NULL, GDK_CURRENT_TIME);

	gdk_device_get_position(pointer, &screen, &x_restore, &y_restore);	
	
	gint x = gdk_screen_get_width(screen) / 2;
	gint y = gdk_screen_get_height(screen) / 2;
	
	x_abs = x;
	y_abs = y;
	
	gdk_device_warp(pointer, screen, x_abs, y_abs);

	if (grab_status != GDK_GRAB_SUCCESS) {
		return(-1);
	}	
	
	GList *list = gdk_device_manager_list_devices (device_manager, GDK_DEVICE_TYPE_MASTER);
	for (GList *link = list; link != NULL; link = g_list_next (link)) {
		GdkDevice *device = GDK_DEVICE (link->data);
		
		if (gdk_device_get_source (device) != GDK_SOURCE_KEYBOARD)
			continue;

		keyboard = device;
		break;
	}
	
	grab_status = gdk_device_grab(keyboard, top_window, GDK_OWNERSHIP_APPLICATION, FALSE, GdkEventMask (GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK), NULL, GDK_CURRENT_TIME);

	if (grab_status != GDK_GRAB_SUCCESS) {
		gdk_device_ungrab(pointer, GDK_CURRENT_TIME);
		return(-2);
	}

	cursor = gdk_window_get_cursor(window);
	gdk_window_set_cursor(window, gdk_cursor_new_for_display(gdk_dpy, GDK_BLANK_CURSOR));
	
	grabbed=1;
	
	std::list<vtt_class *> :: iterator v;
	int c=0;
	
	for (v=vtt_class::main_list.begin(); v!=vtt_class::main_list.end(); v++) {
		if (!(*v)->audio_hidden) {
			vtt_class::focus_no(c);
			break;
		}
		c++;
	}

	warp=TX_MOUSE_SPEED_NORMAL;
	
	return 0;
}

void tx_mouse :: ungrab()
{
	if (!grabbed) return;

	tX_debug("tX_mouse::ungrab(): this: %016" PRIxPTR, (uintptr_t) this);
	
	gdk_device_ungrab(keyboard, GDK_CURRENT_TIME);
	gdk_device_ungrab(pointer, GDK_CURRENT_TIME);

	gdk_window_set_cursor(window, cursor);

	vtt_class::unfocus();

	gdk_window_set_events(top_window, savedEventMask);

	if (enable_auto_mnemonics) {
		g_object_set (gtk_widget_get_settings (main_window), "gtk-auto-mnemonics", enable_auto_mnemonics, NULL);
	}

	ungrab_linux_input();
	
	tX_debug("tX_mouse::ungrab(): done");
	
	grabbed=0;
}

GError *tx_mouse::open_channel() {
	GError *error = NULL;

	linux_input_channel = g_io_channel_new_file("/dev/input/mice", "r", &error);
	if (linux_input_channel) {
		g_io_channel_set_flags(linux_input_channel, G_IO_FLAG_NONBLOCK, NULL);
		return 0;
	} else {
		return error;
	}
	
	return NULL;
}

void tx_mouse::close_channel() {
    if (linux_input_channel) {
	    g_io_channel_shutdown(linux_input_channel, false, NULL);
	    g_io_channel_unref(linux_input_channel);
	    linux_input_channel = NULL;
    }
}

int tx_mouse::grab_linux_input() {
	
	if (linux_input_channel) {
		linux_input_watch = g_io_add_watch_full(linux_input_channel, G_PRIORITY_HIGH, G_IO_IN, tx_mouse::linux_input_wrap, this, NULL);
	} else {
		tX_msg("Linux input channel not available, falling back to pointer warping.");
		return 0;
	}
	return 1;
}

void tx_mouse::ungrab_linux_input() {
	if (grab_mode == LINUX_INPUT) {
	    	// only remove the watch, we keep the channel as we dropped root and might fail to re-open it
		g_source_remove(linux_input_watch);
	}
}

#define vtt vtt_class::focused_vtt

void tx_mouse::motion_notify(GtkWidget *widget, GdkEventMotion *eventMotion) {
	if ((grab_mode == FALLBACK) && vtt) {
		gdouble d_x = eventMotion->x_root - x_abs;
		gdouble d_y = eventMotion->y_root - y_abs;
		
		if ((d_x != 0.0) || (d_y != 0.0)) {
			gdouble xnow, ynow;
			//gdk_device_get_position_double(pointer, NULL, &xnow, &ynow);
			//printf("%lf -> %lf, %lf -> %lf\n", eventMotion->x_root, xnow, eventMotion->y_root, ynow);
			gdk_device_warp(pointer, screen, x_abs, y_abs);
			
			if (warp_override) {
				f_prec value=(abs(d_x)>abs(d_y)) ? d_x : d_y;
				vtt->sp_speed.handle_mouse_input(value*globals.mouse_speed*warp);
			} else {
				vtt->xy_input((f_prec) d_x*warp, (f_prec) d_y*warp);
			}
		}
	}
}

void tx_mouse::linux_input(tx_input_event *event) {
	if ((grab_mode == LINUX_INPUT) && vtt) {
		if ((event->x_motion != 0) || (event->y_motion != 0)) {
			// gdk_device_warp(pointer, screen, x_abs, y_abs);
			gdouble d_x = event->x_motion;
			gdouble d_y = event->y_motion;
		
			if (warp_override) {
				f_prec value=(abs(d_x)>abs(d_y)) ? d_x : d_y;
				vtt->sp_speed.handle_mouse_input(value*globals.mouse_speed*warp);
			} else {
				vtt->xy_input((f_prec) d_x*warp, (f_prec) d_y*warp);
			}
		}
	}
}

void tx_mouse::button_press(GtkWidget *widget, GdkEventButton *eventButton) {
	if (vtt) {
		switch(eventButton->button) {
			case 1: if (vtt->is_playing)
					vtt->set_scratch(1);
				else
					vtt->sp_trigger.receive_input_value(1);
				break;
			case 2: vtt->sp_mute.receive_input_value(1); break;
			case 3: vtt_class::focus_next(); break;
		}
	}
}

void tx_mouse::button_release(GtkWidget *widget, GdkEventButton *eventButton) {
	if (vtt) {
		switch (eventButton->button) {
			case 1: vtt->set_scratch(0); break;
			case 2: vtt->sp_mute.receive_input_value(0); break;
		}
	}
}

void tx_mouse::key_press(GtkWidget *widget, GdkEventKey *eventKey) {
	if (vtt) {
		switch(eventKey->keyval) {
			case GDK_KEY_space: if (press_key(KEY_space)) { vtt->set_scratch(1); } break;
			case GDK_KEY_Escape: if (press_key(KEY_Escape)) { ungrab(); } break;

			case GDK_KEY_Return: if (press_key(KEY_Return)) { vtt->sp_trigger.receive_input_value(1); } break;
			case GDK_KEY_BackSpace: if (press_key(KEY_BackSpace)) { vtt->sp_trigger.receive_input_value(0); } break;

			case GDK_KEY_Tab: if (press_key(KEY_Tab)) { vtt_class::focus_next(); } break;

			case GDK_KEY_s: if (press_key(KEY_s)) { vtt->sp_sync_client.receive_input_value(!vtt->is_sync_client); } break;

			case GDK_KEY_m:  if (press_key(KEY_m)) { vtt->sp_mute.receive_input_value(1); } break;
			case GDK_KEY_Control_L:  if (press_key(KEY_Control_L)) { vtt->sp_mute.receive_input_value(1); } break;
			case GDK_KEY_Control_R:  if (press_key(KEY_Control_R)) { vtt->sp_mute.receive_input_value(1); }	break;

			case GDK_KEY_Alt_L:  if (press_key(KEY_Alt_L)) { vtt->sp_mute.receive_input_value(0); } break;
			case GDK_KEY_Alt_R:  if (press_key(KEY_Alt_R)) { vtt->sp_mute.receive_input_value(0); } break;

			case GDK_KEY_w:  if (press_key(KEY_w)) {
				vtt->sp_mute.receive_input_value(1);
				warp_override=true;
				warp=((float) vtt->samples_in_buffer)/TX_MOUSE_SPEED_WARP;
				vtt->set_scratch(1);
			}
			break;
			case GDK_KEY_f:  if (press_key(KEY_f)) {
				warp_override=true;
				warp=((float) vtt->samples_in_buffer)/TX_MOUSE_SPEED_WARP;
				vtt->set_scratch(1);
			}
			break;

			case GDK_KEY_F1: if (press_key(KEY_F1)) { vtt_class::focus_no(0); } break;
			case GDK_KEY_F2: if (press_key(KEY_F2)) { vtt_class::focus_no(1); } break;
			case GDK_KEY_F3: if (press_key(KEY_F3)) { vtt_class::focus_no(2); } break;
			case GDK_KEY_F4: if (press_key(KEY_F4)) { vtt_class::focus_no(3); } break;
			case GDK_KEY_F5: if (press_key(KEY_F5)) { vtt_class::focus_no(4); } break;
			case GDK_KEY_F6: if (press_key(KEY_F6)) { vtt_class::focus_no(5); } break;
			case GDK_KEY_F7: if (press_key(KEY_F7)) { vtt_class::focus_no(6); } break;
			case GDK_KEY_F8: if (press_key(KEY_F8)) { vtt_class::focus_no(7); } break;
			case GDK_KEY_F9: if (press_key(KEY_F9)) { vtt_class::focus_no(8); } break;
			case GDK_KEY_F10: if (press_key(KEY_F10)) { vtt_class::focus_no(9); } break;
			case GDK_KEY_F11: if (press_key(KEY_F11)) { vtt_class::focus_no(10); } break;
			case GDK_KEY_F12: if (press_key(KEY_F12)) { vtt_class::focus_no(11); } break;
		}
	}
}

void tx_mouse::key_release(GtkWidget *widget, GdkEventKey *eventKey) {
	if (vtt) {
		switch(eventKey->keyval) {
			case GDK_KEY_space: if (release_key(KEY_space)) { vtt->set_scratch(0); } break;
			case GDK_KEY_Escape: release_key(KEY_Escape); break;

			case GDK_KEY_Return: release_key(KEY_Return); break;
			case GDK_KEY_BackSpace: release_key(KEY_BackSpace); break;

			case GDK_KEY_Tab: release_key(KEY_Tab); break;

			case GDK_KEY_s: release_key(KEY_s); break;


			case GDK_KEY_m: if (release_key(KEY_m)) { vtt->sp_mute.receive_input_value(0); } break;
			case GDK_KEY_Control_L: if (release_key(KEY_Control_L)) { vtt->sp_mute.receive_input_value(0); } break;
			case GDK_KEY_Control_R: if (release_key(KEY_Control_R)) { vtt->sp_mute.receive_input_value(0); } break;

			case GDK_KEY_Alt_L: if (release_key(KEY_Alt_L)) { vtt->sp_mute.receive_input_value(1); } break;
			case GDK_KEY_Alt_R: if (release_key(KEY_Alt_R)) { vtt->sp_mute.receive_input_value(1); } break;

			case GDK_KEY_w: if (release_key(KEY_w)) {
				vtt->sp_mute.receive_input_value(0);
				warp=TX_MOUSE_SPEED_NORMAL;
				warp_override=false;
				vtt->set_scratch(0);
			}
			break;

			case GDK_KEY_f: if (release_key(KEY_f)) {
				warp=TX_MOUSE_SPEED_NORMAL;
				warp_override=false;
				vtt->set_scratch(0);
			}
			break;

			case GDK_KEY_F1: release_key(KEY_F1); break;
			case GDK_KEY_F2: release_key(KEY_F2); break;
			case GDK_KEY_F3: release_key(KEY_F3); break;
			case GDK_KEY_F4: release_key(KEY_F4); break;
			case GDK_KEY_F5: release_key(KEY_F5); break;
			case GDK_KEY_F6: release_key(KEY_F6); break;
			case GDK_KEY_F7: release_key(KEY_F7); break;
			case GDK_KEY_F8: release_key(KEY_F8); break;
			case GDK_KEY_F9: release_key(KEY_F9); break;
			case GDK_KEY_F10: release_key(KEY_F10); break;
			case GDK_KEY_F11: release_key(KEY_F11); break;
			case GDK_KEY_F12: release_key(KEY_F12); break;
		}
	}
}

gboolean tx_mouse::motion_notify_wrap(GtkWidget *widget, GdkEventMotion *eventMotion, void *data) {
	tx_mouse* mouse = (tx_mouse *) data;
	if (mouse->grabbed) {
		mouse->motion_notify(widget, eventMotion);
		return TRUE;
	} else {
		return FALSE;
	}
}

gboolean tx_mouse::linux_input_wrap(GIOChannel *source, GIOCondition condition, gpointer data) {
	if (condition == G_IO_IN) {
		tx_input_event sum;
		tx_input_event eventbuffer[512];
		tx_mouse* mouse = (tx_mouse *) data;
		gint fd = g_io_channel_unix_get_fd(mouse->linux_input_channel);
		ssize_t bytes_read = read(fd, &eventbuffer, sizeof(eventbuffer));
		
		//printf("read %lu bytes, %lu events\n", bytes_read, bytes_read / sizeof(tx_input_event));
		
		sum.x_motion = 0;
		sum.y_motion = 0;
		
		for (int i = 0; i < bytes_read / sizeof(tx_input_event); i++) {
			sum.x_motion += eventbuffer[i].x_motion;
			sum.y_motion += eventbuffer[i].y_motion;
		}
		mouse->linux_input(&sum);
	}
	return TRUE;
}

gboolean tx_mouse::button_press_wrap(GtkWidget *widget, GdkEventButton *eventButton, void *data) {
	tx_mouse* mouse = (tx_mouse *) data;
	if (mouse->grabbed) {
		if (mouse->last_button_press != eventButton->time) {
			mouse->last_button_press = eventButton->time;

			tX_debug("tX_mouse::button-press-event (%u)", eventButton->button);
			mouse->button_press(widget, eventButton);
		} else {
			tX_debug("tX_mouse::button-press-event (%u) identical event skipped", eventButton->button);
		}
		return TRUE;
	}
	return FALSE;
}

gboolean tx_mouse::button_release_wrap(GtkWidget *widget, GdkEventButton *eventButton, void *data) {
	tx_mouse* mouse = (tx_mouse *) data;
	if (mouse->grabbed) {
		if (mouse->last_button_release != eventButton->time) {
			mouse->last_button_release = eventButton->time;

			tX_debug("tX_mouse::button-release-event (%u)", eventButton->button);
			mouse->button_release(widget, eventButton);
		} else {
			tX_debug("tX_mouse::button-release-event (%u) identical event skipped", eventButton->button);
		}

		return TRUE;
	}

	return FALSE;
}

gboolean tx_mouse::key_press_wrap(GtkWidget *widget, GdkEventKey *eventKey, void *data) {
	tx_mouse* mouse = (tx_mouse *) data;
	if (mouse->grabbed) {
		tX_debug("tX_mouse::key-press-event (%u)", eventKey->keyval);
		mouse->key_press(widget, eventKey);
		return TRUE;
	} else {
		return FALSE;
	}
}

gboolean tx_mouse::key_release_wrap(GtkWidget *widget, GdkEventKey *eventKey, void *data) {
	tx_mouse* mouse = (tx_mouse *) data;
	if (mouse->grabbed) {
		tX_debug("tX_mouse::key-release-event (%u)", eventKey->keyval);
		mouse->key_release(widget, eventKey);
		return TRUE;
	} else {
		return FALSE;
	}
}
