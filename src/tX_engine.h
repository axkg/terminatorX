/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2004  Alexander König
 
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
 
    File: tX_engine.h
 
    Description: Header to tX_engine.cc
*/    

#ifndef _TX_ENGINE_H_
#define _TX_ENGINE_H_

#include "tX_tape.h"
#include "tX_mouse.h"
#include "tX_audiodevice.h"
#include "tX_midiin.h"
#include <sys/types.h>

#define ENG_ERR 4

#define ENG_RUNNING 0
#define ENG_INIT 1
#define ENG_STOPPED 2
#define ENG_FINISHED 3
#define ENG_ERR_XOPEN 4
#define ENG_ERR_XINPUT 5
#define ENG_ERR_DGA 6
#define ENG_ERR_SOUND 7
#define ENG_ERR_THREAD 8
#define ENG_ERR_GRABMOUSE 9
#define ENG_ERR_GRABKEY 10
#define ENG_ERR_BUSY 11

#include <pthread.h>
enum tX_engine_error {
	NO_ERROR,
	ERROR_TAPE,
	ERROR_AUDIO,
	ERROR_BUSY	
};

enum tX_engine_status {
	RUNNING,
	STOPPED
};

class tX_engine {
	private:
	static tX_engine *engine;
	
	pthread_t thread;
	pthread_mutex_t start;
	bool thread_terminate;
	tx_mouse *mouse;
	tX_audiodevice *device;
	tx_tapedeck *tape;
	bool recording;
	bool recording_request;
	bool stop_flag;
	bool loop_is_active;
	bool grab_request;
	bool grab_active;
	bool runtime_error;
	
#ifdef USE_ALSA_MIDI_IN
	private: tX_midiin *midi;
	public:	tX_midiin *get_midi() { return midi; }
#endif	
	public:

	pthread_t get_thread_id() { return thread; }
	bool get_runtime_error() { return runtime_error; }
	static tX_engine *get_instance();
	tX_engine();
	~tX_engine();
	
	tX_engine_error run();
	void stop();
	void loop();
	
	void set_recording_request(bool recording);
	bool get_recording_request() { return recording_request; }
	bool is_recording() { return recording; }
	int16_t* render_cycle();
	
	void set_grab_request();
	bool is_stopped() { return stop_flag; }
};
#endif
