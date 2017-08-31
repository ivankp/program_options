CXXFLAGS := -std=c++14 -Wall -O2 -g -Iinclude -fmax-errors=3
# -DPROGRAM_OPTIONS_DEBUG
# CXXFLAGS := -std=c++14 -Wall -O3 -Iinclude -flto -funroll-loops

NODEPS := clean
.PHONY: all clean

all: test/test

HH := $(wildcard include/*.hh) \
  $(wildcard include/program_options/*.hh) \
  $(wildcard include/program_options/fwd/*.hh)

test/program_options.o: src/program_options.cc $(HH)
	$(CXX) $(CXXFLAGS) -c $(filter %.cc,$^) -o $@

test/test.o: test/test.cc $(HH)
	$(CXX) $(CXXFLAGS) -c $(filter %.cc,$^) -o $@

test/test: %: %.o test/program_options.o $(HH)
	$(CXX) $(CXXFLAGS) $(filter %.o,$^) -o $@

clean:
	@rm -fv test/program_options.o test/test.o test/test
