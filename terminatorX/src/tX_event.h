/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2002  Alexander K�nig
 
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
 
    File: tX_event.h
 
    Description: Header to tX_event.cc
*/ 

#ifndef _h_tx_event_
#define _h_tx_event_ 1

#include "tX_seqpar.h"
#include "tX_vtt.h"
#include <stdio.h>
#include "tX_types.h"

class tX_event
{
	private:
	float			value;
	u_int32_t 		timestamp;
	tX_seqpar		*sp;
	
	public:
	tX_event(u_int32_t time, float val, tX_seqpar *sp_in)
		{
			timestamp=time;
			value=val;
			sp=sp_in;
		}
	tX_event(FILE *input);
	void store(FILE *output);

	tX_seqpar *get_sp() { return sp; }
	u_int32_t get_timestamp() { return timestamp; }
	float get_value() { return value; }
	void set_value(float val) { value=val; }

        char *get_vtt_name() { return sp->get_vtt_name(); }
        const char *get_seqpar_name() { return sp->get_name(); }

	void playback()
		{
			if (sp->is_untouched()) sp->exec_value(value);
		}
};

#endif
