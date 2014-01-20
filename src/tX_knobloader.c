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
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
    File: tX_knobloader.c
 
    Description: This code loads the knob-images required for tX_dial widget.
*/

#include <gtk/gtk.h>
#include "tX_knobloader.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

#ifdef USE_DIAL

const guint8 * knob_pixs[MAX_KNOB_PIX]={
	 knob0,
	 knob1,
	 knob2,
	 knob3,
	 knob4,
	 knob5,
	 knob6,
	 knob7,
	 knob8,
	 knob9,
	 knob10,
	 knob11,
	 knob12,
	 knob13,
	 knob14,
	 knob15,
	 knob16,
	 knob17,
	 knob18,
	 knob19,
	 knob20,
	 knob21,
	 knob22,
	 knob23,
	 knob24,
	 knob25,
	 knob26,
	 knob27,
	 knob28,
	 knob29,
	 knob30,
	 knob31,
	 knob32,
	 knob33,
	 knob34,
	 knob35,
	 knob36,
	 knob37,
	 knob38,
	 knob39,
	 knob40,
	 knob41,
	 knob42,
	 knob43,
	 knob44,
	 knob45,
	 knob46,
	 knob47,
	 knob48,
	 knob49,	 
	};

GdkPixbuf *knob_pixmaps[MAX_KNOB_PIX];

void load_knob_pixs()
{
	int i;
	GError *error;
	
	for (i=0; i<MAX_KNOB_PIX; i++) {
		knob_pixmaps[i]=gdk_pixbuf_new_from_inline(-1, knob_pixs[i], TRUE, &error);
	}
}

#endif
