CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
SRCDIR   := src
TESTDIR  := tests
BENCHDIR := bench
BINDIR   := bin
RESDIR   := results

MAIN_SRC  := $(SRCDIR)/main.cpp
DEMO_SRC  := $(SRCDIR)/demo.cpp
TEST_SRC  := $(TESTDIR)/tests.cpp
BENCH_SRC := $(BENCHDIR)/bench.cpp

MAIN_BIN  := $(BINDIR)/arvores-balanceadas
DEMO_BIN  := $(BINDIR)/demo
TEST_BIN  := $(BINDIR)/tests
BENCH_BIN := $(BINDIR)/bench

HEADERS := $(SRCDIR)/avl.hpp $(SRCDIR)/rbtree.hpp $(SRCDIR)/bst.hpp \
           $(SRCDIR)/datasets.hpp

.PHONY: all run test bench demo clean

all: $(MAIN_BIN) $(DEMO_BIN) $(TEST_BIN) $(BENCH_BIN)

$(BINDIR):
	mkdir -p $(BINDIR)

$(RESDIR):
	mkdir -p $(RESDIR)

$(MAIN_BIN): $(MAIN_SRC) $(HEADERS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(DEMO_BIN): $(DEMO_SRC) $(HEADERS) | $(BINDIR) $(RESDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TEST_BIN): $(TEST_SRC) $(HEADERS) | $(BINDIR) $(RESDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BENCH_BIN): $(BENCH_SRC) $(HEADERS) | $(BINDIR) $(RESDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

run: $(MAIN_BIN)
	./$(MAIN_BIN)

test: $(TEST_BIN) | $(RESDIR)
	./$(TEST_BIN)

bench: $(BENCH_BIN) | $(RESDIR)
	./$(BENCH_BIN)

demo: $(DEMO_BIN) | $(RESDIR)
	./$(DEMO_BIN)

parity: $(BENCH_BIN) | $(RESDIR)
	./$(BENCH_BIN)

clean:
	rm -f $(BINDIR)/* $(RESDIR)/*
