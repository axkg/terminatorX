/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999, 2000  Alexander König
 
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
 
    File: tX_global.h
 
    Description: Header to tX_global.c / defines the heavily used
    		 tX_global struct.
		 
    Changes:
    
    21 Jul 1999: Introduced the lowpass globals.
*/    

#ifndef _TX_GLOBAL_H
#define _TX_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <limits.h>
#include <stdio.h>
#include "tX_types.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define BUTTON_TYPE_ICON 1
#define BUTTON_TYPE_TEXT 2
#define BUTTON_TYPE_BOTH 3

typedef struct {
	char	audio_device[PATH_MAX];
	
	int 	xinput_enable;
	char	xinput_device[256]; // If your device's name is longer than that
				    // you are insane and you simply don't deserve
				    // running terminatorX ;) (said the guy who invented 8+3)				    				    
	int	store_globals;		// if it should store the globals vals on exit
	char 	*startup_set;	
	char	*alternate_rc;		// a diferent set of stored globals to load
	int	no_gui;			// run without any gui
	
	int	update_idle;
	
	int	buff_no;
	int	buff_size;
	
	int	sense_cycles;
	
	int 	width;
	int 	height;

	int tooltips;
	
	f_prec mouse_speed;
		
	char last_fn[PATH_MAX];

	int use_stdout;
	int show_nag;
	
	int prelis;
	
	f_prec pitch;
	f_prec volume;
	
	int gui_wrap;
	
	char tables_filename[PATH_MAX];
	char record_filename[PATH_MAX];
	int autoname;
	
	float flash_response;
	
	int button_type;
	
	char file_editor[PATH_MAX];
	int true_block_size;
	int update_delay; 
} tx_global;

extern tx_global globals;

extern void load_globals();
extern void store_globals();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
