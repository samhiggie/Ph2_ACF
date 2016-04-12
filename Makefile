ANTENNADIR=CMSPh2_AntennaDriver

ifneq ("$(wildcard $(ANTENNADIR))","")
	SUBDIRS = CMSPh2_AntennaDriver Utils HWDescription HWInterface System tools RootWeb Tracker src miniDAQ
	ANTENNAINSTALLED = yes
else
	SUBDIRS = Utils HWDescription HWInterface System tools RootWeb Tracker src miniDAQ
	ANTENNAINSTALLED = no
	ANTENNAINSTRUCTIONS = To use the USB Antenna, please download the Driver from 'https://github.com/gauzinge/CMSPh2_AntennaDriver.git' and make sure that libusb-devel is installed!
endif

.PHONY: print subdirs $(SUBDIRS) clean

subdirs: print $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@


print:
	@echo '   '   
	@echo '*****************************'
	@echo 'BUILDING PH2 ACF FRAMEWORK'
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
	(cd doc; make clean)
	(rm -f lib/* bin/*)

