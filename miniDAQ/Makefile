CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -std=c++11 -fPIC 
CCFlagsRoot	= `root-config --cflags --evelibs --glibs`

#DevFlags       = -D__CBCDAQ_DEV__
DevFlags        = 

ROOTVERSION := $(shell root-config --has-http)
HttpFlag = -D__HTTP__
#DevFlags                    = -D__CBCDAQ_DEV__


IncludeDirs     =  /opt/cactus/include ../

IncludePaths    = $(IncludeDirs:%=-I%)
IncludePaths    += -I../RootWeb/include


LibraryDirs = /opt/cactus/lib ../lib
RootLibraryDirs = /usr/local/lib/root

LibraryPaths = $(LibraryDirs:%=-L%)
RootLibraryPaths = $(RootLibraryDirs:%=-L%)



ExternalObjects= $(LibraryPaths) -lpthread  -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal \
                                 -lboost_system -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Utils -lboost_filesystem -lboost_program_options -L../RootWeb/lib -lRootWeb
#                                 -lboost_filesystem -lboost_program_options
# ExtObjectsRoot= -lPh2_Tools 
#ExternalObjects= $(LibraryPaths) -lpthread  -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal -lboost_system -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Utils -lboost_filesystem -lboost_program_options -L../RootWeb/lib -lRootWeb

# ifeq "5.34" "$(word 1, $(sort 5.34 $(RootVersion)))"
ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot= $(RootLibraryPaths) -lRHTTP $(HttpFlag)
endif


# %.o: %.cc %.h
# 	$(CXX) -std=c++11 $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

binaries=print miniDQM miniDAQ

all: rootflags clean $(binaries) 

rootflags:
	$(eval CCFlags += $(CCFlagsRoot))
	$(eval ExternalObjects += $(ExtObjectsRoot))

print:
	@echo 'Root Has Http:' $(ROOTVERSION)

publisher.o: publisher.cc publisher.h
	$(CXX) -std=c++11  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

miniDQM: datatestFromfile.cc publisher.h publisher.o
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) publisher.o $(ExternalObjects) 


miniDAQ: miniDAQ.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

.PHONY: clean

clean:
	rm -f *.o $(binaries)
