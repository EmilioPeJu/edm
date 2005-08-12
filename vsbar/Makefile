EDM = $(EPICS_EXTENSIONS)/src/edm
LDFLAGS =  -L/usr/X11R6/lib -lXpm
CXXFLAGS += -I$(EDM)/lib -I$(EDM)/util -I$(EDM)/pvlib -I$(EPICS_BASE)/include -I/home/tools/applications/include -I. -Wno-deprecated

vsbar.so: vsbar.o
	g++ $(LDFLAGS) -shared -o vsbar.so vsbar.o

vsbar.o: vsbar.cc vsbar.h vsbar.str
	g++ $(CXXFLAGS) -c vsbar.cc -g -O -Wall -ansi -pedantic

clean:
	rm -f vsbar.so vsbar.o vsbar.o
