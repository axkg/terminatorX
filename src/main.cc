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

void show_help()
{
			
	fprintf(stderr, "\
usage: terminatorX [options] [save file]\n\
\n\
  -h, --help 			Display help info\n\
  -f, --file			Load saved terminatorX set file\n\
  -r, --rc-file [file]		Load alternate rc file\n\
  -d, --dont-save		Do not save settings at exit\n\
  -s, --std-out                 Use stdout for sound output\n\
   --device=[output device]  Use alternate device for sound output\n\
\n");
/*
  -n, --no-gui			Run terminatorX with no GUI\n\
  -m, --midi-in [file]		Use [file] for midi input\n\
  -o, --midi-out [file]		Use [file] for midi input\n\
  -s, --std-out			Use stdout for sound output\n\
\n");
*/
}


int parse_args(int *argc, char **argv)
{
	// pass over argv once to see if we need to load an alternate_rc file
	for (int i = 1 ; i != *argc ; ++i )
	{
		if ((strcmp(argv[i], "-r") == 0) || (strcmp(argv[i], "--rc-file") == 0)) 
		{
			if (argv[i+1] )
			{	
				++i;
				fprintf(stderr, "tX: Loading alternate rc file %s\n", argv[i]);
				globals.alternate_rc = argv[i];
			}
			else
			{
				show_help();	
				exit(1);
			}
			break;
		}
	}
	
	// load up the global values
	load_globals();

	// default the flag options, or they'll be set from last execution... (think globals.no_gui ;)
	globals.no_gui = 0;
	globals.alternate_rc = 0;
	globals.store_globals = 1;
	globals.startup_set = 0;
		
	// then pass over again, this time setting passed values
	for (int i = 1 ; i < *argc ; ++i )
	{
		if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--file") == 0))
		{
			++i;
			globals.startup_set = argv[i];
		}	
		else if (((strcmp(argv[i], "-r") == 0) || (strcmp(argv[i], "--rc-file") == 0)) && (argv[i+1]))
		{
			++i;
			globals.alternate_rc = argv[i];
		}
		else if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--dont-save") == 0))
		{
			fprintf(stderr, "tX: Do not save settings on exit\n");
			globals.store_globals = 0;

		}
		else if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--std-out") == 0))
		{
			globals.use_stdout_cmdline = 1;
			globals.use_stdout = 1;
		}
		else if ((strncmp(argv[i], "--device",8) == 0))
		{
			if (strlen(argv[i]+9)<=PATH_MAX)
				strcpy(globals.audio_device,argv[i]+9);
			else
			{
				show_help();
                                exit(1);
			}
		}
/*		
		else if ((strcmp(argv[i], "-m") == 0) || (strcmp(argv[i], "--midi-in") == 0))
		{
		}
		else if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--std-out") == 0))
		{
		}	
		else if ((strcmp(argv[i], "-n") == 0) || (strcmp(argv[i], "--no-gui") == 0))
		{
			globals.no_gui = 1;
			fprintf(stderr, "tX: Run without a GUI\n");
		}
*/
		else
		{
			show_help();
			exit(1);
		}
	}
	return 1;
}

int main(int argc, char **argv)
{
	FILE *gtk_rc_file;
	
	fprintf(stderr, "%s - Copyright (C) 1999, 2000 by Alexander König\n", VERSIONSTRING);

#ifdef WIN32
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

	parse_args(&argc, argv); 

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
		
	if (globals.startup_set)
	{
		while (gtk_events_pending()) gtk_main_iteration(); gdk_flush();	
		load_tt_part(globals.startup_set);
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
