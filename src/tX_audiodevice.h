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
 
    File: tX_audiodevice.h
 
    Description: Header to tX_mastergui.cc
*/    

#ifndef _h_tx_audiodevice
#define _h_tx_audiodevice 1

#include "tX_types.h"
#include "tX_global.h"
#include "pthread.h"

#include <sys/time.h>

#define NON_RT_BUFF 12


class audiodevice
{
	friend void* writer_thread(void *parm);
	int fd;
	int blocksize;
	int samples;
	clock_t lastclock;
	clock_t rendercl;
	clock_t writecl;
	
	public:
	int dev_open(int);
	int dev_close();
	
	int getblocksize();
	
	int eat(int16_t*);
	
	audiodevice();

#ifdef USE_WRITER_THREAD	
	pthread_mutex_t stop_mutex;
	pthread_mutex_t write_mutex;
	pthread_mutex_t buffer_ready_mutex;
	pthread_mutex_t buffer_read_mutex;
	int16_t *current_buffer;
	pthread_t writer;
#endif	
};

#endif
