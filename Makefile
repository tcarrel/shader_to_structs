#
#   Thomas Russel Carrel
#
#   Makefile
#
#   Define DEBUG, by uncommenting the last argument in CXXFLAGS, will enable
#  detailed debug out put.
#   
#   All of these compile lines are set up to duplicate all output (errors) from
#  g++ and copy it into a file.
#

CXX = g++
DEBUG = 0
CXXFLAGS = -g -std=c++11 -Wall -W -pedantic -Werror -D TIMED #-D DEBUG
MAIN = glsl_to_c
GCCERREXT = gccerr
ERROR_DIR = ./Errors
COPYOUTPUT = 2>&1 | tee $(ERROR_DIR)/$<.$(GCCERREXT)

$(MAIN): .entry_point.o $(ERROR_DIR)
	$(CXX) $(CXXFLAGS) .*.o -o $(MAIN) 2>&1 \
		| tee $(ERROR_DIR)/$(MAIN).$(GCCERREXT)

.entry_point.o: glsl_to_c.cpp $(ERROR_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(COPYOUTPUT)


$(ERROR_DIR):
	mkdir $@

clean:
	rm -f .*.o $(MAIN) a.out
	rm -r $(ERROR_DIR)

all: Main

Main: $(MAIN)
