/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2002  Alexander König
 
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
		 
    12 Aug 2002: Complete rewrite - tX_engine is now a class and the thread
	is created on startup and kept alive until termination
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
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>

tX_engine *engine=NULL;

void tX_engine :: set_grab_request() {
	grab_request=true;
}

void tX_engine :: loop() {
	int16_t *temp;
	int result;
	
	while (!thread_terminate) {
		/* Waiting for the trigger */
		pthread_mutex_lock(&start);
		loop_is_active=true;
		pthread_mutex_unlock(&start);

		/* Render first block */
		if (!stop_flag) {
			sequencer.step();
			temp=vtt_class::render_all_turntables();
		}
		
		while (!stop_flag) {
			/* Checking whether to grab or not  */
			if (grab_request!=grab_active) {
				if (grab_request) {
					/* Activating grab... */
					result=mouse->grab(); 
					if (result!=0) {
						tX_error("tX_engine::loop(): failed to grab mouse - error %i", result);
						grab_active=false;
						/* Reseting grab_request, too - doesn't help keeping it, does it ? ;) */
						grab_request=false;
						mouse->ungrab();
						grab_off();
					} else {
						grab_active=true;
					}
				} else {
					/* Deactivating grab... */
					mouse->ungrab();
					grab_active=false;
					grab_off(); // for the mastergui this is...
				}
			}

			/* Handling input events... */
			if (grab_active) {
				if (mouse->check_event()) {
					/* If we're here the user pressed ESC */
					grab_request=false;
				}
			}
		
			/* Playback the audio... */
			device->play(temp);
		
			/* Record the audio if necessary... */
			if (is_recording()) tape->eat(temp);
			
			/* Forward the sequencer... */
			sequencer.step();
			
			/* Render the next block... */
			temp=vtt_class::render_all_turntables();					
		}
		
		/* Stopping engine... */
		loop_is_active=false;
	}
}

void *engine_thread_entry(void *engine_void) {
	tX_engine *engine=(tX_engine*) engine_void;
	int result;
	
	/* Dropping root privileges for the engine thread - if running suid. */
	
	if ((!geteuid()) && (getuid() != geteuid())) {
		tX_debug("engine_thread_entry() - Running suid root - dropping privileges.");
		
		result=setuid(getuid());
		
		if (result!=0) {
			tX_error("engine_thread_entry() - Failed to drop root privileges.");
			exit(2);
		}
	}
	
	engine->loop();
	
	tX_debug("engine_thread_entry() - Engine thread terminating.");
	
	pthread_exit(NULL);
}

tX_engine :: tX_engine() {
	int result;
	
	pthread_mutex_init(&start, NULL);
	pthread_mutex_lock(&start);
	thread_terminate=false;
	
	/* Creating the actual engine thread.. */
	
	if (!geteuid()) {
		pthread_attr_t pattr;
		struct sched_param sparm;
		
		tX_debug("tX_engine() - Have root privileges - using SCHED_FIFO.");
		
		pthread_attr_init(&pattr);
		pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_JOINABLE);
		pthread_attr_setschedpolicy(&pattr, SCHED_FIFO);
	
		sched_getparam(getpid(), &sparm);
		sparm.sched_priority=sched_get_priority_max(SCHED_FIFO);
	
		pthread_attr_setschedparam(&pattr, &sparm);
		pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setscope(&pattr, PTHREAD_SCOPE_SYSTEM);
		
		result=pthread_create(&thread, &pattr, engine_thread_entry, (void *) this);
	} else {
		tX_debug("tX_engine() - Lacking root privileges - no realtime scheduling!");
		
		result=pthread_create(&thread, NULL, engine_thread_entry, (void *) this);
	}
	
	if (result!=0) {
		tX_error("tX_engine() - Failed to create engine thread. Errno is %i.", errno);
		exit(1);
	}
	
	/* Dropping root privileges for the main thread - if running suid. */
	
	if ((!geteuid()) && (getuid() != geteuid())) {
		tX_debug("tX_engine() - Running suid root - dropping privileges.");
		
		result=setuid(getuid());
		
		if (result!=0) {
			tX_error("tX_engine() - Failed to drop root privileges.");
			exit(2);
		}
	}
	
	mouse=new tx_mouse();
	tape=new tx_tapedeck();
	device=NULL;
	recording=false;
	recording_request=false;
	loop_is_active=false;
	grab_request=false;
	grab_active=false;
}

void tX_engine :: set_recording_request (bool recording) {
	this->recording_request=recording;
}

tX_engine_error tX_engine :: run() {
	list <vtt_class *> :: iterator vtt;
	
	if (loop_is_active) return ERROR_BUSY;
	
	switch (globals.audiodevice_type) {
#ifdef USE_OSS	
		case TX_AUDIODEVICE_TYPE_OSS:
			device=new tX_audiodevice_oss(); break;
#endif			

#ifdef USE_ALSA			
		case TX_AUDIODEVICE_TYPE_ALSA:
			device=new tX_audiodevice_alsa(); break;
#endif
		
		default:
			device=NULL; return ERROR_AUDIO;
	}
	
	if (device->open()) {
		device->close();
		delete device;
		device=NULL;		
		return ERROR_AUDIO;
	}	

	vtt_class::set_mix_buffer_size(device->get_buffersize()/2); //mixbuffer is mono
	
	if (recording_request) {
		if (tape->start_record(globals.record_filename, device->get_buffersize()*sizeof(int16_t))) {
			device->close();
			delete device;
			device=NULL;			
			return ERROR_TAPE;			
		} else {
			recording=true;
		}
	}
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++) {
		if ((*vtt)->autotrigger) (*vtt)->trigger();
	}
	
	sequencer.forward_to_start_timestamp(1);	
	stop_flag=false;
	/* Trigger the engine thread... */
	pthread_mutex_unlock(&start);
	
	return NO_ERROR;
}

void tX_engine :: stop() {
	list <vtt_class *> :: iterator vtt;
	
	if (!loop_is_active) {
		tX_error("tX_engine::stop() - but loop's not running?");
	}
	
	pthread_mutex_lock(&start);
	stop_flag=true;
	
	tX_debug("tX_engine::stop() - waiting for loop to stop.");
	
	while (loop_is_active) {
		usleep(50);
	}
	
	tX_debug("tX_engine::stop() - loop has stopped.");

	device->close();
	delete device;
	device=NULL;
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++) {
		(*vtt)->stop();
		(*vtt)->ec_clear_buffer();
	}
	
	if (is_recording()) tape->stop_record();
	recording=false;
}

tX_engine :: ~tX_engine() {
	void *dummy;
	
	thread_terminate=true;
	stop_flag=true;
	pthread_mutex_unlock(&start);
	tX_debug("~tX_engine() - Waiting for engine thread to terminate.");
	pthread_join(thread, &dummy);	
}
