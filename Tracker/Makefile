Objs            = TrackerEvent.o ParamSet.o
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC 
#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags                =

IncludeDirs     =  /opt/cactus/include ../ ../lib
IncludePaths            = $(IncludeDirs:%=-I%)

LibraryDirs = /opt/cactus/include ../ ../lib
LibraryPaths = $(LibraryDirs:%=-L%)

ExternalObjects = $(LibraryPaths) -lPh2_Interface -lPh2_Description -lPh2_Utils

.PHONY: print clean
%.o: %.cc %.h
	$(CXX) -std=c++0x  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

all: print $(Objs)
	$(CC) -std=c++0x -shared -o libPh2_Tracker.so $(Objs)
	mv libPh2_Tracker.so ../lib

print:
	@echo '****************************'
	@echo 'Building Tracker Data Format'
	@echo '****************************'
clean:
	rm -f *.o

