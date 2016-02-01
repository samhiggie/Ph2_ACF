Objs            = RegManager.o BeBoardFWInterface.o GlibFWInterface.o CtaFWInterface.o BeBoardInterface.o FpgaConfig.o GlibFpgaConfig.o CtaFpgaConfig.o CbcInterface.o MmcPipeInterface.o Firmware.o
CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC  -Wcpp
#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags	=

.PHONY: clean print


IncludeDirs     =  /opt/cactus/include ../ .

IncludePaths            = $(IncludeDirs:%=-I%)

%.o: %.cc %.h
	$(CXX) -std=c++11  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

all: print $(Objs) ../HWDescription/Definition.h
	$(CC) -std=c++11 -shared -o libPh2_Interface.so $(Objs) -pthread
	mv libPh2_Interface.so ../lib

print:
	@echo '****************************'
	@echo 'Building HWInterfaces '
	@echo '****************************'
clean:
	rm -f *.o
