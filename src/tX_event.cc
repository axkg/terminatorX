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
 
    File: tX_event.cc
 
    Description: This implements the sequencer events.
*/ 

#include <tX_event.h>

tX_event :: tX_event (FILE *input)
{
	unsigned int sp_persistence_id;

	fread((void *) &sp_persistence_id, sizeof(sp_persistence_id), 1, input);
	fread((void *) &timestamp, sizeof(timestamp), 1, input);
	fread((void *) &value, sizeof(value), 1, input);

	sp=tX_seqpar::get_sp_by_persistence_id(sp_persistence_id);
	if (!sp)
	{
		fprintf(stderr, "oops: couldn't resolve sp by persistence id %i.\n", sp_persistence_id);
	}
}

void tX_event :: store (FILE *output)
{
	int res=0;
	unsigned int persistence_id;
	
	persistence_id=sp->get_persistence_id();

	res+=fwrite((void *) &persistence_id, sizeof(persistence_id), 1, output)-1;	
	res+=fwrite((void *) &timestamp, sizeof(timestamp), 1, output)-1;
	res+=fwrite((void *) &value, sizeof(value), 1, output)-1;
}
