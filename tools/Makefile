ANTENNADIR=../CMSPh2_AntennaDriver
AntennaFlag = -D__ANTENNA__

ROOTVERSION := $(shell root-config --has-http)

RootLibraryDirs = /usr/local/lib/root
LibraryDirs = ../lib 
ExternalObjects = 
IncludeDirs = /opt/cactus/include ../
ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot= -lRHTTP -D__HTTP__
else
	ExtObjectsRoot= 
endif

ifneq ("$(wildcard $(ANTENNADIR))","")
	ANTENNAINSTALLED = yes
	IncludeDirs     +=  $(ANTENNADIR) 
	LibraryDirs += ../CMSPh2_AntennaDriver/lib
	ExternalObjects += -lusb -lPh2_Antenna $(AntennaFlag)
else
	ANTENNAINSTALLED = no
endif

Objs            = Tool.o SCurve.o Calibration.o OldCalibration.o Channel.o HybridTester.o CMTester.o  Commissioning.o PulseShape.o PedeNoise.o
CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC `root-config --cflags --evelibs` 


#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags		=


LibraryPaths = $(LibraryDirs:%=-L%) 
IncludePaths = $(IncludeDirs:%=-I%)
RootLibraryPaths = $(RootLibraryDirs:%=-L%)

.PHONY: print clean

%.o: %.cc %.h
	$(CXX) -std=c++0x  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) $(LibraryPaths) $(ExternalObjects) $(RootLibraryPaths) $(ExtObjectsRoot) -c -o $@ $<

all: print $(Objs) ../HWDescription/Definition.h
	$(CC) -std=c++0x -shared -o libPh2_Tools.so $(Objs) -pthread
	mv libPh2_Tools.so ../lib

print:
	@echo '****************************'
	@echo 'Building tools with:'
	@echo 'Root Has Http:' $(ROOTVERSION)
	@echo 'Antenna installed:' $(ANTENNAINSTALLED)
	@echo '****************************'
clean:
	rm -f *.o
