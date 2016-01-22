Objs            = SystemController.o
CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC -Wcpp
#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags                =

.PHONY: clean print


IncludeDirs     =  /opt/cactus/include ../ ../lib
IncludePaths            = $(IncludeDirs:%=-I%)

LibraryDirs = /opt/cactus/include ../ ../lib
LibraryPaths = $(LibraryDirs:%=-L%)

ExternalObjects = $(LibraryPaths) -lPh2_Interface -lPh2_Description

%.o: %.cc %.h
	$(CXX) -std=c++11  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

all: print $(Objs)
	$(CC) -std=c++11 -shared -o libPh2_System.so $(Objs)
	mv libPh2_System.so ../lib

print:
	@echo '****************************'
	@echo 'Building SystemController '
	@echo '****************************'
clean:
	rm -f *.o

