/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999  Alexander K÷nig

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
    
    File: main.c
    
    Description: This contains the main() function. All the initializing
	         happens here.
    
    Changes:
    
    19 Mar 1999: Applied a patch by Andrew C. Bul+hac?k (eMail: acb@zikzak.net)
                 that fixes wavfile reading routine for the overreading bug.
		 
    20 Mar 1999: Big endian support.
    
    23 Mar 1999: display of new keys (<-, ->)
*/

#include <stdio.h>
#include "turntable.h"
#include "tX_gui.h"
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include "endian.h"
#include "tX_types.h"
#include "tX_wavfunc.h"
#include "tX_global.h"
#include "version.h"
#include "tX_dialog.h"
#include <gtk/gtk.h>
#include <glib.h>

Virtual_TurnTable *myvtt;

GTimer *my_time;
gint idle_tag;
/* main(): */

void idle()
{
	gdouble time;
	gulong ms;
	
	time=g_timer_elapsed(my_time, &ms);
	if (time > 1.5)
	{
		gtk_idle_remove(idle_tag);
		destroy_about();		
		display_gui();		
	}
}

int main(int argc, char **argv)
{
#ifndef WIN32
	fprintf(stderr, "%s, Copyright(C)1999 Alexander König, alkoit00@fht-esslingen.de\n", VERSIONSTRING);
#else
        fprintf(stderr, "%s, Copyright(C)1999 Alexander König, alkoit00@fht-esslingen.de\n", VERSIONSTRING);
        setenv ("CYGWIN", "binmode");
#endif
	fprintf(stderr, "terminatorX comes with ABSOLUTELY NO WARRANTY - for details read the license.\n");


        gtk_init (&argc, &argv);         

	load_globals();		

	if (globals.show_nag)
	{	
		show_about(1);

		my_time=g_timer_new();
		g_timer_start(my_time);		
	
		idle_tag=gtk_idle_add((GtkFunction)idle, NULL);
	}
			
	if (strlen(globals.scratch_name))
	{
		fprintf (stderr, "Loading: %s\n", globals.scratch_name);
		if (load_wav(globals.scratch_name, &globals.scratch_data, &globals.scratch_size))
		{
			strcpy(globals.scratch_name , "");
			globals.scratch_size=0;
			globals.scratch_len=0;
		}
		{
			globals.scratch_len=globals.scratch_size/sizeof(int16_t);
		}
	}

	if (strlen(globals.loop_name))
	{
		fprintf (stderr, "Loading: %s\n", globals.loop_name);
		if (load_wav(globals.loop_name, &globals.loop_data, &globals.loop_size))
		{
			strcpy(globals.loop_name , "");
			globals.loop_size=0;
			globals.loop_len=0;
			globals.do_mix=0;
		}
		{
			globals.loop_len=globals.loop_size/sizeof(int16_t);
		}
	}
	else
	{
		globals.do_mix=0;
	}
	
	globals.rec_buffer=0;
	
	if (malloc_recbuffer())
	{
		fprintf(stderr, "Failed to allocate record buffer.");
		return(1);
	}

	vttgl=vtt_new();
	
	create_gui(globals.width, globals.height);
		

	if (!globals.show_nag)	display_gui();
		
	gtk_main();

	if (globals.scratch_data) free(globals.scratch_data);
	if (globals.loop_data) free(globals.loop_data);
	if (globals.rec_buffer) free(globals.rec_buffer);
	
	store_globals();

	fprintf(stderr, "Have a nice life.\n");
	
	return (0);
}
