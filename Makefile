#terminatorX Makefile
OBJECTS=turntable.o wav_read.o wav_write.o main.o tX.o endian.o

#Correct the X11-Path if required
INCLUDES=-I/usr/X11R6/include/
LIBS=-L/usr/X11R6/lib -lX11 -lm

#The tested big endian machines had problems
#with opimized code if you own one of these
#better leave this value the way it is -
#On X86-machines it seems save to use:
#OPT=-O2
OPT=

#Set the compiletime flags. Options:
#-DUSE_CONSOLE        Output information to the console (should be defined)
#-DUSE_STOPSENSE      If this is not defined terminatorX wil not
#                     sense when mouse movement has stopped.
#-DKEEP_DEV_OPEN      Audiodevice will be kept open until
#                     terminatorX exits
#-DBIG_ENDIAN_MACHINE Use this on big endian machines (LinuxPPC, SUN, etc...)
#-DBIG_ENDIAN_AUDIO   Use this with big endian AUDIO HARDWARE (rare ?)
#                     This IS NOT RELATED to BIG_ENDIAN_MACHINE 
#-DUSE_X86_TYPES      Use this only IF you don't have <sys/types.h> AND a X86 machine
#-DHANDLE_STOP        If enabled generated audio data is "correct"
#                     but introduces clicks
#-DUSE_OLD_MIX        re-enables old, bad, but proabably faster mixing routine
CFLAGS=-DUSE_CONSOLE -DUSE_STOPSENSE -DHANDLE_STOP -Wall

#Set the directory you want to install terminatorX in
INSTALLDIR=/usr/local/bin

#Set your compiler
CC=egcs

all:	terminatorX

install:	terminatorX
		cp terminatorX $(INSTALLDIR)

clean:	Makefile
	rm -f $(OBJECTS) terminatorX core licmak licmak.o license.cc

licmak.o:	licmak.c
		$(CC) -o licmak.o -c licmak.c

licmak:		licmak.o
		$(CC) licmak.o -o licmak

license.cc:	COPYING licmak
		./licmak

tX.o:		tX.cc
		$(CC) $(OPT) $(CFLAGS) $(INCLUDES) -o tX.o -c tX.cc	
	
main.o:		main.cc license.cc
		$(CC) $(OPT) $(CFLAGS) $(INCLUDES) -o main.o -c main.cc	
	
wav_read.o:	wav_read.cc
		$(CC) $(OPT) $(CFLAGS) $(INCLUDES) -o wav_read.o -c wav_read.cc

wav_write.o:	wav_write.cc
		$(CC) $(OPT) $(CFLAGS) $(INCLUDES) -o wav_write.o -c wav_write.cc
	
turntable.o:	turntable.h turntable.cc
		$(CC) $(OPT) $(CFLAGS) $(INCLUDES) -o turntable.o -c turntable.cc

endian.o:	endian.h endian.cc
		$(CC) $(OPT) $(CFLAGS) $(INCLUDES) -o endian.o -c endian.cc
			
terminatorX:	$(OBJECTS)
		$(CC) $(LIBS) $(OBJECTS) -o terminatorX
		
