# CMS Ph2 ACF (Acquisition & Control Framework) 

##### Contains:

- A middleware API layer, implemented in C++, which wraps the firmware calls and handshakes into abstracted functions

- A C++ object-based library describing the system components (CBCs,
Hybrids, Boards) and their properties(values, status)

- several utilities (like visitors to execute certain tasks for each item in the hierarchical Item description)

- a tools/ directory with several utilities (currently: calibration, hybrid testing, common-mode analysis)

- some applications: datatest, interfacetest, hybridtest, system, calibrate

##### Different versions

On this Repo, you can find different version of the software :
- a hopefully working and stable version on the master branch
- An in-progress version in the Dev branch
- A development version wiht a GUI
<br>

### Changelog:


- 09/07/14 : Added threading for stack writing registers
- 12/08/14 : Working agnostic version of the new structure on Master
- 15/08/14 : System Controller Class working
- 19/08/14 : Project wrapped, called ACF for Acquisition & Control Framework
- 09/10/14 : added Visitor class and the corresponding accept methods to the HWDescription objects
- 15/10/14 : re-wrote the GlibFWInterface::ReadData() method, completeley re-wrote the parsing of the raw buffer and the offsets, modified the Data and Event classes to be more lightweight and less complex
- 17/10/14 : renamed the project to Ph2_ACF & re-structured the folder architecture and added command line parsing to the executables (v1-00)
- 17/12/14 : major update (v1-01) including:
    - performance improvements (use of C++11 features)
    - general bugfixes
    - a new, faster calibration routine
    - a common-mode-noise tester tool
    - compatability with DIO5 FW for external triggering / clocking
    - updated address table for 2 & 8 CBC setups
    - new FW files for DIO5 FW for 2 & 8 CBC setups
    - a macro directory with a macro to visualize calibration results

- 11/02/15 : new update (v1-02) with the following changes:
    - removed all dependences on ROOT from the HWDescription/ HWInterface / System / Utils Library
    - added a new Tool base-class that inherits from SystemController and handles the creation of Root files for the Tools (the Tools library is the onely one that requires ROOT)
    - all Tools (cmd-line applications) now inherit from Tool instead of SystemController
    - added a loop to the Destructors of HWDescription Objects that explicityl delete the child-objects in the HWDescription ownership strucuter to avoid memory leaks
    - added a parameter to some methods of the SystemController class that allow to pass an ostream - this allows to redirect the output from std::cout as desired
    - new Methods:
        - RegManager::GetHWInterface and RegManager::getUhalNode methods
        - BeBoardInterface::ReadBlockBoardReg and a BeBoardInterface::WriteBlockBoardReg
        - BeBoard::getReg & BeBoard::setReg use uint32_t instead of uint16_t
        - BeBoardFWInterface::ReadBlockRegValue pure virtual method and GlibFWInterface::ReadBlockRegValue that implements the uHAL bug workaround
- 24/02/15 : new update (v1-03) with the following changes:
    - CbcInterface::WriteReg() now return the written Register value
    - writing to the Glib now also takes care of avoiding the 256th word uHAL bug
    - some modifications to HybridTester tool to make it compatible with the GUI
    - added a CMD line option: g for GUI, should only be used by the latter
    - SystemController class can now parse .json files in addition to .xml
- 04/03/15: added GUI (v1-10)
- 05/03/15 : New FpgaConfig object in HWInterface to manage firmware uploading. 
    - added WriteBlockAtAddress and ReadAtAddress functions in RegManager used by the upload dialog. 
    - BeBoardInterface::FlashProm(...) uploads an MCS file into the FPGA
- 15/04/15 : Acquisition in a separate thread (v1-11)
    - Start() should be called when acquisition begins, Stop() when it ends and ReadData(...) at each iteration.
    - New functions BeBoardInterface::StartThread, StopThread, getNumAcqThread, isRunningThread. Abstract class HwInterfaceVisitor.
    - datatest -p option to perform an acquisition in a separate thread
    - datatest -i option to ignore CBC configuration. Can be run on a bare GLIB board without CBC
- 07/05/15: merged Commissioning branch (v1-12)
    - contains class for Commissioning: latency scan, threshold scan, stub latency scan
    - some additions to the Utils/CommonVisitors
    - added a stub bit method to the Event class
    - restructured the ostream operator of the Event class
- 05/06/2015: minor modifications
    - BeBoardFWInterface::ReadData() now returns the number of packets read during the acquisition for faster event counter checks. 
- 11/06/2015: merging a new design for Data and Event classes (v1-20)
    - Data class now holds a vector of Events and the events are directly decoded after acquisition - the char* databuffer does not exist any longer
    - added methods: std::vector<Event*> GetEvents(); Event* GetEvent()
    - updated all scripts
- 17/07/2015: including a new executable fpgaconfig to upload .mcs files to the EPROM, updated tool base-class, included DQM code from the Beamtest
    - fpgaconfig binary allows to upload FW images to EPROM (2 separate images)
    - tool base class now contains containers for histograms along with bookHisto(), getHisto(), saveHisto() methods
    - modified all other tools accordingly
    - merged the DQM code from the June '15 beamtest into miniDQM binary
    - updated Makefiles to build RootWeb & miniDQM
- 04/08/2015: adding a faster & more precise algorithm to extract the parameters from SCurves via differentiating it - implemented in FastCalibration
- 11/08/2015: adding threaded File IO features (v1-21)
    - added FileHandler class for threaded saving of binary data as it comes from the GLIB to file (without speed penalty)
    - FileHandler can also read binary files for playback (see miniDAQ/datatestFromfile.cc)
    - adding miniDAQ executable that saves to file and handles runnumber in a hidden Data/.run_number.txt file
    - miniDQM code based on BT DQM still available for playing back the binary data
    - datatest now has a -s option to specify a filename where binary data should be saved (optionally)
    


### Setup


Firmware for the GLIB can be found in /firmware. Since the "old" FMC flavour is deprecated, only new FMCs (both connectors on the same side) are supported.
You'll need Xilinx Impact and a [Xilinx Platform Cable USB II] (http://uk.farnell.com/xilinx/hw-usb-ii-g/platform-cable-configuration-prog/dp/1649384)

#### Setup on the Strasbourg [VM v1.1.0] (http://sbgcmstrackerupgrade.in2p3.fr/) 

1. Remove the current gcc and old boost libraries:

        $> sudo yum remove devtoolset-1.1-gcc-debuginfo
        $> sudo yum remove boost

2. Install the latest gcc compiler:

        $> sudo yum install devtoolset-2
        $> sudo ln -s /opt/rh/devtoolset-2/root/usr/bin/* /usr/local/bin/
        $> hash -r

   This should give you gcc 4.8.1:

        $> gcc --version

3. Finally, update uHAL to version 2.3:

        $> sudo yum groupremove uhal
        $>wget http://svnweb.cern.ch/trac/cactus/export/28265/tags/ipbus_sw/uhal_2_3_0/scripts/release/cactus.slc5.x86_64.repo 

    (You may need the --no-check-certificate)

        $> sudo cp cactus.slc5.x86_64.repo /etc/yum.repos.d/cactus.repo
        $> sudo yum clean all
        $> sudo yum groupinstall uhal

4. Re-compile ROOT using the new gcc 4.8: [Instructions](http://root.cern.ch/drupal/content/installing-root-source) - make sure to use "fixed location installation"

Note: You may also need to set the environment variables:

    $> export LD_LIBRARY_PATH=/opt/cactus/lib:$LD_LIBRARY_PATH
    $> export PATH=/opt/cactus/bin:$PATH

#### Setup on SLC5/6

1. Install the latest gcc compiler:

        $> sudo yum install devtoolset-2
        $> sudo ln -s /opt/rh/devtoolset-2/root/usr/bin/* /usr/local/bin/
        $> hash -r

   This should give you gcc 4.8.1:

        $> gcc --version

2. Install uHAL  version 2.3:

    For SLC5:

        $> wget http://svnweb.cern.ch/trac/cactus/export/28265/tags/ipbus_sw/uhal_2_3_0/scripts/release/cactus.slc5.x86_64.repo 
   
    or for SLC6:

        $> wget http://svnweb.cern.ch/trac/cactus/export/28265/tags/ipbus_sw/uhal_2_3_0/scripts/release/cactus.slc6.x86_64.repo 

    (You may need the --no-check-certificate)

    for SLC5:

        $> sudo cp cactus.slc5.x86_64.repo /etc/yum.repos.d/cactus.repo

    for SLC6:

        $> sudo cp cactus.slc6.x86_64.repo /etc/yum.repos.d/cactus.repo

    then

        $> sudo yum clean all
        $> sudo yum groupinstall uhal

3. Install CERN ROOT: [Instructions](http://root.cern.ch/drupal/content/installing-root-source) - make sure to use "fixed location installation"

Note: You may also need to set the environment variables:

    $> export LD_LIBRARY_PATH=/opt/cactus/lib:$LD_LIBRARY_PATH
    $> export PATH=/opt/cactus/bin:$PATH


#### The GUI :

 These instructions are provided to install the optional GUI.


1. Source the variables:

    $> source setup.sh
        
2. Make the GUI:

        $> make
    or

        $> make GUI

3. Run it:
        
        $> ./Ph2_ACF


### The Ph2_ACF Software : 

Follow these instructions to install and compile the libraries:
(provided you installed the latest version of gcc, µHal, boost as mentioned above)

1. Clone the GitHub repo.

2.      
    
        $> source setup.sh

3. Do a make in the root of the repo (make sure you have all µHal, root, boost... libraries on your computer).

3. Launch 
        
        $> systemtest --help
     
     command if you want to test the parsing of the HWDescription.xml file

4. Launch

        $> datatest --help
        
    command if you want to test if you can correctly read data.

6. Launch

        $> calibrate --help

    to calibrate a hybrid,

        $> hybridtest --help

    to test a hybird's I2C registers and input channel connectivity

        $> cmtest --help

    to run the CM noise study

7. Launch

        $> commission --help

    to do latency & threshold scans

8. Launch 

        $> fpgaconfig --help

    to upload a new FW image to the GLIB

9. Launch

        $> miniDAQ --help

    to save binary data from the GLIB to file

10. Launch

        $> miniDQM --help

    to run the DQM code from the June '15 beamtest

an example of how to use visitors can be found in src/interfacetest.cc or in the HybridTester class


##### What can you do with the software ?

At the moment the package provides the following features:

- Configure the Glib & Cbcs
- Manipulate the registers in the Glib
- Manipulate the registers in the Cbcs
- Read Data
- Calibrate Hybrids
- Validate Hybrids
- Perform CM noise tests
- user external trigger and clock signals for your tests
- upload .mcs files to the GLIB
- perform simple commissioning procedures
- save binary data to file
- create simple DQM histograms from binary data
- any other routine you want to implement yourself ... 


##### Nota Bene:
When you write a register in the Glib or the Cbc, the corresponding map of the HWDescription object in memory is also updated, 
so that you always have an exact replica of the HW Status in the memory.

Register values are:
- 8-bit unsigend integers for the CBCs that should be edited in hex notation, i.e. '0xFF'
- 32-bit unsigned integers for the GLIB: decimal values

For debugging purpose, you can activate DEV_FLAG in the sources or in the Makefile and also activate the uHal log in RegManager.cc.


#### External Clock and Trigger:

In order to use external Clock and Trigger functionality, a DIO5 mezzanine is required. It is available from the [CERN OHR](http://www.ohwr.org/projects/fmc-dio-5chttla) and sold by several commercial vendors.
For instructions on how to use it, see this [file](https://github.com/gauzinge/Ph2_ACF/blob/Dev/doc/TK_DAQ_MONO_GLIB3_FMCDIO5_v3.0_09-12-2014.pdf). The [firmware](https://github.com/gauzinge/Ph2_ACF/tree/Dev/firmware) is included in this repository.



#### Example HWDescription.xml File with DIO5 support:

```xml

<?xml version='1.0' encoding = 'UTF-8' ?>
<HwDescription>

    <!-- The file containing the connection data & IP addresses -->
    <Connections name="file://settings/connections_2CBC.xml"/>

    <Shelve Id="0" >
        <BeBoard Id="0" connectionId="board0" boardType="GLIB">

            <Module FeId="0" FMCId="0" ModuleId="0" Status="1">

                <!-- Global CBC registers are applied to all CBCs and override register values from the config files -->
                <Global_CBC_Register name="TriggerLatency"> 0x0C </Global_CBC_Register> -->

                <!-- a base path to the CBC files -->
                <CBC_Files path="./settings/"/>

                <CBC Id="0" configfile="Cbc_default_hole.txt"/>
                <CBC Id="1" configfile="Cbc_default_hole.txt"/>
            </Module>

            <!-- Commissioning Mode -->
            <!-- set to 1 to enable commissioning mode -->
            <Register name="COMMISSIONNING_MODE_RQ"> 0 </Register>
            <!-- set to 1 to enable test pulse in commissioning mode -->
            <Register name="COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID"> 0 </Register>
            <!-- Delays after testpulse / L1A -->
            <Register name="COMMISSIONNING_MODE_DELAY_AFTER_FAST_RESET"> 50 </Register>
            <Register name="COMMISSIONNING_MODE_DELAY_AFTER_L1A"> 400 </Register>
            <Register name="COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE"> 201 </Register>

            <!-- Trigger -->
            <!-- set to 1 to use external triggers -->
            <Register name="user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL"> 0 </Register>
            <!-- internal trigger frequency: see address_table*.xml for details -->
            <Register name="user_wb_ttc_fmc_regs.pc_commands.INT_TRIGGER_FREQ"> 10 </Register>

            <!-- ************************************ -->
            <!-- This part is only required with the DIO5 mezzanine -->

            <!-- DIO5 threshold: [v]/3.3*256 -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_trig_in"> 40 </Register>
            <!-- set to 0 for rising edge, 1 for falling -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_edge"> 0 </Register>
            <!-- Leave as default -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_50ohms" > 1 </Register>
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_out_50ohms"> 0 </Register>
            <!-- set to 1 to output L1A signal, 0 for input pulse -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_lemo2_sig_sel"> 1 </Register>

            <!-- Clock -->
            <!-- set to 1 for external clocking -->
            <Register name="user_wb_ttc_fmc_regs.dio5.clk_mux_sel"> 0 </Register>
            <!-- DIO5 threshold: [v]/3.3*256 -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_clk_in"> 40 </Register>
            <!-- Leave as default -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_in_50ohms"> 1 </Register>
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_out_50ohms"> 0 </Register>

           <!-- End of DIO5 registers -->
            <!-- ************************************ -->

            <!-- Acquisition -->
            <Register name="user_wb_ttc_fmc_regs.pc_commands.ACQ_MODE"> 1 </Register>
            <Register name="cbc_stubdata_latency_adjust_fe1"> 1 </Register>
            <Register name="cbc_stubdata_latency_adjust_fe2"> 1 </Register>
            <Register name="user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_GENE"> 1 </Register>
            <Register name="user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_PACKET_NUMBER"> 10 </Register>
            <Register name="user_wb_ttc_fmc_regs.pc_commands2.clock_shift"> 0 </Register>

            <!-- Polarity -->
            <Register name="user_wb_ttc_fmc_regs.pc_commands2.negative_logic_CBC"> 0 </Register>
            <Register name="user_wb_ttc_fmc_regs.pc_commands2.negative_logic_sTTS"> 0 </Register>
            <Register name="user_wb_ttc_fmc_regs.pc_commands2.polarity_tlu"> 0 </Register>
        </BeBoard>
    </Shelve>
</HwDescription>

<!-- Settings node to pass any std::string - int combination to applications -->
<Settings>
    <Setting name="RunNumber"> 1 </Setting>
    <!-- Hole mode from settings overrides  "negative_logic_CBC" register-->
    <Setting name="HoleMode"> 1 </Setting>
</Settings>


```

#### Example HWDescription.json File with DIO5 support:

```json
{
    "HwDescription":{
        "Connections":"file://settings/connections_2CBC.xml",
        "Shelves":[
            {
                "Id":0,
                "BeBoards":[
                    {
                        "Id":0,
                        "boardType":"Glib",
                        "connectionId":"board0",
                        "Modules":[
                            {
                                "FeId":0,
                                "FMCId":0,
                                "ModuleId":0,
                                "Status":1,
                                "Global_CBC_Registers":{
                                    "VCth":"0x78",
                                    "TriggerLatency":"0x0C"
                                },
                                "CbcFilePath":"./settings/",
                                "CBCs":[
                                    {
                                        "Id":0,
                                        "configfile":"Cbc_default_hole.txt",
                                        "Register":{
                                            "VCth":"0x78"
                                        }
                                    },
                                    {
                                        "Id":1,
                                        "configfile":"Cbc_default_hole.txt"
                                    }
                                ]
                            }
                        ],
                        "RegisterName":{
                            "COMMISSIONNING_MODE_RQ":"//COMMISSIONING MODE REGISTERS",
                            "COMMISSIONNING_MODE_RQ":"//set to 1 to enable commissioning mode",
                            "COMMISSIONNING_MODE_RQ":1,
                            "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID":"//set to 1 to enable test pulses in comissioningn mode",
                            "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID":1,
                            "COMMISSIONNING_MODE_DELAY_AFTER_FAST_RESET":50,
                            "COMMISSIONNING_MODE_DELAY_AFTER_L1A":400,
                            "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE":201,

                            "user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL":"//TRIGGER",
                            "user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL":"//set to 1 to enable external triggers",
                            "user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL":0,
                            "user_wb_ttc_fmc_regs.pc_commands.INT_TRIGGER_FREQ":10,
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_trig_in":"//DIO5 threshold: [v]/3.3*256",
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_trig_in":40,
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_edge":"//set to 0 for rising edge, 1 for falling",
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_edge":0,
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_50ohms":1,
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_out_50ohms":0,
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_lemo2_sig_sel":"//set to 1 to output L1A signal, 0 for input pulse ",
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_lemo2_sig_sel":1,

                            "user_wb_ttc_fmc_regs.dio5.clk_mux_sel":"//CLOCK",
                            "user_wb_ttc_fmc_regs.dio5.clk_mux_sel":"//set to 1 to enable external clocking",
                            "user_wb_ttc_fmc_regs.dio5.clk_mux_sel":0,
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_clk_in":"//DIO5 threshold: [v]/3.3*256",
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_clk_in":40,
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_in_50ohms":1,
                            "user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_out_50ohms":0,

                            "user_wb_ttc_fmc_regs.pc_commands.ACQ_MODE":"//ACQUISITION",
                            "user_wb_ttc_fmc_regs.pc_commands.ACQ_MODE":1,
                            "cbc_stubdata_latency_adjust_fe1":1,
                            "cbc_stubdata_latency_adjust_fe2":1,
                            "user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_GENE":1,
                            "user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_PACKET_NUMBER":10,
                            "user_wb_ttc_fmc_regs.pc_commands2.clock_shift":0,
                            
                            "user_wb_ttc_fmc_regs.pc_commands2.negative_logic_CBC":"//POLARITY",
                            "user_wb_ttc_fmc_regs.pc_commands2.negative_logic_CBC":1,
                            "user_wb_ttc_fmc_regs.pc_commands2.negative_logic_sTTS":0,
                            "user_wb_ttc_fmc_regs.pc_commands2.polarity_tlu":0
                        }
                    }
                ]
            }
        ]
    },
    "Settings":{
        "RunNumber":1,
        "HoleMode":1
    }
}
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
