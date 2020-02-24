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
 
    File: tX_ladspa_class.h
 
    Description: Header to tX_ladspa_class.cc - see there for more info
*/

#ifndef _h_tx_ladspa_class
#define _h_tx_ladspa_class 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "tX_ladspa.h"
#include <list>
#include <gtk/gtk.h>
#include "tX_vtt.h"

typedef enum {
	MONO,
	STEREO
} LADSPA_Plugin_Type;

class LADSPA_Class { // Yeah, I know "class" name for C++ class - but it just seems to fit best...
	protected:
	static LADSPA_Class *root;
	static LADSPA_Class *unclassified;
	
	static std::list <char *> rdf_files;
	static vtt_class *current_vtt;
	static bool liblrdf_error;
	
	const char *label;
	bool accept_all;
	std::list <LADSPA_Class *> subclasses;
	std::list <long> registered_ids;
	std::list <LADSPA_Plugin *> plugins;
	std::list <LADSPA_Stereo_Plugin *> stereo_plugins;
	
	static void scandir(char *dir);
	bool add_plugin_instance(LADSPA_Plugin *, LADSPA_Plugin_Type);
	void insert_class(LADSPA_Class *);
	void insert_plugin(LADSPA_Plugin *, LADSPA_Plugin_Type);
	int plugins_in_class(LADSPA_Plugin_Type type);
	void list(char *);
	GtkWidget *get_menu(LADSPA_Plugin_Type);
	
	public:
	LADSPA_Class(const char *uri);
	LADSPA_Class(); // For the unclassified class;	
	
	static bool add_plugin(LADSPA_Plugin *plugin);
	static bool add_stereo_plugin(LADSPA_Stereo_Plugin *plugin);
	
	static void init();
	static void dump();
	static GtkWidget *get_ladspa_menu();
	static GtkWidget *get_stereo_ladspa_menu();
	static void set_current_vtt(vtt_class *vtt) { current_vtt=vtt; }
	static vtt_class *get_current_vtt() { return current_vtt; }
};

#endif
