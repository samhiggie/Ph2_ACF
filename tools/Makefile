Objs                    = Tool.o Calibration.o FastCalibration.o Channel.o HybridTester.o CMTester.o Commissioning.o PulseShape.o PedeNoise.o

CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC `root-config --cflags --evelibs` -Wcpp
ROOTVERSION := $(shell root-config --has-http)

HttpFlag = -D__HTTP__

#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags		=

RootLibraryDirs = /usr/local/lib/root
RootLibraryPaths = $(RootLibraryDirs:%=-L%)

ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot= $(RootLibraryPaths) -lRHTTP $(HttpFlag)
else
	ExtObjectsRoot= $(RootLibraryPaths)
endif

IncludeDirs     =  /opt/cactus/include ../

IncludePaths            = $(IncludeDirs:%=-I%)

%.o: %.cc %.h
	$(CXX) -std=c++11  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) $(ExtObjectsRoot) -c -o $@ $<

all: $(Objs) ../HWDescription/Definition.h
	$(CC) -std=c++11 -shared -o libPh2_Tools.so $(Objs) -pthread
	mv libPh2_Tools.so ../lib

clean:
	rm -f *.o
