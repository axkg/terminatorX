/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999, 2000  Alexander K�nig
 
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
 
    File: tX_prelis.cc
 
    Description: This takes care of the "pre-listening" of audiofiles
    		 in the load dialog.

*/    

extern "C" {
#include <sys/types.h>
#include <sys/wait.h>
};

#include "tX_global.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define PL_SOX_STR "sox \"%s\" -t ossdsp -w -s \"%s\""
#define PL_MPG123_STR "mpg123 -a \"%s\" %s"

//FILE *player_handle=NULL;
pid_t player_pid=0;

void prelis_stop()
{
/*	if  (player_handle) pclose(player_handle);
	player_handle=NULL;*/
	int status;
	
//	printf("killing %i\n", player_pid);
	if (player_pid)
	{
		kill(player_pid, SIGTERM);
		waitpid(player_pid, &status, 0);
		player_pid=0;
		usleep(200);
	}	
}

void prelis_start(char *name)
{
	char buffer[PATH_MAX*2];
	char *ext;
	pid_t temp;
	int usempg123=0;
	int res;
	char dev[PATH_MAX];
	char nm[PATH_MAX];
	
	if (!globals.prelis) return;
	
	if (player_pid) prelis_stop();	
	
	ext=strrchr(name, (int) '.');
	if (ext)
	{
		if (strlen(ext)>3)
		{
			ext++;
#ifdef USE_MPG123_INPUT			
			if (!strncasecmp("mp", ext, 2))			
//			sprintf(buffer, PL_MPG123_STR, name, globals.audio_device);
			usempg123=1;
			else
#endif
#ifdef USE_SOX_INPUT			
//			sprintf(buffer, PL_SOX_STR, name, globals.audio_device);
			usempg123=0;
#else
			return;
#endif						
		}
	}
	
	temp=fork();

	if (temp==-1) /* error */
	{
		return;
	}
	else if (temp==0) /* CHILD */
	{	
		strcpy(dev, globals.audio_device);
		strcpy(nm, name);
#ifdef USE_MPG123_INPUT
		if (usempg123)
			res=execlp("mpg123", "mpg123", "-q", "-a", dev, nm, NULL);
		else
#endif
#ifdef USE_SOX_INPUT		
			res=execlp("sox", "sox", nm, "-t", "ossdsp", "-w", "-s", dev, NULL);
#else
			exit(0);
#endif			
			
		perror("Pre-Listen Error:");
		exit(0);
	}				
	else /* PARENT */
	{
		player_pid=temp;
	}
}
