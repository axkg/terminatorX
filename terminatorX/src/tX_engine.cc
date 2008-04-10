/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2006  Alexander König
 
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

#include <config.h>
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
#include <sched.h>
#include "tX_capabilities.h"

#include <sys/time.h>
#include <sys/resource.h>

tX_engine *tX_engine::engine=NULL;

tX_engine *tX_engine::get_instance() {
	if (!engine) {
		engine=new tX_engine();
	}
	
	return engine;
}

void tX_engine::set_grab_request() {
	grab_request=true;
}

int16_t* tX_engine::render_cycle() {
	/* Checking whether to grab or not  */
	if (grab_request!=grab_active) {
		if (grab_request) {
			/* Activating grab... */
			int result=mouse->grab(); 
			if (result!=0) {
				tX_error("tX_engine::render_cycle(): failed to grab mouse - error %i", result);
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
			grab_off();
		}
	}

	/* Handling input events... */
	if (grab_active) {
		if (mouse->check_event()) {
			/* If we're here the user pressed ESC */
			grab_request=false;
		}
	}

	/* Forward the sequencer... */
	sequencer.step();

	/* Render the next block... */
	int16_t *data=vtt_class::render_all_turntables();
	
	/* Record the audio if necessary... */
	if (is_recording()) tape->eat(data);
	
	/* check for GUI thread starvation */
//	if (cycles_ctr++ > 100) {
//		tX_error("engine detected overload condition.");
//		overload_error=true;
//		stop_flag=true;
		// give the audio thread some air to breathe
		// so it can acquire the mutex
//		usleep(500);
//	}
	
	return  data;
}

void tX_engine::loop() {
	while (!thread_terminate) {
		/* Waiting for the trigger */
		pthread_mutex_lock(&start);
		reset_cycles_ctr();
		
#ifdef USE_SCHEDULER
		pid_t pid=getpid();
		struct sched_param parm;
			
		if (globals.use_realtime && (globals.audiodevice_type!=JACK)) {
			sched_getparam(pid, &parm);
			parm.sched_priority=sched_get_priority_max(SCHED_FIFO);
						
			if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &parm)) {
				tX_msg("loop(): failed to set realtime priority.");
			} else {
				tX_debug("loop(): set SCHED_FIFO.");
			}
		} else {
			sched_getparam(pid, &parm);
			parm.sched_priority=sched_get_priority_max(SCHED_OTHER);
						
			if (pthread_setschedparam(pthread_self(), SCHED_OTHER, &parm)) {
				tX_msg("loop(): failed to set non-realtime priority.");
			} else {
				tX_debug("loop(): set SCHED_OTHER.");
			}			
		}
#endif		
		loop_is_active=true;
		pthread_mutex_unlock(&start);

		if (!stop_flag) device->start(); // Hand flow control over to the device

		// in case we got kicked out by jack we might have
		// to kill the mouse grab
		if (grab_active) {
			mouse->ungrab();
			grab_active=false;
			grab_off();
		}
		
		if (!stop_flag) {
			runtime_error=true;
			usleep(100);
		}
		/* Stopping engine... */
		loop_is_active=false;
	}
}

void *engine_thread_entry(void *engine_void) {
	tX_engine *engine=(tX_engine*) engine_void;
	
#ifdef USE_SCHEDULER
	pid_t pid=getpid();
	struct sched_param parm;

	if (globals.use_realtime) {
		sched_getparam(pid, &parm);
		parm.sched_priority=sched_get_priority_max(SCHED_FIFO);
					
		if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &parm)) {
			// we failed to get max prio, let see whether we can get a little less
			bool success = false;
			for (int i = parm.__sched_priority; i >= 0; i--) {
				parm.__sched_priority = i;
				
				if (!pthread_setschedparam(pthread_self(), SCHED_FIFO, &parm)) {
					success = true;
					break;
				}
			}
			
			if (success) {
				tX_msg("engine_thread_entry(): set SCHED_FIFO with priority %i.", parm.__sched_priority);
			} else {
				tX_warning("engine_thread_entry(): failed to set realtime priority.");
			}
		} else {
			tX_debug("engine_thread_entry(): set SCHED_FIFO with maximum priority.");
		}
	}
#endif //USE_SCHEDULER
		
#ifdef USE_JACK
	/* Create the client now, so the user has something to connect to. */
	tX_jack_client::get_instance();
#endif	
	
	engine->loop();
	tX_debug("engine_thread_entry() engine thread terminating.");
	pthread_exit(NULL);
}

tX_engine :: tX_engine() {
	int result;
	
	pthread_mutex_init(&start, NULL);
	pthread_mutex_lock(&start);
	thread_terminate=false;
	
	result=pthread_create(&thread, NULL, engine_thread_entry, (void *) this);
	
	if (result!=0) {
		tX_error("tX_engine() - Failed to create engine thread. Errno is %i.", errno);
		exit(1);
	}
	
	mouse=new tx_mouse();
#ifdef USE_ALSA_MIDI_IN	
	midi=new tX_midiin();
#endif	
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
	
	runtime_error=false;
	overload_error=false;
	
	if (loop_is_active) return ERROR_BUSY;
	
	switch (globals.audiodevice_type) {
#ifdef USE_OSS	
		case OSS:
			device=new tX_audiodevice_oss(); 
		break;
#endif			

#ifdef USE_ALSA			
		case ALSA:
			device=new tX_audiodevice_alsa(); 
		break;
#endif

#ifdef USE_JACK
		case JACK:
			device=new tX_audiodevice_jack();
		break;
#endif
		
		default:
			device=NULL; return ERROR_AUDIO;
	}
	
	if (device->open()) {
		if (device->get_is_open()) device->close();
		delete device;
		device=NULL;		
		return ERROR_AUDIO;
	}	

	vtt_class::set_sample_rate(device->get_sample_rate());
	
	if (recording_request) {
		if (tape->start_record(globals.record_filename, vtt_class::get_mix_buffer_size(), device->get_sample_rate())) {
			device->close();
			delete device;
			device=NULL;			
			return ERROR_TAPE;			
		} else {
			recording=true;
		}
	}
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++) {
		(*vtt)->sync_countdown=0;
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
		/* Due to gtk+ signal handling this can cause a deadlock
		   on the seqpars' update list. So we need to handle events
		   while waiting...			
		*/
		while (gtk_events_pending()) gtk_main_iteration();
		usleep(50);
	}
	
	tX_debug("tX_engine::stop() - loop has stopped.");

	if (device->get_is_open()) device->close();
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
	
	delete mouse;
#ifdef USE_ALSA_MIDI_IN		
	delete midi;
#endif	
	delete tape;	
}
