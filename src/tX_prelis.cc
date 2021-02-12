/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2021  Alexander König
 
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

pid_t player_pid=0;

void prelis_stop()
{
	int status;
	
	if (player_pid)
	{
		kill(player_pid, SIGTERM);
		waitpid(player_pid, &status, 0);
		player_pid=0;
		usleep(200);
	}	
}

void prelis_start(char *name) {
	char *ext;
	pid_t temp;
#ifdef USE_MPG123_INPUT	
	int usempg123=0;
#endif
#ifdef USE_OGG123_INPUT
	int useogg123=0;
#endif
	char dev[PATH_MAX];
	char ogg123_dev[PATH_MAX]="dev:";
	char nm[PATH_MAX];
	
	if (!globals.prelis) return;
	
	if (player_pid) prelis_stop();	
	
	ext=strrchr(name, (int) '.');
	if (ext)
	{
		if (strlen(ext)>3)
		{
			ext++;
#ifdef USE_OGG123_INPUT
			if (!strncasecmp("ogg", ext, 2))
			useogg123=1;
			else
#endif
#ifdef USE_MPG123_INPUT			
			if (!strncasecmp("mp", ext, 2))			
			usempg123=1;
			else
#endif
#if defined(USE_SOX_INPUT) && defined(USE_MPG123_INPUT)
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
		strcpy(dev, globals.oss_device);
		strcat(ogg123_dev, dev);
		strcpy(nm, name);
#ifdef USE_OGG123_INPUT
		if (useogg123)
			execlp("ogg123", "ogg123", "-q", "-d", "oss", "-o", ogg123_dev, nm, NULL);
		else
#endif
#ifdef USE_MPG123_INPUT
		if (usempg123)
			execlp("mpg123", "mpg123", "-q", "-o","oss","-a", dev, nm, NULL);
		else
#endif
#ifdef USE_SOX_INPUT		
			execlp("play", "play", "-q", nm, NULL);
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
