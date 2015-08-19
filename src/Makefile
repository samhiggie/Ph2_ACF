CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -std=c++11 -fPIC `root-config --cflags --evelibs`
#DevFlags                    = -D__CBCDAQ_DEV__
DevFlags                   =

IncludeDirs     =  /opt/cactus/include ../

IncludePaths            = $(IncludeDirs:%=-I%)

LibraryDirs = /opt/cactus/lib ../lib
# /opt/xdaq/lib
LibraryPaths = $(LibraryDirs:%=-L%)

ExternalObjects = $(LibraryPaths) -lpthread  -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal -lboost_system `root-config --glibs` -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Tools -lPh2_Utils

# -lboost_thread -lboost_filesystem -lboost_regex -lboost_system -lboost_thread

%.o: %.cc %.h
	$(CXX) -std=c++11 $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

binaries=systemtest calibrate interfacetest datatest hybridtest cmtest
all: clean $(binaries)

systemtest: systemtest.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

calibrate: calibrate.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

interfacetest: interfacetest.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

datatest: datatest.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

hybridtest: hybridtest.cc
	$(CXX) -std=c++11 $(CCFlags) -L/usr/lib64/ -lusb -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

cmtest: cmtest.cc
	$(CXX) -std=c++11 $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

.PHONY: clean

clean:
	rm -f $(binaries) *.o
