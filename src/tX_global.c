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
 
    File: tX_global.c
 
    Description:  This file contains the routines for handling the
    		  "globals" block. Intializing, reading setup from
		  disk and storing it.
		  
    Changes:
    
    21 Jul 1999: introduced the lowpass globals.    	  	  	  
*/    

#include <stdio.h>
#include <stdlib.h>
#include "tX_types.h"
#include "tX_global.h"
#include "turntable.h"
#include "string.h"

tx_global globals;
Virtual_TurnTable *vttgl;

void get_rc_name(char *buffer)
{
	strcpy(buffer,"");
	
	if (getenv("HOME"))
	{
		strcpy(buffer, getenv("HOME"));
		if (buffer[strlen(buffer)-1]!='/')
		strcat(buffer, "/");
	}
	strcat(buffer, ".terminatorXrc.bin");
}

void load_globals()
{
	char rc_name[PATH_MAX]="";
	FILE *rc;
	
	strcpy(globals.scratch_name, "scratch.wav");
	strcpy(globals.loop_name, "");
	
	strcpy(globals.prefix, "tX_scratch");
	strcpy(globals.audio_device, "/dev/dsp");
	
	strcpy(globals.xinput_device, "");
	globals.xinput_enable=0;
	
	globals.update_idle=18;
	
	globals.buff_no=2;
	globals.buff_size=9;
	
	globals.rec_size=1024000;

	globals.do_mix=1;
	globals.scratch_vol=0.5;
	
	globals.sense_cycles=12;
	
	globals.mouse_speed=0.8;
	
	globals.width=800;
	globals.height=440;	
	
	globals.vtt_default_speed=1.0;
	globals.tooltips=1;

	globals.time_update=15;
	globals.time_enable=1;
	
	globals.filectr=0;
	globals.reset_filectr=1;
	
	globals.use_y=0;
	globals.use_stdout=0;
	
	globals.show_nag=1;
	globals.prelis=1;
	
	globals.lowpass_enable=0;
	globals.lowpass_reso=0.9;
	
	strcpy(globals.last_fn,"");

	get_rc_name(rc_name);

	rc=fopen(rc_name, "r");
	if (rc)
	{
		fread(&globals, sizeof(tx_global), 1, rc);
		fclose(rc);
	}
	
	globals.scratch_size=0;
	globals.scratch_len=0;
	globals.scratch_data=NULL;

	globals.loop_size=0;
	globals.loop_len=0;
	globals.loop_data=NULL;
	
	globals.rec_buffer=NULL;
	globals.rec_len=0;
	if (globals.reset_filectr) globals.filectr=0;
}

void store_globals()
{
	char rc_name[PATH_MAX]="";
	FILE *rc;
	
	get_rc_name(rc_name);

	rc=fopen(rc_name, "w");
	if (rc)
	{
		fwrite(&globals, sizeof(tx_global), 1, rc);
		fclose(rc);
	}	
}
