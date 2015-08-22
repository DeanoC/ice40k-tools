
CC=clang
CXX=clang++
CFLAGS=-c -Wall -std=c++11
LDFLAGS=
LDLIBS=

SRCS=ice40ktool.cpp docopt.cpp
OBJECTS=$(SRCS:.cpp=.o)
EXECUTABLE=ice40ktool

all: $(SRCS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
	rm $(EXECUTABLE)


include .depend