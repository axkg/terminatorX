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
 
    File: tX_ladspa_class.cc
*/

#include "tX_ladspa_class.h"
#include "tX_global.h"
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_LRDF
#include <lrdf.h>
#endif

LADSPA_Class * LADSPA_Class::root=NULL;
LADSPA_Class * LADSPA_Class::unclassified=NULL;
std::list <char *> LADSPA_Class::rdf_files;
vtt_class *LADSPA_Class::current_vtt;
bool LADSPA_Class::liblrdf_error=false;

/* Why do have to code this myself? */
static int compare(const char *a, const char *b) {
	int lena, lenb, i;

	if (!a && !b) return 0;
	if (!a) return 2;
	if (!b) return 1;
	
	lena=strlen(a);
	lenb=strlen(b);
	
	for (i=0; (i<lena) && (i<lenb); i++) {
		if (a[i]>b[i]) {
			return 2;
		} else if (a[i]<b[i]) {
			return 1;
		}
	}
	
	if (lena>lenb) return 1;
	else if (lenb>lena) return 2;
	return 0;
}

void LADSPA_Class::init() {
	char *start, *end, *buffer;

#ifdef USE_LRDF
	/* Scanning every dir in path */
	start = globals.lrdf_path;
	
	while (*start != '\0') {
		end = start;
		while (*end != ':' && *end != '\0') end++;
    
		buffer = (char *) malloc(1 + end - start);
		if (end > start) strncpy(buffer, start, end - start);
			
		buffer[end - start] = '\0';
		LADSPA_Class::scandir(buffer);
		free (buffer); 
    
    	start = end;
    	if (*start == ':') start++;
  	}
	
	if (rdf_files.size() > 0) {
		char *uris[rdf_files.size()+1];
		std::list <char *> :: iterator i;
		int t;
		
		for (i=rdf_files.begin(), t=0; i!=rdf_files.end(); i++, t++) {
			uris[t]=(*i);
		}
		uris[t]=NULL;
		
		lrdf_init();
	
		if (lrdf_read_files((const char **) uris)) {
			tX_error("liblrdf had problems reading the rdf files - cannot provide structured menu");
			liblrdf_error=true;
        }
#endif
		root=new LADSPA_Class("http://ladspa.org/ontology#Plugin");
#ifdef USE_LRDF		
		lrdf_cleanup();
	} else {
		tX_error("No RDF files found");
	}
#endif
	
	unclassified=new LADSPA_Class();
	/* This is the last class to accpet all plugins not accepted by other classes. */
	root->subclasses.push_back(unclassified);
}

void LADSPA_Class::scandir(char *dirname) {
	int dirlen=strlen(dirname);
	int needslash=0;
	DIR * dir;
	struct dirent * entry;
	char *filename;
	
	if (!dirlen) { tX_error("LADSPA_Class::scandir() Empty directory name"); return; };

	if (dirname[dirlen - 1] != '/') needslash=1;
	
	dir = opendir(dirname);
	
	if (!dir) { tX_error("LADSPA_Class::scandir() couldn't access directory \"%s\"", dirname); return; };
	
	while (1) {
		entry=readdir(dir);
		
		if (!entry) { closedir(dir); return; }
		
		if ((strcmp(entry->d_name, ".")==0) ||
			(strcmp(entry->d_name, "..")==0)) continue;
		
		filename = (char *) malloc (dirlen + strlen(entry->d_name) + 10 + needslash);
		
		strcpy(filename, "file:");
		strcat(filename, dirname);
		if (needslash) strcat(filename, "/");
		strcat(filename, entry->d_name);
		
		tX_debug("Found RDF file: %s", filename);
		rdf_files.push_back(filename);		
	}	
}

void LADSPA_Class::insert_class(LADSPA_Class *cls) {
	std::list <LADSPA_Class *> :: iterator i;
	
	for (i=subclasses.begin(); i!=subclasses.end(); i++) {
		LADSPA_Class *a_class=(*i);
		int res=compare(cls->label, a_class->label);
		
		if (res < 2) {
			subclasses.insert(i, cls);
			return;
		}
	}
	
	subclasses.push_back(cls);
}

LADSPA_Class :: LADSPA_Class (char *uri) : label(NULL), accept_all(false) {
#ifdef USE_LRDF	
	lrdf_uris *ulist;
	char *urilabel;
	int i;

	if (!liblrdf_error) {
		urilabel=lrdf_get_label(uri);
		
		if (urilabel) {
			label=strdup(urilabel);
		}
		
		/* Finding subclasses... */
		ulist = lrdf_get_subclasses(uri);
		
		for (i = 0; ulist && i < ulist->count; i++) {
			insert_class(new LADSPA_Class(ulist->items[i]));
		}
	
		lrdf_free_uris(ulist);
	
		/* Finding instances... */
		ulist=lrdf_get_instances(uri);
		
		for (i = 0; ulist && i < ulist->count; i++) {
			registered_ids.push_back(lrdf_get_uid(ulist->items[i]));
		}
	
		lrdf_free_uris(ulist);
	}
#endif	
}

LADSPA_Class :: LADSPA_Class() : label("Unclassified"), accept_all(true) {
}

bool LADSPA_Class :: add_plugin(LADSPA_Plugin *plugin) {
	return root->add_plugin_instance(plugin);
}

bool LADSPA_Class :: add_plugin_instance(LADSPA_Plugin *plugin) {
	if (accept_all) {
		insert_plugin(plugin);
		return true;
	}
	
	long id=plugin->getUniqueID();
	std::list <long> :: iterator i;
	
	/* Is this plugin an instance of this class? */
	
	for (i=registered_ids.begin(); i!=registered_ids.end(); i++) {
		if ((*i)==id) {
			/* The plugin belongs to this class... */
			insert_plugin(plugin);
			return true;
		}
	}
	
	/* Try to insert the plugin in subclasses */
	std::list <LADSPA_Class *> :: iterator cls;
	
	for (cls=subclasses.begin(); cls!=subclasses.end(); cls++) {
		LADSPA_Class *lrdf_class=(*cls);
		
		if (lrdf_class->add_plugin_instance(plugin)) return true;
	}
	
	/* Giving up... */
	
	return false;
}

void LADSPA_Class::insert_plugin(LADSPA_Plugin *plugin) {
	std::list <LADSPA_Plugin *> :: iterator i;
	
	for (i=plugins.begin(); i!=plugins.end(); i++) {
		LADSPA_Plugin *a_plug=(*i);
		int res=compare(plugin->getName(), a_plug->getName());
		
		if (res < 2) {
			plugins.insert(i, plugin);
			return;
		}
	}
	
	plugins.push_back(plugin);
}

void LADSPA_Class::list(char *buffer) {
	strcat(buffer, "\t");
	
	printf("%s class %s {\n", buffer, label);

	std::list <LADSPA_Plugin *> :: iterator i;	
	
	for (i=plugins.begin(); i!=plugins.end(); i++) {
		printf("%s - plugin: %s\n", buffer, (*i)->getName());
	}
	
	std::list <LADSPA_Class *> :: iterator c;
	
	for (c=subclasses.begin(); c!=subclasses.end(); c++) (*c)->list(buffer);
	
	printf("%s}\n", buffer);
	
	buffer[strlen(buffer)-1]=0;
}

void LADSPA_Class::dump() {
	char buffer[256]="";
	root->list(buffer);
}

static void menu_callback(GtkWidget *wid, LADSPA_Plugin *plugin) {
	vtt_class *vtt=LADSPA_Class::get_current_vtt();
	
	if (vtt) {
		vtt->add_effect(plugin);
	} else {
		tX_error("LADSPA_Class::menu_callback() no vtt");
	}
}


GtkWidget * LADSPA_Class :: get_menu() {
	std::list <LADSPA_Class *> :: iterator cls;
	GtkWidget *menu=gtk_menu_new();
	GtkWidget *item;
	
	for (cls=subclasses.begin(); cls!=subclasses.end(); cls++) {
		LADSPA_Class *c=(*cls);
		
		if (c->plugins.size() || c->subclasses.size()) {
			item=gtk_menu_item_new_with_label(c->label);
			GtkWidget *submenu=c->get_menu();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
			gtk_widget_show(item);
		}
	}
	
	if (subclasses.size() && plugins.size()) {
		item = gtk_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		gtk_widget_set_sensitive (item, FALSE);
		gtk_widget_show (item);
	}
	
	std::list <LADSPA_Plugin *> :: iterator plugin;
	
	for (plugin=plugins.begin(); plugin != plugins.end(); plugin++) {
		char buffer[512];
		LADSPA_Plugin *p=(*plugin);
		
		sprintf(buffer, "%s - (%s, %li)", p->getName(), p->getLabel(), p->getUniqueID());
		item=gtk_menu_item_new_with_label(buffer);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(menu_callback), p);		
		gtk_widget_show(item);
	}
	
	return menu;
}

GtkWidget * LADSPA_Class :: get_ladspa_menu() {
	return root->get_menu();
}
