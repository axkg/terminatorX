/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2003  Alexander König
 
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
 
    File: tX_sequencer.cc
 
    Description: Well, implements the sequencer as you might have
                 guessed.
*/ 

#include "tX_sequencer.h"
#include "tX_mastergui.h"
#include "tX_global.h"

tX_sequencer sequencer;

tX_sequencer :: tX_sequencer()
{
	current_timestamp=0;
	start_timestamp=0;
	max_timestamp=0;
	next_event=song_list.begin();
	mode=TX_SEQMODE_PLAYONLY;
	run=0;
	pthread_mutex_init(&record_lock, NULL);
}

tX_sequencer :: ~tX_sequencer()
{
}

void tX_sequencer :: set_timestamp(guint32 timestamp)
{
	current_timestamp=0;
	start_timestamp=0;
	next_event=song_list.begin();
}

void tX_sequencer :: step()
{
	if (!run) return;
		
	while ((next_event != song_list.end()) && ((*next_event)->get_timestamp()==current_timestamp))
	{
		(*next_event)->playback();
		next_event++;
	}

	current_timestamp++;	
}

tX_event *tX_sequencer :: record_event (tX_seqpar *sp, float value)
{
	tX_event *new_event;
	
	new_event=new tX_event(current_timestamp, value, sp);
	
	pthread_mutex_lock(&record_lock);
	record_list.push_back(new_event);
	pthread_mutex_unlock(&record_lock);
	
	return new_event;
}

int tX_sequencer :: trig_rec()
{
	//set_timestamp(0);
	record_start_timestamp=start_timestamp;

	mode = TX_SEQMODE_PLAYREC;
	return 1;
}

int tX_sequencer :: trig_play()
{
//	set_timestamp(0);
	run=1;
	return 1;
}


//#define SEQ_DEBUG 1
//#define SEQ_DEBUG_MAX 1

void tX_sequencer :: trig_stop()
{
	list <tX_event *> :: iterator song_event;
	list <tX_event *> :: iterator temp_song_event;
	list <tX_event *> :: iterator record_event;
	tX_seqpar *sp;

	int oldmode=mode;

	mode = TX_SEQMODE_PLAYONLY;
	run=0;

	record_stop_timestamp=current_timestamp;
	
	if (oldmode==TX_SEQMODE_PLAYREC)
	{
		pthread_mutex_lock(&record_lock);
#ifdef SEQ_DEBUG		
		printf ("Recorded from %i to %i.\n", record_start_timestamp, record_stop_timestamp);
		printf ("* Song: %i events, Recorded: %i events, sum=%i\n", song_list.size(), record_list.size(), song_list.size() + record_list.size());
#endif		
		/* removing all events for touched parameters in song_list */
		
		song_event=song_list.begin();

		while ((song_event!=song_list.end()) && ((*song_event)->get_timestamp() < record_start_timestamp))
			song_event++;

		while ((song_event!=song_list.end()) && ((*song_event)->get_timestamp() <= record_stop_timestamp))
		{
			sp = (*song_event)->get_sp();
#ifdef SEQ_DEBUG_MAX			
			printf("sp %08x (%i) touched at: %i - timestamp %i.\n", sp, sp->is_touched(), sp->get_touch_timestamp(), (*song_event)->get_timestamp());
#endif			
			
			if (sp->is_touched() && (sp->get_touch_timestamp()<= (*song_event)->get_timestamp()))
			{
				temp_song_event=song_event;
				song_event++;
				delete (*temp_song_event);
				song_list.erase(temp_song_event);
			}
			else
			{
				song_event++;
			}
		}

		/* inserting all recorded events into song_list */
					
		for (record_event=record_list.begin(), song_event=song_list.begin(); record_event != record_list.end();)
		{
			if (song_event==song_list.end())
			{
				song_list.insert(song_event, record_event, record_list.end());
				break;
			}
			
			if ((*song_event)->get_timestamp() >= (*record_event)->get_timestamp())
			{
/*				if (song_event==song_list.begin()) song_list.push_front((*record_event));
				else */
				song_list.insert(song_event, (*record_event));				
				record_event++;
			}
			else 
			{
				song_event++;
			}
		}
//		swap(song_list, record_list);
		record_list.erase(record_list.begin(), record_list.end());	

#ifdef SEQ_DEBUG		
		printf ("- Song: %i events, Recorded: %i events, sum=%i\n", song_list.size(), record_list.size(), song_list.size() + record_list.size());
#endif		
		
		pthread_mutex_unlock(&record_lock);
	}

	tX_seqpar::untouch_all();
	
	song_event=song_list.end();
	
	if (song_event!=song_list.begin()) 
	{
		song_event--;
		max_timestamp=(*song_event)->get_timestamp();
	}
	
#ifdef SEQ_DEBUG_MAX
	/*dump song_list */
	
	for (song_event=song_list.begin(); song_event!=song_list.end(); song_event++)
	{
		printf ("%-15s| %-27s| %8i | %10f\n", (*song_event)->get_vtt_name(), (*song_event)->get_seqpar_name(), (*song_event)->get_timestamp(), (*song_event)->get_value());
	}
#endif	

	current_timestamp=start_timestamp;
	seq_update();
}

void tX_sequencer :: delete_all_events() {
	while (song_list.size()) {
		list <tX_event *> :: iterator ev=song_list.begin();
		delete (*ev);
		song_list.erase(ev);
	}
}

void tX_sequencer :: delete_all_events_for_sp(tX_seqpar *sp)
{
	list <tX_event *> :: iterator song_event;
	list <tX_event *> :: iterator temp_song_event;
	
#ifdef SEQ_DEBUG
	int ctr=0;
#endif		
	
	for (song_event=song_list.begin(); song_event!=song_list.end();)
	{
		if (sp == (*song_event)->get_sp())
		{
			temp_song_event=song_event;
			song_event++;
			delete (*temp_song_event);
			song_list.erase(temp_song_event);
#ifdef SEQ_DEBUG
			ctr++;
#endif			
		}
		else
		{
			song_event++;
		}
	}
#ifdef SEQ_DEBUG
	printf ("removed %i events for seqpar %08x.\n", ctr, sp);
#endif				
}

void tX_sequencer :: save(FILE *rc, gzFile rz, char *indent) {
	guint32 event_count;
	list <tX_event *> :: iterator song_event;
	
	event_count=song_list.size();

	tX_store("%s<sequencer>\n", indent);
	strcat(indent, "\t");
	
	for (song_event=song_list.begin(); song_event!=song_list.end(); song_event++) {
		(*song_event)->store(rc, rz, indent);
	}
	
	indent[strlen(indent)-1]=0;
	tX_store("%s</sequencer>\n", indent);
}


void tX_sequencer :: clear()
{
	if (song_list.size()==0) return;
	
  	song_list.erase(song_list.begin(), song_list.end());
	
	current_timestamp=0;
	max_timestamp=0;
	start_timestamp=0;
}


guint32 tX_sequencer :: set_start_timestamp(float pos)
{
	guint32 timestamp;
	
	if (pos>99.999) pos=99.999;
	
	pos/=100;
	
	timestamp = (guint32) (((float) max_timestamp) * pos);
	
	start_timestamp=timestamp;
	
	return start_timestamp;
}

void tX_sequencer :: forward_to_start_timestamp(int dont_fake)
{
	int gui_update_max, gui_update;
	int run_save=run;
	
	run=1;
	
	gui_update_max=(globals.update_idle * (globals.update_delay+1) * 1000) >> 1;
	gui_update=gui_update_max;
	
	current_timestamp=0;
	
	next_event=song_list.begin();
	
	while (current_timestamp<start_timestamp)
	{
		step();
		if (dont_fake)
		{
			vtt_class :: forward_all_turntables();
			
			gui_update--;		
			if (gui_update < 0)
			{
				gui_update=gui_update_max;
				seq_update();
				while (gtk_events_pending()) gtk_main_iteration();
			}
		}
	}
	
	run=run_save;
	
	tX_seqpar :: update_all_graphics();	
	while (gtk_events_pending()) gtk_main_iteration();
}

void tX_sequencer :: load(xmlDocPtr doc, xmlNodePtr node) {
	tX_event *ev=NULL;
	
	max_timestamp=0;
	
	for (xmlNodePtr cur=node->xmlChildrenNode; cur!=NULL; cur=cur->next) {
		if (cur->type == XML_ELEMENT_NODE) {
			if (xmlStrcmp(cur->name, (xmlChar *) "event")==0) {
				ev=new tX_event(doc, cur);
				max_timestamp=ev->get_timestamp();
				song_list.push_back(ev);
			} else {
				tX_warning("unhandled sequencer element %s.", cur->name);
			}
		}
	}
	
	start_timestamp=0;
	current_timestamp=0;
}
