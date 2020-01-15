
wonderswan_CXX_SRCS   = main.cpp \
			source/audio.cpp \
			source/gpu.cpp \
			source/io.cpp \
			source/log.cpp \
			source/memory.cpp \
			source/emulate.cpp \
			source/rom.cpp \
			source/ws.cpp \
			source/nec/nec.cpp

OBJS       = $(wonderswan_CXX_SRCS:.cpp=.o)



all: wonderswan dumpinfo

CXX       = g++
CXXFLAGS  = -g -O2 `sdl-config --cflags` -Wall -Werror -std=c++98 -Wno-write-strings
#-arch i386
OPTIONS   =  -D_REENTRANT -I. -I../seal-hack/src/ -DVERSION=\"`git describe  --tags --long --dirty`\"

LIBRARY_PATH = -L../seal-hack/lib/SDL
LIBS      = -g $(LIBRARY_PATH) `sdl-config --libs` 
#-arch i386

ALLCFLAGS = $(CFLAGS) $(CEXTRA) $(OPTIONS) $(ALLFLAGS)
ALLCXXFLAGS=$(CXXFLAGS) $(CXXEXTRA) $(OPTIONS) $(ALLFLAGS)

CLEAN_FILES = wonderswan

.SUFFIXES: .cpp 

main.o: main.cpp

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

dumpinfo: dumpinfo.o
	$(CXX) $(LIBS) -o $@ $(<)

wonderswan: $(OBJS)
	$(CXX) $(LIBS) -o $@ $(OBJS) -laudio
