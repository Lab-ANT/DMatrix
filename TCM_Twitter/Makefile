EXEC += tcm
all: $(EXEC)

CFLAGS = -Wall -std=c++11 -O3
HEADER += hash.h datatypes.hpp util.h adaptor.hpp RrConfig.h
SRC += hash.c adaptor.cpp RrConfig.cpp
SKETCHHEADER += sketch.hpp
SKETCHSRC += sketch.cpp
LIBS= -lpcap 

tcm: tcm.cpp $(SRC) $(HEADER) $(SKETCHHEADER)
	g++ $(CFLAGS) $(INCLUDES) -o $@ $< $(SRC) $(SKETCHSRC) $(LIBS)

clean:
	rm -rf $(EXEC)
	rm -rf *log*
	rm -rf *out*
