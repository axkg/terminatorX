/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2016  Alexander König
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
    File: tX_event.cc
 
    Description: This implements the sequencer events.
*/ 

#include <tX_event.h>
#include <tX_global.h>

void tX_event :: store (FILE *rc, gzFile rz, char *indent) {
	tX_store("%s<event pid=\"%i\" value=\"%lf\" time=\"%i\"/>\n", indent, sp->get_persistence_id(), value, timestamp);
}

tX_event* tX_event :: load_event (xmlDocPtr doc, xmlNodePtr node) {
	unsigned int sp_persistence_id;
	char *buffer;
	float value;
	guint32 timestamp;
	tX_event *event=NULL;
	tX_seqpar *sp=NULL;
	
	buffer=(char *) xmlGetProp(node, (xmlChar *) "pid");
	if (buffer) sscanf(buffer, "%i", &sp_persistence_id);
	
	buffer=(char *) xmlGetProp(node, (xmlChar *) "value");
	if (buffer) sscanf(buffer, "%f", &value);
	
	buffer=(char *) xmlGetProp(node, (xmlChar *) "time");
	if (buffer) sscanf(buffer, "%i", &timestamp);

	sp=tX_seqpar::get_sp_by_persistence_id(sp_persistence_id);
	
	if (sp) {
		event=new tX_event(timestamp, sp, value);
	} else {
		tX_error("failed to resolve event at %i - pid [%i]. Event lost.", timestamp, sp_persistence_id);
	}
	
	return event;
}
