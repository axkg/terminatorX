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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <zlib.h>
	
#ifdef ENABLE_DEBUG_OUTPUT	
#define tX_debug(fmt, args...); { fprintf(stderr, "- tX_debug: "); fprintf(stderr, fmt , ## args); fprintf(stderr, "\n"); }
#else
#define tX_debug(fmt, args...);
#endif
	
#define tX_error(fmt, args...); { fprintf(stderr, "* tX_error: "); fprintf(stderr, fmt , ## args); fprintf(stderr, "\n"); }
#define tX_warning(fmt, args...); { fprintf(stderr, "+ tX_warning: "); fprintf(stderr, fmt , ## args); fprintf(stderr, "\n"); }

typedef enum {
	OSS =0,
	ALSA = 1,
	JACK = 2
} tX_audiodevice_type;
	
typedef struct {
	int	xinput_enable;
	char xinput_device[256];
	
	int	store_globals;		// if it should store the globals vals on exit
	char *startup_set;	
	char *alternate_rc;		// a diferent set of stored globals to load
	int	no_gui;			// run without any gui
	
	int	update_idle;	
	int	sense_cycles;
	
	int width;
	int height;

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
	
	char current_path[PATH_MAX];
	
	/* new audiodevice handling 
	   we have *all* variables for *all* audiodevice types -
	   even if support for them is not compiled in - to keep
	   the .terminatorX3rc.bin in sync.
	*/
	
	tX_audiodevice_type audiodevice_type; // TX_AUDIODEVICE_TYPE_OSS etc.
	
	/* OSS specific options  */
	char oss_device[PATH_MAX];
	int oss_buff_no;
	int oss_buff_size; // 2^X Bytes
	int oss_samplerate;
	
	/* ALSA specific options */
	char alsa_device_id[PATH_MAX];
	int alsa_buffer_time;
	int alsa_period_time;
	int alsa_samplerate;
	
	char lrdf_path[PATH_MAX];
	
	int compress_set_files;
	
	int fullscreen_enabled;
	int confirm_events;
	
	double vtt_inertia;
	
	int alsa_free_hwstats;
	int filename_length;
	
	int restore_midi_connections;
	
	char wav_display_bg_focus[8];
	char wav_display_bg_no_focus[8];
	
	char wav_display_fg_focus[8];
	char wav_display_fg_no_focus[8];	
	
	char wav_display_cursor[8];	
	char wav_display_cursor_mute[8];
	
	char vu_meter_bg[8];
	char vu_meter_loud[8];
	char vu_meter_normal[8];
	double vu_meter_border_intensity;
	
} tx_global;

extern tx_global globals;

extern void load_globals();
extern void store_globals();
extern void set_global_defaults();
extern char *encode_xml(char *dest, const char *src);
extern char *decode_xml(char *dest, const char *src);

#define nop

/* some ugly XML macros... */
#define restore_int(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) { sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%i", &i); }}
#define restore_float(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if  (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%lf", &dvalue); i=dvalue;}}
#define restore_string(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {strcpy(i, decode_xml(tmp_xml_buffer, (const char *)  xmlNodeListGetString(doc, cur->xmlChildrenNode, 1))); }}
#define restore_bool(s, i); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {if (xmlStrcmp(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1),  (const xmlChar *) "true")==0) i=true; else i=false; }}
#define restore_id(s, sp); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; pid_attr=(char* ) xmlGetProp(cur, (xmlChar *) "id"); if (pid_attr) { sscanf(pid_attr, "%i",  &pid); sp.set_persistence_id(pid); }}

#define restore_int_ac(s, i, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) { sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%i", &i); init; }}
#define restore_float_ac(s, i, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if  (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%lf", &dvalue); i=dvalue; init; }}
#define restore_string_ac(s, i, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {strcpy(i, decode_xml(tmp_xml_buffer, (const char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1))); init; }}
#define restore_bool_ac(s, i, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {if (xmlStrcmp(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), (const xmlChar *) "true")==0) i=true; else i=false; init; }}

//#define restore_int_id(s, i, sp, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) { sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%i", &i); pid_attr=(char* ) xmlGetProp(cur, (xmlChar *) "id"); if (pid_attr) { sscanf(pid_attr, "%i",  &pid); sp.set_persistence_id(pid); } init; }}
//#define restore_float_id(s, i, sp, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if  (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%lf", &dvalue); i=dvalue; pid_attr=(char* ) xmlGetProp(cur, (xmlChar *) "id"); if (pid_attr) { sscanf(pid_attr, "%i",  &pid); sp.set_persistence_id(pid); } init; }}
//#define restore_bool_id(s, i, sp, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {if (xmlStrcmp(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1),  (const xmlChar *) "true")==0) i=true; else i=false; pid_attr=(char* ) xmlGetProp(cur,  (xmlChar *)"id"); if (pid_attr) { sscanf(pid_attr, "%i",  &pid); sp.set_persistence_id(pid); } init; }}

extern int _store_compress_xml;

#define tX_store(fmt, args...); { _store_compress_xml ? gzprintf(rz, fmt , ## args) : fprintf(rc, fmt , ## args); }

#define store_int(s, i); tX_store("%s<%s>%i</%s>\n", indent, s,(int) i, s);
#define store_float(s, i); tX_store("%s<%s>%lf</%s>\n", indent, s,(double) i, s);
#define store_string(s, i); tX_store("%s<%s>%s</%s>\n", indent, s, encode_xml(tmp_xml_buffer, i) , s);
#define store_bool(s, i); tX_store("%s<%s>%s</%s>\n", indent, s, i ? "true" : "false", s);

#define store_id(s, id); tX_store("%s<%s id=\"%i\"/>\n", indent, s, id);
#define store_int_id(s, i, id); tX_store("%s<%s id=\"%i\">%i</%s>\n", indent, s, id, (int) i, s);
#define store_float_id(s, i, id); tX_store("%s<%s id=\"%i\">%lf</%s>\n", indent, s, id, (double) i, s);
#define store_bool_id(s, i, id); tX_store("%s<%s id=\"%i\">%s</%s>\n", indent, s, id, i ? "true" : "false", s);

#define store_int_sp(name, i, sp); { tX_store("%s<%s ", indent, name); sp.store_meta(rc, rz); tX_store(">%i</%s>\n", (int) i, name); }
#define store_float_sp(name, i, sp); { tX_store("%s<%s ", indent, name); sp.store_meta(rc, rz); tX_store(">%lf</%s>\n", (double) i, name); }
#define store_bool_sp(name, i, sp); { tX_store("%s<%s ", indent, name); sp.store_meta(rc, rz); tX_store(">%s</%s>\n", i ? "true" : "false", name); }

#define restore_int_id(s, i, sp, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) { sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%i", &i);  init; } sp.restore_meta(cur); }
#define restore_float_id(s, i, sp, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if  (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {sscanf((char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "%lf", &dvalue); i=dvalue; init; } sp.restore_meta(cur);}
#define restore_bool_id(s, i, sp, init); if ((!elementFound) && (!xmlStrcmp(cur->name, (const xmlChar *) s))) { elementFound=1; if (xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)) {if (xmlStrcmp(xmlNodeListGetString(doc, cur->xmlChildrenNode, 1),  (const xmlChar *) "true")==0) i=true; else i=false; init; } sp.restore_meta(cur);}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
