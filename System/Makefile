Objs            = FileParser.o SystemController.o
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC 
#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags                =

.PHONY: clean print


IncludeDirs     =  /opt/cactus/include ../ ../lib
IncludePaths            = $(IncludeDirs:%=-I%)

LibraryDirs = /opt/cactus/include ../ ../lib
LibraryPaths = $(LibraryDirs:%=-L%)

ExternalObjects = $(LibraryPaths) -lPh2_Interface -lPh2_Description

%.o: %.cc %.h
	$(CXX) -std=c++0x  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

all: print $(Objs)
	$(CC) -std=c++0x -shared -o libPh2_System.so $(Objs)
	mv libPh2_System.so ../lib

print:
	@echo '****************************'
	@echo 'Building SystemController '
	@echo '****************************'
clean:
	rm -f *.o

