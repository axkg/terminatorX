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
 
    File: tX_engine.c
 
    Description: Contains the code that does the real "Scratching
    		 business": XInput, DGA, Mouse and Keyboardgrabbing
		 etc.

    02 Jun 1999: Implemented high-priority/rt-FIFO-Scheduling use for
                 engine-thread.
		 
    04 Jun 1999: Changed warp-feature behaviour: still connected to
                 mouse-speed (should be changed to maybe) but now
		 depends on sample size -> you can warp through all
		 samples with the same mouse-distance.
*/    

#include "tX_types.h"
#include "tX_engine.h"
#include <pthread.h>
#include <gtk/gtk.h>
#include <gdk/gdkprivate.h>
#include "tX_gui.h"
#include "tX_global.h"
#include "turntable.h"

#ifndef WIN32
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/xf86dga.h>
#include <X11/keysym.h>
#endif

#include "tX_widget.h"
#include <config.h>

#define XWINDOW xwindow

pthread_t engine_thread=0;

pthread_mutex_t stat_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pos_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t run_lock=PTHREAD_MUTEX_INITIALIZER;

int engine_status=ENG_STOPPED;

int realpos=0;

void set_real_pos(int pos)
{
	pthread_mutex_lock(&pos_lock);
	printf("setpos to %i\n", pos);
	fflush(stdout);
	realpos=pos;
	pthread_mutex_unlock(&pos_lock);
}

int get_real_pos()
{
	int pos;
	pthread_mutex_lock(&pos_lock);
	pos=realpos;
	pthread_mutex_unlock(&pos_lock);
	return(realpos);
}

int get_engine_status()
{
	int tmp;
	pthread_mutex_lock(&stat_lock);
	tmp=engine_status;
	pthread_mutex_unlock(&stat_lock);
	return(tmp);
}

void set_engine_status(int status)
{
	pthread_mutex_lock(&stat_lock);
	engine_status=status;
	pthread_mutex_unlock(&stat_lock);
}


#ifndef WIN32
XID OrgXPointer;
XDevice *input_device;

/* XInput grrr. strange strange....

   For now just opens and closes the selected Devices as it seems
   every device produces DGA events then.
*/

int set_xinput(Display *dpy)
{
//	XDevice *input_device=NULL;
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

void reset_xinput(Display *dpy)
{
	
/*	if (globals.xinput_enable)
	{
		input_device=XOpenDevice(dpy, OrgXPointer);
		XChangePointerDevice(dpy, input_device, 0, 1);
		XCloseDevice(dpy,input_device);	
	}*/
}
#endif

void *engine(void *nil)
{
#ifndef WIN32
	XEvent xev;
	long mask=PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;
	XMotionEvent *xmot=(XMotionEvent *) &xev;
	XKeyEvent *xkey=(XKeyEvent *) &xev;
	XButtonEvent *xbut=(XButtonEvent *) &xev;
	Time otime, ntime, dtime;
	Display *dpy;
	KeySym key;
#endif	

	int quit=0;
	int scratch=0;		
	int stop_sense=globals.sense_cycles;
	f_prec warp=1.0;

#ifdef ENABLE_DEBUG_OUTPUT
	fprintf(stderr, "[engine()] Engine thread up, PID: %i\n", getpid());
#endif			
	pthread_mutex_lock(&run_lock);

#ifndef WIN32				
	dpy=XOpenDisplay(NULL); // FIXME: use correct display
	if (!dpy)
	{
		set_engine_status(ENG_ERR_XOPEN);
		pthread_exit(NULL);
	}

	if (vtt_needle_down(vttgl))
	{
		XCloseDisplay(dpy);

		set_engine_status(ENG_ERR_SOUND);
		pthread_exit(NULL);
	}
		
	if (globals.xinput_enable)
	{
		if (set_xinput(dpy))
		{
			XCloseDisplay(dpy);
			vtt_needle_up(vttgl);

			set_engine_status(ENG_ERR_XINPUT);
			pthread_exit(NULL);
		}
	}
				
//	XSelectInput(dpy, xwindow, mask);	

//	XSetInputFocus(dpy, XWINDOW, None, CurrentTime);

        if (GrabSuccess != XGrabPointer(dpy, XWINDOW, False, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync,GrabModeAsync,None,None,CurrentTime))
	{
		reset_xinput(dpy);
		XCloseDisplay(dpy);
		vtt_needle_up(vttgl);

		set_engine_status(ENG_ERR_GRABMOUSE);
		pthread_exit(NULL);		
	}	
	
        if (GrabSuccess != XGrabKeyboard(dpy, XWINDOW, False, GrabModeAsync,GrabModeAsync,CurrentTime))
	{
		XUngrabPointer (dpy, CurrentTime);
		reset_xinput(dpy);		
		XCloseDisplay(dpy);

		vtt_needle_up(vttgl);
		set_engine_status(ENG_ERR_GRABKEY);
		pthread_exit(NULL);		
	}
	
	
	if (!XF86DGADirectVideo(dpy,DefaultScreen(dpy),XF86DGADirectMouse))
	{
		XUngrabKeyboard(dpy, CurrentTime);				
		XUngrabPointer (dpy, CurrentTime);
		reset_xinput(dpy);		
		XCloseDisplay(dpy);

		vtt_needle_up(vttgl);
		set_engine_status(ENG_ERR_DGA);
		pthread_exit(NULL);			
	}

	XAutoRepeatOff(dpy);	
	otime=CurrentTime;
#endif // WIN32

	if (vttgl->mode == MODE_RECORD_SCRATCH)
	{
		vtt_reset_rec_pos(vttgl);
	}

	set_engine_status(ENG_RUNNING);

	while (!quit)
	{
		if (scratch)
		{
			stop_sense--;
	
			if (stop_sense<0)
			{
				vttgl->speed=0;
			}
		}

#ifndef WIN32	
		if (XCheckWindowEvent(dpy, XWINDOW, mask, &xev))
		{
			switch (xev.type)
			{				
				case MotionNotify:
				/*if (scratch)*/
				{
					ntime=xmot->time;
					dtime=ntime-otime;
					if (dtime<=0) dtime=1; //these happens if you move multiple devices at the same time
					otime=ntime;
					
					if (globals.use_y)
					{
						if (scratch) vttgl->speed=((f_prec) xmot->y_root/dtime)*warp*globals.mouse_speed;
						if (xmot->x_root != 0)
						vtt_lowpass_setfreq (vttgl, 0.05*((f_prec) xmot->x_root/dtime)*globals.mouse_speed);
					}
					else
					{
						if (scratch) vttgl->speed=((f_prec) xmot->x_root/dtime)*warp*globals.mouse_speed;	
						if (xmot->y_root != 0)
						vtt_lowpass_setfreq (vttgl, 0.05*((f_prec) xmot->y_root/dtime)*globals.mouse_speed);				
					}
					stop_sense=globals.sense_cycles;
				}
				break;
				
				case ButtonPress:
					switch(xbut->button)
					{
						case 1:
						vttgl->speed=0;
						scratch=1;							
						break;
						
						case 3:
						vttgl->mute_scratch=1;
						break;
						
						case 2:
						quit=1;
						break;
					}
				break;
				
				case ButtonRelease:
					switch(xbut->button)
					{
						case 1:
						vttgl->speed=globals.vtt_default_speed;
						scratch=0;							
						break;
						
						case 3:
						vttgl->mute_scratch=0;
						break;
					}
				break;
				
				case KeyPress:
					key=XKeycodeToKeysym (dpy, xkey->keycode, 0);
				
					switch(key)
					{
						case XK_space:
						vttgl->speed=0;
						scratch=1;
						break;
					
						case XK_Escape:
						case XK_Return:
						quit=1;
						break;
					
						case XK_m:
						case XK_Control_L:
						case XK_Control_R:						
						vttgl->mute_scratch=1;
						break;
						
						case XK_Alt_L:
						case XK_Alt_R:
						vttgl->mute_scratch=0;
						break;
						
						case XK_f:
						warp=((float) globals.scratch_size)/50000.0;						
					}
					break;
				
				case KeyRelease:
					key=XKeycodeToKeysym (dpy, xkey->keycode, 0);
				
					switch(key)
					{
						case XK_space:
						vttgl->speed=globals.vtt_default_speed;
						scratch=0;
						break;
					
						case XK_Escape:
						quit=1;
						break;
					
						case XK_m:
						case XK_Control_L:
						case XK_Control_R:						
						vttgl->mute_scratch=0;
						break;
						
						case XK_Alt_L:
						case XK_Alt_R:
						vttgl->mute_scratch=1;
						break;
						
						case XK_f:
						warp=1.0;
						break;
					}
					break;
			}
		}
#endif // WIN32		
		if (vtt_block_action(vttgl)) quit=2;
	}

#ifndef WIN32
	XF86DGADirectVideo(dpy,DefaultScreen(dpy),0);

	XUngrabKeyboard(dpy, CurrentTime);		
	XUngrabPointer (dpy, CurrentTime);
	XAutoRepeatOn(dpy);

	reset_xinput(dpy);	
	
	XCloseDisplay(dpy);
#endif	

	if (vttgl->mode == MODE_RECORD_SCRATCH)
	{
		vtt_store_rec_pos(vttgl);
	}
	
	vtt_needle_up(vttgl);

	if (quit==1) set_engine_status(ENG_STOPPED);
	else set_engine_status(ENG_FINISHED);

	pthread_mutex_unlock(&run_lock);
		
	pthread_exit(NULL);
}

int run_engine()
{
#ifdef USE_SCHEDULER
	pthread_attr_t pattr;
	struct sched_param sparm;
#endif
	pthread_mutex_lock(&run_lock);
	
	pthread_mutex_lock(&thread_lock);

	if (engine_thread)
	{
		pthread_mutex_unlock(&thread_lock);
		return(ENG_ERR_BUSY);
	}
#ifdef USE_SCHEDULER	
        if (!geteuid())
	{
#ifdef ENABLE_DEBUG_OUTPUT
		fprintf(stderr, "[run_engine()] enabling fifo scheduling.\n");
#endif
		pthread_attr_init(&pattr);
		pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_JOINABLE);
		pthread_attr_setschedpolicy(&pattr, SCHED_FIFO);
	
		sched_getparam(getpid(), &sparm);
		sparm.sched_priority=sched_get_priority_max(SCHED_FIFO);
	
		pthread_attr_setschedparam(&pattr, &sparm);
		pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setscope(&pattr, PTHREAD_SCOPE_SYSTEM);
		
		pthread_create(&engine_thread, &pattr, engine, NULL);
	}
	else
	{
#ifdef ENABLE_DEBUG_OUTPUT
		fprintf(stderr, "[run_engine()] NO fifo scheduling.\n");
#endif
		pthread_create(&engine_thread, NULL, engine, NULL);		
	}
#else
	pthread_create(&engine_thread, NULL, engine, NULL);	
#endif
	
	if (!engine_thread)
	{
		pthread_mutex_unlock(&thread_lock);
		return(ENG_ERR_THREAD);
	}

	gtk_label_set(GTK_LABEL(GTK_BUTTON(action_btn)->child), "Stop");	

	pthread_detach(engine_thread);

	set_engine_status(ENG_INIT);
	
	pthread_mutex_unlock(&thread_lock);
	
	pthread_mutex_unlock(&run_lock);
		
	return (ENG_RUNNING);
}

int stop_engine()
{
	void *ret;
	
	pthread_mutex_lock(&thread_lock);	
	if (!engine_thread)
	{
		pthread_mutex_lock(&thread_lock);
		return(1);
	}
	
	pthread_join(engine_thread, &ret);
	
	engine_thread=0;
	
	pthread_mutex_unlock(&thread_lock);
	
	gtk_label_set(GTK_LABEL(GTK_BUTTON(action_btn)->child), "Start");	
	
	return (0);
}
