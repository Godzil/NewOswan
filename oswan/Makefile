
wonderswan_CXX_SRCS   = main.cpp \
			source/2xSaI.cpp \
			source/audio.cpp \
			source/gpu.cpp \
			source/io.cpp \
			source/log.cpp \
			source/memory.cpp \
			source/rom.cpp \
			source/ws.cpp \
			source/nec/nec.cpp

OBJS       = $(wonderswan_CXX_SRCS:.cpp=.o)



all: wonderswan

CXX       = g++
CXXFLAGS  = -g -O2 `sdl-config --cflags` -Wall -Werror -std=c++98 -Wno-write-strings
#-arch i386
OPTIONS   =  -D_REENTRANT -I. -I../seal-hack/src/

LIBRARY_PATH = -L../seal-hack/lib/SDL
LIBS      = -g $(LIBRARY_PATH) `sdl-config --libs` 
#-arch i386

ALLCFLAGS = $(CFLAGS) $(CEXTRA) $(OPTIONS) $(ALLFLAGS)
ALLCXXFLAGS=$(CXXFLAGS) $(CXXEXTRA) $(OPTIONS) $(ALLFLAGS)

CLEAN_FILES = wonderswan

.SUFFIXES: .cpp 

main.o: main.cpp source/filters/2xsai.h source/filters/doubled.h source/filters/filter_partA.h source/filters/filter_partB.h source/filters/filter_partC.h source/filters/filter_partD.h source/filters/filter_partE.h source/filters/halfscanlines.h source/filters/interpolate.h source/filters/scanlines.h source/filters/special.h source/filters/standard.h source/filters/super2xsai.h source/filters/supereagle.h source/SDLptc.h
	$(CXX) -c $(ALLCXXFLAGS) -o $@ $<

.c.o:
	$(CC) -c $(ALLCFLAGS) -o $@ $<

.cpp.o:
	$(CXX) -c $(ALLCXXFLAGS) -o $@ $<

.PHONY: all install uninstall clean distclean depend dummy

$(SUBDIRS:%=%/__clean__): dummy
	cd `dirname $@` && $(MAKE) clean

$(EXTRASUBDIRS:%=%/__clean__): dummy
	-cd `dirname $@` && $(RM) $(CLEAN_FILES)

clean:: $(SUBDIRS:%=%/__clean__) $(EXTRASUBDIRS:%=%/__clean__)
	$(RM) $(CLEAN_FILES) $(RC_SRCS:.rc=.res) $(OBJS) $(EXES:%.exe=%) $(EXES:%=%.so) $(EXES:%=%.spec.o) $(DLLS:%=%.so) $(DLLS:%=%.spec.o)

wonderswan: $(OBJS)
	$(CXX) $(LIBS) -o $@ $(OBJS) -laudio
