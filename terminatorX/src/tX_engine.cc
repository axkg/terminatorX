/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999, 2000  Alexander König
 
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
#include "tX_audiodevice.h"
#include "tX_mouse.h"
#include "tX_vtt.h"
#include <pthread.h>
#include <gtk/gtk.h>
#include <gdk/gdkprivate.h>
#include "tX_mastergui.h"
#include "tX_global.h"
#include "tX_tape.h"
#include "tX_widget.h"
#include <config.h>
#include "tX_sequencer.h"

#ifdef USE_SCHEDULER
#include <sys/time.h>
#include <sys/resource.h>
#endif

pthread_t engine_thread=0;

pthread_mutex_t stat_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pos_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t run_lock=PTHREAD_MUTEX_INITIALIZER;

tx_mouse *mouse=new tx_mouse();
audiodevice *device=new audiodevice();
tx_tapedeck *tape=new tx_tapedeck();

int engine_quit=0;

int engine_status=ENG_STOPPED;

int realpos=0;

int do_grab_mouse=0;
int new_grab_mouse=0;

int want_recording=0;
int is_recording=0;

void grab_mouse(int newstate)
{
	new_grab_mouse=newstate;
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

void *engine(void *nil)
{
	int scratch=0;		
	int stop_sense=globals.sense_cycles;
	f_prec warp=1.0;
	int16_t *temp;
	
/*	want_recording=0;
	is_recording=0; */

/*
#ifdef USE_SCHEDULER
		setpriority(PRIO_PROCESS, getpid(), -20);
#endif
*/
	
#ifdef ENABLE_DEBUG_OUTPUT
	fprintf(stderr, "[engine()] Engine thread up, PID: %i\n", getpid());
#endif			
	pthread_mutex_lock(&run_lock);

	set_engine_status(ENG_RUNNING);

	/*render first block*/
	sequencer.step();
	temp=vtt_class::render_all_turntables();

	while (!engine_quit)
	{	
		if (new_grab_mouse!=do_grab_mouse)
		{
		
			do_grab_mouse=new_grab_mouse;
			
			if (do_grab_mouse) mouse->grab();
			else
			{
				mouse->ungrab();
				grab_off();
			}
		}
		
		if (do_grab_mouse)
		if (mouse->check_event())
		{
			new_grab_mouse=0;
		}
		
		device->eat(temp);
		if (is_recording) tape->eat(temp);
		sequencer.step();
		temp=vtt_class::render_all_turntables();					
	}
	
//	device->dev_close();

	if (engine_quit==1) set_engine_status(ENG_STOPPED);
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
	char buffer[PATH_MAX];
	list <vtt_class *> :: iterator vtt;	
	
	pthread_mutex_lock(&thread_lock);
	
	if (engine_thread)
	{
		pthread_mutex_unlock(&thread_lock);
		return(TX_ENG_ERR_BUSY);
	}

	pthread_mutex_lock(&run_lock);

	if (device->dev_open(0))
	{
		device->dev_close();
		pthread_mutex_unlock(&run_lock);
		pthread_mutex_unlock(&thread_lock);
		return TX_ENG_ERR_DEVICE;
	}
	
	is_recording=0;
	
	if (want_recording)
	{
		if (!tape->start_record(globals.record_filename, device->getblocksize()))
			is_recording=1;
		else
		{
			device->dev_close();
			pthread_mutex_unlock(&run_lock);
			pthread_mutex_unlock(&thread_lock);
			return TX_ENG_ERR_TAPE;			
		}
	}

	vtt_class::set_mix_buffer_size(device->getblocksize()/sizeof(int16_t));
	
	engine_quit=0;
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
		device->dev_close();
		pthread_mutex_unlock(&run_lock);
		pthread_mutex_unlock(&thread_lock);
		return(TX_ENG_ERR_THREAD);
	}

//	gtk_label_set(GTK_LABEL(GTK_BUTTON(action_btn)->child), "Stop");	

	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		if ((*vtt)->autotrigger) (*vtt)->trigger();
	}
	
	sequencer.forward_to_start_timestamp(1);	
	
	pthread_detach(engine_thread);

	set_engine_status(ENG_INIT);
	
	pthread_mutex_unlock(&thread_lock);
	
	pthread_mutex_unlock(&run_lock);
		
	return (TX_ENG_OK);
}

int stop_engine()
{
	list <vtt_class *> :: iterator vtt;
	void *ret;

	pthread_mutex_lock(&thread_lock);	
	if (!engine_thread)
	{
		pthread_mutex_unlock(&thread_lock);
		return(1);
	}
	
	engine_quit=1;
	
	pthread_join(engine_thread, &ret);
	
	engine_thread=0;
	
	pthread_mutex_unlock(&thread_lock);
	device->dev_close();
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		(*vtt)->stop();
		(*vtt)->ec_clear_buffer();
	}
	
	if (is_recording) tape->stop_record();
	is_recording=0;
	return (0);
}
