CC=g++
CFLAGS=-Wall -c -std=c++11 -g -O0
LDFLAGS=-Wall -o -std=c++11 -g -O0
SOURCES=lex.cpp table.cpp syntax.cpp executer.cpp interpreter.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=interpreter

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o excp.h.gch lex.h.gch table.h.gch 
