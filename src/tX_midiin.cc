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
 
  File: tX_midiin.cc
 
  Description: Implements MIDI input to control turntable parameters.
*/    

#include "tX_midiin.h"
#include "tX_vtt.h"

#ifdef USE_ALSA_MIDI_IN
#include "tX_global.h"
#include <iostream>

using namespace std;

/*
 disabled
void tX_midievent::print( const char* prefix ) const
{
	cerr << prefix << ": channel=" << channel << ", type=" << type << ", number=" << number
		 << ", value=" << value << ", is_noteon=" << is_noteon << endl;		 
}
*/

tX_midiin::tX_midiin()
{
	
	int portid;
	is_open=false;
	
	if (snd_seq_open(&ALSASeqHandle, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
		tX_error("tX_midiin(): failed to open the default sequencer device.");
		return;
	}
	snd_seq_set_client_name(ALSASeqHandle, "TerminatorX");
	portid =
		snd_seq_create_simple_port(ALSASeqHandle,
								   "Control Input",
								   SND_SEQ_PORT_CAP_WRITE
								   | SND_SEQ_PORT_CAP_SUBS_WRITE,
								   SND_SEQ_PORT_TYPE_APPLICATION);
	if (portid < 0) {
		tX_error("tX_midiin(): error creating sequencer port.");
		return;
	}

	snd_seq_nonblock( ALSASeqHandle, 1 );
	
	is_open=true;

	tX_debug("tX_midiin(): sequencer successfully opened."); 
}

tX_midiin::~tX_midiin()
{
	snd_seq_close(ALSASeqHandle);
	tX_debug("tX_midiin(): sequencer closed."); 
}

int tX_midiin::check_event()
{
	snd_seq_event_t *ev;
		
	while( snd_seq_event_input(ALSASeqHandle, &ev) != -EAGAIN )
	{

		//MidiEvent::type MessageType=MidiEvent::NONE;
		//int Volume=0,Note=0,EventDevice=0;
		tX_midievent event;
		event.is_noteon = false;
		bool event_usable = true;
		
		switch (ev->type) {
			case SND_SEQ_EVENT_CONTROLLER: 
				event.type = tX_midievent::CC;
				event.number = ev->data.control.param;
				event.value = ev->data.control.value / 127.0;
				event.channel = ev->data.control.channel;
				break;
			case SND_SEQ_EVENT_PITCHBEND:
				event.type = tX_midievent::PITCHBEND;
				event.number = ev->data.control.param;
				event.value = ev->data.control.value / 127.0;
				event.channel = ev->data.control.channel;
				break;
			case SND_SEQ_EVENT_NOTEON:
				event.type = tX_midievent::NOTE;
				event.number = ev->data.note.note;
				event.value = ev->data.note.velocity / 127.0;
				event.channel = ev->data.note.channel;

				event.is_noteon = true;
				if( event.value == 0 )
					event.is_noteon = false;
				break;
			case SND_SEQ_EVENT_NOTEOFF: 
				event.type = tX_midievent::NOTE;
				event.number = ev->data.note.note;
				event.value = ev->data.note.velocity / 127.0;
				event.channel = ev->data.note.channel;
				
				event.is_noteon = false;
				break;
			default:
				event_usable = false;
		}

		snd_seq_free_event(ev);
		
		if( event_usable )
		{
			if (event.channel<0 || event.channel>15)
			{
				tX_error("tX_midiin::check_event(): invaild event channel %i.", event.channel);
				return -1;
			}

			//cerr << event.type << ", " << event.number << ", " << event.value << endl;
			//event.print( __FUNCTION__ );

			list <tX_seqpar *> :: iterator sp;

			for (sp=tX_seqpar::all.begin(); sp!=tX_seqpar::all.end(); sp++) {
				if ( (*sp)->bound_midi_event.type_matches (event) ) {
					(*sp)->handle_midi_input (event);
				}
			}

			last_event = event;
		}

	}
	return 1;
}

void tX_midiin::configure_bindings( vtt_class* vtt )
{
	list <tX_seqpar *> :: iterator sp;

	/*
	tX_midievent event = {0,tX_midievent::CC,0,0,0};
	event.type = tX_midievent::CC;
	event.number = 11;
	event.channel = 0;
	*/

	GType types[3] = { G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER };
	GtkListStore* model = gtk_list_store_newv(3, types);
	GtkTreeIter iter;
	char tempstr[128];
	
	for (sp=tX_seqpar::all.begin(); sp!=tX_seqpar::all.end(); sp++) {
		if (((*sp)->is_mappable) && ((*sp)->vtt) == (void*) vtt) {
			
			snprintf( tempstr, sizeof(tempstr), "Type: %d, Number: %d, Channel: %d",
					  (*sp)->bound_midi_event.type, (*sp)->bound_midi_event.number,
					  (*sp)->bound_midi_event.channel );

			gtk_list_store_append( model, &iter );
			gtk_list_store_set( model, &iter,
								0, (*sp)->get_name(),
								1, tempstr,
								2, (*sp),
								-1 );

			//cerr << (*sp)->get_name() << endl;
		}
	}

	// it will delete itself.
	new midi_binding_gui(GTK_TREE_MODEL(model), this);

	//cerr << "window created." << endl;

	return;
}

tX_midiin::midi_binding_gui::midi_binding_gui ( GtkTreeModel* _model, tX_midiin* _midi )
	: model(_model), midi( _midi )
{
	GtkWidget *hbox1;
	GtkWidget *scrolledwindow1;
	GtkWidget *vbox1;
	GtkWidget *label1;
	GtkWidget *frame1;
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Configure MIDI Bindings");
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 260);
	
	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_container_add (GTK_CONTAINER (window), hbox1);
	
	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow1);
	gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow1, TRUE, TRUE, 0);
	
	parameter_treeview = gtk_tree_view_new_with_model (model);
	gtk_widget_show (parameter_treeview);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), parameter_treeview);
	
	GtkCellRenderer   *renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW( parameter_treeview ),
											   -1, "Parameter", renderer,
											   "text", 0,
											   NULL );
	gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW( parameter_treeview ),
											   -1, "Event", renderer,
											   "text", 1,
											   NULL );
	gtk_tree_view_set_headers_visible( GTK_TREE_VIEW(parameter_treeview), TRUE );
	
	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_box_pack_start (GTK_BOX (hbox1), vbox1, FALSE, FALSE, 0);
	
	label1 = gtk_label_new ("Selected MIDI Event:");
	gtk_widget_show (label1);
	gtk_box_pack_start (GTK_BOX (vbox1), label1, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
	
	frame1 = gtk_frame_new (NULL);
	gtk_widget_show (frame1);
	gtk_box_pack_start (GTK_BOX (vbox1), frame1, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame1), 1);
	gtk_frame_set_label_align (GTK_FRAME (frame1), 0, 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_IN);
	
	midi_event_info = gtk_label_new ("Use a MIDI thing to select it.");
	gtk_widget_show (midi_event_info);
	gtk_container_add (GTK_CONTAINER (frame1), midi_event_info);
	gtk_label_set_justify (GTK_LABEL (midi_event_info), GTK_JUSTIFY_LEFT);
	
	bind_button = gtk_button_new_with_mnemonic ("Bind");
	gtk_widget_show (bind_button);
	gtk_box_pack_start (GTK_BOX (vbox1), bind_button, FALSE, FALSE, 0);
	
	GtkWidget* close_button = gtk_button_new_with_mnemonic ("Close");
	gtk_widget_show (close_button);
	gtk_box_pack_start (GTK_BOX (vbox1), close_button, FALSE, FALSE, 0);
	
	gtk_signal_connect(GTK_OBJECT(bind_button), "clicked", (GtkSignalFunc) bind_clicked, (void *) this);
	gtk_signal_connect(GTK_OBJECT(close_button), "clicked", (GtkSignalFunc) close_clicked, (void *) this);
	
	timer_tag = gtk_timeout_add( 100, (GtkFunction) timer, (void *) this);
	
	gtk_widget_show_all( GTK_WIDGET( window ) );
}

void tX_midiin::midi_binding_gui::bind_clicked( GtkButton *button, gpointer _this )
{
	tX_midiin::midi_binding_gui* this_ = (tX_midiin::midi_binding_gui*)_this;
	GtkTreeModel* model;
	GtkTreeSelection* selection;
	GtkTreeIter iter;
	tX_seqpar* param;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(this_->parameter_treeview) );
	gtk_tree_selection_get_selected( selection, &model, &iter );
	gtk_tree_model_get( model, &iter, 2, &param, -1 );

	param->bound_midi_event = this_->last_event;
}

void tX_midiin::midi_binding_gui::close_clicked( GtkButton *button, gpointer _this )
{
	tX_midiin::midi_binding_gui* this_ = (tX_midiin::midi_binding_gui*)_this;
	
	gtk_widget_hide( this_->window );

	delete this_;
}

gint tX_midiin::midi_binding_gui::timer( gpointer _this )
{
	tX_midiin::midi_binding_gui* this_ = (tX_midiin::midi_binding_gui*)_this;

	this_->midi->check_event();
	
	tX_midievent tmpevent = this_->midi->get_last_event();

	if( tmpevent.type_matches( this_->last_event ) )
		return TRUE;
	
	this_->last_event = tmpevent;
	this_->last_event.clear_non_type();

	snprintf( this_->tempstr, sizeof(this_->tempstr),
			  "Type: %d (CC=%d, NOTE=%d)\nNumber: %d\nChannel: %d\n",
			  this_->last_event.type, tX_midievent::CC, tX_midievent::NOTE,
			  this_->last_event.number,
			  this_->last_event.channel );

	gtk_label_set_text( GTK_LABEL(this_->midi_event_info), this_->tempstr );

	return TRUE;
}

tX_midiin::midi_binding_gui::~midi_binding_gui ()
{
	gtk_timeout_remove( timer_tag );

	//g_object_unref( window );
	gtk_widget_destroy( window );
}

#endif // USE_ALSA_MIDI_IN
