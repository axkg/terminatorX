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
 
    File: tX_engine.h
 
    Description: Header to tX_engine.cc
*/    

#ifndef _TX_ENGINE_H_
#define _TX_ENGINE_H_

extern int run_engine();
extern int stop_engine();

extern int grab_mouse(int);

#define ENG_ERR 4

#define ENG_RUNNING 0
#define ENG_INIT 1
#define ENG_STOPPED 2
#define ENG_FINISHED 3
#define ENG_ERR_XOPEN 4
#define ENG_ERR_XINPUT 5
#define ENG_ERR_DGA 6
#define ENG_ERR_SOUND 7
#define ENG_ERR_THREAD 8
#define ENG_ERR_GRABMOUSE 9
#define ENG_ERR_GRABKEY 10
#define ENG_ERR_BUSY 11

#define TX_ENG_OK 0
#define TX_ENG_ERR_TAPE 1
#define TX_ENG_ERR_DEVICE 2
#define TX_ENG_ERR_THREAD 3
#define TX_ENG_ERR_BUSY 4

extern int get_engine_status();
extern void set_engine_status(int );

extern int want_recording;

#endif
