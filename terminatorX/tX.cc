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
    
    File: tX.cc
    
    Description: Does all the X-Handling.

    Comment: I know NOTHING about X11. All of the code I wrote here is
    derived from the X-includes and man pages. So assume this is full of
    bad X-Handling. Like all of my code this is: HACKED TO WORK NOT TO BE NICE
    If you can make it better please do so. 

    Changes:
    
    20 Mar 1999 - Fixed a bug that made terminatorX appear with a partly
                  white background on a sun remote x-display.
		  
    20 Mar 1999 - moved short to int16_t
    
    23 Mar 1999 - support for new mixing in Virtual_Turntable
*/

#include "tX.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "version.h"
char title[]=VERSIONSTRING;

#include "tX_types.h"

#define BORDER_ALIGN 5
#define TOOLBAR_SIZE 20

#define DC DefaultColormap(disp, DefaultScreen(disp))

/* tX_Window::tx_Window() : intializes tX_Window, connects to X-Server, and creates Window */

tX_Window :: tX_Window(Virtual_TurnTable *vttptr, int sizex, int sizey, int warp, int cycles, f_prec mspeed, int verb, char *sname, char *lname)
{
	XColor dummy;

	verbose=verb;
	mouse_speed=mspeed;
	
	vtt=vttptr;
	
	playback_active=0;
	
	width = sizex;
	height = sizey;
	
	mouse_border=warp;

	strcpy(s_name, sname);
	if (strlen(s_name)>20)
	{
		s_name[18]='.';
		s_name[19]='.';
		s_name[20]='.';
		s_name[21]=0;		
	}
	strcpy(l_name, lname);
	if (strlen(l_name)>20)
	{
		l_name[18]='.';
		l_name[19]='.';
		l_name[20]='.';
		l_name[21]=0;		
	}
	
	if (width < 3*mouse_border) 
	{
#ifdef USE_CONSOLE
		puts("[tX] Error. Window with should be at least 3 times the warp border.");		
#endif		
		exit(5);
	}

	sense_cycles=cycles;
	
	medx=width/2;
	medy=height/2;
	
	quit=0;
	motor_on=1;
	
	warpx_min=mouse_border;
	warpx_max=width-mouse_border;
	ptrwarped=0;

#ifdef USE_CONSOLE
	if (verbose) puts("[tX] Initializing.");
#endif	
	disp=XOpenDisplay(NULL);
	
	if (!disp)
	{
#ifdef USE_CONSOLE
		puts("[tX] Error: Failed to connect to default display. Fatal. Giving up.");
#endif	
		exit(1);
	}
		
	win=XCreateSimpleWindow(disp, DefaultRootWindow(disp), 0,0, width, height,0,0,0);
	gc=XCreateGC(disp, win, GCBackground | GCForeground, &xgcv);
	XMapWindow(disp, win);
	XStoreName(disp,win,title);	

	event_mask = KeyReleaseMask | KeyPressMask | PointerMotionMask | ExposureMask;

	XSelectInput(disp,win,event_mask);
		
	XSync(disp,False);	
	
	xkeyev=(XKeyEvent *) &xev;
	xmotev=(XMotionEvent *) &xev;
	
	XAllocNamedColor(disp, DC, "green", &col_data, &dummy);
	XAllocNamedColor(disp, DC, "grey", &col_frame, &dummy);
	XAllocNamedColor(disp, DC, "red", &col_pos, &dummy);
	XAllocNamedColor(disp, DC, "yellow", &col_marker, &dummy);
	XAllocNamedColor(disp, DC, "orange", &col_activebtn, &dummy);
	XAllocNamedColor(disp, DC, "grey", &col_inactivebtn, &dummy);
	
	black=BlackPixel(disp, DefaultScreen(disp));
	white=WhitePixel(disp, DefaultScreen(disp));
	
	font = XLoadFont(disp, "fixed");
	if (font)
	{
#ifdef USE_CONSOLE	
		if (verbose) puts("[tX] Font loaded.");
#endif		
		XSetFont(disp, gc, font);
	}
	
}

/* tX_Window::prepare_data_display(): calculates values necessary for data diplay. */

void tX_Window :: prepare_data_display(int16_t *data, unsigned int size)
{
	data_disp_x=BORDER_ALIGN;
	data_disp_y=BORDER_ALIGN;
	
	data_disp_width=width-2*BORDER_ALIGN;
	data_disp_height=height-3*BORDER_ALIGN-TOOLBAR_SIZE;
	
	data_samples=size/2;
	data_ptr=data;
	
	samples_per_pixel=data_samples/width;
	y0=data_disp_y+data_disp_height/2;
	
	pos_y=data_disp_y+data_disp_height+BORDER_ALIGN;
	pos_max_x=data_disp_x+data_disp_width;	
	
	modex=width-3*(TOOLBAR_SIZE)-BORDER_ALIGN;
	modey=height-TOOLBAR_SIZE;
	bt_w=TOOLBAR_SIZE-BORDER_ALIGN;
	bt_h=bt_w;
	bt_a=BORDER_ALIGN;
	modex-=2*(bt_w+bt_a);
}

/* tX_Window::display-data(): actually displays the wave-data */

void tX_Window :: display_data()
{
	int sample;
	int16_t value;
	int x,y1;
	f_prec temp;
	f_prec half_data_height;
	
	half_data_height = (f_prec) data_disp_height/2;
	
	XSetForeground(disp, gc, col_frame.pixel);
	XDrawRectangle(disp, win, gc, data_disp_x, data_disp_y, data_disp_width, data_disp_height); 	

	XSetForeground(disp, gc, col_data.pixel);

	for (x=0; x<data_disp_width; x++)
	{
		value=data_ptr[x*samples_per_pixel];
		for (sample=x*samples_per_pixel; sample<(x+1)*samples_per_pixel; sample++)
		{
			value=(value+data_ptr[sample])/2;
		}
		temp=((f_prec) value)/32767.0;
		y1=(int) (temp * half_data_height);
		XDrawLine(disp, win, gc, data_disp_x+x, y0-y1, data_disp_x+x, y1+y0);
	}
	XSync(disp, False);
}

/* tX_Window::set_pos() : updates the (sample-)position display to position pos

   If SLIM_POS is defined set_pos() draws lines instead of rectangles.
   Might be slightly faster (and uglier).
*/

void tX_Window :: set_pos(int pos)
{
#ifdef SLIM_POS	
	pos+=BORDER_ALIGN;

	XSetForeground(disp, gc, col_pos.pixel);
	XDrawLine(disp, win, gc, data_disp_x, pos_y, pos, pos_y);
	
	XSetForeground(disp, gc, black);
	XDrawLine(disp, win, gc, pos, pos_y, pos_max_x, pos_y); 
#else
	XSetForeground(disp, gc, col_pos.pixel);
	XFillRectangle(disp, win, gc, data_disp_x, pos_y-1, pos-2*BORDER_ALIGN, 3);
	
	XSetForeground(disp, gc, black);
	XFillRectangle(disp, win, gc, pos+BORDER_ALIGN, pos_y-1, pos_max_x-pos, 3); 	
#endif	
}

/* tX_Window::get_samples_per_pixel() : returns value calculated in prepare_data_display()*/

int tX_Window :: get_samples_per_pixel()
{
	return(samples_per_pixel);
}

#include <X11/keysym.h>

/* tx_Window::handle_KeyPress() : Evaluates xkeyev as KeyPress Event */

void tX_Window :: handle_KeyPress()
{
	KeySym key;

	key=XKeycodeToKeysym (disp, xkeyev->keycode, 0);
	
	switch (key)
	{
		case XK_Return:
		{
			if (playback_active)
			{
				playback_active=0;			
				vtt->needle_up();
				if (vtt->mode == MODE_RECORD_SCRATCH)
				{
					vtt->store_rec_pos();
				}
				set_pos(0);
				display_action();
			}
			else
			{
				playback_active=1;
				vtt->speed=vtt->default_speed;
				vtt->needle_down();
				if (vtt->mode == MODE_RECORD_SCRATCH)
				{
					vtt->reset_rec_pos();
				}
				display_action();
			}
		}
		break;
		
		case XK_space:
		{
			XAutoRepeatOff(disp);		
			vtt->speed=0;
			XGrabPointer(disp, win, True, PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
			XWarpPointer(disp, win, win, 0, 0, 0, 0, medx, medy);
			mx=medx;
			first_mot_event=1;
			mtime=CurrentTime;
			motor_on=0;
		}
		break;
		
		case XK_q:
		{
			if (playback_active)
			{
				vtt->needle_up();
				playback_active=0;
				XUngrabPointer(disp, CurrentTime);			
			}
			XAutoRepeatOn(disp);
			XSync(disp, False);			
			quit=1;
		}		
		break;
		
		case XK_n:
		{
			if (!playback_active)
			{
				vtt->set_mode(MODE_SCRATCH);
				display_mode();
			}
		}
		break;
		
		case XK_r:
		{
			if (!playback_active)
			{
				vtt->set_mode(MODE_RECORD_SCRATCH);
				display_mode();
			}
		}
		break;
		
		case XK_p:
		{
			if (!playback_active)
			{
				vtt->set_mode(MODE_PLAYBACK_RECORDED);
				display_mode();
			}
		}
		break;
		
		case XK_m:
			if (!playback_active)
			{
				vtt->toggle_mix();				
				display_text();
			}
		break;
		
		case XK_s:
			if (!playback_active)
			{
				vtt->save(0);
				display_text();				
			}
		break;
		
		case XK_x:
			if (!playback_active)
			{
				vtt->save(1);
				display_text();				
			}
		break;
		
#ifndef USE_OLD_MIX
		case XK_Left:
			if (vtt->vol_loop < 1.0)
			{
				vtt->vol_loop+=0.01;
				vtt->vol_scratch=1.0-vtt->vol_loop;
				display_text();
			}
		break;

		case XK_Right:
			if (vtt->vol_loop > 0.0)
			{
				vtt->vol_loop-=0.01;
				vtt->vol_scratch=1.0-vtt->vol_loop;
				display_text();
			}
		break;
#endif		
	}
}

/* tx_Window::handle_KeyRelease() : Evaluates xkeyev as KeyRelease Event */

void tX_Window :: handle_KeyRelease()
{
	KeySym key;
	
	key=XKeycodeToKeysym (disp, xkeyev->keycode, 0);
	
	switch (key)
	{
		case XK_space:
		{
			vtt->speed=vtt->default_speed;
			XUngrabPointer(disp, CurrentTime);			
			motor_on=1;
			XAutoRepeatOn(disp);			
		}
		break;
	}	
}

/* tx_Window::repaint() : Redraws all components of the window.*/

void tX_Window :: repaint()
{
/*
	No longer just clearing the window, as that produces
	different results on different X-Servers, instead we paint
	it black.
*/
	XSetForeground(disp, gc, black);
	XFillRectangle(disp, win, gc, 0, 0, width, height);	
	display_data();	
	display_mode();
	display_action();
	display_text();
}

/* tx_Window::handle_Motion() : Evaluates xmotev as Motion Event */

void tX_Window :: handle_Motion()
{
	int dist;
	int nx;
	f_prec tspeed;
	Time ntime, dtime;
	
	if (!playback_active) return;

	nx=xmotev->x;
	ntime=xmotev->time;

	if (first_mot_event)
	{
		mx=nx;
		mtime=ntime;
		first_mot_event=0;
		return;
	}

#ifdef USE_STOPSENSE	
	mouse_busy=sense_cycles;
#endif	
			
	if ((ptrwarped) && (nx=medx)) 
	{
		mx=medx;
		ptrwarped=0;
		return;
	}

	if ((nx >= warpx_max) || (nx <= warpx_min))
	{
		XWarpPointer(disp, win, win, 0, 0, 0, 0, medx, medy);
		ptrwarped=1;
	}

	if (motor_on) return;
			
	dtime=ntime-mtime;
	if (dtime==0) dtime=1;
	dist=mx-nx;
	tspeed= (f_prec) dist/dtime;
	vtt->speed=tspeed*mouse_speed;
	
	mx=nx;
	mtime=ntime;
}

/* tx_Window::handle_event() : Evaluates the type of the event and calls
   corresponding method.
*/

void tX_Window :: handle_event()
{
	switch (xev.type)
	{
		case KeyPress: 
			handle_KeyPress();
		break;
		
		case KeyRelease:
			handle_KeyRelease();
		break;
		
		case MotionNotify:
			handle_Motion();
		break;
		
		case Expose:
			repaint();
		break;
		
		default:
			printf("[tX:check_event] Unhandled Event of type: 0x%x\n", xev.type);
	}
}

/* tx_Window::event_wait() : Waits for an event. Is called when playback
   is not active.
*/

int tX_Window :: event_wait()
{
	XWindowEvent(disp, win, event_mask, &xev);
	handle_event();
	return(quit);
}


/* tX_Window::event_check() : Checks for an event. If there is one handle_event is called. 
   For use when playback is active.
*/

int tX_Window :: event_check()
{
#ifdef USE_STOPSENSE
	mouse_busy--;
	if ((mouse_busy==0) && (!motor_on)) vtt->speed=0.0;
#endif	
	if (XCheckWindowEvent(disp, win, event_mask, &xev) == True)
	{
		handle_event();
	}
	return (quit);
}

/* tX_Window::loop() : Calls event_check and event_wait until
   quit is set.
*/

void tX_Window :: loop()
{
	int newpb;
	
	while (!quit)
	{
		if (playback_active)
		{
			event_check();
			if (playback_active)
			{
				newpb=!vtt->block_action();
				if (!newpb)
				{
					vtt->needle_up();
					if (vtt->mode == MODE_RECORD_SCRATCH)
					{
						vtt->store_rec_pos();
					}
					playback_active=newpb;
					
					set_pos(0);
					display_action();
				}
				XUngrabPointer(disp, CurrentTime);
				playback_active=newpb;
			}
		}
		else
		{
			event_wait();
		}
	}
}

/* tX_Window::display_mode() : displays the current vtt mode in the lower
   right edge of the window (empty and full circle & arrow)
*/

void tX_Window :: display_mode()
{
	int x,y,w,h;

	x=modex;
	y=modey;
	w=bt_w;
	h=bt_h;

	if (vtt->mode == MODE_SCRATCH) XSetForeground(disp, gc, col_activebtn.pixel);
	else XSetForeground(disp, gc, col_inactivebtn.pixel);
		
	XDrawArc(disp, win, gc, x, y, w, h, 0, 23040);
	
	x+=bt_a+bt_w;
	
	if (vtt->mode == MODE_RECORD_SCRATCH) XSetForeground(disp, gc, col_activebtn.pixel);
	else XSetForeground(disp, gc, col_inactivebtn.pixel);

	XFillArc(disp, win, gc, x, y, w, h, 0, 23040);
	XDrawArc(disp, win, gc, x, y, w, h, 0, 23040);
	
	x+=bt_a+bt_w;
	
	if (vtt->mode == MODE_PLAYBACK_RECORDED) XSetForeground(disp, gc, col_activebtn.pixel);
	else XSetForeground(disp, gc, col_inactivebtn.pixel);

	XDrawLine(disp, win, gc, x, y, x, y+bt_h);
	XDrawLine(disp, win, gc, x, y, x+bt_w, y+(bt_h/2));
	XDrawLine(disp, win, gc, x, y+bt_h, x+bt_w, y+(bt_h/2));
	
	XSync(disp, False);
}

/* tX_Window :: display_action() : Displays the current playback status. */

void tX_Window :: display_action()
{
	int x,y,w,h;

	x=modex+3*(bt_w+bt_a);
	y=modey;
	w=2*bt_w+bt_a;
	h=bt_h;


	if (playback_active)
	{
		XSetForeground(disp, gc, col_pos.pixel);		
		XDrawRectangle(disp, win, gc, x, y, w, h);
		XFillRectangle(disp, win, gc, x, y, w, h);		
	}	
	else
	{
		XSetForeground(disp, gc, black);
		XFillRectangle(disp, win, gc, x, y, w, h);				
		XSetForeground(disp, gc, col_pos.pixel);	
		XDrawRectangle(disp, win, gc, x, y, w, h);		
	}	
}

/* tX_Window::display_text() : The method name says it all...*/

void tX_Window :: display_text()
{
	int x,y,w,h;
	char buffer[256];
	char out[1024];
	
	x=0; y=height-TOOLBAR_SIZE; w=modex-1; h=TOOLBAR_SIZE;
	
	XSetForeground(disp, gc, black);
	XFillRectangle(disp, win, gc, x, y, w, h);
	
	x=BORDER_ALIGN;
	y=height-TOOLBAR_SIZE+2*BORDER_ALIGN;
	
	strcpy(out, "Scratch: [");
	strcat(out, s_name);
	strcat(out, "] - ");
	
	if (strlen(l_name))
	{
		strcat(out, "Loop: [");
		strcat(out, l_name);
		strcat(out, "] - ");
	}

	if (vtt->do_mix) 
	{
#ifdef USE_OLD_MIX	
		strcat(out, "Mix: [ON]");
#else
		sprintf(buffer, "Mix: [S:%3i/L:%3i]", (int) (vtt->vol_scratch * 100), (int) (vtt->vol_loop * 100));
		strcat(out, buffer);
#endif		
	}
	else strcat(out, "Mix: [OFF]");

	if (strlen(vtt->last_fn))
	{
		sprintf (buffer, " - Saved: [%s] ", vtt->last_fn);		
		strcat(out, buffer);		
	}
	
	XSetForeground(disp, gc, col_frame.pixel);
	XDrawString(disp, win, gc, x, y, out, strlen(out));
	
	XSync(disp, False);
}
















