/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999, 2000  Alexander K÷nig

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
    
    4 October 1999: Rewrite ;) - back to C++
*/

#define TX_GTKRC "/usr/share/themes/terminatorX/gtk/gtkrc"

#define BENCH_CYCLES 100000

#include <stdio.h>
#include "tX_mastergui.h"
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_3DNOW
#include "3dnow.h"
#endif

#include "tX_endian.h"
#include "tX_types.h"
#include "tX_global.h"
#include "version.h"
#include "tX_dialog.h"
#include <gtk/gtk.h>
#include <glib.h>

#include "tX_ladspa.h"

#ifdef CREATE_BENCHMARK 
#include "tX_vtt.h"
#endif

#ifdef USE_DIAL
#include "tX_knobloader.h"
#endif

GTimer *my_time;
gint idle_tag;
/* main(): */

int idle()
{
	gdouble time;
	gulong ms;
	
	time=g_timer_elapsed(my_time, &ms);
	if (time > 1.5)
	{
		gtk_idle_remove(idle_tag);
		destroy_about();		
		display_mastergui();		
	}
	
	return TRUE;
}

int main(int argc, char **argv)
{
	char *startup_set=NULL;
	FILE *gtk_rc_file;
	
#ifndef WIN32
	fprintf(stderr, "%s, Copyright(C)1999 Alexander König, alkoit00@fht-esslingen.de\n", VERSIONSTRING);
#else
        fprintf(stderr, "%s, Copyright(C)1999 Alexander König, alkoit00@fht-esslingen.de\n", VERSIONSTRING);
        setenv ("CYGWIN", "binmode");
#endif
	fprintf(stderr, "terminatorX comes with ABSOLUTELY NO WARRANTY - for details read the license.\n");

#ifdef USE_3DNOW
	if (mm_support()!=5)
	{
		printf("3DNow! not detected. Giving up.\n");
		return(1);
	}
	else
        printf("3DNow! accelerations available.\n");	
#endif
	gtk_set_locale ();
        gtk_init (&argc, &argv);
	
	gtk_rc_file=fopen(TX_GTKRC, "r");
	if (gtk_rc_file)
	{
		fprintf (stderr, "Using terminatorX gtkrc.\n");
		fclose(gtk_rc_file);
		gtk_rc_parse(TX_GTKRC);
	}
	
	if (argc >1) startup_set=argv[1];

	load_globals();		

	if (globals.show_nag)
	{	
		show_about(1);

		my_time=g_timer_new();
		g_timer_start(my_time);		
	
		idle_tag=gtk_idle_add((GtkFunction)idle, NULL);
	}
	
	LADSPA_Plugin :: init();
//	LADSPA_Plugin :: status();
			
	create_mastergui(globals.width, globals.height);
		
	if (!globals.show_nag)	display_mastergui();
		
	if (startup_set)
	{
		while (gtk_events_pending()) gtk_main_iteration(); gdk_flush();	
		load_tt_part(startup_set);
	}
		
#ifndef CREATE_BENCHMARK
#ifdef USE_DIAL
	load_knob_pixs(main_window);
#endif	

	gtk_main();

	store_globals();

	fprintf(stderr, "Have a nice life.\n");
#else
	gtk_widget_hide(main_window);
	while (gtk_events_pending()) gtk_main_iteration(); gdk_flush();	
	gdk_flush();
	
	vtt_class::set_mix_buffer_size(globals.true_block_size);
	printf("\n* BENCHMARKING *\nBlocksize is %i samples.\n", globals.true_block_size);
	
	GTimer *bench_time = g_timer_new();
	gulong micros;
	double ratio;
	double res;
	list <vtt_class *> :: iterator vtt;
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++)
	{
		if ((*vtt)->autotrigger) (*vtt)->trigger();
	}
	sleep(3);
	
	g_timer_start(bench_time);
	for (int i=0; i<BENCH_CYCLES; i++)
	{
		vtt_class::render_all_turntables();
	}
	g_timer_stop(bench_time);
	res=g_timer_elapsed(bench_time, &micros);
	
	ratio=((double) BENCH_CYCLES)/res;
	printf ("Rendered %i blocks in %f secons,\n=> %f blocks per second.\n\n", (long) BENCH_CYCLES, res, ratio);
#endif
	return (0);
}
