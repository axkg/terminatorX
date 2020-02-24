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
 
    File: tX_vttfx.h
 
    Description: Header to tX_vttfx.cc - see there for more info
*/

#ifndef _h_tx_vttfx
#define _h_tx_vttfx 1


//#include "tX_vtt.h"
#include "tX_seqpar.h"
#include "tX_ladspa.h"
#include <stdio.h>
#include <list>
#include <gtk/gtk.h>
#include "tX_panel.h"

/* Ids to guarantee positioning of builtin */
#define TX_FX_BUILTINCUTOFF 1
#define TX_FX_BUILTINECHO 2
#define TX_FX_LADSPA 3

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

/* abstract super class vtt_fx */

typedef enum  {
	NOT_DRYWET_CAPABLE,
	DRYWET_AVAILABLE,
	DRYWET_ACTIVE
} tX_drywet_type;

class vtt_fx
{
	protected:
	void *vtt;
	GtkWidget *panel_widget;
	tX_panel *panel;

	public:	
	vtt_fx() { vtt=NULL; }
	virtual ~vtt_fx();
	void set_vtt(void *v) { vtt=v;}
	void *get_vtt() { return vtt; }
	
	virtual void activate()=0;
	virtual void deactivate()=0;
	virtual void run()=0;	
	virtual int isEnabled()=0;
	virtual void reconnect_buffer();
	virtual void toggle_drywet();
	virtual bool is_stereo();
	virtual tX_drywet_type has_drywet_feature();
	
	virtual const char *get_info_string()=0;
	
	virtual void save(FILE *rc, gzFile rz, char *indent)=0;
	
	GtkWidget* get_panel_widget() { return panel_widget; }
	void set_panel_widget(GtkWidget *widget) { panel_widget=widget; }
	
	void set_panel(tX_panel *p) { panel = p; }
	tX_panel* get_panel() { return panel; }
};

/*********************************/
/* builtin fx */

/* lowpass */
class vtt_fx_lp : public vtt_fx
{
	public:	
	virtual void activate();
	virtual void deactivate();
	virtual void run();	
	virtual int isEnabled();

	virtual void save(FILE *rc, gzFile rz, char *indent);
	virtual const char *get_info_string();
};

/* echo */
class vtt_fx_ec : public vtt_fx
{
	public:	
	virtual void activate();
	virtual void deactivate();
	virtual void run();	
	virtual int isEnabled();

	virtual void save(FILE *rc, gzFile rz, char *indent);
	virtual const char *get_info_string();	
};

/********************************/
/* LADSPA plugin fx */

class vtt_fx_ladspa : public vtt_fx
{
	public:
	list <tX_seqpar_vttfx *> controls;
	protected:
	tX_seqpar_vttfx *sp_enable;
	tX_seqpar_vttfx *sp_wet;
	f_prec *wet_buffer;
	int input_port, output_port;
	LADSPA_Handle *instance;
	LADSPA_Plugin *plugin;

	public:
	vtt_fx_ladspa(LADSPA_Plugin *, void *);
	virtual ~vtt_fx_ladspa();
	
	public:	
	virtual void activate();
	virtual void deactivate();
	virtual void run();		
	virtual int isEnabled();
	virtual void reconnect_buffer();
	virtual const char *get_info_string();
	virtual void realloc_drywet();
	virtual void free_drywet();
	virtual void toggle_drywet();
	void add_drywet();
	void remove_drywet();
	virtual tX_drywet_type has_drywet_feature();
	
	virtual void save(FILE *rc, gzFile rz, char *indent);
#ifdef ENABLE_TX_LEGACY	
	virtual void load(FILE *);
#endif	
	virtual void load(xmlDocPtr, xmlNodePtr);
};

class vtt_fx_stereo_ladspa : public vtt_fx_ladspa {
	private:
	f_prec *wet_buffer2;
	int input2_port, output2_port;
	
	protected:
	virtual void reconnect_buffer();	
	virtual void realloc_drywet();
	virtual void free_drywet();

	public:
	vtt_fx_stereo_ladspa(LADSPA_Stereo_Plugin *, void *);
	virtual bool is_stereo();
	virtual void run();
	virtual ~vtt_fx_stereo_ladspa();
};
#endif
