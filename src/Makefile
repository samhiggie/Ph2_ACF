ZMQ_HEADER_PATH = /usr/include/zmq.hpp
 
CC              = gcc
CXX             = g++
CCFlags         = -g -O0 -w -Wall -pedantic -pthread -std=c++0x -fPIC 
CCFlagsRoot	= `root-config --cflags --glibs`
ROOTVERSION := $(shell root-config --has-http)

HttpFlag = -D__HTTP__
ZmqFlag = -D__ZMQ__
USBINSTFlag     = -D__USBINST__
Amc13Flag     = -D__AMC13__
AntennaFlag = -D__ANTENNA__

DevFlags        =

LibraryDirs = /opt/cactus/lib ../lib 
IncludeDirs     =  /opt/cactus/include ../ ../System/ ../Utils ../HWDescripion ../HWInterface ../tools 
ExternalObjects= $(LibraryPaths) -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal -lboost_system -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Utils -lPh2_Tracker -lPh2_Tools 

##################################################
## check if the Root has THttp
##################################################
ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot += $(RootLibraryPaths) -lRHTTP $(HttpFlag)
else
	ExtObjectsRoot += $(RootLibraryPaths) 
endif

##################################################
## check if the Antenna driver is installed
##################################################
ifneq ("$(wildcard ../$(ANTENNADIR))","")
	IncludeDirs += $(ANTENNADIR)
	LibraryDirs += $(ANTENNADIR)/lib /usr/lib64/ 
	#ExternalObjects += -lboost_thread -lPh2_Antenna $(AntennaFlag) 
	ExternalObjects += -lPh2_Antenna $(AntennaFlag) 
	ANTENNAINSTALLED = yes
else
	ANTENNAINSTALLED = no
endif

##################################################
## check if the AMC13 drivers are installed
##################################################
ifneq ("$(wildcard $(AMC13DIR))","")
	ExternalObjects += -lcactus_amc13_amc13 -lPh2_Amc13
	AMC13INSTALLED = yes
else
	AMC13INSTALLED = no
endif

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
	LibraryDirs += $(USBINSTDIR)/lib $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/ArdNano#$(USBINSTDIR)/Utils 
	IncludeDirs += $(BOOST_INCLUDE) $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/ArdNano $(USBINSTDIR)/Utils $(USBINSTDIR)/Drivers 
	ExternalObjects += -lpthread -lboost_thread -lPh2_HMP4040 -lPh2_Ke2110 -lPh2_ArdNano -lPh2_Drivers -lPh2_USBUtils $(USBINSTFlag)
	USBINSTLIBINSTALLED = yes
else
	USBINSTLIBINSTALLED = no
endif

IncludePaths            = $(IncludeDirs:%=-I%)
RootLibraryDirs = /usr/local/lib/root

LibraryPaths = $(LibraryDirs:%=-L%) 
RootLibraryPaths = $(RootLibraryDirs:%=-L%)


binaries=print systemtest datatest hybridtest cmtest calibrate commission fpgaconfig pulseshape configure integratedtester cbc3test #cbc3irrad
binariesNoRoot=systemtest datatest fpgaconfig configure

.PHONY: clean $(binaries)
all: rootflags clean $(binaries) 
rootflags:
	$(eval CCFlags += $(CCFlagsRoot))
	$(eval ExternalObjects += $(ExtObjectsRoot))

noroot: clean $(binariesNoRoot)

print:
	@echo '****************************'
	@echo 'Building executables in src/'
	@echo 'Root Has Http:' $(ROOTVERSION)
	@echo 'Amc13 SW installed:' $(AMC13INSTALLED)
	@echo 'Antenna installed:' $(ANTENNAINSTALLED)
	@echo 'Ph2_USBInstDriver installed:' $(USBINSTLIBINSTALLED)
	@echo 'ZeroMQ installed:' $(ZMQINSTALLED)
	@echo '****************************'

integratedtester: integratedtester.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
systemtest: systemtest.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
calibrate: calibrate.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
datatest: datatest.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
configure: configure.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
hybridtest: hybridtest.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
cmtest: cmtest.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
commission: commission.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
fpgaconfig: fpgaconfig.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
pulseshape: pulseshape.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
cbc3test: cbc3test.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin
cbc3irrad: cbc3irrad.cc
	$(CXX)  $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

clean:
	rm -f $(binaries) *.o

