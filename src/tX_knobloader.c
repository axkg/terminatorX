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
 
    File: tX_knobloader.c
 
    Description: This code loads the knob-images required for tX_dial widget.
*/

#include <gtk/gtk.h>
#include "tX_knobloader.h"

#ifdef USE_DIAL

#ifdef USE_BIG_BUTTONS
#include "bigknob/knob0.xpm"
#include "bigknob/knob1.xpm"
#include "bigknob/knob2.xpm"
#include "bigknob/knob3.xpm"
#include "bigknob/knob4.xpm"
#include "bigknob/knob5.xpm"
#include "bigknob/knob6.xpm"
#include "bigknob/knob7.xpm"
#include "bigknob/knob8.xpm"
#include "bigknob/knob9.xpm"
#include "bigknob/knob10.xpm"
#include "bigknob/knob11.xpm"
#include "bigknob/knob12.xpm"
#include "bigknob/knob13.xpm"
#include "bigknob/knob14.xpm"
#include "bigknob/knob15.xpm"
#include "bigknob/knob16.xpm"
#include "bigknob/knob17.xpm"
#include "bigknob/knob18.xpm"
#include "bigknob/knob19.xpm"
#include "bigknob/knob20.xpm"
#include "bigknob/knob21.xpm"
#include "bigknob/knob22.xpm"
#include "bigknob/knob23.xpm"
#include "bigknob/knob24.xpm"
#include "bigknob/knob25.xpm"
#include "bigknob/knob26.xpm"
#include "bigknob/knob27.xpm"
#include "bigknob/knob28.xpm"
#include "bigknob/knob29.xpm"
#include "bigknob/knob30.xpm"
#include "bigknob/knob31.xpm"
#include "bigknob/knob32.xpm"
#include "bigknob/knob33.xpm"
#include "bigknob/knob34.xpm"
#include "bigknob/knob35.xpm"
#include "bigknob/knob36.xpm"
#include "bigknob/knob37.xpm"
#include "bigknob/knob38.xpm"
#include "bigknob/knob39.xpm"
#include "bigknob/knob40.xpm"
#include "bigknob/knob41.xpm"
#include "bigknob/knob42.xpm"
#include "bigknob/knob43.xpm"
#include "bigknob/knob44.xpm"
#include "bigknob/knob45.xpm"
#include "bigknob/knob46.xpm"
#include "bigknob/knob47.xpm"
#include "bigknob/knob48.xpm"
#include "bigknob/knob49.xpm"
#else
#include "smallknob/knob0.xpm"
#include "smallknob/knob1.xpm"
#include "smallknob/knob2.xpm"
#include "smallknob/knob3.xpm"
#include "smallknob/knob4.xpm"
#include "smallknob/knob5.xpm"
#include "smallknob/knob6.xpm"
#include "smallknob/knob7.xpm"
#include "smallknob/knob8.xpm"
#include "smallknob/knob9.xpm"
#include "smallknob/knob10.xpm"
#include "smallknob/knob11.xpm"
#include "smallknob/knob12.xpm"
#include "smallknob/knob13.xpm"
#include "smallknob/knob14.xpm"
#include "smallknob/knob15.xpm"
#include "smallknob/knob16.xpm"
#include "smallknob/knob17.xpm"
#include "smallknob/knob18.xpm"
#include "smallknob/knob19.xpm"
#include "smallknob/knob20.xpm"
#include "smallknob/knob21.xpm"
#include "smallknob/knob22.xpm"
#include "smallknob/knob23.xpm"
#include "smallknob/knob24.xpm"
#include "smallknob/knob25.xpm"
#include "smallknob/knob26.xpm"
#include "smallknob/knob27.xpm"
#include "smallknob/knob28.xpm"
#include "smallknob/knob29.xpm"
#include "smallknob/knob30.xpm"
#include "smallknob/knob31.xpm"
#include "smallknob/knob32.xpm"
#include "smallknob/knob33.xpm"
#include "smallknob/knob34.xpm"
#include "smallknob/knob35.xpm"
#include "smallknob/knob36.xpm"
#include "smallknob/knob37.xpm"
#include "smallknob/knob38.xpm"
#include "smallknob/knob39.xpm"
#include "smallknob/knob40.xpm"
#include "smallknob/knob41.xpm"
#include "smallknob/knob42.xpm"
#include "smallknob/knob43.xpm"
#include "smallknob/knob44.xpm"
#include "smallknob/knob45.xpm"
#include "smallknob/knob46.xpm"
#include "smallknob/knob47.xpm"
#include "smallknob/knob48.xpm"
#include "smallknob/knob49.xpm"
#endif
char ** knob_pixs[MAX_KNOB_PIX]={
	 knob0_xpm,
	 knob1_xpm,
	 knob2_xpm,
	 knob3_xpm,
	 knob4_xpm,
	 knob5_xpm,
	 knob6_xpm,
	 knob7_xpm,
	 knob8_xpm,
	 knob9_xpm,
	 knob10_xpm,
	 knob11_xpm,
	 knob12_xpm,
	 knob13_xpm,
	 knob14_xpm,
	 knob15_xpm,
	 knob16_xpm,
	 knob17_xpm,
	 knob18_xpm,
	 knob19_xpm,
	 knob20_xpm,
	 knob21_xpm,
	 knob22_xpm,
	 knob23_xpm,
	 knob24_xpm,
	 knob25_xpm,
	 knob26_xpm,
	 knob27_xpm,
	 knob28_xpm,
	 knob29_xpm,
	 knob30_xpm,
	 knob31_xpm,
	 knob32_xpm,
	 knob33_xpm,
	 knob34_xpm,
	 knob35_xpm,
	 knob36_xpm,
	 knob37_xpm,
	 knob38_xpm,
	 knob39_xpm,
	 knob40_xpm,
	 knob41_xpm,
	 knob42_xpm,
	 knob43_xpm,
	 knob44_xpm,
	 knob45_xpm,
	 knob46_xpm,
	 knob47_xpm,
	 knob48_xpm,
	 knob49_xpm,	 
	};

GdkPixbuf *knob_pixmaps[MAX_KNOB_PIX];
GdkBitmap *knob_mask;
	
void load_knob_pixs()
{
	int i;

	for (i=0; i<MAX_KNOB_PIX; i++) {
		knob_pixmaps[i]=gdk_pixbuf_new_from_xpm_data((const char **) knob_pixs[i]);
	}
	
	gdk_pixbuf_render_pixmap_and_mask(knob_pixmaps[0], NULL, &knob_mask, 1);
}

#endif
