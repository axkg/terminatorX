/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999  Alexander König
 
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
 
    File: tX_vttgui.h
 
    Description: Header to tX_vttgui.cc
*/    

#ifndef _h_tX_vttgui
#define _h_tX_vttgui

#include <gtk/gtk.h>

#ifndef DONT_USE_FLASH
#define USE_FLASH
#endif

typedef struct vtt_gui
{
	GtkWidget *frame;
	
	GtkWidget *notebook;
	GtkWidget *display;
#ifdef USE_FLASH
	GtkWidget *flash;
#endif
	
	/* Widgets in Main Panel */
	GtkWidget *name;
	GtkWidget *file;
	GtkAdjustment *volume;
	GtkAdjustment *pitch;
	GtkWidget *del;
	GtkWidget *clone;
	
	/* Widgets in Trigger Panel */
	GtkWidget *trigger;
	GtkWidget *stop;
	GtkWidget *autotrigger;
	GtkWidget *loop;
	GtkWidget *sync_master;
	GtkWidget *sync_client;
	GtkAdjustment *cycles;
	
	/* Widgets in X-Control Panel */
	GtkWidget *x_scratch;
	GtkWidget *x_volume;
	GtkWidget *x_lp_cutoff;
	GtkWidget *x_ec_feedback;
	GtkWidget *x_nothing;

	/* Widgets in Y-Control Panel */
	GtkWidget *y_scratch;
	GtkWidget *y_volume;
	GtkWidget *y_lp_cutoff;
	GtkWidget *y_ec_feedback;
	GtkWidget *y_nothing;
	
	/* Widgets in Lowpass Panel */
	GtkWidget *lp_enable;
	GtkAdjustment *lp_gain;
	GtkAdjustment *lp_reso;
	GtkAdjustment *lp_freq;
	
	/* Widgets in Echo Panel */
	GtkWidget *ec_enable;
	GtkAdjustment *ec_length;
	GtkAdjustment *ec_feedback;
	GdkWindow *file_dialog;
	GtkWidget *fs;
};

extern void cleanup_all_vtts();
extern void update_all_vtts();
extern void show_all_guis(int);

#endif
