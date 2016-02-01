ANTENNADIR=../CMSPh2_AntennaDriver
AntennaFlag = -D__ANTENNA__
HttpFlag = -D__HTTP__

ROOTVERSION := $(shell root-config --has-http)

ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot= $(RootLibraryPaths) -lRHTTP $(HttpFlag)
else
	ExtObjectsRoot= $(RootLibraryPaths)
endif

ifneq ("$(wildcard $(ANTENNADIR))","")
        ANTENNAINSTALLED = yes
	IncludeDirs     =  /opt/cactus/include ../ $(ANTENNADIR) 
	ExternalObjects = -L../lib -L../CMSPh2_AntennaDriver/lib -lusb -lPh2_Antenna $(AntennaFlag)
else
        ANTENNAINSTALLED = no
	IncludeDirs     =  /opt/cactus/include ../ 
	ExternalObjects = 
endif

Objs            = Tool.o SCurve.o Calibration.o OldCalibration.o Channel.o HybridTester.o CMTester.o  Commissioning.o PulseShape.o PedeNoise.o
CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC `root-config --cflags --evelibs` -Wcpp


#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags		=

RootLibraryDirs = /usr/local/lib/root
RootLibraryPaths = $(RootLibraryDirs:%=-L%)

IncludePaths            = $(IncludeDirs:%=-I%)

.PHONY: print clean

%.o: %.cc %.h
	$(CXX) -std=c++11  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) $(ExternalObjects) $(ExtObjectsRoot) -c -o $@ $<

all: print $(Objs) ../HWDescription/Definition.h
	$(CC) -std=c++11 -shared -o libPh2_Tools.so $(Objs) -pthread
	mv libPh2_Tools.so ../lib

print:
	@echo '****************************'
	@echo 'Building tools with:'
	@echo 'Root Has Http:' $(ROOTVERSION)
	@echo 'Antenna installed:' $(ANTENNAINSTALLED)
	@echo '****************************'
clean:
	rm -f *.o
