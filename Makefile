CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
LDFLAGS  ?=
PREFIX   ?= /usr/local

SRC = src/main.cpp src/eval.cpp src/plot.cpp src/handbook.cpp src/explain.cpp src/repl.cpp
OBJ = $(SRC:.cpp=.o)

.PHONY: all clean test run

all: calcpedia

calcpedia: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LDFLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) calcpedia calcpedia.exe

test: calcpedia
	./calcpedia --self-test

run: calcpedia
	./calcpedia
