CC=g++
 
.PHONY: all

all: webclient clean

webclient: webclient.o  
	$(CC) webclient.o -o webclient

webclient.o: webclient.cpp 
	$(CC) -c webclient.cpp

clean:
	rm *.o




