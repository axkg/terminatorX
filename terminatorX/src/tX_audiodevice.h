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
 
    File: tX_audiodevice.h
 
    Description: Header to tX_mastergui.cc
*/    

#ifndef _h_tx_audiodevice
#define _h_tx_audiodevice 1

#include "tX_types.h"
#include "tX_global.h"
#include "pthread.h"
#include <config.h>

#include <sys/time.h>

#define NON_RT_BUFF 12

class tX_audiodevice
{
	protected:
	int samples_per_buffer;
	void init();
	
	public:
	void set_latency_near(int milliseconds);
	int get_latency(); /* call only valid *after* open() */
	
	void set_buffersize_near(int samples);
	int get_buffersize(); /* call only valid *after* open() */
	
	virtual int open();
	virtual int close();
		
	virtual void play(int16_t*); /* play blocked */
};


#ifdef USE_OSS

class tX_audiodevice_oss : public tX_audiodevice
{
	int fd;
	int blocksize;	

	public:
	virtual int open();
	virtual int close();
		
	virtual void play(int16_t*); /* play blocked */
	
	tX_audiodevice_oss();
};

#endif


#ifdef USE_ALSA

class tX_audiodevice_alsa : public tX_audiodevice
{	
	public:
	virtual int open();
	virtual int close();
		
	virtual void play(int16_t*); /* play blocked */
	
	tX_audiodevice_alsa();
};

#endif

#endif
