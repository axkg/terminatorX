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
 
    File: tX_event.h
 
    Description: Header to tX_event.cc
*/ 

#ifndef _h_tx_event_
#define _h_tx_event_ 1

#include "tX_seqpar.h"
#include "tX_vtt.h"
#include <stdio.h>
#include <glib.h>
#include "tX_types.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

class tX_event
{
	private:
	guint32 		timestamp;
	tX_seqpar		*sp;
	float			value;
	
	public:
	tX_event(guint32 time, tX_seqpar *sp_in, float val) : 
		timestamp(time),sp(sp_in),value(val) {}

#ifdef ENABLE_TX_LEGACY		
	tX_event(FILE *input);
#endif		
	static tX_event* load_event(xmlDocPtr, xmlNodePtr);
	
	void store(FILE *rc, gzFile rz, char *indent);

	tX_seqpar *get_sp() { return sp; }
	guint32 get_timestamp() { return timestamp; }
	float get_value() { return value; }
	void set_value(float val) { value=val; }

	char *get_vtt_name() { return sp->get_vtt_name(); }
	const char *get_seqpar_name() { return sp->get_name(); }

	void playback() {
		if (sp->is_untouched()) sp->exec_value(value);
	}
};

#endif
