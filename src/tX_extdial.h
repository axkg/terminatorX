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
 
    File: tX_extdial.h 
*/    

#ifndef _h_tx_extdial_
#define _h_tx_extdial_

#include <gtk/gtk.h>
#include "tX_dial.h"
#include <stdio.h>

class tX_seqpar;

class tX_extdial
{
	GtkWidget *eventbox;
	GtkWidget *mainbox;
	GtkWidget *subbox;
	GtkWidget *dial;
	GtkWidget *label;
	GtkWidget *entry;
	GtkAdjustment *adj;
	int ignore_adj;
	float fval;
	char sval[30];

	private:
	void s2f() { sscanf(sval, "%f", &fval); /*printf("s2f(): s:%s, f%f\n", sval, fval);*/ };
	void f2s() { sprintf(sval, "%3f", fval); sval[4]=0; /* printf("f2s(): s:%s, f%f\n", sval, fval); */ };
	
	public:
	tX_extdial(const char *l, GtkAdjustment *a, tX_seqpar * sp, bool text_below=false, bool hide_entry=false);
	~tX_extdial();
	GtkWidget *get_widget() { return eventbox; };
	GtkWidget *get_dial() { return dial; }
	GtkWidget *get_entry() { return entry; }
	
	static GCallback f_entry(GtkWidget *w, tX_extdial *ed);
	static GCallback f_adjustment(GtkWidget *w, tX_extdial *ed);
};

#endif
