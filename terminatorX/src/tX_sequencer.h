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
 
    File: tX_sequencer.h
 
    Description: Header to tX_sequencer.cc
*/ 

#ifndef _h_tx_sequencer_ 
#define _h_tx_sequencer_  1

#include <list>
#include <pthread.h>

#include "tX_event.h"
#include "tX_seqpar.h"

#define TX_SEQMODE_PLAYONLY 1
#define TX_SEQMODE_PLAYREC  0

class tX_sequencer
{
	private:
	list <tX_event *> song_list;
	list <tX_event *> record_list;
	pthread_mutex_t record_lock;
	
	u_int32_t current_timestamp;
	u_int32_t start_timestamp;
	u_int32_t max_timestamp;
	u_int32_t record_start_timestamp;		
	u_int32_t record_stop_timestamp;		

	list <tX_event *> :: iterator next_event;
	
	int mode;
	int run;

	public:
	tX_sequencer();
	~tX_sequencer();
	
	void set_timestamp(u_int32_t timestamp);
	int is_recording() { return (mode == TX_SEQMODE_PLAYREC); }
	
	u_int32_t get_timestamp() { return current_timestamp; }
	float get_timestamp_as_float(){ return ((float) (((float) current_timestamp)/((float) max_timestamp))*100.0); }

	void step();
	
	int trig_rec();
	int trig_play();
	int trig_stop();
	
	tX_event *record_event(tX_seqpar *sp, float value);

	tX_event *record(tX_seqpar *sp, float value)
	{
		if (mode == TX_SEQMODE_PLAYREC) return record_event(sp, value);
		else return NULL;
	}
	
	void delete_all_events_for_sp(tX_seqpar *sp);
	
	void save(FILE *);
	void load(FILE *);
	void clear();
	
	u_int32_t set_start_timestamp(float pos);
	void forward_to_start_timestamp(int dont_fake);
};

extern tX_sequencer sequencer;

#endif
