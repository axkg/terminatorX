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
 
    File: tX_mouse.cc
 
    Description: This implements the mouse AND keyboard Input handling in
    		 Grab-Mode.
*/    

#include "tX_mouse.h"
#include "tX_mastergui.h"
#include "tX_global.h"
#include "tX_engine.h"
#include "tX_vtt.h"

tx_mouse :: tx_mouse()
{
	mask=PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;
	xmot=(XMotionEvent *) &xev;
	xkey=(XKeyEvent *) &xev;
	xbut=(XButtonEvent *) &xev;
	grabbed=0;
}

int tx_mouse :: grab()
{	
	if (grabbed) return(0);

	dpy=XOpenDisplay(NULL); // FIXME: use correct display
	if (!dpy)
	{
		return(ENG_ERR_XOPEN);
	}

	if (globals.xinput_enable)
	{
		if (set_xinput())
		{
			XCloseDisplay(dpy);
			return(ENG_ERR_XINPUT);
		}
	}
				
	XSelectInput(dpy, xwindow, mask);	

	XSetInputFocus(dpy, xwindow, None, CurrentTime);

        if (GrabSuccess != XGrabPointer(dpy, xwindow, False, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync,GrabModeAsync,None,None,CurrentTime))
	{
		reset_xinput();
		XCloseDisplay(dpy);
		return(ENG_ERR_GRABMOUSE);
	}	
	
        if (GrabSuccess != XGrabKeyboard(dpy, xwindow, False, GrabModeAsync,GrabModeAsync,CurrentTime))
	{
		XUngrabPointer (dpy, CurrentTime);
		reset_xinput();		
		XCloseDisplay(dpy);

		return(ENG_ERR_GRABKEY);
	}
	
	
	if (!XF86DGADirectVideo(dpy,DefaultScreen(dpy),XF86DGADirectMouse))
	{
		XUngrabKeyboard(dpy, CurrentTime);				
		XUngrabPointer (dpy, CurrentTime);
		reset_xinput();		
		XCloseDisplay(dpy);
		return(ENG_ERR_DGA);
	}

	XAutoRepeatOff(dpy);	
	otime=CurrentTime;
	
	grabbed=1;
	vtt_class::focus_no(0);
	
	return(0);
}

void tx_mouse :: ungrab()
{
	if (!grabbed) return;

	XF86DGADirectVideo(dpy,DefaultScreen(dpy),0);

	XUngrabKeyboard(dpy, CurrentTime);		
	XUngrabPointer (dpy, CurrentTime);
	XAutoRepeatOn(dpy);

	reset_xinput();	
	
	XCloseDisplay(dpy);

	vtt_class::unfocus();

	grabbed=0;
}



int tx_mouse :: set_xinput()
{
	XDeviceInfo *devlist;			
	int listmax, i;
	int match=-1;
	
	if (globals.xinput_enable)
	{	
		devlist=XListInputDevices(dpy, &listmax);
	
		for (i=0; i<listmax; i++)
		{
			if(!strcmp(globals.xinput_device,devlist[i].name))
			{
				match=i;
			}
		
			if(devlist[i].use == IsXPointer)
			{
				OrgXPointer=devlist[i].id;
			}
		}
		
		if (match>=0)
		{
			input_device=NULL;
			input_device=XOpenDevice(dpy,devlist[match].id);
/*			if (XChangePointerDevice(dpy,input_device, 0, 1)!=Success)
			{
				match=-1;
			}*/
			XCloseDevice(dpy, input_device);
		}
		
		XFreeDeviceList(devlist);		
	
		if (match>=0) return(0); 
		else return(1);
	}
	
	return(0);
}

#define vtt vtt_class::focused_vtt

int tx_mouse :: check_event()
{
	if (XCheckWindowEvent(dpy, xwindow, mask, &xev))
	{
		if (vtt)
		switch(xev.type)
		{
			case MotionNotify:
				ntime=xmot->time;
				dtime=(f_prec) ntime-otime;
				if (dtime<=0) dtime=1.0;
				otime=ntime;
				
				vtt->xy_input(((f_prec) xmot->x_root)/dtime,((f_prec) xmot->y_root)/dtime);
				break;
			
			case ButtonPress:
				switch(xbut->button)
				{
					case 1: if (vtt->is_playing)
							vtt->set_scratch(1); 
						else
							vtt->trigger();
						break;
					case 2: vtt->set_mute(1); break;
					case 3: vtt_class::focus_next(); break;
				}
				break;
			
			case ButtonRelease:
				switch (xbut->button)
				{	
					case 1: vtt->set_scratch(0); break;
					case 2: vtt->set_mute(0); break;
				}
				break;
			
			case KeyPress:
			{
				key=XKeycodeToKeysym(dpy, xkey->keycode, 0);
				
				switch(key)
				{
					case XK_space: vtt->set_scratch(1); break;
					case XK_Escape: return(1);
					
					case XK_Return: vtt->trigger(); break;
					case XK_BackSpace : vtt->stop(); break;
					
					case XK_Tab: vtt_class::focus_next(); break;
					
					case XK_s: vtt->set_sync_client(!vtt->is_sync_client, vtt->sync_cycles); break;
					
					case XK_m:
					case XK_Control_L:
					case XK_Control_R:						
					vtt->set_mute(1);
					break;
						
					case XK_Alt_L:
					case XK_Alt_R:
					vtt->set_mute(0);
					break;
						
					case XK_F1: vtt_class::focus_no(0); break;
					case XK_F2: vtt_class::focus_no(1); break;
					case XK_F3: vtt_class::focus_no(2); break;
					case XK_F4: vtt_class::focus_no(3); break;
					case XK_F5: vtt_class::focus_no(4); break;
					case XK_F6: vtt_class::focus_no(5); break;
					case XK_F7: vtt_class::focus_no(6); break;
					case XK_F8: vtt_class::focus_no(7); break;
					case XK_F9: vtt_class::focus_no(8); break;
					case XK_F10: vtt_class::focus_no(9); break;
					case XK_F11: vtt_class::focus_no(10); break;
					case XK_F12: vtt_class::focus_no(11); break;
					/*
					case XK_f:
					warp=((float) globals.scratch_size)/50000.0;						
					*/
				}
			} break;
			
			case KeyRelease:
			{
				key=XKeycodeToKeysym (dpy, xkey->keycode, 0);
				
				switch(key)
				{
					case XK_space: vtt->set_scratch(0); break;
					
					case XK_m:
					case XK_Control_L:
					case XK_Control_R:						
					vtt->set_mute(0);
					break;
						
					case XK_Alt_L:
					case XK_Alt_R:
					vtt->set_mute(1);
					break;					
				}
			}
		}
		else {	puts("no vtt"); return(1); }
	}
	return(0);
}

void tx_mouse :: reset_xinput()
{
	
/*	if (globals.xinput_enable)
	{
		input_device=XOpenDevice(dpy, OrgXPointer);
		XChangePointerDevice(dpy, input_device, 0, 1);
		XCloseDevice(dpy,input_device);	
	}*/
}
