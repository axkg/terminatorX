/*
    licmak - include a program's license into c/c++-code
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

    File: licmak.c

    Description: Turns COPYING into a C-String. This EXTREMELY ugly and
    should be done with either perl, awk or sed. But I´m too lazy to learn
    either one of these...
*/

#include <stdio.h>
#include <string.h>

int main()
{
	FILE *in, *out;
	char buffer[1024];
	char outbuffer[2048];
	char c;
	int i,o;
	
	in = fopen ("COPYING", "r");
	if (!in) 
	{
		puts("COPYING not found.");
		return(1);
	}
	
	out = fopen ("license.cc", "w");
	if (!out)
	{
		puts("failed to open license.cc");
		return(1);
	}
	
	fprintf(out, "char license[]=\"\\\n");
	
	while (!feof(in))
	{
		fgets(buffer, 1024, in);
		if (!feof(in))
		{
		for (i=0, o=0; i<strlen(buffer); i++, o++)
		{
			c=buffer[i];

			if (c=='"')
			{
				outbuffer[o]='\\';
				o++;
			}
			
			outbuffer[o]=buffer[i];
		}
		outbuffer[o-1]=0;
		strcat(outbuffer, "\\n\\\n");
		/*
		outbuffer[o-1]='\\';
		outbuffer[o]='\n';
		outbuffer[o+1]=0;
		*/
		fprintf(out, "%s", outbuffer);
		}
	}
	fprintf(out, "\";\n");
	return(0);
}
