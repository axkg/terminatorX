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
 
    File: endian.c
 
    Description: swap byte order for big endian systems/audiohardware.
*/    


#include "tX_endian.h"

#ifdef WORDS_BIGENDIAN

void swap16(int16_t * val)
{
	int8_t temp;
	int8_t *p;
	
	p=(int8_t *) val;
	temp=*p;
	*p=*++p;
	*p=temp;
}

void swap32(int32_t * val)
{
	/*
		This one is very inefficient but it wont be
		called from performace critical areas so
		who cares...
       */
	int8_t temp;
	int8_t *p;
	
	p=(int8_t *) val;
	temp=p[0];
	p[0]=p[3];
	p[3]=temp;
	
	temp=p[1];
	p[1]=p[2];
	p[2]=temp;
}

void swapbuffer(int16_t *buffer, int samples)
{
	int i;
	int8_t temp;
	int8_t *p;
	int16_t *val;
	
	val=buffer;

	for (i=0; i<samples; i++)
	{
		p=(int8_t *) val;		
		temp=*p;
		*p=*++p;
		*p=temp;		
		val++;
	}
}


/* The following main() is just for testing */

#ifdef TEST_ENDIAN

#include <netinet/in.h>

int main(int argc, char **argv)
{
	int16_t t16=0x1234;
	int32_t t32=0x12345678;
	
	int16_t buffer[8]={0x1234, 0x5678, 0x9ABC, 0xDEF0, 10, 20, 30, 0};

	int i;
	
	printf("16: %4x\n", (int) t16);
	swap16(&t16);
	printf("16: %4x\n", (int) t16);
	swap16(&t16);
	printf("16: %4x\n", (int) t16);
	t16=htons(t16);
	printf("16: %4x\n", (int) t16);
	t16=htons(t16);
	printf("16: %4x\n", (int) t16);

	printf("32: %8x\n", (int) t32);
	swap32(&t32);
	printf("32: %8x\n", (int) t32);
	swap32(&t32);
	printf("32: %8x\n", (int) t32);
	t32=htonl(t32);
	printf("32: %8x\n", (int) t32);
	t32=htonl(t32);
	printf("32: %8x\n", (int) t32);
	
	printf("buf: ");
	for (i=0; i<8; i++) printf("%4hx ", buffer[i]);	
	swapbuffer(buffer, 8);
	printf("\nbuf: ");	
	for (i=0; i<8; i++) printf("%4hx ",  buffer[i]);
	swapbuffer(buffer, 8);
	printf("\nbuf: ");	
	for (i=0; i<8; i++) printf("%4hx ", buffer[i]);
	
	puts("\nDone.\n");
}
#endif

#endif
