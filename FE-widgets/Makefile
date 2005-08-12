EDM = $(EPICS_EXTENSIONS)/src/edm
LDFLAGS =  -L/usr/X11R6/lib -lXpm
CXXFLAGS += -I$(EDM)/lib -I$(EDM)/util -I$(EDM)/pvlib -I$(EPICS_BASE)/include -I/home/tools/applications/include -I. -Wno-deprecated

ExtendedRelatedDisplay.so: ExtendedRelatedDisplay.o
	g++ $(LDFLAGS) -shared -o ExtendedRelatedDisplay.so ExtendedRelatedDisplay.o

ExtendedRelatedDisplay.o: ExtendedRelatedDisplay.cc
	g++ $(CXXFLAGS) -c ExtendedRelatedDisplay.cc -g -O -Wall -ansi -pedantic


clean:
	rm -f ExtendedRelatedDisplay.so ExtendedRelatedDisplay.o
