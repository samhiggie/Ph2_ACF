ANTENNADIR=CMSPh2_AntennaDriver
AMC13DIR=/opt/cactus/include/amc13

SUBDIRS := Utils HWDescription HWInterface System tools RootWeb Tracker src miniDAQ
ANTENNAINSTALLED = no
AMC13INSTALLED = no

ifneq ("$(wildcard $(ANTENNADIR))","")
	SUBDIRS := CMSPh2_AntennaDriver $(SUBDIRS)
	ANTENNAINSTALLED = yes
else
	ANTENNAINSTRUCTIONS = To use the USB Antenna, please download the Driver from 'https://github.com/gauzinge/CMSPh2_AntennaDriver.git' and make sure that libusb-devel is installed!
endif

ifneq ("$(wildcard $(AMC13DIR))","")
	SUBDIRS := AMC13 $(SUBDIRS)
	AMC13INSTALLED = yes
else
	AMC13INSTRUCTIONS = This feature can only be built if the AMC13 libraries are installed with cactus!
endif

.PHONY: print subdirs $(SUBDIRS) clean

subdirs: print $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@


print:
	@echo '   '   
	@echo '*****************************'
	@echo 'BUILDING PH2 ACF FRAMEWORK'
	@echo 'with the following targets:'
	@echo $(SUBDIRS)
	@echo '*****************************'
	@echo 'Amc13 SW installed:' $(AMC13INSTALLED)
	@echo $(AMC13INSTRUCTIONS)
	@echo '*****************************'
	@echo 'Antenna Driver installed:' $(ANTENNAINSTALLED)
	@echo $(ANTENNAINSTRUCTIONS)
	@echo '*****************************'

src: HWDescription HWInterface Utils System tools
System: HWDescription HWInterface Utils

clean:
	(cd System; make clean)
	(cd Utils; make clean)
	(cd HWInterface; make clean)
	(cd HWDescription; make clean)
	(cd tools; make clean)
	(cd RootWeb; make clean)
	(cd miniDAQ; make clean)
	(cd Tracker; make clean)
	(cd AMC13; make clean)
	(cd doc; make clean)
	(rm -f lib/* bin/*)

