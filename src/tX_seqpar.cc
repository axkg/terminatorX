/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999, 2000 Alexander König
 
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
 
    File: tX_seqpar.cc
 
    Description: This implements the "sequenceable parameters".
*/    

#include "tX_seqpar.h"
#include "tX_vtt.h"
#include <stdio.h>
#include "tX_mastergui.h"
#include "tX_global.h"
#include "tX_sequencer.h"
#include "tX_extdial.h"
#include <malloc.h>

#define TX_SEQPAR_DEFAULT_SCALE 0.05

list <tX_seqpar *> tX_seqpar :: all;
list <tX_seqpar *> tX_seqpar :: update;
pthread_mutex_t tX_seqpar :: update_lock = PTHREAD_MUTEX_INITIALIZER;

#define tt ((vtt_class *) vtt)

tX_seqpar :: tX_seqpar () : bound_midi_event()
{
	touched=0;
	gui_active=1;
	vtt=NULL;
	max_value=0;
	min_value=0;
	scale_value=0;
	is_boolean=false;
	is_mappable=1;
	all.push_back(this);
	last_event_recorded=NULL;
}

void tX_seqpar :: set_mapping_parameters(float max, float min, float scale, int mappable)
{
	max_value=max;
	min_value=min;
	scale_value=scale;
	is_mappable=mappable;
}

void tX_seqpar :: handle_mouse_input(float adjustment)
{
	float tmpvalue;
	
	tmpvalue=get_value()+adjustment*scale_value;
	if (tmpvalue>max_value) tmpvalue=max_value;
	if (tmpvalue<min_value) tmpvalue=min_value;
	
	/*printf("Handling %s, max %f, min %f, scale %f,  val: %f\n", get_name(), max_value, min_value, scale_value, tmpvalue);*/
	
	receive_input_value(tmpvalue);
}

#ifdef USE_ALSA_MIDI_IN
void tX_seqpar :: handle_midi_input( const tX_midievent& event )
{
	float tmpvalue = -1000;

	//event.print( (string(__FUNCTION__) + " - " + get_name()).c_str() );
	
	if( !is_boolean )
	{
		if( event.type == tX_midievent::CC || event.type == tX_midievent::PITCHBEND )
		{	
			tmpvalue = event.value * (max_value-min_value) + min_value;
		}
		else if( event.type == tX_midievent::NOTE )
		{
			tmpvalue = event.is_noteon;
		}
		else
		{
			return;
		}

		if (tmpvalue>max_value) tmpvalue=max_value;
		else if (tmpvalue<min_value) tmpvalue=min_value;
	}
	else
	{
		tmpvalue=event.value;
	}
	
	touch();

	/* Not using receive() as we want immediate GUI update... */
	do_exec(tmpvalue);
	record_value(tmpvalue);
	do_update_graphics();
}
#endif

void tX_seqpar :: set_vtt (void *mytt)
{
	vtt=mytt;
}

tX_seqpar :: ~tX_seqpar()
{
	pthread_mutex_lock(&update_lock);
	update.remove(this);
	pthread_mutex_unlock(&update_lock);
	sequencer.delete_all_events_for_sp(this, tX_sequencer::DELETE_ALL);
	all.remove(this);
}

void tX_seqpar :: do_touch()
{
	if (sequencer.is_recording())
	{
		touched=1;
		touch_timestamp=sequencer.get_timestamp();
	}
}

void tX_seqpar :: untouch_all()
{
	list <tX_seqpar *> :: iterator sp;
	
	for (sp=all.begin(); sp!=all.end(); sp++)
	{
		(*sp)->untouch();
	}
}

void tX_seqpar :: create_persistence_ids()
{
	list <tX_seqpar *> :: iterator sp;
	int pid=0;
	
	for (sp=all.begin(); sp!=all.end(); sp++)
	{
		pid++;
		(*sp)->set_persistence_id(pid);
	}
}

tX_seqpar* tX_seqpar :: get_sp_by_persistence_id(unsigned int pid)
{
	list <tX_seqpar *> :: iterator sp;
	
	for (sp=all.begin(); sp!=all.end(); sp++)
	{
		if ((*sp)->get_persistence_id()==pid) return ((*sp));
	}
	
	fprintf (stderr, "oops: failed to lookup persistence id [%i].\n", pid);	
	return (NULL);
}


void tX_seqpar :: record_value(const float value)
{
#define last_event ((tX_event *) last_event_recorded)

	/* recording more than one event per seqpar for
	  one timestamp doesn't make sense... so if the 
	  last_event_recorded was for the current timestamp
	  we simply set that event's value to the current one.
	*/
	if ((last_event) && (last_event->get_timestamp() == sequencer.get_timestamp()))
	{
		last_event->set_value(value);
	}
	else last_event_recorded=(void *) sequencer.record(this, value);
}

void tX_seqpar :: receive_gui_value(const float value)
{
	if (gui_active)
	{
		touch();
		do_exec(value);
		record_value(value);
	}
}

void tX_seqpar :: receive_input_value(const float value)
{
	touch();
	exec_value(value);
	record_value(value);
}

void tX_seqpar :: receive_forward_value(const float value)
{
	fwd_value=value;
}

void tX_seqpar :: materialize_forward_values()
{
	list <tX_seqpar *> :: iterator sp;
	
	for (sp=all.begin(); sp!=all.end(); sp++)
	{
		(*sp)->exec_value((*sp)->fwd_value);
	}	
	gdk_flush();
}

char * tX_seqpar :: get_vtt_name()
{       
        if (vtt) return tt->name;
        else return "Master Track";
}

void tX_seqpar :: restore_meta(xmlNodePtr node) {
	char *buffer;
	
	buffer=(char *) xmlGetProp(node, (xmlChar *) "id");
	if (buffer) { sscanf(buffer, "%i", &persistence_id); }
	else { tX_error("no ID for seqpar %s", this->get_name()); }
	
	buffer=(char *) xmlGetProp(node, (xmlChar *) "midiType");
	if (buffer) {
		if (strcmp("cc", buffer)==0) {
			bound_midi_event.type=tX_midievent::CC;
		} else if (strcmp("note", buffer)==0) {
			bound_midi_event.type=tX_midievent::NOTE;
		} else if (strcmp("pitchbend", buffer)==0) {
			bound_midi_event.type=tX_midievent::PITCHBEND;
		} else {
			tX_error("unknown midiType \"%s\" for seqpar %s", buffer, this->get_name());
		}
		
		buffer=(char *) xmlGetProp(node, (xmlChar *) "midiChannel");
		if (buffer) { sscanf(buffer, "%i", &bound_midi_event.channel); }
		else { tX_error("no midiChannel for seqpar %s", this->get_name()); }
			
		buffer=(char *) xmlGetProp(node, (xmlChar *) "midiNumber");
		if (buffer) { sscanf(buffer, "%i", &bound_midi_event.number); }
		else { tX_error("no midiNumber for seqpar %s", this->get_name()); }
	} 
	/* else: no MIDI init.... */
}

void tX_seqpar :: store_meta(FILE *rc, gzFile rz) {
	char buffer[256];
	
	if (bound_midi_event.type!=tX_midievent::NONE) {
		char *type;
		
		switch (bound_midi_event.type) {
			case tX_midievent::NOTE: type="note"; break;
			case tX_midievent::CC: type="cc"; break;
			case tX_midievent::PITCHBEND: type="pitchbend"; break;
			default: type="error";
		}
		sprintf(buffer, "id=\"%i\" midiType=\"%s\" midiChannel=\"%i\" midiNumber=\"%i\"", persistence_id, type, bound_midi_event.channel, bound_midi_event.number);
	} else {
		sprintf(buffer, "id=\"%i\"", persistence_id);
	}
	tX_store(buffer);
}


const char * tX_seqpar :: get_name()
{
        return "This string means trouble!";
}

float tX_seqpar :: get_value()
{
	printf("Ooops. tX_seqpar::get_value() called. Trouble.");
	return 0.0;	
}

void tX_seqpar :: update_graphics()
{
	gui_active=0;
	do_update_graphics();
	while (gtk_events_pending()) gtk_main_iteration();	/* gtk_flush */
	gui_active=1;
}

void tX_seqpar :: update_all_graphics()
{
	list <tX_seqpar *> :: iterator sp;

	pthread_mutex_lock(&update_lock);

	if (!update.size())
	{
		pthread_mutex_unlock(&update_lock);
		return;	
	}
	
	while (gtk_events_pending()) gtk_main_iteration();	
	for (sp=update.begin(); sp!=update.end(); sp++)
	{
		(*sp)->update_graphics();
	}
	update.erase(update.begin(), update.end());
	pthread_mutex_unlock(&update_lock);
}

void tX_seqpar :: init_all_graphics()
{
	list <tX_seqpar *> :: iterator sp;

	pthread_mutex_lock(&update_lock);
	
	for (sp=all.begin(); sp!=all.end(); sp++)
	{
		(*sp)->update_graphics();
	}
	while (gtk_events_pending()) gtk_main_iteration();	

	pthread_mutex_unlock(&update_lock);
}

void tX_seqpar_update :: exec_value(const float value)
{
	do_exec(value);
	pthread_mutex_lock(&update_lock);
	update.push_front(this);
	pthread_mutex_unlock(&update_lock);
}

void tX_seqpar_no_update :: exec_value(const float value)
{
	do_exec(value);
}

void tX_seqpar_no_update :: do_update_graphics()
{
	/* NOP */
}


void tX_seqpar_no_update_active_forward :: receive_forward_value(const float value)
{
	fwd_value=value;
	do_exec(value);
}	

void tX_seqpar_update_active_forward :: receive_forward_value(const float value)
{
	fwd_value=value;
	do_exec(value);
}

/* "real" classes */

/**** Sequencable Parameter: MASTER VOLUME ****/

tX_seqpar_master_volume :: tX_seqpar_master_volume() 
{
	set_mapping_parameters(2.5, 0, 0.1, 0);
}

void tX_seqpar_master_volume :: do_exec(const float value)
{
	vtt_class :: set_master_volume(value);
}

void tX_seqpar_master_volume :: do_update_graphics ()
{
	gtk_adjustment_set_value(volume_adj, 2.0-vtt_class::master_volume);
}

const char * tX_seqpar_master_volume :: get_name()
{
        return "Master Volume";
}

/**** Sequencable Parameter: MASTER PITCH ****/

tX_seqpar_master_pitch :: tX_seqpar_master_pitch()
{
	set_mapping_parameters(3.0, -3.0, 0.1, 0);
}

void tX_seqpar_master_pitch :: do_exec(const float value)
{
	vtt_class :: set_master_pitch(value);
}

void tX_seqpar_master_pitch :: do_update_graphics ()
{
	gtk_adjustment_set_value(pitch_adj, globals.pitch);
}

const char * tX_seqpar_master_pitch :: get_name()
{
        return "Master Pitch";
}

/**** Sequencable Parameter: TURNTABLE SPEED ****/

tX_seqpar_vtt_speed :: tX_seqpar_vtt_speed()
{
	// min max scale are not required for this parameter
	set_mapping_parameters(3.0, -3.0, 0.1, 1);
}

/* speed works differently so we need an extra input-handler */

void tX_seqpar_vtt_speed :: handle_mouse_input(float adjustment)
{
	if (tt->do_scratch) tt->sp_speed.receive_input_value(adjustment);
	tt->sense_cycles=globals.sense_cycles;
}

void tX_seqpar_vtt_speed :: do_exec(const float value)
{
	tt->speed=value*tt->audiofile_pitch_correction;
}

const char * tX_seqpar_vtt_speed :: get_name()
{
        return "Speed (Scratching)";
}

/**** Sequencable Parameter: TURNTABLE SPIN ****/

tX_seqpar_spin :: tX_seqpar_spin()
{
	set_mapping_parameters(1, 0, 0, 0);
}

void tX_seqpar_spin :: do_exec(const float value)
{
	if (value > 0) tt->speed=tt->res_pitch;
	else tt->speed=0;
}

const char * tX_seqpar_spin :: get_name()
{
        return "Motor Spin (On/Off)";
}

/**** Sequencable Parameter: TURNTABLE VOLUME ****/

tX_seqpar_vtt_volume :: tX_seqpar_vtt_volume()
{
	set_mapping_parameters(2.0, 0, TX_SEQPAR_DEFAULT_SCALE, 1);
}

float tX_seqpar_vtt_volume :: get_value(){ return tt->rel_volume; }

void tX_seqpar_vtt_volume :: do_exec(const float value)
{
	tt->set_volume(value);
}

void tX_seqpar_vtt_volume :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.volume, 2.0-tt->rel_volume);
}

const char * tX_seqpar_vtt_volume :: get_name()
{
        return "Volume";
}

/**** Sequencable Parameter : Pan ****/

tX_seqpar_vtt_pan :: tX_seqpar_vtt_pan()
{
	set_mapping_parameters(1.0, -1.0, TX_SEQPAR_DEFAULT_SCALE, 1);
}

float tX_seqpar_vtt_pan :: get_value(){ return tt->pan; }

void tX_seqpar_vtt_pan :: do_exec(const float value)
{
	tt->set_pan(value);
}

void tX_seqpar_vtt_pan :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.pan, tt->pan);
}

const char * tX_seqpar_vtt_pan :: get_name()
{
        return "Pan";
}

/**** Sequencable Parameter: TURNTABLE PITCH ****/

tX_seqpar_vtt_pitch :: tX_seqpar_vtt_pitch()
{
	set_mapping_parameters(3.0, -3.0, TX_SEQPAR_DEFAULT_SCALE, 1);
}

float tX_seqpar_vtt_pitch :: get_value(){ return tt->rel_pitch; }

void tX_seqpar_vtt_pitch :: do_exec(const float value)
{
	tt->set_pitch(value);
}

void tX_seqpar_vtt_pitch :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.pitch, tt->rel_pitch);
}

const char * tX_seqpar_vtt_pitch :: get_name()
{
        return "Pitch";
}

/**** Sequencable Parameter: TURNTABLE TRIGGER ****/

tX_seqpar_vtt_trigger :: tX_seqpar_vtt_trigger()
{
	set_mapping_parameters(0.01, 0, 1, 1);
	is_boolean=true;
}

void tX_seqpar_vtt_trigger :: do_exec(const float value)
{
	if (value > 0) tt->trigger();
	else tt->stop();
}

const char * tX_seqpar_vtt_trigger :: get_name()
{
        return "Trigger (Start/Stop)";
}

/**** Sequencable Parameter: TURNTABLE LOOP ****/

tX_seqpar_vtt_loop :: tX_seqpar_vtt_loop()
{
	set_mapping_parameters(0, 0, 0, 0);
	
	is_boolean=true;
}

void tX_seqpar_vtt_loop :: do_exec(const float value)
{
	tt->set_loop(value>0);
}

const char * tX_seqpar_vtt_loop :: get_name()
{
        return "Loop (On/Off)";
}

void tX_seqpar_vtt_loop :: do_update_graphics ()
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tt->gui.loop), tt->loop);
}

/**** Sequencable Parameter: TURNTABLE SYNC CLIENT ****/

tX_seqpar_vtt_sync_client :: tX_seqpar_vtt_sync_client()
{
	set_mapping_parameters(0,0,0,0);
}

void tX_seqpar_vtt_sync_client :: do_exec(const float value)
{
	tt->set_sync_client((value>0), tt->sync_cycles);
}

void tX_seqpar_vtt_sync_client :: do_update_graphics ()
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tt->gui.sync_client), tt->is_sync_client);
}

const char * tX_seqpar_vtt_sync_client :: get_name()
{
        return "Sync Client (On/Off)";
}

/**** Sequencable Parameter: TURNTABLE SYNC CYCLES ****/

tX_seqpar_vtt_sync_cycles :: tX_seqpar_vtt_sync_cycles()
{
	set_mapping_parameters(0,0,0,0);
}

void tX_seqpar_vtt_sync_cycles :: do_exec(const float value)
{
	tt->set_sync_client(tt->is_sync_client, (int) value);
}

void tX_seqpar_vtt_sync_cycles :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.cycles, tt->sync_cycles);
}

const char * tX_seqpar_vtt_sync_cycles :: get_name()
{
        return "Sync Cycles";
}

/**** Sequencable Parameter: TURNTABLE LP ENABLE ****/

tX_seqpar_vtt_lp_enable :: tX_seqpar_vtt_lp_enable()
{
	set_mapping_parameters(0.01,0,1,1);
	is_boolean=true;
}

void tX_seqpar_vtt_lp_enable :: do_exec(const float value)
{
	tt->lp_set_enable(value>0);
}

void tX_seqpar_vtt_lp_enable :: do_update_graphics ()
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tt->gui.lp_enable), tt->lp_enable);
}

const char * tX_seqpar_vtt_lp_enable :: get_name()
{
        return "Lowpass: Enable (On/Off)";
}

/**** Sequencable Parameter: TURNTABLE LP GAIN ****/

tX_seqpar_vtt_lp_gain :: tX_seqpar_vtt_lp_gain()
{
	set_mapping_parameters(2.0,0, TX_SEQPAR_DEFAULT_SCALE, 1);
}

float tX_seqpar_vtt_lp_gain :: get_value() { return tt->lp_gain; }

void tX_seqpar_vtt_lp_gain :: do_exec(const float value)
{
	tt->lp_set_gain(value);
}

const char * tX_seqpar_vtt_lp_gain :: get_name()
{
        return "Lowpass: Input Gain";
}

void tX_seqpar_vtt_lp_gain :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.lp_gain, tt->lp_gain);
}

/**** Sequencable Parameter: TURNTABLE LP RESO ****/

tX_seqpar_vtt_lp_reso :: tX_seqpar_vtt_lp_reso()
{
	set_mapping_parameters(0.99, 0, TX_SEQPAR_DEFAULT_SCALE, 1);
}

float tX_seqpar_vtt_lp_reso :: get_value() { return tt->lp_reso; }

void tX_seqpar_vtt_lp_reso :: do_exec(const float value)
{
	tt->lp_set_reso(value);
}

void tX_seqpar_vtt_lp_reso :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.lp_reso, tt->lp_reso);
}

const char * tX_seqpar_vtt_lp_reso :: get_name()
{
        return "Lowpass: Resonance";
}

/**** Sequencable Parameter: TURNTABLE LP FREQUENCY ****/

tX_seqpar_vtt_lp_freq :: tX_seqpar_vtt_lp_freq()
{
	set_mapping_parameters(0.99, 0, TX_SEQPAR_DEFAULT_SCALE, 1);
}

float tX_seqpar_vtt_lp_freq :: get_value() { return tt->lp_freq; }

void tX_seqpar_vtt_lp_freq :: do_exec(const float value)
{
	tt->lp_set_freq(value);
}

const char * tX_seqpar_vtt_lp_freq :: get_name()
{
        return "Lowpass: Cutoff Frequency";
}

void tX_seqpar_vtt_lp_freq :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.lp_freq, tt->lp_freq);
}

/**** Sequencable Parameter: TURNTABLE ECHO ENABLE ****/

tX_seqpar_vtt_ec_enable :: tX_seqpar_vtt_ec_enable()
{
	set_mapping_parameters(0.01,0,1,1);
	is_boolean=true;
}

void tX_seqpar_vtt_ec_enable :: do_exec(const float value)
{
	tt->ec_set_enable(value>0);
}

void tX_seqpar_vtt_ec_enable :: do_update_graphics ()
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tt->gui.ec_enable), tt->ec_enable);
}

const char * tX_seqpar_vtt_ec_enable :: get_name()
{
        return "Echo: Enable (On/Off)";
}

/**** Sequencable Parameter: TURNTABLE ECHO LENGTH ****/

tX_seqpar_vtt_ec_length :: tX_seqpar_vtt_ec_length()
{
	set_mapping_parameters(1.0, 0, TX_SEQPAR_DEFAULT_SCALE, 1);
}

float tX_seqpar_vtt_ec_length :: get_value() { return tt->ec_length; }

void tX_seqpar_vtt_ec_length :: do_exec(const float value)
{
	tt->ec_set_length(value);
}

void tX_seqpar_vtt_ec_length :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.ec_length, tt->ec_length);
}

const char * tX_seqpar_vtt_ec_length :: get_name()
{
        return "Echo: Duration";
}

/**** Sequencable Parameter: TURNTABLE ECHO FEEDBACK ****/

tX_seqpar_vtt_ec_feedback :: tX_seqpar_vtt_ec_feedback()
{
	set_mapping_parameters(1.0, 0, TX_SEQPAR_DEFAULT_SCALE, 1);
}

float tX_seqpar_vtt_ec_feedback :: get_value() { return tt->ec_feedback; }

void tX_seqpar_vtt_ec_feedback :: do_exec(const float value)
{
	tt->ec_set_feedback(value);
}

void tX_seqpar_vtt_ec_feedback :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.ec_feedback, tt->ec_feedback);
}

const char * tX_seqpar_vtt_ec_feedback :: get_name()
{
        return "Echo: Feedback";
}

/**** Sequencable Parameter: TURNTABLE ECHO PAN ****/

tX_seqpar_vtt_ec_pan :: tX_seqpar_vtt_ec_pan()
{
	set_mapping_parameters(1.0, -1.0, TX_SEQPAR_DEFAULT_SCALE, 1);
}

float tX_seqpar_vtt_ec_pan :: get_value() { return tt->ec_pan; }

void tX_seqpar_vtt_ec_pan :: do_exec(const float value)
{
	tt->ec_set_pan(value);
}

void tX_seqpar_vtt_ec_pan :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.ec_pan, tt->ec_pan);
}

const char * tX_seqpar_vtt_ec_pan :: get_name()
{
        return "Echo: Pan";
}

/**** Sequencable Parameter: TURNTABLE ECHO VOLUME ****/

tX_seqpar_vtt_ec_volume :: tX_seqpar_vtt_ec_volume()
{
	set_mapping_parameters(0.0, 3.0, TX_SEQPAR_DEFAULT_SCALE, 1);
}

float tX_seqpar_vtt_ec_volume :: get_value() { return tt->ec_volume; }

void tX_seqpar_vtt_ec_volume :: do_exec(const float value)
{
	tt->ec_set_volume(value);
}

void tX_seqpar_vtt_ec_volume :: do_update_graphics ()
{
	gtk_adjustment_set_value(tt->gui.ec_volume, tt->ec_volume);
}

const char * tX_seqpar_vtt_ec_volume :: get_name()
{
        return "Echo: Volume";
}


/**** Sequencable Parameter: TURNTABLE MUTE ****/

tX_seqpar_vtt_mute :: tX_seqpar_vtt_mute()
{
	set_mapping_parameters(0.01,0,1,1);
	is_boolean=true;
}

void tX_seqpar_vtt_mute :: do_exec(const float value)
{
	tt->set_mute(value>0);
}

const char * tX_seqpar_vtt_mute :: get_name()
{
        return "Mute (On/Off)";
}

/** LADSPA fx parameters **/

tX_seqpar_vttfx :: tX_seqpar_vttfx()
{
	fx_value=(float *) malloc(sizeof(float));
	*fx_value=0;
	set_mapping_parameters(0,0,0,0);	
}

tX_seqpar_vttfx :: ~tX_seqpar_vttfx()
{
	free(fx_value);
}

void tX_seqpar_vttfx :: set_name(const char *n, const char *sn)
{
	strcpy(name, n);
	strcpy(label_name, sn);
	create_widget();
}

float tX_seqpar_vttfx :: get_value()
{
	return *fx_value;
}

void tX_seqpar_vttfx :: create_widget()
{
	fprintf(stderr, "tX: Ooops. create_widget() for tX_seqpar_vttfx.\n");
}

const char * tX_seqpar_vttfx :: get_name()
{
	return name;	
}

void tX_seqpar_vttfx_float :: create_widget()
{
	float tmp=max_value - min_value/1000;

	*fx_value=min_value;
	//myadj=GTK_ADJUSTMENT(gtk_adjustment_new(*fx_value, min_value+tmp/10, max_value-tmp/10, tmp, tmp, tmp));
	myadj=GTK_ADJUSTMENT(gtk_adjustment_new(*fx_value, min_value, max_value, tmp, tmp, tmp));
	mydial=new tX_extdial(label_name, myadj);
	gtk_signal_connect(GTK_OBJECT(myadj), "value_changed", (GtkSignalFunc) tX_seqpar_vttfx_float :: gtk_callback, this);
	widget = mydial->get_widget();	
}

tX_seqpar_vttfx_float :: ~tX_seqpar_vttfx_float()
{
	delete mydial;	
}

void tX_seqpar_vttfx_float :: do_exec(const float value)
{
	*fx_value=value;
}

void tX_seqpar_vttfx_float :: do_update_graphics()
{
	gtk_adjustment_set_value(myadj, *fx_value);
}

GtkSignalFunc tX_seqpar_vttfx_float :: gtk_callback(GtkWidget* w, tX_seqpar_vttfx_float *sp)
{
	sp->receive_gui_value(sp->myadj->value);	
	return NULL;	
}

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0

void tX_seqpar_vttfx_int :: create_widget()
{
	float tmp=max_value - min_value/1000;
	GtkWidget *tmpwid;

	*fx_value=min_value;
	myadj=GTK_ADJUSTMENT(gtk_adjustment_new(*fx_value, min_value, max_value, 1, 10, tmp));
	widget=gtk_vbox_new(FALSE, 2);

	tmpwid=gtk_spin_button_new(myadj,1.0,0);
	gtk_widget_show(tmpwid);
	gtk_box_pack_start(GTK_BOX(widget), tmpwid, WID_DYN);
	
	gtk_signal_connect(GTK_OBJECT(myadj), "value_changed", (GtkSignalFunc) tX_seqpar_vttfx_int :: gtk_callback, this);

    tmpwid=gtk_label_new(label_name);
	gtk_widget_show(tmpwid);
	gtk_box_pack_start(GTK_BOX(widget), tmpwid, WID_FIX);
}

tX_seqpar_vttfx_int :: ~tX_seqpar_vttfx_int()
{
	gtk_widget_destroy(widget);
}

void tX_seqpar_vttfx_int :: do_exec(const float value)
{
	*fx_value=value;
}

void tX_seqpar_vttfx_int :: do_update_graphics()
{
	gtk_adjustment_set_value(myadj, *fx_value);
}

GtkSignalFunc tX_seqpar_vttfx_int :: gtk_callback(GtkWidget* w, tX_seqpar_vttfx_int *sp)
{
	sp->receive_gui_value(sp->myadj->value);	
	return NULL;
}

void tX_seqpar_vttfx_bool :: create_widget()
{
	*fx_value=min_value;
	widget=gtk_check_button_new_with_label(label_name);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), 0);
	gtk_signal_connect(GTK_OBJECT(widget), "clicked", (GtkSignalFunc) tX_seqpar_vttfx_bool :: gtk_callback, this);
}

tX_seqpar_vttfx_bool :: ~tX_seqpar_vttfx_bool()
{
	gtk_widget_destroy(widget);
}

void tX_seqpar_vttfx_bool :: do_exec(const float value)
{
	*fx_value=value;
}

GtkSignalFunc tX_seqpar_vttfx_bool :: gtk_callback(GtkWidget* w, tX_seqpar_vttfx_bool *sp)
{
	sp->receive_gui_value(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sp->widget)));	
	return NULL;
}

void tX_seqpar_vttfx_bool :: do_update_graphics()
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), *fx_value==max_value);
}
