
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

# CXX       = g++
CXXFLAGS  = -g -O2 `sdl-config --cflags` -Wall -std=c++98 -Wno-write-strings -Wno-unused-result
OPTIONS   =  -D_REENTRANT -I. -DVERSION=\"`git describe  --tags --long --dirty`\"

LIBRARY_PATH =
SDL_LIBS  = `sdl-config --libs`
LIBS      = -g $(LIBRARY_PATH) $(SDL_LIBS)

ALLCFLAGS = $(CFLAGS) $(CEXTRA) $(OPTIONS) $(ALLFLAGS)
ALLCXXFLAGS=$(CXXFLAGS) $(CXXEXTRA) $(OPTIONS) $(ALLFLAGS)

CLEAN_FILES = wonderswan

.SUFFIXES: .cpp 

main.o: main.c

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
	$(CXX) -o $@ $(OBJS) $(LIBS)
