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
 
    File: tX_loaddlg.cc
 
    Description: Displays the progress indicator dialog for file
    		 loading.
		 
*/ 
#include <gtk/gtk.h>
#include "tX_loaddlg.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "tX_mastergui.h"

GtkWidget *ld_loaddlg=(GtkWidget *) NULL;
GtkWidget *ld_single_l=(GtkWidget *)NULL;
GtkWidget *ld_single_p=(GtkWidget *)NULL;
GtkWidget *ld_multi_l=(GtkWidget *)NULL;
GtkWidget *ld_multi_p=(GtkWidget *)NULL;
GtkWindow *ld_window=(GtkWindow *)NULL;

int ld_mode;
int ld_count;
int ld_current;

gfloat ld_old_prog;

#define WID_DYN TRUE, TRUE, 0
#define WID_FIX FALSE, FALSE, 0
#define add_widget_dyn(wid); gtk_box_pack_start(GTK_BOX(vbox), wid, WID_DYN);\
	gtk_widget_show(wid);
	
#define add_widget_fix(wid); gtk_box_pack_start(GTK_BOX(vbox), wid, WID_FIX);\
	gtk_widget_show(wid);

#define gtk_flush(); while (gtk_events_pending()) gtk_main_iteration(); gdk_flush();


int ld_create_loaddlg(int mode, int count)
{
	GtkWidget *vbox;
	GtkWidget *actionarea;
	GtkWidget *dummy;
	
	if (ld_loaddlg) return(1);
	
//	if(needinit) ld_init();
	
	ld_mode=mode;
	ld_count=count;
	
	ld_loaddlg=gtk_dialog_new();
	ld_window=&(GTK_DIALOG(ld_loaddlg)->window);
	gtk_window_set_title(ld_window, "terminatorX - Loading");
	gtk_container_set_border_width(GTK_CONTAINER(ld_window), 5);
	
	vbox=GTK_WIDGET(GTK_DIALOG(ld_loaddlg)->vbox);
	gtk_box_set_spacing(GTK_BOX(vbox), 5);

	actionarea=GTK_WIDGET(GTK_DIALOG(ld_loaddlg)->action_area);
	gtk_box_set_spacing(GTK_BOX(actionarea), 5);
	
	if (mode==TX_LOADDLG_MODE_MULTI)
	{
		ld_multi_l=gtk_label_new("Loading Set");
		gtk_misc_set_alignment(GTK_MISC(ld_multi_l), 0.5, 0.5);
		add_widget_fix(ld_multi_l);
		
		ld_multi_p=gtk_progress_bar_new();
		add_widget_fix(ld_multi_p);
		
		dummy=gtk_hseparator_new();
		add_widget_fix(dummy);
		
		ld_current=0;
	}
	
	ld_single_l=gtk_label_new("Loading File");
	add_widget_fix(ld_single_l);
		
	ld_single_p=gtk_progress_bar_new();
	add_widget_fix(ld_single_p);
	
	dummy=gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(actionarea), dummy, WID_DYN);
	gtk_widget_show(dummy);

	dummy=gtk_widget_get_toplevel(dummy);


	gtk_window_set_modal(ld_window, TRUE);
	gtk_window_set_default_size(ld_window, 400, 100);
	gtk_window_set_position(ld_window, GTK_WIN_POS_CENTER_ALWAYS);
	gtk_widget_realize(ld_loaddlg);
	gdk_window_set_decorations(gtk_widget_get_parent_window(vbox),(GdkWMDecoration) 0);

//	gtk_window_reposition(ld_window, gdk_screen_width()/2-200, gdk_screen_height()/2-50);
	//gtk_window_reposition(ld_window, gdk_screen_width()/2-200, gdk_screen_height()/2-50);
	gtk_widget_show(ld_loaddlg);

	gtk_flush();
		
	return(0);
}

char *strip_path(char *name)
{
	char *tmp;
	
	tmp=strrchr(name, (int) '/');
	
	if (tmp)
	{
		if (strlen(tmp)>1)
		{
			tmp++;
		}
	}
	else tmp=name;
		
	return(tmp);
}

void ld_set_setname(char *name)
{
	char *setname;
	char buffer[1024];
	
	setname=strip_path(name);
	sprintf(buffer, "Loading tX-set [%s]", setname);
	gtk_label_set_text(GTK_LABEL(ld_multi_l), buffer);
	gtk_flush();
}

void ld_set_filename(char *name)
{
	char *filename;
	char buffer[1024];
	gfloat setprog;
	
	ld_current++;
	ld_old_prog=-1;
	filename=strip_path(name);
	if (ld_mode==TX_LOADDLG_MODE_MULTI)
	{
		sprintf(buffer, "Loading file No. %i of %i [%s]", ld_current, ld_count, filename);
	}
	else
	{
		sprintf(buffer, "Loading file [%s]", filename);	
	}
	gtk_label_set_text(GTK_LABEL(ld_single_l), buffer);
	
	if (ld_mode==TX_LOADDLG_MODE_MULTI)
	{
		setprog=(((float) ld_current)/((float) ld_count));
		gtk_progress_bar_update(GTK_PROGRESS_BAR(ld_multi_p), setprog);
		gtk_flush();		
	}
	gtk_flush();
}

void ld_set_progress(gfloat progress)
{
	progress=floor(progress*1000.0)/1000.0;
	
	//printf("%f\n", progress);
	
	if (progress!=ld_old_prog)
	{
		gtk_progress_bar_update(GTK_PROGRESS_BAR(ld_single_p), progress);
		gtk_flush();
	}
	
	ld_old_prog=progress;
}

void ld_destroy()
{
	if (ld_loaddlg)
	{
		gtk_widget_hide(ld_loaddlg);
		gtk_widget_destroy(ld_loaddlg);
	}
	
	ld_loaddlg=NULL;
	mg_update_status();
}
