EDM = $(EPICS_EXTENSIONS)/src/edm
CXXFLAGS += -I$(EDM)/lib -I$(EDM)/util -I$(EDM)/pvlib -I$(EPICS_BASE)/include -I/home/tools/applications/include -I. -Wno-deprecated -g

all: wheelswitch.so

wheelswitch.so : onekeypad.o wheelSwitch.o
	gcc $(LDFLAGS) -shared -static $^ -o $@

clean:
	rm -f *.o *.so
