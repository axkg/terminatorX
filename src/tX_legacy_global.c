/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2016  Alexander König
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
    File: tX_legacy_global.cc
*/    

#include <config.h>

#ifdef ENABLE_TX_LEGACY

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tX_global.h"

void get_rc_name_old(char *buffer)
{
	strcpy(buffer,"");

	if (getenv("HOME"))
	{
		strcpy(buffer, getenv("HOME"));
		if (buffer[strlen(buffer)-1]!='/')
		strcat(buffer, "/");
	}
	
	strcat(buffer, ".terminatorX3rc.bin");
}

void load_globals_old() {	
	char rc_name[PATH_MAX+256]="";	
	FILE *rc;
	get_rc_name_old(rc_name);
	
	rc=fopen(rc_name, "r");
	if (rc)
	{
		fread(&globals, sizeof(tx_global), 1, rc);
		fclose(rc);
	}
	else
	{
		fprintf(stderr, "tX: .rc-file '%s' doesn't exist, reverting to defaults\n", rc_name);
	}

	/* i'll have to keep these as they're in the code
          everywhere but I think it doesn't make sense resetting
	  to old values on startup....
	*/
	globals.use_stdout_cmdline=0;
	globals.current_path[0] = 0;
	globals.pitch=1.0;
	globals.volume=1.0;	
}

#endif
