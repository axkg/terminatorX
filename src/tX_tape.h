/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2011  Alexander K�nig
 
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
 
    File: tX_tape.h
 
    Description: Header to tX_tape.cc
*/    

#ifndef _h_tx_tapedeck_
#define _h_tx_tapedeck_ 1

#include "wav_file.h"
#include "tX_types.h"

class tx_tapedeck
{
	wav_sig file;
	int is_recording;
	unsigned int written_bytes;
	int blocksize;
	
	public:
	int start_record(char *name, int bs, int samplerate);
	void stop_record();
	void eat(int16_t *);
	
	tx_tapedeck();
};

#endif
