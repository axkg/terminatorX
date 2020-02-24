/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2020  Alexander König
 
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
 
    File: tX_legacy_vtt.cc
*/    

#include <config.h>

#ifdef ENABLE_TX_LEGACY

#include <stdio.h>
#include "tX_vtt.h"
#include "tX_global.h"
#include "tX_loaddlg.h"
#include "tX_mastergui.h"
#include "tX_sequencer.h"
#include <string.h>

#define atload(data); if (fread((void *) &data, sizeof(data), 1, input)!=1) res+=1;

int vtt_class :: load_10(FILE * input)
{
	int res=0;
	int obsolete_int;
	
	atload(name);
	atload(filename);
	atload(is_sync_master);
	atload(is_sync_client);
	atload(sync_cycles);
	atload(rel_volume);
	recalc_volume();
	atload(rel_pitch);
	recalc_pitch();
	
	atload(autotrigger);
	atload(loop);
	
	atload(mute);
	atload(obsolete_int); //x_control
	atload(obsolete_int); //y_control
	
	atload(lp_enable);
	atload(lp_gain);
	atload(lp_reso);
	atload(lp_freq);
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	atload(ec_enable);
	atload(ec_length);
	ec_set_length(ec_length);
	atload(ec_feedback);
	ec_set_feedback(ec_feedback);
	
	return(res);
}


int vtt_class :: load_11(FILE * input)
{
	int res=0;
	guint32 pid;
	int32_t gui_page;
	int obsolete_int;
	
	atload(name);
	atload(filename);
	atload(is_sync_master);
	atload(is_sync_client);
	atload(sync_cycles);
	atload(rel_volume);
	recalc_volume();
	atload(rel_pitch);
	recalc_pitch();
	
	atload(autotrigger);
	atload(loop);
	
	atload(mute);
	atload(obsolete_int); //x_control
	atload(obsolete_int); //y_control
	
	atload(lp_enable);
	atload(lp_gain);
	atload(lp_reso);
	atload(lp_freq);
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	atload(ec_enable);
	atload(ec_length);
	ec_set_length(ec_length);
	atload(ec_feedback);
	ec_set_feedback(ec_feedback);

	atload(pid);
	sp_speed.set_persistence_id(pid);
	atload(pid);
	sp_volume.set_persistence_id(pid);
	atload(pid);
	sp_pitch.set_persistence_id(pid);
	atload(pid);
	sp_trigger.set_persistence_id(pid);
	atload(pid);
	sp_loop.set_persistence_id(pid);
	atload(pid);
	sp_sync_client.set_persistence_id(pid);
	atload(pid);
	sp_sync_cycles.set_persistence_id(pid);
	atload(pid);
	sp_lp_enable.set_persistence_id(pid);
	atload(pid);
	sp_lp_gain.set_persistence_id(pid);
	atload(pid);
	sp_lp_reso.set_persistence_id(pid);
	atload(pid);
	sp_lp_freq.set_persistence_id(pid);
	atload(pid);
	sp_ec_enable.set_persistence_id(pid);
	atload(pid);
	sp_ec_length.set_persistence_id(pid);
	atload(pid);
	sp_ec_feedback.set_persistence_id(pid);
	atload(pid);
	sp_mute.set_persistence_id(pid);
	atload(pid);
	sp_spin.set_persistence_id(pid);
	
	atload(gui_page);
	
	return(res);
}

int vtt_class :: load_12(FILE * input)
{
	int res=0;
	guint32 pid;
	int32_t counter;
	int32_t type;
	long id;
	int i;
	unsigned int t;
	LADSPA_Plugin *plugin;
	char buffer[256];
	vtt_fx_ladspa *ladspa_effect;
	guint8 hidden;
	
	atload(buffer);
	this->set_name(buffer);
	atload(filename);
	atload(is_sync_master);
	atload(is_sync_client);
	atload(sync_cycles);
	atload(rel_volume);
	recalc_volume();
	atload(rel_pitch);
	recalc_pitch();
	
	atload(autotrigger);
	atload(loop);
	
	atload(mute);
	
	atload(lp_enable);
	atload(lp_gain);
	atload(lp_reso);
	atload(lp_freq);
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	atload(ec_enable);
	atload(ec_length);
	ec_set_length(ec_length);
	atload(ec_feedback);
	ec_set_feedback(ec_feedback);

	atload(pid);
	sp_speed.set_persistence_id(pid);
	atload(pid);
	sp_volume.set_persistence_id(pid);
	atload(pid);
	sp_pitch.set_persistence_id(pid);
	atload(pid);
	sp_trigger.set_persistence_id(pid);
	atload(pid);
	sp_loop.set_persistence_id(pid);
	atload(pid);
	sp_sync_client.set_persistence_id(pid);
	atload(pid);
	sp_sync_cycles.set_persistence_id(pid);
	atload(pid);
	sp_lp_enable.set_persistence_id(pid);
	atload(pid);
	sp_lp_gain.set_persistence_id(pid);
	atload(pid);
	sp_lp_reso.set_persistence_id(pid);
	atload(pid);
	sp_lp_freq.set_persistence_id(pid);
	atload(pid);
	sp_ec_enable.set_persistence_id(pid);
	atload(pid);
	sp_ec_length.set_persistence_id(pid);
	atload(pid);
	sp_ec_feedback.set_persistence_id(pid);
	atload(pid);
	sp_mute.set_persistence_id(pid);
	atload(pid);
	sp_spin.set_persistence_id(pid);
		
	atload(counter);
	
	for (i=0; i<counter; i++)
	{
		atload(type);
		switch(type)
		{
			case TX_FX_BUILTINCUTOFF:
				for (t=0; t<fx_list.size(); t++) effect_down(lp_fx);
			break;
			
			case TX_FX_BUILTINECHO:
				for (t=0; t<fx_list.size(); t++) effect_down(ec_fx);
			break;
			
			case TX_FX_LADSPA:
				atload(id);
				plugin=LADSPA_Plugin::getPluginByUniqueID(id);
				if (plugin)
				{
					ladspa_effect=add_effect(plugin);
					ladspa_effect->load(input);
				}
				else
				{
					sprintf(buffer,"Couldn't find required plugin with ID [%li].", id);
					tx_note(buffer, true);
					res++;
				}
			break;
			
			default:
				tx_note("Fatal error loading set: unknown effect type!", true);
				res++;
		}		
	}

	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_x_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_x_input_parameter(NULL);
	
	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_y_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_y_input_parameter(NULL);

	atload(hidden);
	gui.main_panel->hide(hidden);

	atload(hidden);
	gui.trigger_panel->hide(hidden);

	atload(hidden);
	gui.lp_panel->hide(hidden);

	atload(hidden);
	gui.ec_panel->hide(hidden);
	
	return(res);
}

int vtt_class :: load_13(FILE * input)
{
	int res=0;
	guint32 pid;
	int32_t counter;
	int32_t type;
	long id;
	int i;
	unsigned int t;
	LADSPA_Plugin *plugin;
	char buffer[256];
	vtt_fx_ladspa *ladspa_effect;
	guint8 hidden;
	
	atload(buffer);
	this->set_name(buffer);
	atload(filename);
	atload(is_sync_master);
	atload(is_sync_client);
	atload(sync_cycles);
	atload(rel_volume);
	atload(rel_pitch);
	recalc_pitch();
	
	atload(autotrigger);
	atload(loop);
	
	atload(mute);
	atload(pan);
	
	atload(lp_enable);
	atload(lp_gain);
	atload(lp_reso);
	atload(lp_freq);
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	atload(ec_enable);
	atload(ec_length);
	ec_set_length(ec_length);
	atload(ec_feedback);
	ec_set_feedback(ec_feedback);
	atload(ec_pan);
	ec_set_pan(ec_pan);
	atload(ec_volume);
	ec_set_volume(ec_volume);

	recalc_volume();

	atload(pid);
	sp_speed.set_persistence_id(pid);
	atload(pid);
	sp_volume.set_persistence_id(pid);
	atload(pid);
	sp_pitch.set_persistence_id(pid);
	atload(pid);
	sp_trigger.set_persistence_id(pid);
	atload(pid);
	sp_loop.set_persistence_id(pid);
	atload(pid);
	sp_sync_client.set_persistence_id(pid);
	atload(pid);
	sp_sync_cycles.set_persistence_id(pid);
	atload(pid);
	sp_lp_enable.set_persistence_id(pid);
	atload(pid);
	sp_lp_gain.set_persistence_id(pid);
	atload(pid);
	sp_lp_reso.set_persistence_id(pid);
	atload(pid);
	sp_lp_freq.set_persistence_id(pid);
	atload(pid);
	sp_ec_enable.set_persistence_id(pid);
	atload(pid);
	sp_ec_length.set_persistence_id(pid);
	atload(pid);
	sp_ec_feedback.set_persistence_id(pid);
	atload(pid);
	sp_ec_volume.set_persistence_id(pid);
	atload(pid);
	sp_ec_pan.set_persistence_id(pid);
	atload(pid);
	sp_mute.set_persistence_id(pid);
	atload(pid);
	sp_spin.set_persistence_id(pid);
	atload(pid);
	sp_pan.set_persistence_id(pid);
		
	atload(counter);
	
	for (i=0; i<counter; i++)
	{
		atload(type);
		switch(type)
		{
			case TX_FX_BUILTINCUTOFF:
				for (t=0; t<fx_list.size(); t++) effect_down(lp_fx);
			break;
			
			case TX_FX_BUILTINECHO:
				for (t=0; t<fx_list.size(); t++) effect_down(ec_fx);
			break;
			
			case TX_FX_LADSPA:
				atload(id);
				plugin=LADSPA_Plugin::getPluginByUniqueID(id);
				if (plugin)
				{
					ladspa_effect=add_effect(plugin);
					ladspa_effect->load(input);
				}
				else
				{
					sprintf(buffer,"Couldn't find required plugin with ID [%li].", id);
					tx_note(buffer, true);
					res++;
				}
			break;
			
			default:
				tx_note("Fatal error loading set: unknown effect type!", true);
				res++;
		}		
	}

	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_x_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_x_input_parameter(NULL);
	
	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_y_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_y_input_parameter(NULL);

	atload(hidden);
	gui.main_panel->hide(hidden);

	atload(hidden);
	gui.trigger_panel->hide(hidden);

	atload(hidden);
	gui.lp_panel->hide(hidden);

	atload(hidden);
	gui.ec_panel->hide(hidden);
	
	return(res);
}

int vtt_class :: load_14(FILE * input)
{
	int res=0;
	guint32 pid;
	int32_t counter;
	int32_t type;
	long id;
	int i;
	unsigned int t;
	LADSPA_Plugin *plugin;
	char buffer[256];
	vtt_fx_ladspa *ladspa_effect;
	guint8 hidden;
	
	atload(buffer);
	this->set_name(buffer);
	atload(filename);
	atload(is_sync_master);
	atload(is_sync_client);
	atload(sync_cycles);
	atload(rel_volume);
	atload(rel_pitch);
	recalc_pitch();
	
	atload(autotrigger);
	atload(loop);
	
	atload(mute);
	atload(pan);
	
	atload(lp_enable);
	atload(lp_gain);
	atload(lp_reso);
	atload(lp_freq);
	lp_setup(lp_gain, lp_reso, lp_freq);
	
	atload(ec_enable);
	atload(ec_length);
	ec_set_length(ec_length);
	atload(ec_feedback);
	ec_set_feedback(ec_feedback);
	atload(ec_pan);
	ec_set_pan(ec_pan);
	atload(ec_volume);
	ec_set_volume(ec_volume);

	atload(audio_hidden);
	atload(control_hidden);
	
	recalc_volume();

	atload(pid);
	sp_speed.set_persistence_id(pid);
	atload(pid);
	sp_volume.set_persistence_id(pid);
	atload(pid);
	sp_pitch.set_persistence_id(pid);
	atload(pid);
	sp_trigger.set_persistence_id(pid);
	atload(pid);
	sp_loop.set_persistence_id(pid);
	atload(pid);
	sp_sync_client.set_persistence_id(pid);
	atload(pid);
	sp_sync_cycles.set_persistence_id(pid);
	atload(pid);
	sp_lp_enable.set_persistence_id(pid);
	atload(pid);
	sp_lp_gain.set_persistence_id(pid);
	atload(pid);
	sp_lp_reso.set_persistence_id(pid);
	atload(pid);
	sp_lp_freq.set_persistence_id(pid);
	atload(pid);
	sp_ec_enable.set_persistence_id(pid);
	atload(pid);
	sp_ec_length.set_persistence_id(pid);
	atload(pid);
	sp_ec_feedback.set_persistence_id(pid);
	atload(pid);
	sp_ec_volume.set_persistence_id(pid);
	atload(pid);
	sp_ec_pan.set_persistence_id(pid);
	atload(pid);
	sp_mute.set_persistence_id(pid);
	atload(pid);
	sp_spin.set_persistence_id(pid);
	atload(pid);
	sp_pan.set_persistence_id(pid);
		
	atload(counter);
	
	for (i=0; i<counter; i++)
	{
		atload(type);
		switch(type)
		{
			case TX_FX_BUILTINCUTOFF:
				for (t=0; t<fx_list.size(); t++) effect_down(lp_fx);
			break;
			
			case TX_FX_BUILTINECHO:
				for (t=0; t<fx_list.size(); t++) effect_down(ec_fx);
			break;
			
			case TX_FX_LADSPA:
				atload(id);
				plugin=LADSPA_Plugin::getPluginByUniqueID(id);
				if (plugin)
				{
					ladspa_effect=add_effect(plugin);
					ladspa_effect->load(input);
				}
				else
				{
					sprintf(buffer,"Couldn't find required plugin with ID [%li].", id);
					tx_note(buffer, true);
					res++;
				}
			break;
			
			default:
				tx_note("Fatal error loading set: unknown effect type!", true);
				res++;
		}		
	}

	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_x_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_x_input_parameter(NULL);
	
	atload(pid);
	
	if (pid)
	{
		atload(pid);
		set_y_input_parameter(tX_seqpar :: get_sp_by_persistence_id(pid));
	}
	else set_y_input_parameter(NULL);

	atload(hidden);
	gui.main_panel->hide(hidden);

	atload(hidden);
	gui.trigger_panel->hide(hidden);

	atload(hidden);
	gui.lp_panel->hide(hidden);

	atload(hidden);
	gui.ec_panel->hide(hidden);
	
	return(res);
}

int  vtt_class :: load_all_10(FILE* input, char *fname)
{
	int res=0, restmp=0;
	unsigned int i, max;
	vtt_class *newvtt;
	char ftmp[PATH_MAX];
	
	while (main_list.size())
	{
		delete((*main_list.begin()));
	}
		
	atload(max);
	atload(master_volume);
	set_master_volume(master_volume);
	globals.volume=master_volume;
	atload(globals.pitch);	
	set_master_pitch(globals.pitch);

	ld_create_loaddlg(TX_LOADDLG_MODE_MULTI, max);
	ld_set_setname(fname);

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(1);
		res+=newvtt->load_10(input);
		
		if (strlen(newvtt->filename))
		{
			/* ftmp IS NECESSARY !!! */
			strcpy(ftmp, newvtt->filename);
			ld_set_filename(ftmp);
			
			//restmp=load_wav(newvtt->filename, &newbuffer, &size);
			restmp=(int) newvtt->load_file(ftmp);
			res+=restmp;
		}
		gtk_box_pack_start(GTK_BOX(control_parent), newvtt->gui.control_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(audio_parent), newvtt->gui.audio_box, TRUE, TRUE, 0);
	}
	
	sequencer.delete_all_events(tX_sequencer::DELETE_ALL);
	
	ld_destroy();
	
	return(res);
}


int  vtt_class :: load_all_11(FILE* input, char *fname)
{
	int res=0, restmp=0;
	unsigned int i, max;
	vtt_class *newvtt;
	char ftmp[PATH_MAX];
	guint32 pid;
	
	while (main_list.size())
	{
		delete((*main_list.begin()));
	}
		
	atload(max);
	atload(master_volume);
	set_master_volume(master_volume);
	globals.volume=master_volume;
	atload(globals.pitch);	
	set_master_pitch(globals.pitch);
	atload(pid);
	sp_master_volume.set_persistence_id(pid);
	atload(pid);
	sp_master_pitch.set_persistence_id(pid);

	ld_create_loaddlg(TX_LOADDLG_MODE_MULTI, max);
	ld_set_setname(fname);

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(1);
		res+=newvtt->load_11(input);
		
		if (strlen(newvtt->filename))
		{
			/* ftmp IS NECESSARY !!! */
			strcpy(ftmp, newvtt->filename);
			ld_set_filename(ftmp);
			
			//restmp=load_wav(newvtt->filename, &newbuffer, &size);
			restmp=(int) newvtt->load_file(ftmp);
			res+=restmp;
		}
		gtk_box_pack_start(GTK_BOX(control_parent), newvtt->gui.control_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(audio_parent), newvtt->gui.audio_box, TRUE, TRUE, 0);
		
	}
	
	sequencer.load(input);
	
	ld_destroy();
	
	return(res);
}


int  vtt_class :: load_all_12(FILE* input, char *fname)
{
	int res=0, restmp=0;
	unsigned int i, max;
	vtt_class *newvtt;
	char ftmp[PATH_MAX];
	guint32 pid;
	
	while (main_list.size())
	{
		delete((*main_list.begin()));
	}
		
	atload(max);
	atload(master_volume);
	set_master_volume(master_volume);
	globals.volume=master_volume;
	atload(globals.pitch);	
	set_master_pitch(globals.pitch);
	atload(pid);
	sp_master_volume.set_persistence_id(pid);
	atload(pid);
	sp_master_pitch.set_persistence_id(pid);

	ld_create_loaddlg(TX_LOADDLG_MODE_MULTI, max);
	ld_set_setname(fname);

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(1);
		res+=newvtt->load_12(input);
		
		if (strlen(newvtt->filename))
		{
			/* ftmp IS NECESSARY !!! */
			strcpy(ftmp, newvtt->filename);
			ld_set_filename(ftmp);
			
			//restmp=load_wav(newvtt->filename, &newbuffer, &size);
			restmp=(int) newvtt->load_file(ftmp);
			res+=restmp;
		}
		gtk_box_pack_start(GTK_BOX(control_parent), newvtt->gui.control_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(audio_parent), newvtt->gui.audio_box, TRUE, TRUE, 0);
		
	}
	
	sequencer.load(input);
	
	ld_destroy();
	
	return(res);
}

int  vtt_class :: load_all_13(FILE* input, char *fname)
{
	int res=0, restmp=0;
	unsigned int i, max;
	vtt_class *newvtt;
	char ftmp[PATH_MAX];
	guint32 pid;
	
	while (main_list.size())
	{
		delete((*main_list.begin()));
	}
		
	atload(max);
	atload(master_volume);
	set_master_volume(master_volume);
	globals.volume=master_volume;
	atload(globals.pitch);	
	set_master_pitch(globals.pitch);
	atload(pid);
	sp_master_volume.set_persistence_id(pid);
	atload(pid);
	sp_master_pitch.set_persistence_id(pid);

	ld_create_loaddlg(TX_LOADDLG_MODE_MULTI, max);
	ld_set_setname(fname);

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(1);
		res+=newvtt->load_13(input);
		
		if (strlen(newvtt->filename))
		{
			/* ftmp IS NECESSARY !!! */
			strcpy(ftmp, newvtt->filename);
			ld_set_filename(ftmp);
			
			//restmp=load_wav(newvtt->filename, &newbuffer, &size);
			restmp=(int) newvtt->load_file(ftmp);
			res+=restmp;
		}
		gtk_box_pack_start(GTK_BOX(control_parent), newvtt->gui.control_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(audio_parent), newvtt->gui.audio_box, TRUE, TRUE, 0);
		
	}
	
	sequencer.load(input);
	
	ld_destroy();
	
	return(res);
}

int  vtt_class :: load_all_14(FILE* input, char *fname)
{
	int res=0, restmp=0;
	unsigned int i, max;
	vtt_class *newvtt;
	char ftmp[PATH_MAX];
	guint32 pid;
	
	while (main_list.size())
	{
		delete((*main_list.begin()));
	}
		
	atload(max);
	atload(master_volume);
	set_master_volume(master_volume);
	globals.volume=master_volume;
	atload(globals.pitch);	
	set_master_pitch(globals.pitch);
	atload(pid);
	sp_master_volume.set_persistence_id(pid);
	atload(pid);
	sp_master_pitch.set_persistence_id(pid);

	ld_create_loaddlg(TX_LOADDLG_MODE_MULTI, max);
	ld_set_setname(fname);

	for (i=0; i<max; i++)
	{
		newvtt=new vtt_class(1);
		res+=newvtt->load_14(input);
		
		if (strlen(newvtt->filename))
		{
			/* ftmp IS NECESSARY !!! */
			strcpy(ftmp, newvtt->filename);
			ld_set_filename(ftmp);
			
			//restmp=load_wav(newvtt->filename, &newbuffer, &size);
			restmp=(int) newvtt->load_file(ftmp);
			res+=restmp;
		}
		gtk_box_pack_start(GTK_BOX(control_parent), newvtt->gui.control_box, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(audio_parent), newvtt->gui.audio_box, TRUE, TRUE, 0);
	    if (newvtt->audio_hidden) newvtt->hide_audio(newvtt->audio_hidden);
	    if (newvtt->control_hidden) newvtt->hide_control(newvtt->control_hidden);
	}
	
	sequencer.load(input);
	
	ld_destroy();
	
	return(res);
}

void vtt_fx_ladspa :: load (FILE *input)
{
	guint32 count;
	unsigned int i;
	list <tX_seqpar_vttfx *> :: iterator sp;
	guint32 pid;
	guint8 hidden;
	float value;
	
	fread((void *) &count, sizeof(count), 1, input);
	
	if (count!=controls.size())
	{
		fprintf(stderr, "tX: Ouch! Plugin %li has less/more controls than saved!\n", plugin->getUniqueID());
	}
	
	for (i=0, sp=controls.begin(); (i<count) && (sp!=controls.end()); i++, sp++) {
		fread((void *) &pid, sizeof(pid), 1, input);
		(*sp)->set_persistence_id(pid);
		fread((void *) &value, sizeof(value), 1, input);
		(*sp)->do_exec(value);
		(*sp)->do_update_graphics();
	} 
	
	fread((void *) &hidden, sizeof(hidden), 1, input);
	panel->hide(hidden);
}

void tX_sequencer :: load(FILE *in)
{
	guint32 event_count=0;
	guint32 i;
	tX_event *new_event=NULL;
	
	delete_all_events(tX_sequencer::DELETE_ALL);
	
	fread ((void *) &event_count, sizeof(event_count), 1, in);
	
	max_timestamp=0;
	
	for (i=0; i<event_count; i++)
	{
		new_event=new tX_event(in);
		song_list.push_back(new_event);
	}
	
	start_timestamp=0;
	current_timestamp=0;
	
	if (new_event) max_timestamp=new_event->get_timestamp();
}

tX_event :: tX_event (FILE *input)
{
	unsigned int sp_persistence_id;

	fread((void *) &sp_persistence_id, sizeof(sp_persistence_id), 1, input);
	fread((void *) &timestamp, sizeof(timestamp), 1, input);
	fread((void *) &value, sizeof(value), 1, input);

	sp=tX_seqpar::get_sp_by_persistence_id(sp_persistence_id);
	if (!sp)
	{
		fprintf(stderr, "oops: couldn't resolve sp by persistence id %i.\n", sp_persistence_id);
	}
}

#endif
