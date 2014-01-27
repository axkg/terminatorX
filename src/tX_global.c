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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
    File: tX_global.c
 
    Description:  This file contains the routines for handling the
    		  "globals" block. Intializing, reading setup from
		  disk and storing it.
*/    

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include "tX_types.h"
#include "tX_global.h"
#include "string.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/encoding.h>


#define TX_XML_RC_VERSION "1.0"

tx_global globals;
int _store_compress_xml=0;

#ifdef USE_ALSA_MIDI_IN
extern void tX_midiin_store_connections(FILE *rc, char *indent);
extern void tX_midiin_restore_connections(xmlNodePtr node);
#endif

void get_rc_name(char *buffer, int length)
{
	strncpy(buffer,"", length);
	if (globals.alternate_rc) {
		strncpy(buffer, globals.alternate_rc, length);
	} else {
		if (getenv("HOME")) {
			strncpy(buffer, getenv("HOME"), length-16);
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
	
	globals.update_idle=14;
	globals.update_delay=1;
	
	strcpy(globals.oss_device, "/dev/dsp");
	globals.oss_buff_no=2;
	globals.oss_buff_size=9;
	globals.oss_samplerate=44100;

	strcpy(globals.alsa_device_id, "hw:0,0");	
	globals.alsa_buffer_time=80000;
	globals.alsa_period_time=20000;
	globals.alsa_samplerate=44100;
	
	globals.sense_cycles=80;
	
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
		
#ifdef USE_ALSA
	globals.audiodevice_type=ALSA;
#else
#ifdef USE_OSS
	globals.audiodevice_type=OSS;
#endif	
#endif		
	globals.use_stdout_cmdline=0;
	strcpy(globals.current_path, "");
	strcpy(globals.lrdf_path, "/usr/share/ladspa/rdf:/usr/local/share/ladspa/rdf");
	globals.fullscreen_enabled=0;
	globals.confirm_events=0;
	globals.compress_set_files=0;
	
	globals.vtt_inertia=10.0;
	
	globals.alsa_free_hwstats=0;
	globals.filename_length=20;
	globals.restore_midi_connections=1;
	
	strcpy(globals.wav_display_bg_focus, "#00004C");
	strcpy(globals.wav_display_bg_no_focus, "#000000");

	strcpy(globals.wav_display_fg_focus, "#FFFF00");
	strcpy(globals.wav_display_fg_no_focus, "#00FF00");

	strcpy(globals.wav_display_cursor, "#FF6666");
	strcpy(globals.wav_display_cursor_mute, "#FFFFFF");	
	
	strcpy(globals.vu_meter_bg, "#000000");
	strcpy(globals.vu_meter_loud, "#FF0000");
	strcpy(globals.vu_meter_normal, "#00FF00");	
	globals.vu_meter_border_intensity=0.7;
	globals.quit_confirm=1;
	globals.use_realtime=1;
	globals.auto_assign_midi=0;
	
	globals.verbose_plugin_loading=0;
	globals.force_nonrt_plugins=0;	
}

int load_globals_xml() {
	char rc_name[PATH_MAX]="";	
	char device_type[16]="oss";
	xmlDocPtr doc;
	xmlNodePtr cur;
	int elementFound;
	double dvalue;
	char tmp_xml_buffer[4096];
	
	get_rc_name(rc_name, sizeof(rc_name));
	
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

			restore_string("alsa_device_id", globals.alsa_device_id);
			restore_int("alsa_buffer_time", globals.alsa_buffer_time);
			restore_int("alsa_period_time", globals.alsa_period_time);
			restore_int("alsa_samplerate", globals.alsa_samplerate);
			restore_int("alsa_free_hwstats", globals.alsa_free_hwstats);
			
			restore_string("xinput_device", globals.xinput_device);
			restore_int("xinput_enable", globals.xinput_enable);
			restore_int("update_idle", globals.update_idle);
			restore_int("update_delay", globals.update_delay);
			restore_int("sense_cycles", globals.sense_cycles);
			restore_float("mouse_speed", globals.mouse_speed);
			restore_int("width", globals.width);
			restore_int("height", globals.height);
			restore_int("filename_length", globals.filename_length);
			restore_int("tooltips", globals.tooltips);
			restore_int("use_stdout", globals.use_stdout);
			restore_int("show_nag", globals.show_nag);
			restore_int("prelis", globals.prelis);
			restore_string("last_fn", globals.last_fn);
			restore_float("pitch", globals.pitch);
			restore_float("volume", globals.volume);
			restore_float("flash_response", globals.flash_response);
			restore_int("button_type", globals.button_type);
			restore_string("tables_filename", globals.tables_filename);
			restore_string("record_filename", globals.record_filename);
			restore_string("file_editor", globals.file_editor);
			restore_string("lrdf_path", globals.lrdf_path);
			restore_string("last_path", globals.current_path);
			
			restore_int("compress_set_files", globals.compress_set_files);
			restore_int("fullscreen_enabled", globals.fullscreen_enabled);
			restore_int("confirm_events", globals.confirm_events);
			restore_float("vtt_inertia", globals.vtt_inertia);
			restore_int("restore_midi_connections", globals.restore_midi_connections);
			
			restore_string("wav_display_bg_focus", globals.wav_display_bg_focus);
			restore_string("wav_display_bg_no_focus", globals.wav_display_bg_no_focus);
			restore_string("wav_display_fg_focus", globals.wav_display_fg_focus);
			restore_string("wav_display_fg_no_focus", globals.wav_display_fg_no_focus);
			restore_string("wav_display_cursor", globals.wav_display_cursor);
			restore_string("wav_display_cursor_mute", globals.wav_display_cursor_mute);
			
			restore_string("vu_meter_bg", globals.vu_meter_bg);
			restore_string("vu_meter_normal", globals.vu_meter_normal);
			restore_string("vu_meter_loud", globals.vu_meter_loud);
			
			restore_float("vu_meter_border_intensity", globals.vu_meter_border_intensity);

			restore_int("quit_confirm", globals.quit_confirm);
			restore_int("use_realtime", globals.use_realtime);
			restore_int("auto_assign_midi", globals.auto_assign_midi);
			restore_int("force_nonrt_plugins", globals.force_nonrt_plugins);
			restore_int("verbose_plugin_loading", globals.verbose_plugin_loading);

#ifdef USE_ALSA_MIDI_IN
			if (!elementFound && (xmlStrcmp(cur->name, (xmlChar *) "midi_connections")==0)) {
				if (globals.restore_midi_connections) {
					tX_midiin_restore_connections(cur);
				}
				elementFound=1;
			}
#endif			

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
	char rc_name[PATH_MAX+256]="";
	char device_type[16];
	char indent[32]="\t";
	FILE *rc=NULL;
	gzFile rz=NULL;
	char tmp_xml_buffer[4096];
	_store_compress_xml=0;
	
	get_rc_name(rc_name, sizeof(rc_name));

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
		fprintf(rc, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\n");
		fprintf(rc, "<!-- Warning: this file will be rewritten by terminatorX on exit!\n     Don\'t waste your time adding comments - they will be erased -->\n\n" );
		fprintf(rc, "<terminatorXrc version=\"%s\">\n", TX_XML_RC_VERSION);

		store_int("store_globals", globals.store_globals);

		store_string("audio_driver", device_type);
		
		store_string("oss_device", globals.oss_device);
		store_int("oss_buff_no", globals.oss_buff_no);
		store_int("oss_buff_size", globals.oss_buff_size);
		store_int("oss_samplerate", globals.oss_samplerate);

		store_string("alsa_device_id", globals.alsa_device_id);
		store_int("alsa_buffer_time", globals.alsa_buffer_time);
		store_int("alsa_period_time", globals.alsa_period_time);
		store_int("alsa_samplerate", globals.alsa_samplerate);		
		store_int("alsa_free_hwstats", globals.alsa_free_hwstats);
		
		store_string("xinput_device", globals.xinput_device);
		store_int("xinput_enable", globals.xinput_enable);
		store_int("update_idle", globals.update_idle);
		store_int("update_delay", globals.update_delay);
		store_int("sense_cycles", globals.sense_cycles);
		store_float("mouse_speed", globals.mouse_speed);
		store_int("width", globals.width);
		store_int("height", globals.height);
		store_int("filename_length", globals.filename_length);
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
		store_string("tables_filename", globals.tables_filename);
		store_string("record_filename", globals.record_filename);
		store_string("file_editor", globals.file_editor);
		store_string("lrdf_path", globals.lrdf_path);
		store_int("compress_set_files", globals.compress_set_files);
		store_int("fullscreen_enabled", globals.fullscreen_enabled);
		store_int("confirm_events", globals.confirm_events);
		store_float("vtt_inertia", globals.vtt_inertia);

		store_string("last_path", globals.current_path);
		store_int("restore_midi_connections", globals.restore_midi_connections);

		store_string("wav_display_bg_focus", globals.wav_display_bg_focus);
		store_string("wav_display_bg_no_focus", globals.wav_display_bg_no_focus);
		store_string("wav_display_fg_focus", globals.wav_display_fg_focus);
		store_string("wav_display_fg_no_focus", globals.wav_display_fg_no_focus);
		store_string("wav_display_cursor", globals.wav_display_cursor);
		store_string("wav_display_cursor_mute", globals.wav_display_cursor_mute);

		store_string("vu_meter_bg", globals.vu_meter_bg);
		store_string("vu_meter_normal", globals.vu_meter_normal);
		store_string("vu_meter_loud", globals.vu_meter_loud);
			
		store_float("vu_meter_border_intensity", globals.vu_meter_border_intensity);

		store_int("quit_confirm", globals.quit_confirm);
		store_int("use_realtime", globals.use_realtime);
		store_int("auto_assign_midi", globals.auto_assign_midi);
		
		store_int("verbose_plugin_loading", globals.verbose_plugin_loading);
		store_int("force_nonrt_plugins", globals.force_nonrt_plugins);

#ifdef USE_ALSA_MIDI_IN
		tX_midiin_store_connections(rc, indent);
#endif		

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
	char tmp[4096];
	int outlen=4096;
	int inlen;
	int res;
	
	tmp[0]=0;
	t=0;
	max=strlen(src);
	
	for (i=0; i<max; i++) {
		switch (src[i]) {
			case '<': tmp[t]=0; strcat(tmp, "&lt;"); t+=4; break;
			case '>': tmp[t]=0; strcat(tmp, "&gt;"); t+=4; break;
			case '&': tmp[t]=0; strcat(tmp, "&amp;"); t+=5; break;
			case '"': tmp[t]=0; strcat(tmp, "&quot;"); t+=6; break;
			case '\'': strcat(tmp, "&apos;"); t+=7; break;
			default: tmp[t]=src[i]; t++;
		}
	}
	tmp[t]=0;

	inlen=t;
	res=isolat1ToUTF8((unsigned char *) dest, &outlen, (unsigned char *) tmp, &inlen);
	dest[outlen]=0;
	if (res<0) {
		tX_error("failed to encode string (%s) to UTF-8.", src);
	}
	
	return dest;
}

char *decode_xml(char *dest, const char *src) {
	int inlen=strlen(src);
	int outlen=4096;
	
	int res=UTF8Toisolat1((unsigned char *) dest, &outlen, (const unsigned char *) src, &inlen);
	dest[outlen]=0;
	
	if (res<0) {
		tX_error("failed to decode UTF-8 string (%s).", src);
	}
	
	return dest;
}
