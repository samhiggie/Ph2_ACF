CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -std=c++11 -fPIC 
CCFlagsRoot	= `root-config --cflags --evelibs --glibs`
#DevFlags                    = -D__CBCDAQ_DEV__
DevFlags                   =

IncludeDirs     =  /opt/cactus/include ../

IncludePaths            = $(IncludeDirs:%=-I%)

LibraryDirs = /opt/cactus/lib ../lib
# /opt/xdaq/lib
LibraryPaths = $(LibraryDirs:%=-L%)

ExternalObjects= $(LibraryPaths) -lpthread  -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal -lboost_system -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Utils -lboost_filesystem -lboost_program_options -L../RootWeb/lib -lRootWeb
# ExtObjectsRoot= -lPh2_Tools 

# %.o: %.cc %.h
# 	$(CXX) -std=c++11 $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

binaries=miniDQM miniDAQ

all: rootflags clean $(binaries) 

rootflags:
	$(eval CCFlags += $(CCFlagsRoot))
	$(eval ExternalObjects += $(ExtObjectsRoot))

miniDQM: datatestFromfile.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

miniDAQ: miniDAQ.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

.PHONY: clean

clean:
	rm -f *.o $(binaries)
