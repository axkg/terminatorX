/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2002  Alexander König
 
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

#include <stdio.h>
#include <stdlib.h>
#include "tX_types.h"
#include "tX_global.h"
#include "string.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define TX_XML_RC_VERSION "1.0"

tx_global globals;

void get_rc_name_old(char *buffer)
{
	strcpy(buffer,"");

	if (getenv("HOME"))
	{
		strcpy(buffer, getenv("HOME"));
		if (buffer[strlen(buffer)-1]!='/')
		strcat(buffer, "/");
	}
	
	strcat(buffer, ".terminatorX3rc.bin");
}

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
	
	strcpy(globals.audio_device, "/dev/dsp");
	
	strcpy(globals.xinput_device, "");
	globals.xinput_enable=0;
	
	globals.update_idle=18;
	globals.update_delay=1;
	
	globals.buff_no=2;
	globals.buff_size=9;
	
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
	globals.audiodevice_type=TX_AUDIODEVICE_TYPE_OSS;		
#else
#ifdef USE_ALSA
	globals.audiodevice_type=TX_AUDIODEVICE_TYPE_ALSA;
#endif	
#endif		
	globals.audiodevice_buffersize=4096;
	strcpy(globals.audiodevice_oss_devicename, "/dev/dsp");
	globals.audiodevice_alsa_card=0;
	globals.audiodevice_alsa_pcm=0;		

	globals.use_stdout_cmdline=0;
	globals.current_path = NULL;
	globals.pitch=1.0;
	globals.volume=1.0;	
	if (!globals.true_block_size) globals.true_block_size=1<globals.buff_size;

}

void load_globals_old()
{	
	char rc_name[PATH_MAX]="";	
	FILE *rc;
	get_rc_name_old(rc_name);
	
	rc=fopen(rc_name, "r");
	if (rc)
	{
		fread(&globals, sizeof(tx_global), 1, rc);
		fclose(rc);
	}
	else
	{
		fprintf(stderr, "tX: .rc-file '%s' doesn't exist, reverting to defaults\n", rc_name);
		set_global_defaults();
	}

	/* i'll have to keep these as they're in the code
          everywhere but I think it doesn't make sense resetting
	  to old values on startup....
	*/
	globals.use_stdout_cmdline=0;
	globals.current_path = NULL;
	globals.pitch=1.0;
	globals.volume=1.0;	
	if (!globals.true_block_size) globals.true_block_size=1<globals.buff_size;
}

void load_globals() {
	if (load_globals_xml()!=0) {
		fprintf(stderr, "tX: Failed loading terminatorXrc - trying to load old binary rc.\n");
		load_globals_old();
	}
}

#define restore_int(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) { sscanf(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%i", &i); }}
#define restore_float(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if  (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {sscanf(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%lf", &dvalue); i=dvalue;}}
#define restore_string(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {strcpy(i, xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)); }}

int load_globals_xml() {
	char rc_name[PATH_MAX]="";	
	xmlDocPtr doc;
	xmlNodePtr cur;
	int elementFound;
	double dvalue;

	set_global_defaults();
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
			restore_string("audio_device", globals.audio_device);
			restore_string("xinput_device", globals.xinput_device);
			restore_int("xinput_enable", globals.xinput_enable);
			restore_int("update_idle", globals.update_idle);
			restore_int("update_delay", globals.update_delay);
			restore_int("buff_no", globals.buff_no);
			restore_int("buff_size", globals.buff_size);
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

			if (!elementFound) {
				fprintf(stderr, "tX: Unhandled XML element: \"%s\"\n", cur->name);
			}
		}
	}
	
	return 0;
}

#define store_int(s, i); fprintf(rc, "\t<%s>%i</%s>\n", s,(int) i, s);
#define store_float(s, i); fprintf(rc, "\t<%s>%lf</%s>\n", s,(double) i, s);
#define store_string(s, i); fprintf(rc, "\t<%s>%s</%s>\n", s, i, s);

void store_globals() {
	char rc_name[PATH_MAX]="";
	FILE *rc;
	
	get_rc_name(rc_name);

	rc=fopen(rc_name, "w");
	
	if (rc) {		
		fprintf(rc, "<?xml version=\"1.0\" encoding=\"US-ASCII\"?>\n\n");
		fprintf(rc, "<!-- Warning: this file will be rewritten by terminatorX on exit!\n     Don\'t waste your time adding comments - they will be erased -->\n\n" );
		fprintf(rc, "<terminatorXrc version=\"%s\">\n", TX_XML_RC_VERSION);

		store_int("store_globals", globals.store_globals);
		store_string("audio_device", globals.audio_device);
		store_string("xinput_device", globals.xinput_device);
		store_int("xinput_enable", globals.xinput_enable);
		store_int("update_idle", globals.update_idle);
		store_int("update_delay", globals.update_delay);
		store_int("buff_no", globals.buff_no);
		store_int("buff_size", globals.buff_size);
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
		
		fprintf(rc,"</terminatorXrc>\n");
	}
}
