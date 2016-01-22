ANTENNADIR=../CMSPh2_AntennaDriver
AntennaFlag = -D__ANTENNA__

ifneq ("$(wildcard $(ANTENNADIR))","")
	IncludeDirs     =  /opt/cactus/include ../ $(ANTENNADIR)
	LibraryDirs = /opt/cactus/lib ../lib $(ANTENNADIR)/lib
	ExternalObjects= $(LibraryPaths) -lpthread  -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal \
                                 -lboost_system -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Utils -lPh2_Tracker -lPh2_Antenna $(AntennaFlag) 
        ANTENNAINSTALLED = yes
else
	IncludeDirs     =  /opt/cactus/include ../ 
	LibraryDirs = /opt/cactus/lib ../lib 
	ExternalObjects= $(LibraryPaths) -lpthread  -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal \
                                 -lboost_system -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Utils -lPh2_Tracker
        ANTENNAINSTALLED = no
endif

CC              = gcc
CXX             = g++
CCFlags         = -g -O0 -w -Wall -pedantic -std=c++11 -fPIC 
CCFlagsRoot	= `root-config --cflags --evelibs --glibs`
ROOTVERSION := $(shell root-config --has-http)
HttpFlag = -D__HTTP__

DevFlags        =

#IncludeDirs     =  /opt/cactus/include ../ ../../CMSPh2_AntennaDriver

IncludePaths            = $(IncludeDirs:%=-I%)

#LibraryDirs = /opt/cactus/lib ../lib ../../CMSPh2_AntennaDriver/lib
RootLibraryDirs = /usr/local/lib/root

LibraryPaths = $(LibraryDirs:%=-L%) 
RootLibraryPaths = $(RootLibraryDirs:%=-L%)


ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot= $(RootLibraryPaths) -lPh2_Tools -lRHTTP $(HttpFlag)
else
	ExtObjectsRoot= $(RootLibraryPaths) -lPh2_Tools
endif

binaries=print systemtest datatest hybridtest cmtest calibrate commission fpgaconfig pulseshape configure
binariesNoRoot=systemtest datatest fpgaconfig configure

.PHONY: clean $(binaries)
all: rootflags clean $(binaries) 

rootflags:
	$(eval CCFlags += $(CCFlagsRoot))
	$(eval ExternalObjects += $(ExtObjectsRoot))

noroot: clean $(binariesNoRoot)

print:
	@echo '****************************'
	@echo 'Building executables in src/'
	@echo 'Root Has Http:' $(ROOTVERSION)
	@echo '****************************'

systemtest: systemtest.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

calibrate: calibrate.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

datatest: datatest.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

configure: configure.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

hybridtest: hybridtest.cc
	$(CXX) -std=c++11 $(CCFlags) -L/usr/lib64/ -lusb -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

cmtest: cmtest.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

commission: commission.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

fpgaconfig: fpgaconfig.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

pulseshape: pulseshape.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

clean:
	rm -f $(binaries) *.o
