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
#include "string.h"

tx_global globals;

void get_rc_name(char *buffer)
{
	strcpy(buffer,"");
	if (globals.alternate_rc)
	{
		strcpy(buffer, globals.alternate_rc);
	}
	else 
	{
		if (getenv("HOME"))
		{
			strcpy(buffer, getenv("HOME"));
			if (buffer[strlen(buffer)-1]!='/')
			strcat(buffer, "/");
		}
		strcat(buffer, ".terminatorX3rc.bin");
	}
}

void load_globals()
{
	char rc_name[PATH_MAX]="";
	FILE *rc;

	get_rc_name(rc_name);
	
	rc=fopen(rc_name, "r");
	if (rc)
	{
		fread(&globals, sizeof(tx_global), 1, rc);
		fclose(rc);
	}
	else
	{
		fprintf(stderr, "tX: .rc-file '%s' doesn't exist, reverting to defaults\n", rc_name);

		globals.startup_set = 0;
		globals.store_globals = 1;
		globals.no_gui = 0;
		globals.alternate_rc = 0;
	
		strcpy(globals.audio_device, "/dev/dsp");
	
		strcpy(globals.xinput_device, "");
		globals.xinput_enable=0;
	
		globals.update_idle=18;
		globals.update_delay=1;
	
		globals.buff_no=2;
		globals.buff_size=9;
		
		globals.sense_cycles=12;
	
		globals.mouse_speed=0.8;
	
		globals.width=800;
		globals.height=440;	
	
		globals.tooltips=1;
	
		globals.use_stdout=0;
		globals.use_stdout_from_conf_file=0;
	
		globals.show_nag=1;
		globals.prelis=1;
	
		strcpy(globals.last_fn,"");
	
		globals.pitch=1.0;
		globals.volume=1.0;
		globals.gui_wrap=3;
	
		globals.flash_response=0.95;
	
		globals.button_type=BUTTON_TYPE_BOTH;
		
		globals.true_block_size=0;
	
		strcpy(globals.tables_filename, "");
		strcpy(globals.record_filename, "tX_record.wav");
		strcpy(globals.file_editor, "");
	}

	/* i'll have to keep these as they're in the code
          everywhere but I think it doesn't make sense resetting
	  to old values on startup....
	*/
	globals.use_stdout_cmdline=0;
	globals.current_path = NULL;
	globals.pitch=1.0;
	globals.volume=1.0;	
	if (!globals.true_block_size) globals.true_block_size=1<globals.buff_size;
}

void store_globals()
{
	char rc_name[PATH_MAX]="";
	FILE *rc;
	
	if (globals.store_globals)
	{

		get_rc_name(rc_name);

		rc=fopen(rc_name, "w");
		if (rc)
		{
			// doesn't really make sense to save pointers...
			globals.startup_set = NULL;
			globals.alternate_rc = NULL;
			globals.current_path = NULL; 
			
			if (globals.use_stdout_cmdline)
				globals.use_stdout = globals.use_stdout_from_conf_file;
			fwrite(&globals, sizeof(tx_global), 1, rc);
			fclose(rc);
		}
	}	
}
