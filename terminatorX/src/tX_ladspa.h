/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2011  Alexander König
 
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
 
    File: tX_ladspa.h
 
    Description: Header to ladspa.cc - see there for more info
*/

#ifndef _h_tx_ladspa
#define _h_tx_ladspa 1

#include <list>
#include <ladspa.h>

using namespace std;

class LADSPA_Plugin
{
	protected:
	const LADSPA_Descriptor *ladspa_descriptor;
	LADSPA_Plugin(const LADSPA_Descriptor *ld, char *filename);
	LADSPA_Plugin() {}
	char info_string[4096];
	char file[1024];
	
	private:
	static list <LADSPA_Plugin *> plugin_list;
	static void scandir(char *dir);
	static void handlelib(void *lib, LADSPA_Descriptor_Function desc_func, char* filename);
	
	public:
	static void init();
	static void status();
	static void debug_display();
	virtual bool is_stereo();
	char *get_info_string() { return info_string; }
	char *get_file_name() { return file; }
	
	long getUniqueID() { return ladspa_descriptor->UniqueID; }
	const char * getName() { return ladspa_descriptor->Name; }
	const char * getLabel() { return ladspa_descriptor->Label; }
	long getPortCount() { return ladspa_descriptor->PortCount; }
	
	static int getPluginCount() { return plugin_list.size(); }
	static LADSPA_Plugin * getPluginByIndex(int i);
	static LADSPA_Plugin * getPluginByUniqueID(long ID);
	const LADSPA_Descriptor *getDescriptor() { return ladspa_descriptor; }
	virtual ~LADSPA_Plugin() {};
};

class LADSPA_Stereo_Plugin : public LADSPA_Plugin
{
	private:
	static list <LADSPA_Stereo_Plugin *> stereo_plugin_list;
	
	public:
	LADSPA_Stereo_Plugin(const LADSPA_Descriptor *ld, char *filename);

	public:
	virtual bool is_stereo();
	static LADSPA_Stereo_Plugin * getPluginByIndex(int i);
	static LADSPA_Stereo_Plugin * getPluginByUniqueID(long ID);	
};

#endif
