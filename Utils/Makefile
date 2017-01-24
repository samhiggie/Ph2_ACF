Objs            = Exception.o UsbUtilities.o Utilities.o Event.o Cbc2Event.o Cbc3Event.o Data.o argvparser.o  FileHandler.o
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC `root-config --cflags --evelibs` -Wcpp -L/usr/lib64/
#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags			 = 
USBINSTFlag     = -D__USBINST__
ZmqFlag = -D__ZMQ__

LibraryDirs = /opt/cactus/lib ../lib 
	IncludeDirs     =  /opt/cactus/include ../ 
	ExternalObjects= $(IncludeDirs:%=-I%) -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal -lboost_system
#IncludePaths            = $(IncludeDirs:%=-I%)
#ExternalObjects= $(LibraryPaths)

#################################################
# check if ZMQ library is available on this machine
#################################################
ifneq ("$(wildcard $(ZMQ_HEADER_PATH))","")
        ExternalObjects += -lzmq $(ZmqFlag)
        ZMQINSTALLED = yes
else
		ZMQINSTALLED = no
        ZMQINSTRUCTIONS = ZeroMQ library is required to build the lvSupervisor with the server option - get it with \'sudo yum install zeromq-devel\'
endif

##################################################
## check if the Ph2_USBInstDriver drivers are installed
##################################################
ifneq ("$(wildcard $(USBINSTDIR))","")
	LibraryDirs += $(USBINSTDIR)/lib $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/Utils 
	IncludeDirs += $(BOOST_INCLUDE) $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/Utils $(USBINSTDIR)/Drivers 
	ExternalObjects +=  -lpthread -lboost_thread -lPh2_HMP4040 -lPh2_Ke2110 $(USBINSTFlag) -lPh2_Drivers -lPh2_USBUtils 
	USBINSTLIBINSTALLED = yes
else
	USBINSTLIBINSTALLED = no
endif

.PHONY: clean print

%.o: %.cc %.h 
	$(CXX) -std=c++11  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) $(ExternalObjects) -c -o $@ $<

all: print $(Objs) ../HWDescription/Definition.h
	$(CC) -std=c++11 -pthread -shared -L/usr/lib64/ -o libPh2_Utils.so $(Objs) -pthread  
	mv libPh2_Utils.so ../lib

print:
	@echo '****************************'
	@echo 'Building Utils'
	@echo 'Using zmq library:'$(ZMQINSTALLED)
	@echo 'Using zmq Ph2_USBInstDriver:'$(USBINSTLIBINSTALLED)
	@echo '****************************'
clean:
	rm -f *.o
