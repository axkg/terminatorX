/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2020  Alexander König
 
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
 
    File: tX_tape.cc
 
    Description: This implements a "harddisk recording tapedeck" - simply dumps
    		 the generated audio into a plain mono wav-file.
*/    

#include "tX_tape.h"
#include <string.h>

tx_tapedeck :: tx_tapedeck()
{
	is_recording=0;
	written_bytes=0;
}

int tx_tapedeck :: start_record (char *name, int bs, int samplerate)
{
	if (is_recording) return 1;
		
	strcpy(file.name, name);
	file.srate=samplerate;
	file.chans=2;
	file.depth=16;
	file.bps=samplerate*2;
	file.blkalign=2;
	file.len=0;
	file.sofar=0;

	blocksize=bs * sizeof(int16_t) * 2;

	written_bytes=0;	

	if (!open_wav_rec(&file)) {
		return 1;
	}
	
	is_recording=1;
	
	return 0;
}

void tx_tapedeck :: eat(int16_t *buffer)
{
	written_bytes+=fwrite((void *) buffer, blocksize, 1, file.handle);
}

void tx_tapedeck :: stop_record()
{
	if (!is_recording) return;
	
	written_bytes*=blocksize;
	
	file.len=written_bytes;
	file.sofar=written_bytes;

	rewrite_head(&file);
	fclose(file.handle);
	
	is_recording=0;
}
