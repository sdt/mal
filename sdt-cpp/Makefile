RLPATH=/usr/local/Cellar/readline/6.3.8

DEBUG=-ggdb
CFLAGS=-Wall $(DEBUG)
CXXFLAGS=-Wall $(DEBUG) -std=c++11 -I$(RLPATH)/include
LDFLAGS=$(DEBUG) -L$(RLPATH)/lib -lreadline -lhistory
CXX=g++
LD=g++

SOURCES=Environment.cpp Reader.cpp ReadLine.cpp String.cpp Types.cpp
OBJS=$(SOURCES:%.cpp=%.o)

MAINS=$(wildcard step*.cpp)
TARGETS=$(MAINS:%.cpp=%)

.PHONY:	all clean

.SUFFIXES: .cpp .o

all: $(TARGETS)

.deps: *.cpp *.h
	$(CXX) $(CXXFLAGS) -MM *.cpp > .deps

$(TARGETS): %: %.o $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(TARGETS)

-include .deps
