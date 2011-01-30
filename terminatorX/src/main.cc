/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2011  Alexander König

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

#include "tX_endian.h"
#include "tX_types.h"
#include "tX_global.h"
#include "tX_audiodevice.h"
#include "version.h"
#include "tX_dialog.h"
#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>

#include "tX_ladspa.h"
#include "tX_ladspa_class.h"
#include "tX_engine.h"
#include "tX_capabilities.h"
#include "tX_pbutton.h"

#ifdef CREATE_BENCHMARK 
#include "tX_vtt.h"
#endif

#ifdef USE_SCHEDULER
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef USE_JACK	
void jack_check()
{
	if ((!tX_jack_client::get_instance()) && (globals.audiodevice_type==JACK)) {
		tx_note("Couldn't connect to JACK server - JACK output not available.\n\nIf you want to use JACK, ensure the JACK daemon is running before you start terminatorX.", true);
	}
}
#endif // USE_JACK

static bool timesup=false;

gboolean timeout(void *)
{
	timesup=true;
	return FALSE;
}

void show_help()
{
			
	fprintf(stderr, "\
usage: terminatorX [options]n\
\n\
  -h, --help 			Display help info\n\
  -f, --file			Load saved terminatorX set file\n\
  -r, --rc-file [file]		Load alternate rc file\n\
  -d, --dont-save		Do not save settings at exit\n\
  -s, --std-out                 Use stdout for sound output\n\
  --device=[output device]      Use alternate device for sound output\n\
\n");
}

int parse_args(int *argc, char **argv)
{
	// pass over argv once to see if we need to load an alternate_rc file
	for (int i = 1 ; i != *argc ; ++i ) {
		if ((strcmp(argv[i], "-r") == 0) || (strcmp(argv[i], "--rc-file") == 0)) {
			if (argv[i+1] ) {	
				++i;
				fprintf(stderr, "tX: Loading alternate rc file %s\n", argv[i]);
				globals.alternate_rc = argv[i];
			} else {
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
	for (int i = 1 ; i < *argc ; ++i ) {
		if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--file") == 0)) {
			++i;
			globals.startup_set = argv[i];
		} else if (((strcmp(argv[i], "-r") == 0) || (strcmp(argv[i], "--rc-file") == 0)) && (argv[i+1])) {
			++i;
			globals.alternate_rc = argv[i];
		} else if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--dont-save") == 0)) {
			fprintf(stderr, "tX: Do not save settings on exit\n");
			globals.store_globals = 0;

		} else if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--std-out") == 0)) {
			globals.use_stdout_cmdline = 1;
			globals.use_stdout = 1;
		} else if ((strncmp(argv[i], "--device",8) == 0)) {
			if (strlen(argv[i]+9)<=PATH_MAX)
				strcpy(globals.oss_device,argv[i]+9);
			else {
				show_help();
				exit(1);
			}
		} else {
			show_help();
			exit(1);
		}
	}
	return 1;
}

void checkenv(const char *name)
{
	char *value;
	int length;
	
	value=getenv(name);
	if (value) {
		length=strlen(value);
		/*
		 strnlen requires extra macros...
		 length=strnlen(value, PATH_MAX+1);
		*/
		
		if (length>=PATH_MAX) {
			tX_error("Your \"%s\" environment variable seems malicious (%i chars).", name, length);
			tX_error("Please correct that and restart terminatorX.");
			exit(-1);
		}
	}
}

int main(int argc, char **argv)
{
	fprintf(stderr, "%s - Copyright (C) 1999-2011 by Alexander König\n", VERSIONSTRING);
	fprintf(stderr, "terminatorX comes with ABSOLUTELY NO WARRANTY - for details read the license.\n");

#ifdef USE_CAPABILITIES	
	if (!geteuid()) {
		if (prctl(PR_SET_KEEPCAPS, 1, -1, -1, -1)) {
			tX_error("failed to keep capabilites.");
		}
		set_nice_capability(CAP_PERMITTED);
	}
#endif

	if ((!geteuid()) && (getuid() != geteuid())) {
		tX_msg("runnig suid-root - dropping root privileges.");
		
		int result=setuid(getuid());
		
		if (result) {
			tX_error("main() Panic: can't drop root privileges.");
			exit(2);
		}
	}
	
	/* No suidroot below this comment. */
	
#ifdef USE_CAPABILITIES		
	set_nice_capability(CAP_EFFECTIVE);	
#endif
	
	/* Theses checks are now sort of unecessary... Anyway... */
	checkenv("HOME");
	checkenv("XLOCALEDIR");	

	gtk_init (&argc, &argv);
	gtk_set_locale();
	
#ifdef USE_STARTUP_NOTIFICATION
	// startup isn't really finished with the nagbox alone...
	gtk_window_set_auto_startup_notification(FALSE);
#endif	
	
	parse_args(&argc, argv); // loads settings

	if (globals.show_nag) {	
		show_about(1);
		g_timeout_add(2000, (GSourceFunc) timeout, NULL);
	}
	
	tX_engine *engine=tX_engine::get_instance();
	LADSPA_Class::init();
	LADSPA_Plugin::init();

#ifdef USE_JACK	
	tX_jack_client::init();
#endif	

#ifdef USE_SCHEDULER
	tX_debug("main() GUI thread is p:%i, t:%i and has policy %i.", getpid(), (int) pthread_self(), sched_getscheduler(getpid()));
#endif	
	tx_icons_init();
	create_mastergui(globals.width, globals.height);
	
	if (globals.show_nag) {
		while (!timesup) {
			while (gtk_events_pending()) gtk_main_iteration(); 
			gdk_flush();				
			usleep(250);
		}
		destroy_about();
	}
	
#ifdef USE_JACK
	jack_check();
#endif
	display_mastergui();
		
	if (globals.startup_set) {
		while (gtk_events_pending()) gtk_main_iteration(); gdk_flush();	
		tX_cursor::set_cursor(tX_cursor::WAIT_CURSOR);
		load_tt_part(globals.startup_set);
		tX_cursor::reset_cursor();
	} else {
#ifdef USE_ALSA_MIDI_IN
		if (globals.auto_assign_midi) tX_midiin::auto_assign_midi_mappings(NULL, NULL);
#endif		
	}

#ifdef USE_STARTUP_NOTIFICATION
	gdk_notify_startup_complete();
#endif	
	
#ifndef CREATE_BENCHMARK
	gtk_main();

	store_globals();

	delete engine;
#ifdef USE_JACK	
	if (tX_jack_client::get_instance()) {
		delete tX_jack_client::get_instance();
	}
#endif // USE_JACK
	
	fprintf(stderr, "Have a nice life.\n");
#else // CREATE_BENCHMARK
	gtk_widget_hide(main_window);
	while (gtk_events_pending()) gtk_main_iteration(); gdk_flush();	
	gdk_flush();
	
	vtt_class::set_sample_rate(48000);
	
	printf("\n* BENCHMARKING *\n");
	
	GTimer *bench_time = g_timer_new();
	gulong micros;
	double ratio;
	double res;
	list <vtt_class *> :: iterator vtt;
	
	for (vtt=vtt_class::main_list.begin(); vtt!=vtt_class::main_list.end(); vtt++) {
		if ((*vtt)->autotrigger) (*vtt)->trigger();
	}
	
	sleep(3);	
	g_timer_start(bench_time);
	
	for (int i=0; i<BENCH_CYCLES; i++) {
		vtt_class::render_all_turntables();
	}
	g_timer_stop(bench_time);
	res=g_timer_elapsed(bench_time, &micros);
	
	ratio=((double) BENCH_CYCLES)/res;
	printf ("Rendered %i blocks in %f secons,\n=> %f blocks per second.\n\n", (long) BENCH_CYCLES, res, ratio);
#endif // CREATE_BENCHMARK
	return (0);
}
