/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2011  Alexander K�nig
 
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
 
    File: tX_mouse.cc
 
    Description: This implements the mouse AND keyboard Input handling in
    		 Grab-Mode.
*/

#include <sys/wait.h>
#include <X11/Xlib.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdint.h>

#include <config.h>

#ifdef HAVE_X11_EXTENSIONS_XXF86DGA_H
#include <X11/extensions/Xxf86dga.h>
#endif

#ifdef HAVE_X11_EXTENSIONS_XF86DGA_H
#include <X11/extensions/xf86dga.h>
#endif

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "tX_mouse.h"
#include "tX_mastergui.h"
#include "tX_global.h"
#include "tX_engine.h"
#include "tX_vtt.h"
#include <stdlib.h>

#define TX_MOUSE_SPEED_NORMAL 0.05
#define TX_MOUSE_SPEED_WARP 250000

tx_mouse :: tx_mouse()
{
	mask=PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;
	xmot=(XMotionEvent *) &xev;
	xkey=(XKeyEvent *) &xev;
	xbut=(XButtonEvent *) &xev;
	
#ifdef USE_DGA2
	xdgamot=(XDGAMotionEvent *) &xev;
	xdgakey=(XDGAKeyEvent *) &xev;
	xdgabut=(XDGAButtonEvent *) &xev;
#endif	
	
	grabbed=0;
	warp=TX_MOUSE_SPEED_NORMAL;
}


int tx_mouse :: grab()
{	
#ifdef USE_DGA2
	XDGAMode *mode;
#endif	

	if (grabbed) return 0;

	warp_override=false;
	dpy = gdk_x11_get_default_xdisplay();
//	GdkDisplay* gdk_dpy = gdk_display_get_default();

	if (!dpy/* && !gdk_dpy*/) {
		fputs("GrabMode Error: couldn't connect to XDisplay.", stderr);
		return(ENG_ERR_XOPEN);
	}

#ifdef USE_DGA2
	mode=XDGAQueryModes(dpy,DefaultScreen(dpy), &num);
	
	printf("Found %i DGA2-Modes:\n", num);
	
	for(i=0; i<num; i++) {
		printf("%2i: %s\n", i, mode[i].name);
	}
	XFree(mode);
#endif	

	gtk_window_present(GTK_WINDOW(main_window));

	savedEventMask = gdk_window_get_events(top_window);
	GdkEventMask newEventMask = GdkEventMask ((int) savedEventMask |  GDK_POINTER_MOTION_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gdk_window_set_events(top_window, newEventMask);

//	if (globals.xinput_enable) {
//		if (set_xinput()) {
//			XCloseDisplay(dpy);
//			fputs("GrabMode Error: failed to setup XInput.", stderr);
//			return(ENG_ERR_XINPUT);
//		}
//	}


	GdkGrabStatus grab_status =gdk_pointer_grab(top_window, FALSE, GdkEventMask (GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK), NULL, NULL, GDK_CURRENT_TIME);

	if (grab_status != GDK_GRAB_SUCCESS) {
		//reset_xinput();
		//XCloseDisplay(dpy);
		//fputs("GrabMode Error: XGrabPointer failed.", stderr);
		return(-1);
	}	
	
	grab_status = gdk_keyboard_grab(top_window, FALSE, GDK_CURRENT_TIME);

	if (grab_status != GDK_GRAB_SUCCESS) {
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
		//reset_xinput();
		//XCloseDisplay(dpy);
		//fputs("GrabMode Error: XGrabKeyboard failed.", stderr);
		return(-2);
	}
	

#ifdef USE_DGA2
	if (!XDGASetMode(dpy, DefaultScreen(dpy), 1)) {
#else	
	if (!XF86DGADirectVideo(dpy,DefaultScreen(dpy),XF86DGADirectMouse)) {
#endif
		GdkDisplay* display = gdk_display_get_default();
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
		gdk_keyboard_ungrab(GDK_CURRENT_TIME);

//		reset_xinput();
//		XCloseDisplay(dpy);
//		fputs("GrabMode Error: Failed to enable XF86DGA.", stderr);
		return(ENG_ERR_DGA);
	}

#ifdef USE_DGA2
	XDGASelectInput(dpy, DefaultScreen(dpy), mask);
#endif	

	XAutoRepeatOff(dpy);	
	otime=CurrentTime;
	
	grabbed=1;
	
	std::list<vtt_class *> :: iterator v;
	int c=0;
	
	for (v=vtt_class::main_list.begin(); v!=vtt_class::main_list.end(); v++) {
		if (!(*v)->audio_hidden) {
			vtt_class::focus_no(c);
			break;
		}
		c++;
		//vtt_class::focus_no(0);
	}

	warp=TX_MOUSE_SPEED_NORMAL;
	
	return(0);
}

void tx_mouse :: ungrab()
{
	if (!grabbed) return;

	tX_debug("tX_mouse::ungrab(): this: %016" PRIxPTR ", dpy: %016" PRIxPTR, (uintptr_t) this, (uintptr_t) dpy);
	
#ifdef USE_DGA2	
	XDGASetMode(dpy, DefaultScreen(dpy), 0);
#else
	XF86DGADirectVideo(dpy,DefaultScreen(dpy),0);
#endif	

	gdk_keyboard_ungrab(GDK_CURRENT_TIME);
	gdk_pointer_ungrab(GDK_CURRENT_TIME);

	XAutoRepeatOn(dpy);
	
	if (globals.xinput_enable) {
	//	reset_xinput();
	}

	vtt_class::unfocus();

	gdk_window_set_events(top_window, savedEventMask);

	tX_debug("tX_mouse::ungrab(): done");
	
	grabbed=0;
}

#ifdef USE_XSETPOINTER

void tx_mouse :: set_x_pointer(char *devname)
{
	pid_t pid;
	int status;
		
	pid = fork();
	
	if (pid==-1) { 
		/* OOPS. fork failed */
		perror("tX: Error: Failed to fork a new process!");
		return; 
	}
	
	if (pid==0) {
		/* The child execlps xsetpointer */
		execlp("xsetpointer", "xsetpointer", devname, NULL);
		perror("tX: Error: Failed to execute xpointer!");
		exit(0);
	}
	
	/* parent waits for xsetpointer to finish */
	waitpid(pid,  &status, WUNTRACED);
}

#else

void tx_mouse :: set_x_pointer(char *devname)
{
	XDeviceInfo *devlist;			
	XDevice *device;
	int listmax, i;
	
	devlist=XListInputDevices(dpy, &listmax);
	
	for (i=0; i<listmax; i++) {
		if(strcmp(devlist[i].name, devname)==0) {
			device=XOpenDevice(dpy, devlist[i].id);
			if (device) {
				if (XChangePointerDevice(dpy, device, 0, 1)) {
					printf("tX: Error: failed to set pointer device.");			
				}
				
				XCloseDevice(dpy, device);
			} else {
				printf("tX: Error: failed to open XInput device.");
			}		
		}
	}
		
	XFreeDeviceList(devlist);		
}

#endif

int tx_mouse :: set_xinput()
{
	XDeviceInfo *devlist;			
	int listmax, i;
	
	OrgXPointer=0;
	
	if (globals.xinput_enable) {	
		devlist=XListInputDevices(dpy, &listmax);
	
		for (i=0; i<listmax; i++) {
			if(devlist[i].use == IsXPointer) {
				OrgXPointer=devlist[i].id;
				strcpy(OrgXPointerName, devlist[i].name);
			}
		}
		
		XFreeDeviceList(devlist);				
		set_x_pointer(globals.xinput_device);
	}
	
	if (OrgXPointer==0) printf("tX: Error failed to detect core pointer.");
	return(0);
}


void tx_mouse :: reset_xinput()
{
	if (globals.xinput_enable) {
		if (OrgXPointer) set_x_pointer(OrgXPointerName);
	}
}


#define vtt vtt_class::focused_vtt


void tx_mouse::motion_notify(GtkWidget *widget, GdkEventMotion *eventMotion) {
	if (vtt) {
		if (warp_override) {
			f_prec value=(abs(eventMotion->x_root)>abs(eventMotion->y_root)) ? eventMotion->x_root : eventMotion->y_root;
			vtt->sp_speed.handle_mouse_input(value*globals.mouse_speed*warp);
		} else {
			vtt->xy_input((f_prec) eventMotion->x_root*warp, (f_prec) eventMotion->y_root*warp);
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
			case GDK_KEY_space: vtt->set_scratch(1); break;
			case GDK_KEY_Escape: ungrab(); break;

			case GDK_KEY_Return: vtt->sp_trigger.receive_input_value(1); break;
			case GDK_KEY_BackSpace : vtt->sp_trigger.receive_input_value(0); break;

			case GDK_KEY_Tab: vtt_class::focus_next(); break;

			case GDK_KEY_s: vtt->sp_sync_client.receive_input_value(!vtt->is_sync_client); break;

			case GDK_KEY_m:
			case GDK_KEY_Control_L:
			case GDK_KEY_Control_R:
			vtt->sp_mute.receive_input_value(1);
			break;

			case GDK_KEY_Alt_L:
			case GDK_KEY_Alt_R:
			vtt->sp_mute.receive_input_value(0);
			break;

			case GDK_KEY_w:
			vtt->sp_mute.receive_input_value(1);
			case GDK_KEY_f:
			warp_override=true;
			warp=((float) vtt->samples_in_buffer)/TX_MOUSE_SPEED_WARP;
			vtt->set_scratch(1);
			break;

			case GDK_KEY_F1: vtt_class::focus_no(0); break;
			case GDK_KEY_F2: vtt_class::focus_no(1); break;
			case GDK_KEY_F3: vtt_class::focus_no(2); break;
			case GDK_KEY_F4: vtt_class::focus_no(3); break;
			case GDK_KEY_F5: vtt_class::focus_no(4); break;
			case GDK_KEY_F6: vtt_class::focus_no(5); break;
			case GDK_KEY_F7: vtt_class::focus_no(6); break;
			case GDK_KEY_F8: vtt_class::focus_no(7); break;
			case GDK_KEY_F9: vtt_class::focus_no(8); break;
			case GDK_KEY_F10: vtt_class::focus_no(9); break;
			case GDK_KEY_F11: vtt_class::focus_no(10); break;
			case GDK_KEY_F12: vtt_class::focus_no(11); break;
		}
	}
}

void tx_mouse::key_release(GtkWidget *widget, GdkEventKey *eventKey) {
	if (vtt) {
		switch(eventKey->keyval) {
			case GDK_KEY_space: vtt->set_scratch(0); break;

			case GDK_KEY_m:
			case GDK_KEY_Control_L:
			case GDK_KEY_Control_R:
			vtt->sp_mute.receive_input_value(0);
			break;

			case GDK_KEY_Alt_L:
			case GDK_KEY_Alt_R:
			vtt->sp_mute.receive_input_value(1);
			break;

			case GDK_KEY_w:
			vtt->sp_mute.receive_input_value(0);
			case GDK_KEY_f: warp=TX_MOUSE_SPEED_NORMAL;
			warp_override=false;
			vtt->set_scratch(0);
			break;
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

gboolean tx_mouse::button_press_wrap(GtkWidget *widget, GdkEventButton *eventButton, void *data) {
	tx_mouse* mouse = (tx_mouse *) data;
	if (mouse->grabbed) {
		tX_debug("tX_mouse::button-press-event (%u)", eventButton->button);
		mouse->button_press(widget, eventButton);
		return TRUE;
	} else {
		return FALSE;
	}
}

gboolean tx_mouse::button_release_wrap(GtkWidget *widget, GdkEventButton *eventButton, void *data) {
	tx_mouse* mouse = (tx_mouse *) data;
	if (mouse->grabbed) {
		tX_debug("tX_mouse::button-release-event (%u)", eventButton->button);
		mouse->button_release(widget, eventButton);
		return TRUE;
	} else {
		return FALSE;
	}
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
