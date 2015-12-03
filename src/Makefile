CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -std=c++11 -fPIC 
CCFlagsRoot	= `root-config --cflags --evelibs --glibs`
ROOTVERSION := $(shell root-config --has-http)
HttpFlag = -D__HTTP__

#DevFlags                    = -D__CBCDAQ_DEV__
DevFlags                   =

IncludeDirs     =  /opt/cactus/include ../ ../../CMSPh2_AntennaDriver

IncludePaths            = $(IncludeDirs:%=-I%)

LibraryDirs = /opt/cactus/lib ../lib ../../CMSPh2_AntennaDriver/lib
RootLibraryDirs = /usr/local/lib/root

LibraryPaths = $(LibraryDirs:%=-L%) 
RootLibraryPaths = $(RootLibraryDirs:%=-L%)


ExternalObjects= $(LibraryPaths) -lpthread  -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal \
                                 -lboost_system -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Utils -lPh2_Antenna

# ifeq "5.34" "$(word 1, $(sort 5.34 $(RootVersion)))"
ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot= $(RootLibraryPaths) -lPh2_Tools -lRHTTP $(HttpFlag)
else
	ExtObjectsRoot= $(RootLibraryPaths) -lPh2_Tools
endif
# ExtObjectsRoot= -lPh2_Tools -lRHTTP $(HttpFlag)

# -lboost_thread -lboost_filesystem -lboost_regex -lboost_system -lboost_thread

# %.o: %.cc %.h
# 	$(CXX) -std=c++11 $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

binaries=print systemtest datatest hybridtest cmtest calibrate commission fpgaconfig pulseshape configure
binariesNoRoot=systemtest datatest fpgaconfig configure


all: rootflags clean $(binaries) 

rootflags:
	$(eval CCFlags += $(CCFlagsRoot))
	$(eval ExternalObjects += $(ExtObjectsRoot))

noroot: clean $(binariesNoRoot)

print:
	@echo 'Root Has Http:' $(ROOTVERSION)

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


.PHONY: clean

clean:
	rm -f $(binaries) *.o
