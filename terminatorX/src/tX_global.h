/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2002  Alexander K�nig
 
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
 
    File: tX_global.h
 
    Description: Header to tX_global.c / defines the heavily used
    		 tX_global struct.
		 
    Changes:
    
    21 Jul 1999: Introduced the lowpass globals.
*/    

#ifndef _TX_GLOBAL_H
#define _TX_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <limits.h>
#include <stdio.h>
#include "tX_types.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define BUTTON_TYPE_ICON 1
#define BUTTON_TYPE_TEXT 2
#define BUTTON_TYPE_BOTH 3

#define TX_AUDIODEVICE_TYPE_OSS 0
#define TX_AUDIODEVICE_TYPE_ALSA 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
	
#ifdef ENABLE_DEBUG_OUTPUT	
#define tX_debug(fmt, args...); { fprintf(stderr, "- tX_debug: "); fprintf(stderr, fmt , ## args); fprintf(stderr, "\n"); }
#else
#define tX_debug(fmt, args...);
#endif
	
#define tX_error(fmt, args...); { fprintf(stderr, "* tX_error: "); fprintf(stderr, fmt , ## args); fprintf(stderr, "\n"); }
#define tX_warning(fmt, args...); { fprintf(stderr, "+ tX_warning: "); fprintf(stderr, fmt , ## args); fprintf(stderr, "\n"); }

typedef struct {
	char	audio_device[PATH_MAX];
	
	int 	xinput_enable;
	char	xinput_device[256]; // If your device's name is longer than that
				    // you are insane and you simply don't deserve
				    // running terminatorX ;) (said the guy who invented 8+3)				    				    
	int	store_globals;		// if it should store the globals vals on exit
	char 	*startup_set;	
	char	*alternate_rc;		// a diferent set of stored globals to load
	int	no_gui;			// run without any gui
	
	int	update_idle;
	
	int	buff_no;
	int	buff_size;
	
	int	sense_cycles;
	
	int 	width;
	int 	height;

	int tooltips;
	
	f_prec mouse_speed;
		
	char last_fn[PATH_MAX];

	int use_stdout;
	int use_stdout_cmdline;
	int use_stdout_from_conf_file;
	int show_nag;
	
	int prelis;
	
	f_prec pitch;
	f_prec volume;
	
	int gui_wrap;
	
	char tables_filename[PATH_MAX];
	char record_filename[PATH_MAX];
	int autoname;
	
	float flash_response;
	
	int button_type;
	
	char file_editor[PATH_MAX];
	int true_block_size;
	int update_delay; 
	
	char *current_path;
	
	/* new audiodevice handling 
	   we have *all* variables for *all* audiodevice types -
	   even if support for them is not compiled in - to keep
	   the .terminatorX3rc.bin in sync.
	*/
	
	int audiodevice_type; // TX_AUDIODEVICE_TYPE_OSS etc.
	int audiodevice_buffersize; // buffer in samples
	
	/* OSS specific options */
	char audiodevice_oss_devicename[PATH_MAX];
	
	/* ALSA specific options */
	int audiodevice_alsa_card;
	int audiodevice_alsa_pcm;		
} tx_global;

extern tx_global globals;

extern void load_globals();
extern void store_globals();
extern char *encode_xml(char *dest, const char *src);

#define nop

/* some ugly XML macros... */
#define restore_int(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) { sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%i", &i); }}
#define restore_float(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if  (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%lf", &dvalue); i=dvalue;}}
#define restore_string(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {strcpy(i, (const char *)  xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)); }}
#define restore_bool(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {if (xmlStrcmp(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1),  (const xmlChar *) "true")==0) i=true; else i=false; }}
#define restore_id(s, sp); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; pid_attr=(char* ) xmlGetProp(cur, (xmlChar *) "id"); if (pid_attr) { sscanf(pid_attr, "%i",  &pid); sp.set_persistence_id(pid); }}

#define restore_int_ac(s, i, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) { sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%i", &i); init; }}
#define restore_float_ac(s, i, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if  (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%lf", &dvalue); i=dvalue; init; }}
#define restore_string_ac(s, i, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {strcpy(i, (const char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)); init; }}
#define restore_bool_ac(s, i, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {if (xmlStrcmp(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), (const xmlChar *) "true")==0) i=true; else i=false; init; }}

#define restore_int_id(s, i, sp, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) { sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%i", &i); pid_attr=(char* ) xmlGetProp(cur, (xmlChar *) "id"); if (pid_attr) { sscanf(pid_attr, "%i",  &pid); sp.set_persistence_id(pid); } init; }}
#define restore_float_id(s, i, sp, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if  (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%lf", &dvalue); i=dvalue; pid_attr=(char* ) xmlGetProp(cur, (xmlChar *) "id"); if (pid_attr) { sscanf(pid_attr, "%i",  &pid); sp.set_persistence_id(pid); } init; }}
#define restore_bool_id(s, i, sp, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {if (xmlStrcmp(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1),  (const xmlChar *) "true")==0) i=true; else i=false; pid_attr=(char* ) xmlGetProp(cur,  (xmlChar *)"id"); if (pid_attr) { sscanf(pid_attr, "%i",  &pid); sp.set_persistence_id(pid); } init; }}

#define store_int(s, i); fprintf(rc, "%s<%s>%i</%s>\n", indent, s,(int) i, s);
#define store_float(s, i); fprintf(rc, "%s<%s>%lf</%s>\n", indent, s,(double) i, s);
#define store_string(s, i); fprintf(rc, "%s<%s>%s</%s>\n", indent, s, encode_xml(tmp_xml_buffer, i) , s);
#define store_bool(s, i); fprintf(rc, "%s<%s>%s</%s>\n", indent, s, i ? "true" : "false", s);

#define store_id(s, id); fprintf(rc, "%s<%s id=\"%i\"/>\n", indent, s, id);
#define store_int_id(s, i, id); fprintf(rc, "%s<%s id=\"%i\">%i</%s>\n", indent, s, id, (int) i, s);
#define store_float_id(s, i, id); fprintf(rc, "%s<%s id=\"%i\">%lf</%s>\n", indent, s, id, (double) i, s);
#define store_bool_id(s, i, id); fprintf(rc, "%s<%s id=\"%i\">%s</%s>\n", indent, s, id, i ? "true" : "false", s);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif