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
 
    File: tX_global.c
 
    Description:  This file contains the routines for handling the
    		  "globals" block. Intializing, reading setup from
		  disk and storing it.
		  
    Changes:
    
    21 Jul 1999: introduced the lowpass globals.
	There were some changes in between...
	14 Jul 2002: switched to libxml instead of binary saving.
*/    

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include "tX_types.h"
#include "tX_global.h"
#include "string.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define TX_XML_RC_VERSION "1.0"

tx_global globals;

void get_rc_name(char *buffer)
{
	strcpy(buffer,"");
	if (globals.alternate_rc)
	{
		strcpy(buffer, globals.alternate_rc);
	}
	else 
	{
		if (getenv("HOME"))
		{
			strcpy(buffer, getenv("HOME"));
			if (buffer[strlen(buffer)-1]!='/')
			strcat(buffer, "/");
		}
		strcat(buffer, ".terminatorXrc");
	}
}

void set_global_defaults() {
	globals.startup_set = 0;
	globals.store_globals = 1;
	globals.no_gui = 0;
	globals.alternate_rc = 0;
	
	
	strcpy(globals.xinput_device, "");
	globals.xinput_enable=0;
	
	globals.update_idle=18;
	globals.update_delay=1;
	
	strcpy(globals.oss_device, "/dev/dsp");
	globals.oss_buff_no=2;
	globals.oss_buff_size=9;
	globals.oss_samplerate=44100;

	strcpy(globals.alsa_device, "hw:0,0");	
	globals.alsa_buff_no=2;
	globals.alsa_buff_size=1024;
	globals.alsa_samplerate=44100;
	
	globals.sense_cycles=12;
	
	globals.mouse_speed=0.8;
	
	globals.width=800;
	globals.height=440;	
	
	globals.tooltips=1;
	
	globals.use_stdout=0;
	globals.use_stdout_from_conf_file=0;
	
	globals.show_nag=1;
	globals.prelis=1;
	
	strcpy(globals.last_fn,"");
	
	globals.pitch=1.0;
	globals.volume=1.0;
	globals.gui_wrap=3;
	
	globals.flash_response=0.95;
	
	globals.button_type=BUTTON_TYPE_BOTH;
		
	globals.true_block_size=0;
	
	strcpy(globals.tables_filename, "");
	strcpy(globals.record_filename, "tX_record.wav");
	strcpy(globals.file_editor, "");
		
#ifdef USE_OSS
	globals.audiodevice_type=OSS;		
#else
#ifdef USE_ALSA
	globals.audiodevice_type=ALSA;
#endif	
#endif		
	globals.use_stdout_cmdline=0;
	globals.current_path = NULL;
	globals.pitch=1.0;
	globals.volume=1.0;
	
	strcpy(globals.lrdf_path, "/usr/share/ladspa/rdf:/usr/local/share/ladspa/rdf");
	globals.fullscreen_enabled=1;
	
	if (!globals.true_block_size) globals.true_block_size=1<globals.oss_buff_size;
}

int load_globals_xml() {
	char rc_name[PATH_MAX]="";	
	char device_type[16]="oss";
	xmlDocPtr doc;
	xmlNodePtr cur;
	int elementFound;
	double dvalue;

	get_rc_name(rc_name);
	
	doc = xmlParseFile(rc_name);
	
	if (doc == NULL ) {
		fprintf(stderr, "tX: err: Error parsing terminatorXrc.\n");
		return 1;
	}
	
	cur = xmlDocGetRootElement(doc);
	
	if (cur == NULL) {
		fprintf(stderr,"tX: err: terminatorXrc contains no elements.\n");
		xmlFreeDoc(doc);
		return 2;
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "terminatorXrc")) {
		fprintf(stderr,"tX: err: This terminatorXrc is not a terminatorXrc.");
		xmlFreeDoc(doc);
		return 3;
	}
		
	for (cur=cur->xmlChildrenNode; cur != NULL; cur = cur->next) {
		if (cur->type == XML_ELEMENT_NODE) {			
			elementFound=0;

			restore_int("store_globals", globals.store_globals);

			restore_string("audio_driver", device_type);
			
			restore_string("oss_device", globals.oss_device);
			restore_int("oss_buff_no", globals.oss_buff_no);
			restore_int("oss_buff_size", globals.oss_buff_size);
			restore_int("oss_samplerate", globals.oss_samplerate);

			restore_string("alsa_device", globals.alsa_device);
			restore_int("alsa_buff_no", globals.alsa_buff_no);
			restore_int("alsa_buff_size", globals.alsa_buff_size);
			restore_int("alsa_samplerate", globals.alsa_samplerate);

			restore_string("xinput_device", globals.xinput_device);
			restore_int("xinput_enable", globals.xinput_enable);
			restore_int("update_idle", globals.update_idle);
			restore_int("update_delay", globals.update_delay);
			restore_int("sense_cycles", globals.sense_cycles);
			restore_float("mouse_speed", globals.mouse_speed);
			restore_int("width", globals.width);
			restore_int("height", globals.height);
			restore_int("tooltips", globals.tooltips);
			restore_int("use_stdout", globals.use_stdout);
			restore_int("show_nag", globals.show_nag);
			restore_int("prelis", globals.prelis);
			restore_string("last_fn", globals.last_fn);
			restore_float("pitch", globals.pitch);
			restore_float("volume", globals.volume);
			restore_float("flash_response", globals.flash_response);
			restore_int("button_type", globals.button_type);
			restore_int("true_block_size", globals.true_block_size);
			restore_string("tables_filename", globals.tables_filename);
			restore_string("record_filename", globals.record_filename);
			restore_string("file_editor", globals.file_editor);
			restore_string("lrdf_path", globals.lrdf_path);
			
			restore_int("fullscreen_enabled", globals.fullscreen_enabled);

			if (!elementFound) {
				fprintf(stderr, "tX: Unhandled XML element: \"%s\"\n", cur->name);
			}
		}
	}

	xmlFreeDoc(doc);
	
	if (strcmp(device_type, "alsa")==0) globals.audiodevice_type=ALSA;
	else if (strcmp(device_type, "jack")==0) globals.audiodevice_type=JACK;
	else globals.audiodevice_type=OSS;
	
	return 0;
}

void store_globals() {
	char rc_name[PATH_MAX]="";
	char device_type[16];
	char indent[]="\t";
	FILE *rc;
	char tmp_xml_buffer[4096];
	
	get_rc_name(rc_name);

	rc=fopen(rc_name, "w");
	
	switch (globals.audiodevice_type) {
		case JACK:
			strcpy(device_type, "jack");
			break;
		case ALSA:
			strcpy(device_type, "alsa");
			break;
		case OSS:
		default:
			strcpy(device_type, "oss");
		
	}
	
	if (rc) {		
		fprintf(rc, "<?xml version=\"1.0\" encoding=\"US-ASCII\"?>\n\n");
		fprintf(rc, "<!-- Warning: this file will be rewritten by terminatorX on exit!\n     Don\'t waste your time adding comments - they will be erased -->\n\n" );
		fprintf(rc, "<terminatorXrc version=\"%s\">\n", TX_XML_RC_VERSION);

		store_int("store_globals", globals.store_globals);

		store_string("audio_driver", device_type);
		
		store_string("oss_device", globals.oss_device);
		store_int("oss_buff_no", globals.oss_buff_no);
		store_int("oss_buff_size", globals.oss_buff_size);
		store_int("oss_samplerate", globals.oss_samplerate);

		store_string("alsa_device", globals.alsa_device);
		store_int("alsa_buff_no", globals.alsa_buff_no);
		store_int("alsa_buff_size", globals.alsa_buff_size);
		store_int("alsa_samplerate", globals.alsa_samplerate);		
		
		store_string("xinput_device", globals.xinput_device);
		store_int("xinput_enable", globals.xinput_enable);
		store_int("update_idle", globals.update_idle);
		store_int("update_delay", globals.update_delay);
		store_int("sense_cycles", globals.sense_cycles);
		store_float("mouse_speed", globals.mouse_speed);
		store_int("width", globals.width);
		store_int("height", globals.height);
		store_int("tooltips", globals.tooltips);
		store_int("use_stdout", globals.use_stdout);
		// globals.use_stdout_from_conf_file=0; What the heck is this?
		store_int("show_nag", globals.show_nag);
		store_int("prelis", globals.prelis);
		store_string("last_fn", globals.last_fn);
		store_float("pitch", globals.pitch);
		store_float("volume", globals.volume);
		store_float("flash_response", globals.flash_response);
		store_int("button_type", globals.button_type);
		store_int("true_block_size", globals.true_block_size);
		store_string("tables_filename", globals.tables_filename);
		store_string("record_filename", globals.record_filename);
		store_string("file_editor", globals.file_editor);
		store_string("lrdf_path", globals.lrdf_path);
		store_int("fullscreen_enabled", globals.fullscreen_enabled);
		
		fprintf(rc,"</terminatorXrc>\n");
	}
}

#ifdef ENABLE_TX_LEGACY
extern void load_globals_old();
#endif

void load_globals() {
	set_global_defaults();

	if (load_globals_xml()!=0) {
		fprintf(stderr, "tX: Failed loading terminatorXrc - trying to load old binary rc.\n");
#ifdef ENABLE_TX_LEGACY		
		load_globals_old();
#endif		
	}
}

char *encode_xml(char *dest, const char *src) {
	int i, t, max;
	
	dest[0]=0;
	t=0;
	max=strlen(src);
	
	for (i=0; i<max; i++) {
		switch (src[i]) {
			case '<': dest[t]=0; strcat(dest, "&lt;"); t+=4; break;
			case '>': dest[t]=0; strcat(dest, "&gt;"); t+=4; break;
			case '&': dest[t]=0; strcat(dest, "&amp;"); t+=5; break;
			case '"': dest[t]=0; strcat(dest, "&quot;"); t+=6; break;
			case '\'': strcat(dest, "&apos;"); t+=7; break;
			default: dest[t]=src[i]; t++;
		}
	}
	dest[t]=0;
	
	//tX_debug("encode_xml: from \"%s\" to \"%s\".", src, dest); 
	return dest;
}
