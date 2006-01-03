/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2006  Alexander König
 
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
 
    File: tX_loaddlg.h
 
    Description: Header to tX_loaddlg.cc
		 
*/    
#ifndef _h_tX_loaddlg
#define _h_tX_loaddlg 1
#define TX_LOADDLG_MODE_SINGLE 0
#define TX_LOADDLG_MODE_MULTI 1

#include <gtk/gtk.h>
extern GtkWidget *ld_loaddlg;

extern int ld_create_loaddlg(int mode, int count);
extern void ld_set_setname(char *name);
extern void ld_set_filename(char *name);
extern void ld_set_progress(gfloat progress);
extern void ld_destroy();
extern char *strip_path(char *name);

#endif
