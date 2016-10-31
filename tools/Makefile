AntennaFlag = -D__ANTENNA__
Amc13Flag     = -D__AMC13__

ROOTVERSION := $(shell root-config --has-http)
Objs            = Tool.o SCurve.o Calibration.o Channel.o HybridTester.o CMTester.o  LatencyScan.o SignalScan.o PulseShape.o PedeNoise.o RegisterTester.o ShortFinder.o AntennaTester.o

RootLibraryDirs = /usr/local/lib/root
LibraryDirs = ../lib 
#ExternalObjects = -lPh1_Utils -lPh2_System -lPh2_Interface -lPh2_Description
ExternalObjects = 
IncludeDirs = /opt/cactus/include ../

##################################################
## check if Root has Http
##################################################
ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot= -lRHTTP -D__HTTP__
else
	ExtObjectsRoot= 
endif

##################################################
## check if Antenna Driver is installed
##################################################
ifneq ("$(wildcard $(ANTENNADIR))","")
	ANTENNAINSTALLED = yes
	IncludeDirs     +=  $(ANTENNADIR) 
	LibraryDirs += ../CMSPh2_AntennaDriver/lib
	ExternalObjects += -lusb -lPh2_Antenna $(AntennaFlag)
else
	ANTENNAINSTALLED = no
endif

##################################################
## check if AMC13 drivers are installed
##################################################
ifneq ("$(wildcard $(AMC13DIR))","")
	ExternalObjects += -lcactus_amc13_amc13 -lPh2_Amc13
	AMC13INSTALLED = yes
else
	AMC13INSTALLED = no
endif

##################################################
## check if the Ph2_USBInstDriver drivers are installed
##################################################
ifneq ("$(wildcard $(USBINSTDIR))","")
	LibraryDirs += $(USBINSTDIR)/lib $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/Utils 
	IncludeDirs += $(BOOST_INCLUDE) $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/Utils #$(USBINSTDIR)/Drivers 
	ExternalObjects += -lpthread -lboost_thread -lPh2_HMP4040 -lPh2_Ke2110 $(USBINSTFlag) -lPh2_Drivers -lPh2_USBUtils 
	USBINSTLIBINSTALLED = yes
	Objs += BiasSweep.o
else
	USBINSTLIBINSTALLED = no
endif

CC              = g++
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
	@echo 'Amc13 SW installed:' $(AMC13INSTALLED)
	@echo 'Antenna installed:' $(ANTENNAINSTALLED)
	@echo 'Ph2_USBInstDriver installed:' $(USBINSTLIBINSTALLED)
	@echo '****************************'
clean:
	rm -f *.o
