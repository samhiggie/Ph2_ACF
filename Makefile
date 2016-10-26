ANTENNADIR=CMSPh2_AntennaDriver
AMC13DIR=/opt/cactus/include/amc13
HMP4040DIR=../Ph2_USBInstDriver

#DEPENDENCIES := Utils HWDescription HWInterface System tools RootWeb Tracker src miniDAQ
DEPENDENCIES := Utils HWDescription HWInterface RootWeb Tracker
ANTENNAINSTALLED = no
AMC13INSTALLED = no
HMP4040INSTALLED = no

ifneq ("$(wildcard $(ANTENNADIR))","")
	DEPENDENCIES := CMSPh2_AntennaDriver $(DEPENDENCIES)
	ANTENNAINSTALLED = yes
else
	ANTENNAINSTRUCTIONS = To use the USB Antenna, please download the Driver from 'https://gitlab.cern.ch/cms_tk_ph2/CMSPh2_AntennaDriver.git' and make sure that libusb-devel is installed!
endif

ifneq ("$(wildcard $(AMC13DIR))","")
	DEPENDENCIES := AMC13 $(DEPENDENCIES)
	AMC13INSTALLED = yes
else
	AMC13INSTRUCTIONS = This feature can only be built if the AMC13 libraries are installed with cactus!
endif

ifneq ("$(wildcard $(HMP4040DIR))","")
	DEPENDENCIES := ../Ph2_USBInstDriver $(DEPENDENCIES)
	HMP4040INSTALLED = yes
	HMP4040INSTRUCTIONS = HMP4040 instrument driver found.
else
	HMP4040INSTRUCTIONS = To use the HMP4040 Instrument driver, please download the Driver from 'https://:@gitlab.cern.ch:8443/cms_tk_ph2/Ph2_USBInstDriver.git'.
endif

.PHONY: print dependencies $(DEPENDENCIES) clean src miniDAQ tools

all: src miniDAQ

dependencies: print $(DEPENDENCIES)

$(DEPENDENCIES):
	$(MAKE) -C $@

System: dependencies
	$(MAKE) -C System

tools: System
	$(MAKE) -C tools

src miniDAQ: tools
	$(MAKE) -C $@



print:
	@echo '   '   
	@echo '*****************************'
	@echo 'BUILDING PH2 ACF FRAMEWORK'
	@echo 'with the following Dependencies:'
	@echo $(DEPENDENCIES)
	@echo '*****************************'
	@echo 'Amc13 SW installed:' $(AMC13INSTALLED)
	@echo $(AMC13INSTRUCTIONS)
	@echo '*****************************'
	@echo 'Antenna Driver installed:' $(ANTENNAINSTALLED)
	@echo $(ANTENNAINSTRUCTIONS)
	@echo '*****************************'
	@echo 'HMP4040 Driver installed:' $(HMP4040INSTALLED)
	@echo $(HMP4040INSTRUCTIONS)
	@echo '*****************************'


clean:
	#$(MAKE) -C System clean 
	(cd System; make clean)
	(cd Utils; make clean)
	(cd HWInterface; make clean)
	(cd HWDescription; make clean)
	(cd tools; make clean)
	(cd RootWeb; make clean)
	(cd miniDAQ; make clean)
	(cd Tracker; make clean)
	(cd AMC13; make clean)
	(cd ../Ph2_USBInstDriver; make clean)
	#(cd doc; make clean)
	(rm -f lib/* bin/*)