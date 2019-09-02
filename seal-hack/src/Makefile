# Makefile for Linux and SDL wrapper driver...

CC = gcc
AR = ar
RANLIB = ranlib
STRIP = strip
CFLAGS = -g -c -Wall -O2 -D__SDL__ `sdl-config --cflags` -Wall -Werror 
LFLAGS = -L../lib/SDL -laudio `sdl-config --libs`

OBJS = audio.o wavfile.o xmfile.o s3mfile.o modfile.o mtmfile.o \
	iofile.o modeng.o nondrv.o mixdrv.o sdldrv.o 
#_mixdrv.o

LIB = ../lib/SDL/libaudio.a 

all: mp $(LIB)
	@echo done.

mp : mp.o $(LIB)
	$(CC) -o mp mp.o $(LFLAGS)

$(LIB) : $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

.c.o:
	$(CC) $(CFLAGS) $<

.s.o .S.o :
	$(CC) $(CFLAGS) $< -D__LINUX__

clean:
	-rm -rf *.o
	-rm mp
