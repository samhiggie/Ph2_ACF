all: Utils HWDescription HWInterface System tools RootWeb Tracker src miniDAQ

libs: Utils HWDescription HWInterface System Tracker srcnoroot

#gui: Utils HWDescription HWInterface System tools src miniDAQ GUI

simple: Utils HWDescription HWInterface System tools RootWeb Tracker src miniDAQ

HWDescription::
	$(MAKE) -C $@
Utils::
	$(MAKE) -C $@
HWInterface::
	$(MAKE) -C $@
System::
	$(MAKE) -C $@
tools::
	$(MAKE) -C $@
Tracker::
	$(MAKE) -C $@
srcnoroot::
	$(MAKE) -C src noroot
src::
	$(MAKE) -C $@
	#GUI::
	#$(MAKE) -C GUI/Macros
	#$(MAKE) -C $@
	#cp $@/Ph2_ACF ./bin
RootWeb::
	$(MAKE) -C $@
miniDAQ::
	$(MAKE) -C $@
doc::
	$(MAKE) -C $@


clean:
	(cd System; make clean)
	(cd Utils; make clean)
	(cd HWInterface; make clean)
	(cd HWDescription; make clean)
	(cd tools; make clean)
	(cd RootWeb; make clean)
	(cd miniDAQ; make clean)
	(cd Tracker; make clean)
	#(cd GUI; make clean)
	#(cd GUI; make clean; cd GUI/Macros; make clean)
	(cd doc; make clean)
	(rm -f lib/* bin/*)

