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

typedef struct {
	char	scratch_name[PATH_MAX];
	unsigned int	scratch_size; // In Bytes
	unsigned int	scratch_len; // In samples
	int16_t	*scratch_data; 
	
	char	loop_name[PATH_MAX];
	unsigned int	loop_len;
	unsigned int	loop_size;
	int16_t	*loop_data;
	
	char	prefix[PATH_MAX];
	char	audio_device[PATH_MAX];
	
	int 	xinput_enable;
	char	xinput_device[256]; // If your device's name is longer than that
				    // you are insane and you simply don't deserve
				    // running terminatorX ;) (said the guy who invented 8+3)				    				    

	int 	do_mix;
	f_prec	scratch_vol;
	
	int	update_idle;
	
	int	buff_no;
	int	buff_size;
	
	int	rec_size; // In Bytes	
	int16_t	*rec_buffer;
	
	int	sense_cycles;
	
	int 	width;
	int 	height;
	
	f_prec	vtt_default_speed;
	
	int tooltips;
	
	f_prec mouse_speed;
	
	int time_enable;
	int time_update;

	int rec_len;
	
	int filectr;
	int reset_filectr;
	
	char last_fn[PATH_MAX];
	int use_y;
	int use_stdout;
	int show_nag;
	
	int prelis;
	
	int lowpass_enable;
	f_prec lowpass_reso;
	
	f_prec pitch;
	f_prec volume;
	
	int gui_wrap;
	
	char tables_filename[PATH_MAX];
	char record_filename[PATH_MAX];
	int autoname;
	
} tx_global;

extern tx_global globals;

extern void load_globals();
extern void store_globals();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
