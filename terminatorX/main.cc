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
    
    File: main.cc
    
    Description: This contains the main() function. All the initializing
    happens here:
    - Commandline parsing
    - Loading of loop and scratch wavfiles
    - Creating of one instance of tX_Window and one of Virtual_Turntable
    - Running of the event loop in tX_Window	
    
    Changes:
    
    19 Mar 1999: Applied a patch by Andrew C. Bul+hac?k (eMail: acb@zikzak.net)
                 that fixes wavfile reading routine for the overreading bug.
		 
    20 Mar 1999: Big endian support.
    
    23 Mar 1999: display of new keys (<-, ->)
*/
#include <stdio.h>
#include "wav_file.h"
#include "turntable.h"
#include "tX.h"
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include "endian.h"
#include "tX_types.h"
#include "version.h"

#define min(a,b) ((a) < (b) ? (a) : (b))

Virtual_TurnTable *myvtt;
tX_Window *mywin;

#include "license.cc"

/* glob_parm: struct for global parameters */

typedef struct glob_parm
{
	float mouse_speed;		// -m, --mouse
	char s_wav[256];		// no_opt
	char m_wav[256];		// -l, --loop
	char prefix[128];		// -p, --prefix
	char dev_name[128];		// -d, --device
	int buff_no;			// -n, --buffno
	int buff_size;			// -b, --buffsize
	int rec_size;			// -t, --rectime
	int stop_sense_cycles;		// -c, --sensecycles
	int verbose;
	int warp_border;		// -w, --warp
	int width;			// -x, --width
	int height;			// -y, --height
	float vtt_default_speed;	// -s, --speed
} glob_parm;

glob_parm parms;

/* set_defaults(): sets defaults in parms struct */

void set_defaults()
{
	parms.mouse_speed=0.5;
	strcpy(parms.s_wav, "");
	strcpy(parms.m_wav, "");
	strcpy(parms.prefix, "tX_scratch");
	strcpy(parms.dev_name, "/dev/dsp");
	
	parms.buff_no=2;
	parms.buff_size=9;
	
	parms.rec_size=512000;
	parms.stop_sense_cycles=3;
	parms.vtt_default_speed=1.0;
	parms.verbose=0;
	parms.warp_border=200;
	parms.height=600;
	parms.width=1000;
}

/* usage(): displays parameters */

void usage()
{
	puts("usage:   terminatorX [options] wavfile\n");
	puts("options:");
	puts("[short] [long]          [type]      [default]    [description]");
	puts("-m      --mouse         float       0.5          mouse sensitivity");
	puts("-l      --loop          string      -            loop wavfile");
	puts("-p      --prefix        string      tX_scratch   recfile prefix");
	puts("-d      --device        string      /dev/dsp     audio output device");
	puts("-n      --buffno        integer     2            number of audio buffers");
	puts("-b      --buffsize      integer     9            size of audio buffers (2^size)");
	puts("-r      --recbuff       integer     500          size of record buffer in KB");
	puts("-c      --sensecycles   integer     3            cycles to mouse stop sense");
	puts("-s      --speed         float       1.0          speed of turntable motor");
	puts("-w      --warp          integer     200          border for mouse warp");
	puts("-x      --width         integer     1000         window width");
	puts("-y      --height        integer     600          window height");
	puts("-v      --verbose                                verbose output");
	puts("-k      --keys                                   display window keys");
	puts("-h      --help                                   this page");
	puts("-g      --license                                display license (GPL V2)");
}

/* keys(): display available keys in Window */

void keys()
{
	puts("Available keys in terminatorX:\n");
	puts("[key]     [description]");
	puts("<RETURN>  start/stop playback");
	puts("<SPACE>   activate mouse when playback active (to scratch)");
	puts("            pressed:  hand on turntable");
	puts("            released: hand off turntable");
	puts("            symbol: lower right red rectangle");
	puts("<n>       switch to \"normal\" scratch mode (no recording)");
	puts("            symbol: empty circle. mouse active.");
	puts("<r>       switch to record mode. playback stops when record buffer is full.");
	puts("            symbol: full circle. mouse active.");
	puts("<p>       switch to playback mode. plays the recorded scratch.");
	puts("            symbol: arrow. mouse inactive");
	puts("<m>       toggle mixing of loop on/off");
#ifndef USE_OLD_MIX
	puts("<LEFT>    increase loop volume, decrease scratch volume");
	puts("<RIGHT>   increase scratch volume, decrease loop volume");
#endif	
	puts("<s>       save RAW scratch to wavfile");
	puts("<x>       save MIXED scratch to wavfile");
	puts("<q>       exit terminatorX\n");
	puts("Saving, mode changes and mix toggles are only available when playback has stopped.");
}

/* parse_cd: parses the commandline using getopt

   this could use some more error checking.
*/

void parse_cmd(int argc, char **argv)
{  
	int c;
	
	/* make -Wall shut up */	
	// int digit_optind = 0;

	set_defaults();

	while (1)
	{
		/* make -Wall shut up */
		// int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] =
		{
			{"mouse", 1, 0, 'm'},
			{"loop", 1, 0, 'l'},
			{"prefix", 1, 0, 'p'},
			{"device", 1, 0, 'd'},
			{"buffno", 1, 0, 'n'},
			{"buffsize", 1, 0, 'b'},
			{"recbuff", 1, 0, 'r'},
			{"sensecycles", 1, 0, 'c'},
			{"speed", 1, 0, 's'},			
			{"verbose", 1, 0, 'v'},			
			{"help", 0, 0, 'h'},
			{"warp", 1, 0, 'w'},
			{"height", 1, 0, 'y'},
			{"width", 1, 0, 'x'},
			{"keys", 0, 0, 'k'},
			{"license", 0, 0, 'g'},
			{0, 0, 0, 0}
		};

		c = getopt_long (argc, argv, "m:l:p:d:n:b:r:c:s:vkhgw:x:y:", long_options, &option_index);
		
		if (c == -1)
		break;

		switch (c)
		{
			case 'm':	
				sscanf(optarg, "%f", &parms.mouse_speed);
			break;
	
			case 'l':
				strcpy(parms.m_wav, optarg);
			break;
		
			case 'p':
				strcpy(parms.prefix, optarg);
			break;
	
			case 'd':
				strcpy(parms.dev_name, optarg);
			break;
	
			case 'n':
				sscanf(optarg, "%i", &parms.buff_no);
			break;
	
			case 'b':
				sscanf(optarg, "%i", &parms.buff_size);
			break;
	
			case 'r':
				sscanf(optarg, "%i", &parms.rec_size);
				parms.rec_size*=1024;
			break;
	
			case 'c':
				sscanf(optarg, "%i", &parms.stop_sense_cycles);
			break;
		
			case 's':
				sscanf(optarg, "%f", &parms.vtt_default_speed);
			break;
	
			case 'v':
				parms.verbose=1;
			break;
	
			case 'h':
				usage();
				exit(0);
			break;
			
			case 'k':
				keys();
				exit(0);
			break;
	
			case 'w':
				sscanf(optarg, "%i", &parms.warp_border);
			break;
	
			case 'x':		
				sscanf(optarg, "%i", &parms.width);
			break;
			
			case 'y':
				sscanf(optarg, "%i", &parms.height);
			break;
			
			case 'g':
				puts(license);
				exit(0);
			break;
	
			case '?':
			  break;
	
			default:
			  printf ("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (optind != argc-1)
    	{
    		puts("Error: You must specify one WAV-file you want to scratch on.\n");
		usage();
		exit(1);
	}
   	else
	{
  		strcpy(parms.s_wav, argv[optind]);
	}
	

	if (parms.verbose)
	{	
		printf("Mouse speed	: %f\n", parms.mouse_speed);
		printf("Wavefile	: %s\n", parms.s_wav);
		
		if (strlen(parms.m_wav)) 
		printf("Loop wavefile	: %s\n", parms.m_wav);
		else puts("No loop file.");
		
		printf("Rec-prefix	: %s\n", parms.prefix);
		printf("Device		: %s\n", parms.dev_name);
		printf("Audio-Buffers	: %i\n", parms.buff_no);	
		printf("Size		: %i (%i Bytes)\n", parms.buff_size, 1 << parms.buff_size);
		printf("Recbuffsize	: %i (%f Seconds)\n", parms.rec_size, ((float) parms.rec_size) / 88200.0);
		printf("Stopsensecycles : %i\n", parms.stop_sense_cycles);
		printf("VTT speed	: %f\n", parms.vtt_default_speed);
		printf("Warp border	: %i\n", parms.warp_border);
		printf("Window height	: %i\n", parms.height);
		printf("Window width	: %i\n", parms.width);
	}
}

/* load_wav(): load a wavfile with filename name.
               writes size of file to *size, and
	       returns a pointer to the loaded wave-data.
*/

int16_t *load_wav(char *name, unsigned int *size)
{
	wav_sig wav_in;
	int16_t *data;
	int16_t *p;
	ssize_t allbytes=0;
	ssize_t bytes=0;
	int i;
	
	if (!init_wav_read(name, &wav_in))
	{
#ifdef USE_CONSOLE
		printf("[load_wav] Error. Couldn't open \"%s\".\n", name);
#endif	
	}

#ifdef USE_CONSOLE
	printf("Loading: %s\n", name);
	if (parms.verbose) printf("File: %i Bytes Data, %i Bit Depth, %i Hz Samplerate.\n", wav_in.len, wav_in.depth, wav_in.srate);	
#endif	
	
	if (wav_in.depth != 16)
	{
#ifdef USE_CONSOLE
		puts("[load_wav] Error: Wave-File is not 16 Bit. Fatal. Giving up.");		
#endif		
		exit(1);
	}

	if (wav_in.chans != 1)
	{
#ifdef USE_CONSOLE	
		puts("[load_wav] Error: Wave-File is not Mono. Fatal. Giving up.");
#endif
		exit(2);		
	}

#ifdef USE_CONSOLE	
	if (wav_in.srate != 44100) 
	{
		puts("[load_wav] Warning: Wave-File was not recorded at 44.100 Hz!");
	}
	if (wav_in.blkalign != 2)
	{
		printf("[load_wav] Warning: Unexpected block alignment: %i.\n", wav_in.blkalign);
	}
#endif

	*size=wav_in.len;
	data = (int16_t *) malloc (*size);
	
	if (!data)
	{
#ifdef USE_CONSOLE
		puts("[load_wav] Error: Failed to allocate sample memory. Fatal. Giving up.");
		exit(3);
#endif	
	}

	p=data;


	while (wav_in.len>allbytes)
	{
		bytes = read(wav_in.handle, p, min(1024, wav_in.len-allbytes));
#ifdef BIG_ENDIAN_MACHINE
		swapbuffer(p, bytes/sizeof(int16_t));
#endif		
		
		if (bytes<=0)
		{
#ifdef USE_CONSOLE
		       puts("[load_wav] Error: Failed to read Wave-Data (Corrupt?). Fatal. Giving up.");
                       if(bytes<0) perror("terminatorX");
#endif		
			exit(4);
		}
				
		allbytes+=bytes;
		
		for (i=0; i<bytes; i+=2) p++;
	}
	
	close(wav_in.handle);
	
#ifdef USE_CONSOLE
	if (parms.verbose) printf("Read: %i Bytes of Wave-Data.\n", allbytes);
#endif	
	return(data);
}

/* main(): */

int main(int argc, char **argv)
{
	
	int16_t *data_in;
	unsigned int size_in;
	
	int16_t *beat_data;
	unsigned int beat_size;
	
	int16_t *recordbuffer;
		
	TT_init init_data;
	
	printf("%s, Copyright (C) 1999 Alexander König, alkoit00@fht-esslingen.de\n", VERSIONSTRING);
	puts("terminatorX comes with ABSOLUTELY NO WARRANTY; for details use option --license");
	puts("This is free software, and you are welcome to redistribute it");
	puts("under certain conditions; see option --license\n"); 
	
	parse_cmd(argc, argv);
	
	data_in = load_wav(parms.s_wav, &size_in);
	
	if (strlen(parms.m_wav))
	{
		beat_data = load_wav(parms.m_wav, &beat_size);
	}
	else
	{
		beat_data = NULL; beat_size=0;
	}
	
#ifdef USE_CONSOLE
	if (parms.verbose) printf("Allocating Recorder buffer of %i Bytes.\n", parms.rec_size);
#endif
	recordbuffer=(int16_t *) malloc(parms.rec_size);
	
	if (!recordbuffer)
	{
#ifdef USE_CONSOLE
			puts("[main] Error: Allocate Recorder buffer. Fatal. Giving up.");
#endif						
			exit(1);
	}
	
	init_data.speed=(f_prec) parms.vtt_default_speed;
	init_data.default_speed=(f_prec) parms.vtt_default_speed;
	init_data.buff_cfg=(parms.buff_no<<16) | parms.buff_size;
	init_data.bsize=1<<parms.buff_size;
	init_data.verbose=parms.verbose;
	init_data.playback_data=data_in;
	init_data.playback_size=size_in;
	init_data.record_data=recordbuffer;
	init_data.record_size=parms.rec_size;
	init_data.mix_data=beat_data;
	init_data.mix_size=beat_size;	
	
	strcpy(init_data.file_name, parms.prefix);
	strcpy(init_data.devicename, parms.dev_name);
	
	myvtt=new Virtual_TurnTable(&init_data);

	mywin=new tX_Window(myvtt, parms.width, parms.height, parms.warp_border, parms.stop_sense_cycles, parms.mouse_speed, parms.verbose, parms.s_wav, parms.m_wav);
	myvtt->set_window(mywin);
	
	mywin->prepare_data_display(data_in, size_in);

	mywin->loop();
	
	free(recordbuffer);
	free(data_in);
	
	puts("Have a nice life.");
}
