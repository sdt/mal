CFLAGS=-Wall -g
CXXFLAGS=-Wall -g -std=c++11
LDFLAGS=-g
LD=g++

SOURCES=Tokeniser.cpp
OBJS=$(SOURCES:%.cpp=%.o)

.PHONY:	all clean

.SUFFIXES: .cpp .o

all: step0_repl

.deps: *.cpp *.h
	$(CXX) -MM *.cpp > .deps

step0_repl: step0_repl.o
	g++ $(LDFLAGS) $< -o $@

.cpp.o:
	g++ $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o step0_repl

-include .deps
