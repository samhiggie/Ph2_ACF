# CMS Ph2 ACF (Acquisition & Control Framework) 

##### Contains:

- A middleware API layer, implemented in C++, which wraps the firmware calls and handshakes into abstracted functions

- A C++ object-based library describing the system components (CBCs,
        Hybrids, Boards) and their properties(values, status)

- several utilities (like visitors to execute certain tasks for each item in the hierarchical Item description)

- a tools/ directory with several utilities (currently: calibration, hybrid testing, common-mode analysis)

    - some applications: datatest, interfacetest, hybridtest, system, calibrate, commission, fpgaconfig

##### Different versions

On this Repo, you can find different version of the software :
    - a hopefully working and stable version on the master branch
    - An in-progress version in the Dev branch

### Setup

Firmware for the GLIB can be found in /firmware. Since the "old" FMC flavour is deprecated, only new FMCs (both connectors on the same side) are supported.
You'll need Xilinx Impact and a [Xilinx Platform Cable USB II] (http://uk.farnell.com/xilinx/hw-usb-ii-g/platform-cable-configuration-prog/dp/1649384)

#### Setup on SLC6

1. Install the latest gcc compiler:

        $> sudo wget -O /etc/yum.repos.d/slc6-devtoolset.repo http://linuxsoft.cern.ch/cern/devtoolset/slc6-devtoolset.repo
        $> sudo yum install devtoolset-2
        $> . /opt/rh/devtoolset-2/enable   # add this to your .bashrc
        $> sudo ln -s /opt/rh/devtoolset-2/root/usr/bin/* /usr/local/bin/
        $> hash -r

    This should give you a more recent gcc (e.g. gcc 4.8.2)

        $> gcc --version

    Alternatively you can use a gcc version > 4.7 from AFS

1. Install uHAL  version 2.3:

        $> wget http://svnweb.cern.ch/trac/cactus/export/28265/tags/ipbus_sw/uhal_2_3_0/scripts/release/cactus.slc6.x86_64.repo 

    (You may need the --no-check-certificate)

        $> sudo cp cactus.slc6.x86_64.repo /etc/yum.repos.d/cactus.repo

    then

        $> sudo yum clean all
        $> sudo yum groupinstall uhal

1. Install CERN ROOT version 5.34.32: [Instructions](http://root.cern.ch/drupal/content/installing-root-source) - make sure to use "fixed location installation" when building yourself. If root is installed on a CERN computer of virtual machine you can use:
       
        $> sudo yum install root
        $> sudo yum install root-net-http root-graf3d-gl root-physics libusb-devel root-montecarlo-eg root-graf3d-eve root-geom

1. If you are working on a remote machine, you need these packages for the Canvases to show

        $> sudo yum install xorg-x11-xauth.x86_64

    Note: You may also need to set the environment variables (or source setup.sh):

        $> export LD_LIBRARY_PATH=/opt/cactus/lib:$LD_LIBRARY_PATH
        $> export PATH=/opt/cactus/bin:$PATH

### The Ph2_ACF Software : 

Follow these instructions to install and compile the libraries:
(provided you installed the latest version of gcc, µHal, boost as mentioned above).

1. Clone the GitHub repo and run setup.sh
  
        $> git clone https://:@gitlab.cern.ch:8443/cms_tk_ph2/Ph2_ACF.git
        $> source setup.sh

1. Do a `make` in the root of the repo (make sure you have all µHal, root libraries on your computer).

1. Alternatively you can use CMAKE (you will still need to source setup.sh to put the binaries in the path):

        $> cd build/
        $> cmake ..
        $> make -jN
        $> cd .. & source setup.sh

1. Launch 

        $> systemtest --help

    command if you want to test the parsing of the HWDescription.xml file.

1. Launch

        $> datatest --help

    command if you want to test if you can correctly read data

1. Launch

        $> calibrate --help

    to calibrate a hybrid,

        $> hybridtest --help

    to test a hybird's I2C registers and input channel connectivity

          $> cmtest --help

    to run the CM noise study

          $> pulseshape --help

    to measure the analog pulseshape of the cbc

          $> configure --help

    to apply a configuration to the CBCs

1. Launch

          $> commission --help

    to do latency & threshold scans

1. Launch 

          $> fpgaconfig --help

    to upload a new FW image to the GLIB

1. Launch

          $> miniDAQ --help

    to save binary data from the GLIB to file

1. Launch

          $> miniDQM --help

    to run the DQM code from the June '15 beamtest


##### Nota Bene:
When you write a register in the Glib or the Cbc, the corresponding map of the HWDescription object in memory is also updated, so that you always have an exact replica of the HW Status in the memory.

Register values are:
  - 8-bit unsigend integers for the CBCs that should be edited in hex notation, i.e. '0xFF'
  - 32-bit unsigned integers for the GLIB: decimal values

For debugging purpose, you can activate DEV_FLAG in the sources or in the Makefile and also activate the uHal log in RegManager.cc.


#### External Clock and Trigger:

In order to use external Clock and Trigger functionality, a DIO5 mezzanine is required. It is available from the [CERN OHR](http://www.ohwr.org/projects/fmc-dio-5chttla) and sold by several commercial vendors.


#### Example HWDescription.xml File with DIO5 support:

```xml

<?xml version='1.0' encoding='utf-8'?>
<HwDescription>
  <BeBoard Id="0" boardType="D19C" eventType="VR">
      <connection id="board" uri="ipbusudp-2.0://192.168.1.80:50001" address_table="file://settings/address_tables/d19c_address_table.xml" />

    <Module FeId="0" FMCId="0" ModuleId="0" Status="1">
       <!--<Global>-->
           <!--<Settings threshold="120" latency="12"/>-->
           <!--<TestPulse enable="1" polarity="0" amplitude="0x08" channelgroup="0" delay="0" groundothers="1"/>-->
           <!--<ClusterStub clusterwidth="4" ptwidth="3" layerswap="0" off1="0"/>-->
           <!--<Misc analogmux="0b00000"/>-->
           <!--<ChannelMask disable="1"/>-->
       <!--</Global>-->
        <CBC_Files path="./settings/CbcFiles/" />
        <CBC Id="0" configfile="Cbc_default_electron.txt" />
        <CBC Id="1" configfile="Cbc_default_electron.txt" />
        <CBC Id="2" configfile="Cbc_default_electron.txt" />
        <CBC Id="3" configfile="Cbc_default_electron.txt" />
        <CBC Id="4" configfile="Cbc_default_electron.txt" />
        <CBC Id="5" configfile="Cbc_default_electron.txt" />
        <CBC Id="6" configfile="Cbc_default_electron.txt" />
        <CBC Id="7" configfile="Cbc_default_electron.txt" />
            <!--<Settings threshold="120" latency="80"/>-->
            <!--<TestPulse enable="0" polarity="0" amplitude="0xFF" channelgroup="0" delay="0" groundothers="1"/>-->
            <!--<ClusterStub clusterwidth="4" ptwidth="3" layerswap="0" off1="0"/>-->
            <!--<Misc analogmux="0b00000"/>-->
            <!--<ChannelMask disable=""/>-->
            <!--<Register name="Pipe&StubInpSel&Ptwidth"> 0x63 </Register>-->
    </Module>

    <SLink>
        <DebugMode type="FULL"/>
        <ConditionData type="I2C" Register="VCth" FeId="0" CbcId="0"/>
        <ConditionData type="User" UID="0x80" FeId="0" CbcId="0"> 0x22 </ConditionData>
        <ConditionData type="HV" FeId="0" Sensor="2"> 250 </ConditionData>
        <ConditionData type="TDC" FeId="0xFF"/>
    </SLink>

    <!--CONFIG-->
    <Register name="fc7_daq_cnfg">
        <!-- Fast Command Block -->
        <Register name="fast_command_block">
            <Register name="triggers_to_accept"> 0 </Register>
		    <Register name="trigger_source"> 6 </Register>
		    <Register name="user_trigger_frequency"> 1 </Register>
		    <Register name="stubs_mask"> 1 </Register>
		    <Register name="stub_trigger_latency"> 194 </Register>
            <Register name="test_pulse">
                <Register name="delay_after_fast_reset"> 50 </Register>
                <Register name="delay_after_test_pulse"> 200 </Register>
	            <Register name="delay_before_next_pulse"> 400 </Register>
            </Register>
        </Register>
	<!-- I2C manager -->
        <Register name="command_processor_block">
                <Register name="i2c_write_mask"> 0xFF </Register>
	</Register>
	<!-- Phy Block -->
	<Register name="physical_interface_block">
		<Register name="i2c">
                	<Register name="frequency"> 4 </Register>
		</Register>
	</Register>
	<!-- Readout Block -->
    	<Register name="readout_block">
            <Register name="packet_nbr"> 99 </Register>
            <Register name="global">
		    <Register name="data_handshake_enable"> 1 </Register>
                    <Register name="int_trig_enable"> 0 </Register>
                    <Register name="int_trig_rate"> 0 </Register>
                    <Register name="trigger_type"> 0 </Register>
                    <Register name="data_type"> 0 </Register>
                    <Register name="common_stubdata_delay"> 194 </Register>
            </Register>
    	</Register>
	<!-- DIO5 Block -->
	<Register name="dio5_block">
	    <Register name="dio5_en"> 0 </Register>
            <Register name="ch1">
                <Register name="out_enable"> 1 </Register>
                <Register name="term_enable"> 0 </Register>
                <Register name="threshold"> 0 </Register>
            </Register>
	    <Register name="ch2">
                <Register name="out_enable"> 0 </Register>
                <Register name="term_enable"> 1 </Register>
                <Register name="threshold"> 50 </Register>
            </Register>
	    <Register name="ch3">
                <Register name="out_enable"> 1 </Register>
                <Register name="term_enable"> 0 </Register>
                <Register name="threshold"> 0 </Register>
            </Register>
	    <Register name="ch4">
                <Register name="out_enable"> 0 </Register>
                <Register name="term_enable"> 1 </Register>
                <Register name="threshold"> 50 </Register>
            </Register>
	    <Register name="ch5">
                <Register name="out_enable"> 0 </Register>
                <Register name="term_enable"> 1 </Register>
                <Register name="threshold"> 50 </Register>
            </Register>
	</Register>
	<!-- TLU Block -->
	<Register name="tlu_block">
		<Register name="handshake_mode"> 2 </Register>
	</Register>
    </Register>
  </BeBoard>
</HwDescription>

<Settings>
    <!--Calibration-->
   <Setting name="TargetVcth">0x78</Setting>
    <Setting name="TargetOffset">0x50</Setting>
    <Setting name="Nevents">50</Setting>
    <Setting name="TestPulsePotentiometer">0x00</Setting>
    <Setting name="HoleMode">0</Setting>
    <Setting name="VerificationLoop">1</Setting>

</Settings>


```


### Known Issues:

Several bugs / problematic behavior has been reported by various users that is not direclty linked to the Ph2_ACF libraries, however, some workarounds are provided:

- When configuring a CBC object (writing all registers at once), the MSB of the Register "FrontEncControl" is read back incorrectly. This only manifests in electron mode (0xC3 instead of 0x43). The cause of this problem is identified as a FW artefact and the error itself can be safely ignored until the problem is solved. The chips will still properly configure and data quality should not be affected.

- uHAL exceptions and UDP timeouts when reading larger packet sizes from the GLIB board: 
this can happen for some users (cause not yet identified) but can be circumvented by changing the line

"ipbusudp-2.0://192.168.000.175:50001"

in the connections.xml file to

"chtcp-2.0://localhost:10203?target=192.168.000.175:50001"

& then launching the CACTUS control hub by the command:

/opt/cactus/bin/controlhub_start

This uses TCP protocol instead of UDP which accounts for packet loss but decreases the performance.

- SegmentationViolations on lines that contain

gStyle->Set ... ;

statements. This has been observed by several users on the VM and can be fixed by re-compiling ROOT using GCC 4.8


### Support, Suggestions ?

For any support/suggestions, mail georg.auzingerSPAMNOT@cern.ch
