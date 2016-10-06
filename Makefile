CC=g++
CFLAGS=-Wall -c -std=c++11 -g -O0
LDFLAGS=-Wall -o -std=c++11 -g -O0
SOURCES=cgiprog.cpp client.cpp cl_sock.cpp filestat.cpp Server_v5.cpp serv_sock.cpp sock.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=Server_v5

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o *.gch
