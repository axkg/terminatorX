/*
    terminatorX - realtime audio scratching software
    Copyright (C) 2002 Arthur Peters
	
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
 
    File: tX_midiin.h
 
    Description: Header to tX_midiin.cc
*/    

#ifndef _tx_midiin_h
#define _tx_midiin_h 1

class vtt_class;

class tX_midievent
{
  public:
	int channel;
	enum
	{
		NONE=0,
		CC=1,
		NOTE,
		PITCHBEND,
	} type;
	int number; // note # or controller #
	
	float value; // controller value or note velocity
	bool is_noteon; // note on or off?

	bool type_matches( const tX_midievent& other ) const
	{
		return channel == other.channel && type == other.type && number == other.number;
	}

	void clear_non_type()
	{
		value = 0;
		is_noteon = false;
	}
	
	/* not being used anyway */
	// void print( const char* prefix ) const;
};

#include <config.h>
#ifdef USE_ALSA_MIDI_IN

#include <alsa/asoundlib.h>
#include <gtk/gtk.h>

class tX_midiin
{
	snd_seq_t *ALSASeqHandle;
	tX_midievent last_event;
	bool is_open;
	
  public:
 	tX_midiin();
	~tX_midiin();

	bool get_is_open() {
		return is_open;
	}
	
	int check_event();

	void configure_bindings( vtt_class* );

	const tX_midievent& get_last_event()
	{
		return last_event;
	}

  private:

	class midi_binding_gui
	{
	  public:
		midi_binding_gui( GtkTreeModel* _model, tX_midiin* _midi );
		~midi_binding_gui();

		GtkWidget *window;
		
		GtkWidget *parameter_treeview;
		GtkWidget *midi_event_info;
		GtkWidget *bind_button;

		GtkTreeModel* model;

		tX_midiin* midi;

		tX_midievent last_event;

		static void bind_clicked( GtkButton *button, gpointer _this );
		static void close_clicked( GtkButton *button, gpointer _this );
		static gint timer( gpointer _this );

	  private:
		char tempstr[128];

		gint timer_tag;
	};
};

#endif // USE_ALSA_MIDI_IN

#endif // ndef _tx_midiin_h 
