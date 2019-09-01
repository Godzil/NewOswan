# Makefile for MacOS X using SDL

CC = clang
AR = ar
RANLIB = ranlib
STRIP = strip
CFLAGS = -c -Wall -O2 -fomit-frame-pointer -D__OSX__ `allegro-config --cflags` -arch i386
LFLAGS = -L../lib/MacOSX -laudio `allegro-config --libs`  -arch i386

OBJS = audio.o wavfile.o xmfile.o s3mfile.o modfile.o mtmfile.o \
	iofile.o modeng.o nondrv.o mixdrv.o allegdrv.o
#osxdrv.c
# Not supported on x86_64 platforms
#_mixdrv.o

LIB = ../lib/MacOSX/libaudio.a

all : mp $(LIB) clean
	@echo done.

mp : mp.o $(LIB)
	$(CC) -o mp mp.o $(LFLAGS)
	$(STRIP) mp

$(LIB) : $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

.c.o .s.o .S.o :
	@echo CC $<
	@$(CC) $(CFLAGS) $<

clean :
	@for f in *.o; do rm -f $$f; done
