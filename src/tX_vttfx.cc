/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2014  Alexander König
 
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
 
    File: tX_vttfx.cc
 
    Description: This handles the effects in the per vtt fx chain. Supports the
                 buitlin echo/lowpass effects and ladspa plugins.
*/

#include "tX_vttfx.h"
#include <stdio.h>
#include <glib.h>
#include "tX_vtt.h"
#define myvtt ((vtt_class *) vtt)
#include "tX_global.h"

float ladspa_dummy_output_port;

void vtt_fx :: reconnect_buffer()
{
	/* NOP */
}

vtt_fx :: ~vtt_fx() {}
void vtt_fx::toggle_drywet() {}
bool vtt_fx::is_stereo() { return false; }
tX_drywet_type vtt_fx::has_drywet_feature() { return NOT_DRYWET_CAPABLE; }

/******************* builtin fx ***/

/* lowpass */ 
void vtt_fx_lp :: activate() { myvtt->lp_reset(); }
void vtt_fx_lp :: deactivate() { /* NOP */ }
void vtt_fx_lp :: run() { myvtt->render_lp(); }
int vtt_fx_lp :: isEnabled() { return myvtt->lp_enable; }

void vtt_fx_lp :: save (FILE *rc, gzFile rz, char *indent) { 
	tX_store("%s<cutoff/>\n", indent);
}


const char *vtt_fx_lp :: get_info_string()
{
	return "TerminatorX built-in resonant lowpass filter.";
}


/* echo */
void vtt_fx_ec :: activate() { /* NOP */ }
void vtt_fx_ec :: deactivate() { myvtt->ec_clear_buffer(); }
void vtt_fx_ec :: run() { myvtt->render_ec(); }
int vtt_fx_ec :: isEnabled() { return myvtt->ec_enable; }

void vtt_fx_ec :: save (FILE *rc, gzFile rz, char *indent) { 
	tX_store("%s<lowpass/>\n", indent);	
}

const char *vtt_fx_ec :: get_info_string()
{
	return "TerminatorX built-in echo effect.";
}

/******************** LADSPA fx ***/
/* short cut "cpd" macro to current port descriptor */

#define cpd plugin->getDescriptor()->PortDescriptors[port]
#define cpn plugin->getDescriptor()->PortNames[port]
#define cph plugin->getDescriptor()->PortRangeHints[port]

void vtt_fx_ladspa :: reconnect_buffer()
{
	plugin->getDescriptor()->connect_port(instance, input_port, myvtt->output_buffer);
	if (wet_buffer) {
		plugin->getDescriptor()->connect_port(instance, output_port, wet_buffer);	
	} else {
		plugin->getDescriptor()->connect_port(instance, output_port, myvtt->output_buffer);	
	}
}

static void wrapstr(char *str)
{
	char temp[256]="";
	char target[2048]="";
	char *token;

	token=strtok(str, " ");	
	
	while(token)
	{
		if (strlen(token)+strlen(temp)<10)
		{
			if (strlen(temp)) strcat(temp, " ");
			strcat(temp, token);
		}
		else
		{
			if (strlen(temp))
			{
				if(strlen(target)) strcat(target, "\n");
				if(strlen(temp)>10)
				{
					temp[8]='.';
					temp[9]='.';
					temp[10]='.';
					temp[11]=0;
				}
				strcat(target, temp);
				strcpy(temp,token);
			}
		}
		token=strtok(NULL, " ");
	}

       if (strlen(temp))
       {
               if(strlen(target)) strcat(target, "\n");
               strcat(target, temp);
       }

	strcpy(str, target);
 }

vtt_fx_ladspa :: vtt_fx_ladspa(LADSPA_Plugin *p, void *v)
{
	int port;
	float min, max;
	char buffer[2048];
	char buffer2[2048];
	
	tX_seqpar_vttfx *sp;

	plugin=p; vtt=v;
	
	instance=(LADSPA_Handle *) plugin->getDescriptor()->instantiate(plugin->getDescriptor(), 44100);
	
	if (!instance)
	{
		fprintf (stderr, "tX: Fatal Error: failed to instantiate plugin \"%s\".\n", plugin->getDescriptor()->Name);
		/* How to handle this ? */
	}
	
	sp = sp_enable = new tX_seqpar_vttfx_bool();
	sp->set_mapping_parameters(1, 0, 0, 0);
	sprintf(buffer, "%s: Enable", plugin->getName());
	sp->set_name(buffer, "Enable");
	sp->set_vtt(vtt);
	controls.push_back(sp);	

	sp_wet=NULL;
	wet_buffer=NULL;
	
	/* connecting ports */
	for (port=0; port < plugin->getPortCount(); port++) {
		if (LADSPA_IS_PORT_AUDIO(cpd)) {
			if (LADSPA_IS_PORT_INPUT(cpd)) input_port=port;
			else if (LADSPA_IS_PORT_OUTPUT(cpd)) output_port=port;
		} else if ((LADSPA_IS_PORT_CONTROL(cpd)) && (LADSPA_IS_PORT_INPUT(cpd))) {
			min=-22100;
			max=+22100;
			
			if (LADSPA_IS_HINT_BOUNDED_BELOW(cph.HintDescriptor)) min=cph.LowerBound;
			if (LADSPA_IS_HINT_BOUNDED_ABOVE(cph.HintDescriptor)) max=cph.UpperBound;
			
			if (LADSPA_IS_HINT_SAMPLE_RATE(cph.HintDescriptor)) {
				min*=44100; max*=44100;
			}
			
			if (LADSPA_IS_HINT_TOGGLED(cph.HintDescriptor)) {
				sp=new tX_seqpar_vttfx_bool();
				sp->set_mapping_parameters(max, min, 0, 0);
			} else if (LADSPA_IS_HINT_INTEGER(cph.HintDescriptor)) {
				sp=new tX_seqpar_vttfx_int();
				sp->set_mapping_parameters(max, min, 0, 0);
			} else {
				sp=new tX_seqpar_vttfx_float();
				sp->set_mapping_parameters(max, min, (max-min)/100.0, 1);
			}
			
			sprintf(buffer, "%s: %s", plugin->getLabel(), cpn);
			strcpy(buffer2, cpn);
			wrapstr(buffer2);
			
			sp->set_name(buffer, buffer2);
			sp->set_vtt(vtt);
			plugin->getDescriptor()->connect_port(instance, port, sp->get_value_ptr());
			controls.push_back(sp);
		} else if ((LADSPA_IS_PORT_CONTROL(cpd)) && (LADSPA_IS_PORT_OUTPUT(cpd))) {
			plugin->getDescriptor()->connect_port(instance, port, &ladspa_dummy_output_port);
		}
	}
}

void vtt_fx_ladspa :: realloc_drywet() 
{
	free_drywet();
	wet_buffer=(f_prec *) malloc(sizeof(float)*vtt_class::samples_in_mix_buffer);
}

void vtt_fx_ladspa :: free_drywet()
{
	if (wet_buffer) {
		free(wet_buffer);
		wet_buffer=NULL;
	}
}

void vtt_fx_ladspa :: activate()
{
	if (sp_wet) {
		realloc_drywet();
	}
	reconnect_buffer(); // we always have to reconnect...
	if (plugin->getDescriptor()->activate) plugin->getDescriptor()->activate(instance);
}

void vtt_fx_ladspa :: deactivate()
{
	if (plugin->getDescriptor()->deactivate) plugin->getDescriptor()->deactivate(instance);
	
	free_drywet();
}

void vtt_fx_ladspa :: run()
{
	plugin->getDescriptor()->run(instance, (vtt_class::samples_in_mix_buffer)>>1);
	
	if (wet_buffer) {
		f_prec wet=sp_wet->get_value();
		f_prec dry=1.0-wet;
		
		for (int sample=0; sample < (vtt_class::samples_in_mix_buffer)>>1; sample++) {
			myvtt->output_buffer[sample]=dry*myvtt->output_buffer[sample]+wet*wet_buffer[sample];
		}
	}
}

int vtt_fx_ladspa :: isEnabled()
{
	return (int) sp_enable->get_value();
}

const char *vtt_fx_ladspa :: get_info_string()
{
	return plugin->get_info_string();
}

vtt_fx_ladspa :: ~vtt_fx_ladspa()
{
	list <tX_seqpar_vttfx *> :: iterator sp;
	
	while (controls.size()) {
		sp=controls.begin();
		controls.remove((*sp));
		
		delete (*sp);
	}		
	plugin->getDescriptor()->cleanup(instance);
	
	if (wet_buffer) free(wet_buffer);
	delete panel;
}


void vtt_fx_ladspa :: save (FILE *rc, gzFile rz, char *indent) {
	long ID=plugin->getUniqueID();
	list <tX_seqpar_vttfx *> :: iterator sp;
	
	tX_store("%s<ladspa_plugin>\n", indent);
	strcat (indent, "\t");
	
	store_int("ladspa_id", ID);
	store_bool("has_drywet", (sp_wet!=NULL));
	
	for (sp=controls.begin(); sp!=controls.end(); sp++) {
		store_float_sp("param", (*sp)->get_value(), (*(*sp)));
	}
	
	store_bool("panel_hidden", panel->is_hidden());
	
	indent[strlen(indent)-1]=0;
	tX_store("%s</ladspa_plugin>\n", indent);
}

void vtt_fx_ladspa :: load(xmlDocPtr doc, xmlNodePtr node) {
	int dummy;
	bool hidden=false;
	list <tX_seqpar_vttfx *> :: iterator sp=controls.begin();
	int elementFound;
	double val;
	
	for (xmlNodePtr cur=node->xmlChildrenNode; cur!=NULL; cur=cur->next) {
		if (cur->type == XML_ELEMENT_NODE) {
			bool drywet=false;
			elementFound=0;
			
			restore_int("ladspa_id", dummy);
			restore_bool("panel_hidden", hidden);
			restore_bool("has_drywet", drywet);
			if (drywet) add_drywet();
			
			if ((!elementFound) && (xmlStrcmp(cur->name, (xmlChar *) "param")==0)) {
				val=0;
				elementFound=0;
				double dvalue;
			
				if (sp==controls.end()) {
					tX_warning("found unexpected parameters for ladspa plugin [%i].", dummy);
				} else {
					restore_float_id("param", val, (*(*sp)), (*sp)->do_exec(val));					
					(*sp)->do_update_graphics();
					sp++;
				}
			}
			
			if (!elementFound) {
				tX_warning("unhandled ladspa_plugin element %s.", cur->name);
			}
		}
	}
	
	panel->hide(!hidden);
}

void vtt_fx_ladspa :: toggle_drywet() {
	if (sp_wet) {
		remove_drywet();
	} else {
		add_drywet();
	}
}

void vtt_fx_ladspa :: add_drywet() {
	char buffer[1024];
	
	sp_wet=new tX_seqpar_vttfx_float();
	sp_wet->set_mapping_parameters(1.0, 0, 0.01, 1);
	sprintf(buffer, "%s: Dry/Wet", plugin->getLabel());
	sp_wet->set_name(buffer, "Dry/Wet");
	sp_wet->set_vtt(vtt);
	panel->add_client_widget(sp_wet->get_widget());
	
	pthread_mutex_lock(&vtt_class::render_lock);
	controls.push_back(sp_wet);
	deactivate();
	activate();
	pthread_mutex_unlock(&vtt_class::render_lock);
}

void vtt_fx_ladspa :: remove_drywet() {
	pthread_mutex_lock(&vtt_class::render_lock);
	deactivate();

	controls.remove(sp_wet);
	delete sp_wet;
	sp_wet=NULL;
	
	activate();
	pthread_mutex_unlock(&vtt_class::render_lock);
}

tX_drywet_type vtt_fx_ladspa::has_drywet_feature()
{ 
	if (sp_wet) return DRYWET_ACTIVE;
	else return DRYWET_AVAILABLE;
}

/****** STEREO plugins **********/

vtt_fx_stereo_ladspa::vtt_fx_stereo_ladspa(LADSPA_Stereo_Plugin *p, void *v):vtt_fx_ladspa(p,v)
{
	input_port=input2_port=-1; 
	output_port=output2_port=-1; 
	wet_buffer2=NULL;
	
	for (int port=0; port < plugin->getPortCount(); port++) {
		if (LADSPA_IS_PORT_AUDIO(cpd)) {
			if (LADSPA_IS_PORT_INPUT(cpd)) {
				if (input_port<0) { input_port=port; }
				else if (input2_port<0) { input2_port=port; }
				else { tX_error("Extra input port for plugin %s?", plugin->getName()); }
			} else if (LADSPA_IS_PORT_OUTPUT(cpd)) {
				if (output_port<0) { output_port=port; }
				else if (output2_port<0) { output2_port=port; }
				else { tX_error("Extra output port for plugin %s?", plugin->getName()); }
			}
		}
	}
};

void vtt_fx_stereo_ladspa :: reconnect_buffer()
{
	plugin->getDescriptor()->connect_port(instance, input_port, myvtt->output_buffer);
	plugin->getDescriptor()->connect_port(instance, input2_port, myvtt->output_buffer2);

	if (wet_buffer) {
		plugin->getDescriptor()->connect_port(instance, output_port, wet_buffer);	
		plugin->getDescriptor()->connect_port(instance, output2_port, wet_buffer2);
	} else {
		plugin->getDescriptor()->connect_port(instance, output_port, myvtt->output_buffer);	
		plugin->getDescriptor()->connect_port(instance, output2_port, myvtt->output_buffer2);	
	}
}

void vtt_fx_stereo_ladspa :: realloc_drywet() 
{
	free_drywet();
	wet_buffer=(f_prec *) malloc(sizeof(float)*vtt_class::samples_in_mix_buffer);
	wet_buffer2=(f_prec *) malloc(sizeof(float)*vtt_class::samples_in_mix_buffer);
}

void vtt_fx_stereo_ladspa :: free_drywet()
{
	if (wet_buffer) {
		free(wet_buffer);
		wet_buffer=NULL;
	}
	if (wet_buffer2) {
		free(wet_buffer2);
		wet_buffer2=NULL;
	}
}

void vtt_fx_stereo_ladspa :: run()
{
	plugin->getDescriptor()->run(instance, (vtt_class::samples_in_mix_buffer)>>1);
	
	if (wet_buffer) {
		f_prec wet=sp_wet->get_value();
		f_prec dry=1.0-wet;
		
		for (int sample=0; sample < (vtt_class::samples_in_mix_buffer)>>1; sample++) {
			myvtt->output_buffer[sample]=dry*myvtt->output_buffer[sample]+wet*wet_buffer[sample];
			myvtt->output_buffer2[sample]=dry*myvtt->output_buffer2[sample]+wet*wet_buffer2[sample];
		}
	}
}

vtt_fx_stereo_ladspa :: ~vtt_fx_stereo_ladspa()
{
	// rest should be handeld in parent's destrucutor.
	if (wet_buffer2) free(wet_buffer2);
}

bool vtt_fx_stereo_ladspa::is_stereo() { return true; }
