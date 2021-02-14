/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2021  Alexander König

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

    File: tX_sequencer.h

    Description: Header to tX_sequencer.cc
*/

#ifndef _h_tx_sequencer_
#define _h_tx_sequencer_ 1

#include <glib.h>
#include <list>
#include <pthread.h>

#include "tX_event.h"
#include "tX_seqpar.h"

#define TX_SEQMODE_PLAYONLY 1
#define TX_SEQMODE_PLAYREC 0

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

class tX_sequencer {
  public:
    typedef enum {
        DELETE_ALL,
        DELETE_UPTO_CURRENT,
        DELETE_FROM_CURRENT
    } del_mode;

  private:
    list<tX_event*> song_list;
    list<tX_event*> record_list;
    pthread_mutex_t record_lock;

    guint32 current_timestamp;
    guint32 start_timestamp;
    guint32 max_timestamp;
    guint32 record_start_timestamp;
    guint32 record_stop_timestamp;

    list<tX_event*>::iterator next_event;

    int mode;
    int run;

  public:
    tX_sequencer();
    ~tX_sequencer();

    void set_timestamp(guint32 timestamp);
    int is_recording() { return (mode == TX_SEQMODE_PLAYREC); }

    guint32 get_timestamp() { return current_timestamp; }
    float get_timestamp_as_float() { return ((float)(((float)current_timestamp) / ((float)max_timestamp)) * 100.0); }

    void step();

    int trig_rec();
    int trig_play();
    void trig_stop();

    tX_event* record_event(tX_seqpar* sp, float value);

    tX_event* record(tX_seqpar* sp, float value) {
        if (mode == TX_SEQMODE_PLAYREC)
            return record_event(sp, value);
        else
            return NULL;
    }

    void delete_all_events_for_vtt(vtt_class* vtt, del_mode mode);
    void delete_all_events_for_sp(tX_seqpar* sp, del_mode mode);
    void delete_all_events(del_mode);

    void save(FILE*, gzFile, char* indent);
#ifdef ENABLE_TX_LEGACY
    void load(FILE*);
#endif
    void load(xmlDocPtr, xmlNodePtr);

    guint32 set_start_timestamp(float pos);
    void forward_to_start_timestamp(int dont_fake);
    bool is_empty() { return song_list.empty(); }
};

extern tX_sequencer sequencer;

#endif
