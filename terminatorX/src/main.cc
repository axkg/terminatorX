/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2002  Alexander König

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

#include "tX_engine.h"

#ifdef CREATE_BENCHMARK 
#include "tX_vtt.h"
#endif

GTimer *my_time;
gint idle_tag;

#ifdef USE_3DNOW
/*	Function to test if multimedia instructions are supported...
*/
inline int
tx_mm_support(void)
{
	/* Returns 1 if MMX instructions are supported,
	   3 if Cyrix MMX and Extended MMX instructions are supported
	   5 if AMD MMX and 3DNow! instructions are supported
	   0 if hardware does not support any of these
	*/
	register int rval = 0;

	__asm__ __volatile__ (
		/* See if CPUID instruction is supported ... */
		/* ... Get copies of EFLAGS into eax and ecx */
		"pushf\n\t"
		"popl %%eax\n\t"
		"movl %%eax, %%ecx\n\t"

		/* ... Toggle the ID bit in one copy and store */
		/*     to the EFLAGS reg */
		"xorl $0x200000, %%eax\n\t"
		"push %%eax\n\t"
		"popf\n\t"

		/* ... Get the (hopefully modified) EFLAGS */
		"pushf\n\t"
		"popl %%eax\n\t"

		/* ... Compare and test result */
		"xorl %%eax, %%ecx\n\t"
		"testl $0x200000, %%ecx\n\t"
		"jz NotSupported1\n\t"		/* CPUID not supported */


		/* Get standard CPUID information, and
		       go to a specific vendor section */
		"movl $0, %%eax\n\t"
		"cpuid\n\t"

		/* Check for Intel */
		"cmpl $0x756e6547, %%ebx\n\t"
		"jne TryAMD\n\t"
		"cmpl $0x49656e69, %%edx\n\t"
		"jne TryAMD\n\t"
		"cmpl $0x6c65746e, %%ecx\n"
		"jne TryAMD\n\t"
		"jmp Intel\n\t"

		/* Check for AMD */
		"\nTryAMD:\n\t"
		"cmpl $0x68747541, %%ebx\n\t"
		"jne TryCyrix\n\t"
		"cmpl $0x69746e65, %%edx\n\t"
		"jne TryCyrix\n\t"
		"cmpl $0x444d4163, %%ecx\n"
		"jne TryCyrix\n\t"
		"jmp AMD\n\t"

		/* Check for Cyrix */
		"\nTryCyrix:\n\t"
		"cmpl $0x69727943, %%ebx\n\t"
		"jne NotSupported2\n\t"
		"cmpl $0x736e4978, %%edx\n\t"
		"jne NotSupported3\n\t"
		"cmpl $0x64616574, %%ecx\n\t"
		"jne NotSupported4\n\t"
		/* Drop through to Cyrix... */


		/* Cyrix Section */
		/* See if extended CPUID level 80000001 is supported */
		/* The value of CPUID/80000001 for the 6x86MX is undefined
		   according to the Cyrix CPU Detection Guide (Preliminary
		   Rev. 1.01 table 1), so we'll check the value of eax for
		   CPUID/0 to see if standard CPUID level 2 is supported.
		   According to the table, the only CPU which supports level
		   2 is also the only one which supports extended CPUID levels.
		*/
		"cmpl $0x2, %%eax\n\t"
		"jne MMXtest\n\t"	/* Use standard CPUID instead */

		/* Extended CPUID supported (in theory), so get extended
		   features */
		"movl $0x80000001, %%eax\n\t"
		"cpuid\n\t"
		"testl $0x00800000, %%eax\n\t"	/* Test for MMX */
		"jz NotSupported5\n\t"		/* MMX not supported */
		"testl $0x01000000, %%eax\n\t"	/* Test for Ext'd MMX */
		"jnz EMMXSupported\n\t"
		"movl $1, %0\n\n\t"		/* MMX Supported */
		"jmp Return\n\n"
		"EMMXSupported:\n\t"
		"movl $3, %0\n\n\t"		/* EMMX and MMX Supported */
		"jmp Return\n\t"


		/* AMD Section */
		"AMD:\n\t"

		/* See if extended CPUID is supported */
		"movl $0x80000000, %%eax\n\t"
		"cpuid\n\t"
		"cmpl $0x80000000, %%eax\n\t"
		"jl MMXtest\n\t"	/* Use standard CPUID instead */

		/* Extended CPUID supported, so get extended features */
		"movl $0x80000001, %%eax\n\t"
		"cpuid\n\t"
		"testl $0x00800000, %%edx\n\t"	/* Test for MMX */
		"jz NotSupported6\n\t"		/* MMX not supported */
		"testl $0x80000000, %%edx\n\t"	/* Test for 3DNow! */
		"jnz ThreeDNowSupported\n\t"
		"movl $1, %0\n\n\t"		/* MMX Supported */
		"jmp Return\n\n"
		"ThreeDNowSupported:\n\t"
		"movl $5, %0\n\n\t"		/* 3DNow! and MMX Supported */
		"jmp Return\n\t"


		/* Intel Section */
		"Intel:\n\t"

		/* Check for MMX */
		"MMXtest:\n\t"
		"movl $1, %%eax\n\t"
		"cpuid\n\t"
		"testl $0x00800000, %%edx\n\t"	/* Test for MMX */
		"jz NotSupported7\n\t"		/* MMX Not supported */
		"movl $1, %0\n\n\t"		/* MMX Supported */
		"jmp Return\n\t"

		/* Nothing supported */
		"\nNotSupported1:\n\t"
		"#movl $101, %0\n\n\t"
		"\nNotSupported2:\n\t"
		"#movl $102, %0\n\n\t"
		"\nNotSupported3:\n\t"
		"#movl $103, %0\n\n\t"
		"\nNotSupported4:\n\t"
		"#movl $104, %0\n\n\t"
		"\nNotSupported5:\n\t"
		"#movl $105, %0\n\n\t"
		"\nNotSupported6:\n\t"
		"#movl $106, %0\n\n\t"
		"\nNotSupported7:\n\t"
		"#movl $107, %0\n\n\t"
		"movl $0, %0\n\n\t"

		"Return:\n\t"
		: "=g" (rval)
		: /* no input */
		: "eax", "ebx", "ecx", "edx"
	);

	/* Return */
	return(rval);
}

#endif

int idle()
{
	gdouble time;
	gulong ms;
	
	time=g_timer_elapsed(my_time, &ms);
	if (time > 1.5)
	{
		gtk_idle_remove(idle_tag);
		g_timer_destroy(my_time);
		destroy_about();		
		display_mastergui();		
	}
	
	return TRUE;
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
	fprintf(stderr, "%s - Copyright (C) 1999-2002 by Alexander König\n", VERSIONSTRING);
	fprintf(stderr, "terminatorX comes with ABSOLUTELY NO WARRANTY - for details read the license.\n");

	engine=new tX_engine();
	
#ifdef USE_3DNOW
	if (tx_mm_support()!=5) {
		printf("3DNow! not detected. Giving up.\n");
		return(1);
	} else printf("3DNow! accelerations available.\n");	
#endif
	
    gtk_init (&argc, &argv);
	gtk_set_locale ();
	parse_args(&argc, argv); 

	if (globals.show_nag) {	
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

//	gdk_input_init();

	gtk_main();

	store_globals();

	delete engine;
	
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
