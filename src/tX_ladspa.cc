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
 
    File: tX_ladspa.cc
 
    Description: LADSPA_Plugin takes care of loading LADSPA plugins.
                 Most of this code is based on the LADSPA SDK code by
		 Richard W.E. Furse. For more information about ladspa
		 checkout http://www.ladspa.org		 
*/

#include <tX_ladspa.h>
#include <tX_ladspa_class.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

std::list <LADSPA_Plugin *> LADSPA_Plugin :: plugin_list;
std::list <LADSPA_Stereo_Plugin *> LADSPA_Stereo_Plugin :: stereo_plugin_list;

void LADSPA_Plugin :: init ()
{
	char *ladspa_path_ptr;
	char ladspa_path[PATH_MAX];
	char *start, *end, *buffer;

	/* Finding the LADSPA Path */
	ladspa_path_ptr=getenv("LADSPA_PATH");
	
	if (!ladspa_path_ptr)  {
		tX_warning("LADSPA_PATH not set. Trying /usr/lib/ladspa:/usr/local/lib/ladspa");
		strcpy(ladspa_path, "/usr/lib/ladspa:/usr/local/lib/ladspa");
	}
	else strncpy(ladspa_path, ladspa_path_ptr, sizeof(ladspa_path));
	
	/* Scanning every dir in path */
	start = ladspa_path;
	
	while (*start != '\0')
	{
		end = start;
		while (*end != ':' && *end != '\0') end++;
    
		buffer = (char *) malloc(1 + end - start);
		if (end > start) strncpy(buffer, start, end - start);
			
		buffer[end - start] = '\0';
		LADSPA_Plugin::scandir(buffer);
		free (buffer); 
    
    		start = end;
    		if (*start == ':') start++;
  	}	
}

/* This routine expects to get *valid* port descriptors.
   There's no error checking as in the LADSPA SDK's "analyseplugin".
*/

void LADSPA_Plugin :: handlelib(void *lib, LADSPA_Descriptor_Function desc_func, char *filename)
{
	long i;
	unsigned long port;
	const LADSPA_Descriptor *descriptor;
	int in_audio, out_audio, in_ctrl;	
	
	for (i=0; (descriptor = desc_func(i)) != NULL; i++) {		
		if (LADSPA_IS_INPLACE_BROKEN(descriptor->Properties)) {
			tX_plugin_warning("Plugin \"%s\" [%li] disabled: No in-place processing support.", descriptor->Label, descriptor->UniqueID);
		} else if (!LADSPA_IS_HARD_RT_CAPABLE(descriptor->Properties) && !globals.force_nonrt_plugins) {
			tX_plugin_warning("Plugin \"%s\" [%li] disabled: Not realtime capable.", descriptor->Label, descriptor->UniqueID);
		} else {
			if (!LADSPA_IS_HARD_RT_CAPABLE(descriptor->Properties)) {
				tX_warning("Plugin \"%s\" [%li] is classified as non-rt capable: loading forced.", descriptor->Label, descriptor->UniqueID);
			}
			in_audio=0; out_audio=0; in_ctrl=0;
		
			for (port = 0; port<descriptor->PortCount; port++) {			
				if (LADSPA_IS_PORT_AUDIO(descriptor->PortDescriptors[port])) {
					if (LADSPA_IS_PORT_INPUT(descriptor->PortDescriptors[port])) in_audio++;
					else
					if (LADSPA_IS_PORT_OUTPUT(descriptor->PortDescriptors[port])) out_audio++;
				} else 
					if (LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[port]) && LADSPA_IS_PORT_INPUT(descriptor->PortDescriptors[port])) in_ctrl++;			
			}
			
			if ((in_audio == 1) && (out_audio == 1)) {
				new LADSPA_Plugin(descriptor, filename);
			} if ((in_audio == 2) && (out_audio == 2)) {
				new LADSPA_Stereo_Plugin(descriptor, filename);
			}
			else { tX_plugin_warning("Plugin \"%s\" [%li] disabled: Neither mono nor stereo.", descriptor->Label, descriptor->UniqueID); }
		}
	}
}

void LADSPA_Plugin :: scandir(char *dirname)
{
	int dirlen=strlen(dirname);
	int needslash=0;
	DIR * dir;
	struct dirent * entry;
	char *filename;
	void *handle;
	LADSPA_Descriptor_Function desc_func;
	
	if (!dirlen) { tX_error("empty directory name?"); return; };

	if (dirname[dirlen - 1] != '/') needslash=1;
	
	dir = opendir(dirname);
	
	if (!dir) { tX_error("couldn't access directory \"%s\".", dirname); return; };
	
	while (1) {
		entry=readdir(dir);		
		if (!entry) { closedir(dir); return; }
		
		filename = (char *) malloc (dirlen + strlen(entry->d_name) + 1 + needslash);
		
		strcpy(filename, dirname);
		if (needslash) strcat(filename, "/");
		strcat(filename, entry->d_name);
		
		handle = dlopen(filename, RTLD_LAZY);
		
		if (handle) {
			/* clear dlerror */
			dlerror();
			
			/* check wether this is a LADSPA lib */
			desc_func = (LADSPA_Descriptor_Function) dlsym(handle, "ladspa_descriptor");
			
			if (dlerror() == NULL && desc_func) {
				LADSPA_Plugin :: handlelib(handle, desc_func, entry->d_name);
			} else {
				tX_error("\"%s\" is not a LADSPA plugin library.", filename);
				dlclose(handle);
			}
		}
		
		free (filename);
	}
}

void LADSPA_Plugin :: status ()
{
	debug_display();
}

void LADSPA_Plugin :: debug_display()
{
	std::list <LADSPA_Plugin *> :: iterator plugin;
	
	for (plugin=plugin_list.begin(); plugin != plugin_list.end(); plugin++) {
		printf("plugin: %60s | id: %5li | ports: %2li\n", (*plugin)->getName(), (*plugin)->getUniqueID(), (*plugin)->getPortCount());
	}
}

LADSPA_Plugin :: LADSPA_Plugin (const LADSPA_Descriptor *ld, char *filename)
{
	ladspa_descriptor = ld;
	
	plugin_list.push_back(this);
	strcpy(file, filename);
	sprintf(info_string, "LADSPA-Plugin: %s\nLabel: %s\nFile: %s\nUnique ID: %li\nMaker: %s\nCopyright: %s", ld->Name, ld->Label, file, ld->UniqueID, ld->Maker, ld->Copyright);
	LADSPA_Class::add_plugin(this);
}

LADSPA_Plugin * LADSPA_Plugin :: getPluginByIndex(int i)
{
	std::list <LADSPA_Plugin *> :: iterator plugin;
	int p;
	
	plugin = plugin_list.begin();
	for (p=0; (p<i) && (plugin != plugin_list.end()); p++, plugin++);
	
	if (plugin==plugin_list.end()) return NULL;
	
	else return (*plugin);
}

LADSPA_Plugin * LADSPA_Plugin :: getPluginByUniqueID(long ID)
{
	std::list <LADSPA_Plugin *> :: iterator plugin;
	
	for (plugin=plugin_list.begin(); plugin != plugin_list.end(); plugin++) {
		if ((*plugin)->getUniqueID()==ID) return (*plugin);
	}

	return NULL;
}

/* STEREO */

LADSPA_Stereo_Plugin :: LADSPA_Stereo_Plugin (const LADSPA_Descriptor *ld, char *filename)
{
	ladspa_descriptor = ld;
	
	stereo_plugin_list.push_back(this);
	strcpy(file, filename);
	sprintf(info_string, "LADSPA-Stereo-Plugin: %s\nLabel: %s\nFile: %s\nUnique ID: %li\nMaker: %s\nCopyright: %s", ld->Name, ld->Label, file, ld->UniqueID, ld->Maker, ld->Copyright);
	LADSPA_Class::add_stereo_plugin(this);
}

LADSPA_Stereo_Plugin * LADSPA_Stereo_Plugin :: getPluginByIndex(int i)
{
	std::list <LADSPA_Stereo_Plugin *> :: iterator plugin;
	int p;
	
	plugin = stereo_plugin_list.begin();
	for (p=0; (p<i) && (plugin != stereo_plugin_list.end()); p++, plugin++);
	
	if (plugin==stereo_plugin_list.end()) return NULL;
	
	else return (*plugin);
}

LADSPA_Stereo_Plugin * LADSPA_Stereo_Plugin :: getPluginByUniqueID(long ID)
{
	std::list <LADSPA_Stereo_Plugin *> :: iterator plugin;
	
	for (plugin=stereo_plugin_list.begin(); plugin != stereo_plugin_list.end(); plugin++) {
		if ((*plugin)->getUniqueID()==ID) return (*plugin);
	}

	return NULL;
}

bool LADSPA_Stereo_Plugin::is_stereo() { return true; }
bool LADSPA_Plugin::is_stereo() { return false; }
