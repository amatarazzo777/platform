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

vis.out: main.o uxdevice.o
	$(CC) -o vis.out main.o uxdevice.o -lpthread -lm -lX11-xcb -lX11 -lxcb -lxcb-image -lxcb-keysyms -lstdc++ $(LFLAGS) 
	
main.o: main.cpp uxdevice.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp -o main.o

uxdevice.o: uxdevice.cpp uxdevice.hpp
	$(CC) $(CFLAGS) $(INCLUDES) -c uxdevice.cpp -o uxdevice.o

clean:
	rm *.o *.out

