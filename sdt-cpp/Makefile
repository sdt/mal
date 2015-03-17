DEBUG=-ggdb
CFLAGS=-Wall $(DEBUG)
CXXFLAGS=-Wall $(DEBUG) -std=c++11
LDFLAGS=$(DEBUG)
CXX=g++
LD=g++

SOURCES=Reader.cpp String.cpp Types.cpp
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
