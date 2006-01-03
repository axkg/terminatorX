/*
    terminatorX - realtime audio scratching software
    Copyright (C) 1999-2006  Alexander K�nig
 
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
 
    File: version.h
 
    Description: pretty dumb version management header...
*/    


#ifndef _H_VERSION
#define _H_VERSION

#ifdef HAVE_CONFIG_H
#include <config.h>
#define VERSIONSTRING PACKAGE " Release " VERSION
#else 
#define VERSION "3.82"
#define VERSIONSTRING "terminatorX Release 3.82"
#endif


#endif
