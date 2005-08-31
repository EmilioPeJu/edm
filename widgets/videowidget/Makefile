EDM = $(EPICS_EXTENSIONS)/src/edm
LDFLAGS =  -L/usr/X11R6/lib -lXpm
CXXFLAGS += -I$(EDM)/lib -I$(EDM)/util -I$(EDM)/pvlib -I$(EPICS_BASE)/include -I/home/tools/applications/include -I. -Wno-deprecated

TwoDProfileMonitor.so: TwoDProfileMonitor.o widget.o
	g++ $(LDFLAGS) -shared -o TwoDProfileMonitor.so TwoDProfileMonitor.o widget.o

TwoDProfileMonitor.o: TwoDProfileMonitor.cc widget.h
	g++ $(CXXFLAGS) -c TwoDProfileMonitor.cc -g -O -Wall -ansi -pedantic

widget.o: widget.cc widget.h pixmap.xpm twoD.xpm
	g++ $(CXXFLAGS) -c widget.cc -g -O -Wall -ansi -pedantic -I/usr/X11R6/include

clean:
	rm -f TwoDProfileMonitor.so TwoDProfileMonitor.o widget.o
