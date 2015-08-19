Objs                    = Exception.o Utilities.o Event.o Data.o argvparser.o FileHandler.o
CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC -Wcpp
#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags		=

IncludeDirs     =  /opt/cactus/include ../

IncludePaths            = $(IncludeDirs:%=-I%)

%.o: %.cc %.h 
	$(CXX) -std=c++11  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

all: $(Objs) ../HWDescription/Definition.h
	$(CC) -std=c++11 -shared -o libPh2_Utils.so $(Objs) -pthread
	mv libPh2_Utils.so ../lib

clean:
	rm -f *.o
