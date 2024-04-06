/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2022 Alexander König

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

    File: tX_seqpar.h

    Description: Header to tX_seqpar.cc
*/

#pragma once

#include "tX_extdial.h"
#include "tX_global.h"
#include "tX_midiin.h"
#include <gtk/gtk.h>
#include <list>
#include <pthread.h>

#define NO_MOUSE_MAPPING 0
#define MOUSE_MAPPING_OK 1

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

/* required for gcc>=3.0 */
using namespace std;

class tX_seqpar {
  public:
    static list<tX_seqpar*>* all;
    void* vtt; /* have to make this void as tX_vtt.h includes this */

    tX_midievent bound_midi_event;

  protected:
    static list<tX_seqpar*>* update;
    static pthread_mutex_t update_lock;

    int touched;
    int gui_active;

    float fwd_value;

    guint32 persistence_id;
    guint32 touch_timestamp;
    bool reverse_midi;
    void* last_event_recorded;

  public:
    /* default constructor */
    tX_seqpar();

    virtual float get_value();
    void set_mapping_parameters(float max, float min, float scale, int mappable);
    virtual ~tX_seqpar();

    void set_vtt(void*);
    int is_touched() { return touched; }
    int is_untouched() { return !touched; }

    bool get_reverse_midi() { return reverse_midi; }
    void set_reverse_midi(bool reverse) { reverse_midi = reverse; }

    void do_touch(); /* can't be inline as requires tX_sequencer.h */
    void touch() {
        if (!touched)
            do_touch();
    }
    void untouch() {
        touched = 0;
        last_event_recorded = NULL;
    }

    static void untouch_all();

    int is_gui_active() { return gui_active; }
    guint32 get_touch_timestamp() { return touch_timestamp; }

    static void create_persistence_ids();
    guint32 get_persistence_id() { return persistence_id; }
    void set_persistence_id(guint32 pid) { persistence_id = pid; }
    static tX_seqpar* get_sp_by_persistence_id(unsigned int pid);

    void record_value(const float value);
    virtual void do_exec(const float value) = 0;
    virtual void exec_value(const float value) = 0;
    virtual void do_update_graphics() = 0;
    void update_graphics(bool gtk_flush = false);
    static void update_all_graphics();
    static void init_all_graphics();

    /* slot for gui actions */
    void receive_gui_value(const float value);

    /* slot for fastforwarding (no effect for vtt) */
    virtual void receive_forward_value(const float value);

    /* slot for mouse and keyboard actions (updating graphics) */
    virtual void handle_mouse_input(float adjustment);
    void receive_input_value(const float value);
#ifdef USE_ALSA_MIDI_IN
    virtual void handle_midi_input(const tX_midievent&);
#endif

    /* Make it so ;) */
    static void materialize_forward_values();

    /* info functions for the editor */
    const char* get_vtt_name();
    virtual const char* get_name();

    /* For Mouse and MIDI Mapping */
  protected:
    float max_value;
    float min_value;
    float scale_value;

    bool midi_upper_bound_set;
    double midi_upper_bound;

    bool midi_lower_bound_set;
    double midi_lower_bound;

    bool is_boolean;

  public:
    int is_mappable;
    void restore_meta(xmlNodePtr node);
    void store_meta(FILE* rc, gzFile rz);

    void set_upper_midi_bound(double val) {
        midi_upper_bound = val;
        midi_upper_bound_set = true;
    }

    void reset_upper_midi_bound() { midi_upper_bound_set = false; }

    void set_lower_midi_bound(double val) {
        midi_lower_bound = val;
        midi_lower_bound_set = true;
    }

    void reset_lower_midi_bound() { midi_lower_bound_set = false; }

    static gboolean tX_seqpar_press(GtkWidget* widget, GdkEventButton* event, gpointer data);

#ifdef USE_ALSA_MIDI_IN
    static gboolean reverse_midi_binding(GtkWidget* widget, gpointer data);

    static gboolean remove_midi_binding(GtkWidget* widget, gpointer data);
    static gboolean learn_midi_binding(GtkWidget* widget, gpointer data);

    static gboolean set_midi_upper_bound(GtkWidget* widget, gpointer data);
    static gboolean reset_midi_upper_bound(GtkWidget* widget, gpointer data);

    static gboolean set_midi_lower_bound(GtkWidget* widget, gpointer data);
    static gboolean reset_midi_lower_bound(GtkWidget* widget, gpointer data);
#endif
};

class tX_seqpar_update : public tX_seqpar {
  public:
    virtual void exec_value(const float value);
};

class tX_seqpar_no_update : public tX_seqpar {
    virtual void do_update_graphics();
    virtual void exec_value(const float value);
};

class tX_seqpar_update_active_forward : public tX_seqpar_update {
    virtual void receive_forward_value(const float value);
};

class tX_seqpar_no_update_active_forward : public tX_seqpar_no_update {
    virtual void receive_forward_value(const float value);
};

/* now the classes for *real* sequenceable parameters */

class tX_seqpar_main_volume : public tX_seqpar_update {
  public:
    tX_seqpar_main_volume();
    virtual void do_exec(const float value);
    virtual void do_update_graphics();

  private:
    virtual const char* get_name();
};

class tX_seqpar_main_pitch : public tX_seqpar_update_active_forward {
  public:
    tX_seqpar_main_pitch();
    virtual void do_exec(const float value);
    virtual void do_update_graphics();

  private:
    virtual const char* get_name();
};

class tX_seqpar_vtt_speed : public tX_seqpar_no_update_active_forward {
  public:
    tX_seqpar_vtt_speed();
    virtual void handle_mouse_input(float adjustment);

  private:
    virtual void do_exec(const float value);
    virtual const char* get_name();
};

class tX_seqpar_spin : public tX_seqpar_no_update_active_forward {
  public:
    tX_seqpar_spin();

  private:
    virtual void do_exec(const float value);
    virtual const char* get_name();
};

class tX_seqpar_vtt_volume : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_volume();
    virtual float get_value();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_pan : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_pan();
    virtual float get_value();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_pitch : public tX_seqpar_update_active_forward {
  public:
    tX_seqpar_vtt_pitch();
    virtual float get_value();
    virtual void do_exec(const float value);

  private:
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_trigger : public tX_seqpar_no_update_active_forward {
  public:
    tX_seqpar_vtt_trigger();

  private:
    virtual void do_exec(const float value);
    virtual const char* get_name();
};

class tX_seqpar_vtt_loop : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_loop();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_sync_follower : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_sync_follower();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_sync_cycles : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_sync_cycles();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_lp_enable : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_lp_enable();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_lp_gain : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_lp_gain();
    virtual float get_value();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_lp_reso : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_lp_reso();
    virtual float get_value();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_lp_freq : public tX_seqpar_update {
  public:
    virtual float get_value();
    tX_seqpar_vtt_lp_freq();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_ec_enable : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_ec_enable();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_ec_length : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_ec_length();
    virtual float get_value();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_ec_feedback : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_ec_feedback();
    virtual float get_value();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_ec_pan : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_ec_pan();
    virtual float get_value();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_ec_volume : public tX_seqpar_update {
  public:
    tX_seqpar_vtt_ec_volume();
    virtual float get_value();

  private:
    virtual void do_exec(const float value);
    virtual void do_update_graphics();
    virtual const char* get_name();
};

class tX_seqpar_vtt_mute : public tX_seqpar_no_update {
  public:
    tX_seqpar_vtt_mute();

  private:
    virtual void do_exec(const float value);
    virtual const char* get_name();
};

/* Seqpars for the LADSPA Plugins */

class tX_seqpar_vttfx : public tX_seqpar_update {
    /* fx_value will be malloc()ed by constructor */
  protected:
    GtkWidget* widget;
    char label_name[2048];
    char name[2048];
    float* fx_value;

  public:
    tX_seqpar_vttfx();
    virtual ~tX_seqpar_vttfx();
    void set_name(const char*, const char*);
    const char* get_label_name() { return label_name; }
    GtkWidget* get_widget() { return widget; }

    virtual float get_value();
    float* get_value_ptr() { return fx_value; }

  private:
    virtual void create_widget();

  public:
    virtual const char* get_name();
};

class tX_seqpar_vttfx_float : public tX_seqpar_vttfx {
  private:
    tX_extdial* mydial;
    GtkAdjustment* myadj;

    virtual void create_widget();
    virtual void do_exec(const float value);
    virtual void do_update_graphics();

  public:
    virtual ~tX_seqpar_vttfx_float();

    static GCallback gtk_callback(GtkWidget*, tX_seqpar_vttfx_float*);
};

class tX_seqpar_vttfx_bool : public tX_seqpar_vttfx {
  private:
    virtual void create_widget();
    virtual void do_exec(const float value);
    virtual void do_update_graphics();

    static GCallback gtk_callback(GtkWidget*, tX_seqpar_vttfx_bool*);

  public:
    virtual ~tX_seqpar_vttfx_bool();
};

class tX_seqpar_vttfx_int : public tX_seqpar_vttfx {
  private:
    GtkAdjustment* myadj;

  private:
    virtual void create_widget();
    virtual void do_exec(const float value);
    virtual void do_update_graphics();

  public:
    virtual ~tX_seqpar_vttfx_int();

    static GCallback gtk_callback(GtkWidget*, tX_seqpar_vttfx_int*);
};
