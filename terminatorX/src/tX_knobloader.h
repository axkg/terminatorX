/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2004  Alexander König
 
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
 
    File: tX_knobloader.h
 
    Description: Header to tX_knobloader.c
*/
    
#ifndef _tX_knobloader_
#define _tX_knobloader_ 1

#include <gtk/gtk.h>
#include "icons/knobs.pixbuf"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_DIAL

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define MAX_KNOB_PIX 50
#define TX_MAX_KNOB_PIX 49

#ifdef USE_BIG_BUTTONS
#define KNOB_SIZE 51
#else
#define KNOB_SIZE 42
#endif

extern GdkPixbuf *knob_pixmaps[MAX_KNOB_PIX];
	
extern void load_knob_pixs();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#endif
