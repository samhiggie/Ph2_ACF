Objs            = Exception.o Utilities.o Event.o Cbc2Event.o Cbc3Event.o Data.o argvparser.o FileHandler.o
ObjsExt      	= UsbUtilities
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC -Wcpp

LibraryDirs =
IncludeDirs =
ExternalObjects=

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
	LibraryDirs += $(USBINSTLIB) $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/Utils 
	IncludeDirs += $(BOOST_INCLUDE) $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/Utils $(USBINSTDIR)/Drivers 
	ExternalObjects +=  -lpthread -lboost_thread -lPh2_HMP4040 -lPh2_Ke2110 -lPh2_Drivers $(USBINSTFlag) 
	USBINSTLIBINSTALLED = yes
else
	USBINSTLIBINSTALLED = no
endif

IncludePaths = $(IncludeDirs:%=-I%)
LibraryPaths = $(LibraryDirs:%=-L%)

.PHONY: clean print

all: print $(Objs) $(ObjsExt).o ../HWDescription/Definition.h
	$(CC) -std=c++11 -pthread -shared -L/usr/lib64/ -o libPh2_Utils.so $(Objs) $(ObjsExt).o
	mv libPh2_Utils.so ../lib

%.o: %.cc %.h 
	$(CXX) -std=c++11 $(CCFlags) -c -o $@ $<

$(ObjsExt).o: $(ObjsExt).cc $(ObjsExt).h 
	$(CXX) -std=c++11 $(CCFlags) $(IncludePaths) $(LibraryPaths) $(ExternalObjects) -c -o $@ $<

print:
	@echo '****************************'
	@echo 'Building Utils'
	@echo 'Using zmq library:'$(ZMQINSTALLED)
	@echo 'Using zmq Ph2_USBInstDriver:'$(USBINSTLIBINSTALLED)
	@echo '****************************'
clean:
	rm -f *.o
