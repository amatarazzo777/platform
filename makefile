CC=clang-9
#CC=g++
CFLAGS=-std=c++17 -Os 
INCLUDES=-I/projects/guidom `pkg-config --cflags cairo pango pangocairo  librsvg-2.0` -fexceptions

LFLAGS=`pkg-config --libs cairo pango pangocairo  librsvg-2.0` 

debug: CFLAGS += -g
debug: vis.out

release: LFLAGS += -s
release: vis.out

all: vis.out

vis.out: main.o uxdevice.o uxdisplaycontext.o uxdisplayunits.o uxpaint.o uxcairoimage.o
	$(CC) -o vis.out main.o uxdevice.o uxdisplaycontext.o uxdisplayunits.o uxpaint.o uxcairoimage.o -lpthread -lm -lX11-xcb -lX11 -lxcb -lxcb-image -lxcb-keysyms -lstdc++ $(LFLAGS) 
	
main.o: main.cpp uxdevice.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp -o main.o

uxdevice.o: uxdevice.cpp uxdevice.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c uxdevice.cpp -o uxdevice.o
	
uxdisplaycontext.o: uxdisplaycontext.cpp uxdisplaycontext.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c uxdisplaycontext.cpp -o uxdisplaycontext.o
	
uxdisplayunits.o: uxdisplayunits.cpp uxdisplayunits.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c uxdisplayunits.cpp -o uxdisplayunits.o
	
uxpaint.o: uxpaint.cpp uxpaint.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c uxpaint.cpp -o uxpaint.o
	
uxcairoimage.o: uxcairoimage.cpp uxcairoimage.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c uxcairoimage.cpp -o uxcairoimage.o

clean:
	rm *.o *.out

