//#include <cstring>
#include <iostream>
//#include <unistd.h>
//#include <limits.h>
//#include <signal.h>
//#include <chrono>
//#include <thread>
#include <sys/wait.h>
#include <sys/stat.h>
#include "boost/tokenizer.hpp"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_numeric.hpp>

//#include "../HWDescription/Cbc.h"
//#include "../HWDescription/Module.h"
//#include "../HWDescription/BeBoard.h"
//#include "../HWInterface/CbcInterface.h"
//#include "../HWInterface/BeBoardInterface.h"
//#include "../HWDescription/Definition.h"
//#include "../tools/HybridTester.h"
//#include "../tools/RegisterTester.h"
//#include "../tools/ShortFinder.h"
//#include "../tools/AntennaTester.h"
#include "../tools/Calibration.h"
#include "../tools/BiasSweep.h"
#include "../tools/PedeNoise.h"
#include "../Utils/argvparser.h"
#include "../Utils/UsbUtilities.h"
#include "../Utils/Timer.h"

#include "TROOT.h"
#include "TApplication.h"

#ifdef __USBINST__
#include <zmq.hpp>
#include "AppLock.cc"
#include "HMP4040Client.h"
#include "Ke2110Controller.h"
using namespace Ph2_UsbInst;
#endif

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

INITIALIZE_EASYLOGGINGPP

// need this to reset terminal output
const std::string rst ("\033[0m");


// Typedefs for Containers
typedef std::map<std::string, double> HMP4040_currents;
typedef std::map<std::string, double> HMP4040_voltages;
typedef std::pair< time_t, HMP4040_currents> HMP4040_measurement;
typedef std::pair<double, double> InstMeasurement;


//start monitoring the voltages and currents on the HMP4040
void startMonitoring_HMP4040 (   std::string pHostname, PortsInfo pPortsInfo, int pMeasureInterval_s)
{
#ifdef __USBINST__
    HMP4040Client* cClient = new HMP4040Client (pHostname, pPortsInfo.first);
    cClient->StartMonitoring();
    std::this_thread::sleep_for (std::chrono::seconds (pMeasureInterval_s * 2) );
    delete cClient;
#endif
}
// turn on HMP4040
void toggle_HMP4040 (   std::string pHostname, PortsInfo pPortsInfo, int pMeasureInterval_s, int cState = 0 )
{
#ifdef __USBINST__
    HMP4040Client* cClient = new HMP4040Client (pHostname, pPortsInfo.first);
    // make sure that the power supply is configured first
    cClient->ToggleOutput ( cState );
    delete cClient;
    std::this_thread::sleep_for (std::chrono::seconds (pMeasureInterval_s * 2) );
#endif
}

// measure current consumption on 4 power supply rails : LVDS + VREG + VDDA + VDDD 
HMP4040_measurement get_HMP4040currents ( std::string pHostname  , PortsInfo pPortsInfo  )
{
    HMP4040_measurement cMeasurement;
    HMP4040_currents cCurrents ;
    std::vector<std::string> cChannelList{ "LVDS", "Vreg", "Vdda", "Vddd"};

    #ifdef __USBINST__
        HMP4040Client* cClient = new HMP4040Client (pHostname, pPortsInfo.first);

        int iterations = 0 ;
        // instead force a measurement 
        // and get the latest reading from the logged using the HMP4040 monitoring function.
        cClient->MeasureValues();
        cClient->GetLatestReadValues();
        
        MeasurementValues cValues = cClient->fValues;
        cMeasurement.first = cValues.fTimestamp;
        for ( int i = 0 ; i < 4 ; i += 1 )
        {
            cCurrents.insert ( std::pair<std::string, double> ( cChannelList[i] , cValues.fCurrents.at (i) * 1e3 ) );
        }
        cMeasurement.second = cCurrents;
        delete cClient;
    #endif
    return cMeasurement;
}
double read_HMP4040analogueRail( std::string pHostname  , PortsInfo pPortsInfo )
{
    HMP4040_measurement m = get_HMP4040currents ( pHostname , pPortsInfo );
    HMP4040_currents cCurrents = m.second; 

#ifdef __USBINST__
    HMP4040Client* cClient = new HMP4040Client (pHostname, pZmqPortNumber);

    int iterations = 0 ;
    // instead force a measurement
    // and get the latest reading from the logged using the HMP4040 monitoring function.
    cClient->MeasureValues();
    cClient->GetLatestReadValues();

    MeasurementValues cValues = cClient->fValues;
    cMeasurement.first = cValues.fTimestamp;

    for ( int i = 0 ; i < 4 ; i += 1 )
    {
        cCurrents.insert ( std::pair<std::string, double> ( cChannelList[i], cValues.fCurrents.at (i) * 1e3 ) );
        //auto search = cVoltages.find (cValues.fVoltages.at (i) );
        //if (search != cVoltages.end() )
        //    cCurrents.insert (std::pair<std::string, double> (search->second , cValues.fCurrents.at (i) * 1e3 ) );
    }

    cMeasurement.second = cCurrents;
    delete cClient;
#endif
    return cMeasurement;
}
void config_KeithleyDmm()
{
#ifdef __USBINST__
    //create a controller, and immediately send a "pause" command to any running server applications
    Ke2110Controller* cKeController = new Ke2110Controller ();
    cKeController->InitializeClient ("localhost", 8083);

    //tell any server to pause the monitoring
    cKeController->SendPause();
    //are we measuring a voltage for the currents as well?
    //std::string cConfString = (pBias.find ("I") != std::string::npos) ? "CURRENT:DC" : "VOLTAGE:DC";
    std::string cConfString = "VOLTAGE:DC";
    //set up to either measure Current or Voltage, autorange, 10^-4 resolution and autozero
    cKeController->Configure (cConfString, 0, 0.0001, true);
    cKeController->Autozero();
    //tell any server to resume the monitoring
    cKeController->SendResume();
#endif
}
double read_KeithleyDmm()
{
    double cReading = 0.;
#ifdef __USBINST__
    //create a controller, and immediately send a "pause" command to any running server applications
    Ke2110Controller* cKeController = new Ke2110Controller ();
    cKeController->InitializeClient ("localhost", 8083);

    //tell any server to pause the monitoring
    cKeController->SendPause();
    // configure
    cKeController->Measure();
    cReading = cKeController->GetLatestReadValue();
    //tell any server to resume the monitoring
    cKeController->SendResume();
#endif
    return cReading;
}
// want to measure DMM and power supply at the same time ... so
// check settling time AMUX reading 
void check_AMUX(Tool* pTool , std::string pRegName , std::string pHostname = "localhost" , PortsInfo pPowerSupplyPortsInfo = defaultPorts ,  double pSettlingTime_s = 0.5 )
{
    int cNumReadings_perMeasurement = 5;
#ifdef __USBINST__
    Ke2110Controller* cKeController = new Ke2110Controller ();
    //create a controller, and immediately send a "pause" command to any running server applications
    cKeController->InitializeClient ("localhost", 8083);
#endif

    double cCurrentVdda;
    double cReading, cReadingMean, cReadingSqSum, cReadingUnc ;
    InstMeasurement cMeasurement;
    std::vector<double> cReadings, cReadingsPS ;


    BiasSweep cSweep;
    cSweep.Inherit (pTool);

    cSweep.ConfigureHw();
    cSweep.Initialize();

    for (auto cBoard : cSweep.fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            for (auto cCbc : cFe->fCbcVector)
            {
                // // Trying to understand AMUX reg + bias sweep
                // // can remove later
                //cSweep.ReadRegister(pRegName, cCbc);

                // read value at start-up
                // #ifdef __USBINST__
                //     cKeController->SendPause();
                //     cReadings.clear();
                //     for( unsigned int i = 0 ; i < cNumReadings_perMeasurement ; i++)
                //     {
                //         cKeController->Measure();
                //         cReadings.push_back( cKeController->GetLatestReadValue() ) ;
                //     }
                //     cKeController->SendResume();
                //     cMeasurement.first  = std::accumulate(cReadings.begin(), cReadings.end(), 0.0)/cReadings.size();
                //     cMeasurement.second = std::sqrt( std::inner_product(cReadings.begin(), cReadings.end(), cReadings.begin(), 0.0)/cReadings.size() - std::pow(cMeasurement.first,2.0) );
                // #endif
	           cCurrentVdda = read_HMP4040analogueRail( pHostname , pPowerSupplyPortsInfo);
                double cCuurent_orig = cCurrentVdda;
                // //InstMeasurement cAMUX_origReading = cMeasurement;
                LOG (DEBUG) << BOLDMAGENTA << "\t\t HMP4040 Reading [ before switching AMUX ] = " << (cCurrentVdda / 254) * 1e3 <<  " uA/channel ." << RESET ;
                // //LOG (DEBUG) << BOLDMAGENTA << "\t\t DMM Reading [ before switching AMUX ] = " << (cAMUX_origReading.first)*1e3 << " ± "  << cAMUX_origReading.second*1e3 <<  " mV." << RESET ;

                // // // configure AMUX register and read DMM
                // LOG (DEBUG) << BOLDMAGENTA << "Configuring AMUX register" << RESET ;

                uint8_t cOriginalRegValue = cSweep.ReadRegister(pRegName, cCbc);
                uint8_t cOriginalAmuxValue =  cSweep.ConfigureAMUX(pRegName, cCbc, pSettlingTime_s);
                LOG (INFO) << "Original mux value =  0x" << std::hex << +cOriginalAmuxValue << std::dec << " --- " << pRegName << " set to : 0x" <<  std::hex << +cOriginalRegValue  ; 
                cCurrentVdda = read_HMP4040analogueRail( pHostname , pPowerSupplyPortsInfo);

                // // InstMeasurement cAMUX_reading1 = cMeasurement;
                LOG (DEBUG) << BOLDMAGENTA << "\t\t HMP4040 Reading [ after configuring AMUX ] = " << (cCurrentVdda / 254) * 1e3 <<  " uA/channel ." << RESET ;
                // // LOG (DEBUG) << BOLDMAGENTA << "\t\t DMM Reading [ after configuring AMUX ] = " << (cAMUX_reading1.first)*1e3 << " ± "  << cAMUX_reading1.second*1e3 <<  " mV." << RESET ;

                uint16_t cRange  = 16;//255
                double cCurrent_pWrite;

                for (uint8_t cRegValue = 0; cRegValue <= cRange; cRegValue++)
                {
                    if ( cRegValue % 2 == 0 )
                    {
                        //uint8_t cRegValue = (0) ;

                        cSweep.WriteRegister(pRegName , cRegValue , cCbc, pSettlingTime_s);
                        cCurrentVdda = read_HMP4040analogueRail( pHostname , pPowerSupplyPortsInfo);

                        cCurrent_pWrite = cCurrentVdda;
                        LOG (DEBUG) << BOLDMAGENTA << "\t\t HMP4040 Reading [ after setting register to " << std::hex << +cRegValue << " ] = " << (cCurrentVdda / 254) * 1e3 <<  " uA/channel ." << RESET ;
                    }
                }

                // LOG (DEBUG) << "\t\t DMM Reading [ setting to 0 ] = " << (cAMUX_reading1)*1e3 << " mV.";

                // re-set AMUX register to original value and read DMM again
                LOG (DEBUG) << BOLDMAGENTA << "Resetting " << pRegName << " to original value." << RESET ;

                cSweep.WriteRegister(pRegName , cOriginalRegValue , cCbc, pSettlingTime_s);
                cCurrentVdda = read_HMP4040analogueRail( pHostname , pPowerSupplyPortsInfo);
                LOG (DEBUG) << BOLDMAGENTA << "\t\t HMP4040 Reading [ after re-setting register value ] = " << (cCurrentVdda/254)*1e3 <<  " uA/channel ." << RESET ;
                
                LOG (DEBUG) << BOLDMAGENTA << "Resetting AMUX register" << RESET ;
                cSweep.ResetAMUX( cOriginalAmuxValue , cCbc, pSettlingTime_s);
                cCurrentVdda = read_HMP4040analogueRail( pHostname , pPowerSupplyPortsInfo);


                // // InstMeasurement cAMUX_reading2 = cMeasurement ;
                LOG (DEBUG) << BOLDMAGENTA << "\t\t HMP4040 Reading [ after resetting AMUX ] = " << (cCurrentVdda / 254) * 1e3 <<  " uA/channel ." << RESET ;
                // // LOG (DEBUG) << BOLDMAGENTA << "\t\t DMM Reading [ after resetting AMUX ] = " << (cAMUX_reading2.first)*1e3 << " ± "  << cAMUX_reading2.second*1e3 << " mV."  << RESET ;
                // //LOG (INFO) << MAGENTA << "DMM Reading [ after configuration/after switching AMUX/after resetting AMUX ] :\n\t\t\t" << cAMUX_origReading << "\t" << cAMUX_reading1 << "\t" << cAMUX_reading2 << " ." << RESET ;

                LOG (DEBUG) << BOLDMAGENTA << "\t\t HMP4040 Reading [ initially ] = " <<  (cCuurent_orig / 254) * 1e3 << " --->  "  << (cCurrent_pWrite / 254) * 1e3 << " after write."  << RESET ;

            }
        }
    }

#ifdef __USBINST__
    cKeController->SendResume();
#endif

    cSweep.SaveResults();
}
// sweep bias settings on CBC3
void perform_BiasSweep (Tool* pTool)
{
    std::vector<std::string> RegToSweep{"Icomp", "Ipa", "Ipaos", "Ipre1", "Ipre2", "Ipsf", "CAL_Vcasc", "Vth", "Vplus1"};

    std::string cRegName = "CAL_I";
    LOG (INFO) << "Starting bias sweep for CBC3." ;

    BiasSweep cSweep;
    cSweep.Inherit (pTool);

    cSweep.ConfigureHw();
    cSweep.Initialize();

    for (auto cBoard : cSweep.fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            for (auto cCbc : cFe->fCbcVector)
            {
                for ( unsigned int i = 0 ; i < RegToSweep.size(); i++ )
                    cSweep.SweepBias (RegToSweep[i], cCbc);

                //cSweep.ReadRegister("Icomp", cCbc);
                //cSweep.SweepBias (cRegName, cCbc);
                //cSweep.SweepBias ("Vth", cCbc);
            }
        }
    }

    cSweep.SaveResults();
}

// perform the CBC Vplus, Voffset calibration
void perform_Calibration (Tool* pTool)
{
    Calibration cCalibration;
    cCalibration.Inherit (pTool);
    cCalibration.Initialise ( false );

    cCalibration.FindVplus();
    cCalibration.FindOffsets();
    cCalibration.SaveResults();
    cCalibration.dumpConfigFiles();
}

int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);


    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  Integrated validation test performs the following actions:");
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/Cbc3HWDescription.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "output", "Output Directory . Default value: Results/Irrad_Xrays", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );

    // cmd.defineOption ( "hport", "Port number on which to run THttpServer for HMP4040 logger., Default: 8080", ArgvParser::OptionRequiresValue );
    // cmd.defineOptionAlternative ( "hport", "p" );

    // cmd.defineOption ( "cport", "Port number on which to run ZMQ server for HMP4040 client., Default: 8081", ArgvParser::OptionRequiresValue );
    // cmd.defineOptionAlternative ( "cport", "r" );

    // cmd.defineOption ( "settingsHMP4040", "Hw Description File for HMP4040. Default value: ../Ph2_USBInstDriver/settings/HMP4040.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    // cmd.defineOptionAlternative ( "settingsHMP4040", "s" );

    // cmd.defineOption ( "logHMP4040", "Save the data to a text file. Default value: LV_log.txt", ArgvParser::OptionRequiresValue );
    // cmd.defineOptionAlternative ( "logHMP4040", "l" );

    // cmd.defineOption ( "interval", "Read Interval for the Power supply monitoring in seconds, Default: 2s", ArgvParser::OptionRequiresValue );
    // cmd.defineOptionAlternative ( "interval", "i" );

    cmd.defineOption ( "Currents", "Monitor the current consumption of the CBC3 on the 4 power supply rails [2.0 V , 3.3 V , 1.2 V (Analogue) , 1.2 V (Digital)].", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "Currents", "c" );

    cmd.defineOption ( "Sweep", "Sweep the bias settings on the CBC3 using the on-chip AMUX register and a Keithley DMM.", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "Sweep", "s" );

    std::string pOptionDescription;
    pOptionDescription = "Configure AMUX register on CBC3 to monitor a given register and read value on Keithley DMM.<value> is the name of a valid AMUX register.";
    pOptionDescription += "\nValid registers : [none, Ipa, Ipre2, CAL_I, Ibias, Vth, VBGbias, VBG_LDO, Vpafb, Nc50, Ipre1, Ipsf, Ipaos, Icomp, Ihyst, CAL_Vcasc, VPLUS2, VPLUS1].";
    cmd.defineOption ( "AMUX", pOptionDescription, ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "AMUX", "a" );

    // cmd.defineOption ( "checkRegisters", "Check WRITE/READ to and from registers on DUT.", ArgvParser::NoOptionAttribute );

    // cmd.defineOption ( "calibrate", "Calibration CBCs' Vplus/Voffset on DUT.", ArgvParser::NoOptionAttribute );

    // cmd.defineOption ( "checkShorts", "Identify shorts on DUT.", ArgvParser::NoOptionAttribute );

    // cmd.defineOption ( "measureOccupancy", "Measure (NOISE) occuapncy on DUT.", ArgvParser::NoOptionAttribute );

    // cmd.defineOption ( "antennaTest", "Measure occuapncy on the DUT using the Antenna.", ArgvParser::NoOptionAttribute );

    cmd.defineOption ( "all", "Do everything : monitor currents ..... ", ArgvParser::NoOptionAttribute );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/Cbc3HWDescription.xml";
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/Irrad_Xrays";
    //int cNumCBCs = ( cmd.foundOption ( "numCBCs" ) ) ? atoi (cmd.optionValue ( "numCBCs" ).c_str() ) : 2;

    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;

    // configure the power supply monitor (HMP4040)
    // hard code the options for now
    std::string cHostname = "localhost";
    int cPowerSupplyHttpPortNumber = 8080;
    //TODO
    // get from settings file
    int cPowerSupplyZmqPortNumber = 8090;
    std::string cPowerSupplyHWFile = "HMP4040_cbc3.xml";
    std::string cPowerSupplyOutputFile = cDirectory + "/Current_log.txt";
    int cPowerSupplyInterval = 1;
    PortsInfo cPowerSupplyPortsInfo = std::pair<int, int> (cPowerSupplyZmqPortNumber, cPowerSupplyHttpPortNumber);


    // check for AMUX monitor tester
    std::string cAmuxRegName = (cmd.foundOption ("AMUX") ) ? cmd.optionValue ("AMUX") : "";

    // comment out for now
    // std::string cHostname = (cmd.foundOption ("hostname") ) ? cmd.optionValue ("hostname") : "localhost";
    // int httpPortNumber = ( cmd.foundOption ( "hport" ) ) ? atoi (cmd.optionValue ( "hport" ).c_str() ) : 8080;
    // int zmqPortNumber = ( cmd.foundOption ( "cport" ) ) ? atoi (cmd.optionValue ( "cport" ).c_str() ) : 8081;
    // std::string cPowerSupplyHWFile = ( cmd.foundOption ( "settingsHMP4040" ) ) ? cmd.optionValue ( "settingsHMP4040" ) : "../Ph2_USBInstDriver/settings/HMP4040.xml";
    // std::string cPowerSupplyOutputFile = ( cmd.foundOption ( "logHMP4040" ) ) ? cmd.optionValue ( "logHMP4040" ) : "LV_log.txt";
    // int cInterval = ( cmd.foundOption ( "interval" ) ) ? atoi (cmd.optionValue ( "interval" ).c_str() ) : 2;

    bool cCurrents = ( cmd.foundOption ( "Currents" ) ) ? true : false;
    bool cBiasSweep = ( cmd.foundOption ( "Sweep" ) ) ? true : false;
    bool cAmuxTest = ( cmd.foundOption ( "AMUX" ) ) ? true : false;

    // bool cRegisters = ( cmd.foundOption ( "checkRegisters" ) ) ? true : false;
    // bool cCalibrate = ( cmd.foundOption ( "calibrate" ) ) ? true : false;
    // bool cShorts = ( cmd.foundOption ( "checkShorts" ) ) ? true : false;
    // bool cOccupancy = ( cmd.foundOption ( "measureOccupancy" ) ) ? true : false;
    // bool cAntennaMeasurement  = ( cmd.foundOption ( "antennaTest" ) ) ? true : false;

    bool cAll = ( cmd.foundOption ( "all" ) ) ? true : false;


    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );
    // commented out to make sure application does not close if the canvas is closed ( useful for running multiple tests with opening/closing canvases)
    else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );


    // set all test flags ON if all flag set in arguments passed to tester.
    if ( cAll )
    {
        cCurrents = true;
        cBiasSweep = true;
        //cRegisters = true;
        //cCalibrate = true;
        //cShorts = true ;
        //cOccupancy = true;
    }

    //Start server to communicate with HMP404 instrument via usbtmc and SCPI
    pid_t childPid;  // the child process that the execution will soon run inside of.
    childPid = fork();

    if (childPid < 0) // fork failed
    {
        // log the error
        exit (1);
    }
    else if (childPid == 0) // fork succeeded
    {
        // launch HMP4040 server
        if ( cCurrents )
        {
            LOG (INFO) << BOLDBLUE << "Trying to launch server to monitor currents on the HMP4040" << RESET ;
            launch_Server ( cPowerSupplyHWFile, cHostname, cPowerSupplyPortsInfo, cPowerSupplyInterval );
            exit (0);
        }
    }
    else  // Main (parent) process after fork succeeds
    {
        int returnStatus = -1 ;
        waitpid (childPid, &returnStatus, 0); // Parent process waits here for child to terminate.

        if (returnStatus == 0)  // Verify child process terminated without error.
        {
            if ( cCurrents )
            {
                LOG (INFO) << BOLDBLUE << "Starting monitoring of power supply currents on the HMP4040" << RESET ;
                startMonitoring_HMP4040 ( cHostname, cPowerSupplyPortsInfo, cPowerSupplyInterval );
            }

            // make sure power supply is switched  on before doing anything else
            LOG (INFO) << BOLDBLUE << "Switching on the power supply" << RESET ;
            toggle_HMP4040 ( cHostname, cPowerSupplyPortsInfo, cPowerSupplyInterval, 1 );

            Timer tGlobal;
            tGlobal.start();
            Timer t;

            // LOG (INFO) << "I'm trying to sleep...." ;
            // std::this_thread::sleep_for(std::chrono::milliseconds( 10000 ));
            // LOG (INFO) << "I'm awake again...." ;

            std::stringstream outp;
            char line[120];

            //create a genereic Tool Object, I can then construct all other tools from that using the Inherit() method
            //this tool stays on the stack and lives until main finishes - all other tools will update the HWStructure from cTool
            Tool cTool;
            cTool.InitializeHw ( cHWFile, outp );
            cTool.InitializeSettings ( cHWFile, outp );
            LOG (INFO) << outp.str();
            outp.str ("");
            cTool.CreateResultDirectory ( cDirectory );
            cTool.InitResultFile ( "Irrad_Xrays" );
            cTool.StartHttpServer();
            cTool.ConfigureHw ();
            cTool.CreateReport();

            // // this is where I do stuff....
            if ( cAmuxTest )
            {
                LOG (INFO) << MAGENTA << "Performing test/monitor on AMUX register using " << cAmuxRegName << RESET ;
                t.start();
                // check AMUX register behaves as expected
                //check_AMUX( &cTool , cAmuxRegName , cHostname , cPowerSupplyZmqPortNumber , cPowerSupplyHttpPortNumber , 0.2 );
                t.stop();
                LOG (INFO) << MAGENTA << "AMUX register monitor/tester finished." << RESET ;
                sprintf (line, "# %.3f s required to read defaul value of %s using the AMUX register on CBC3.", cAmuxRegName.c_str(), t.getElapsedTime() );
                cTool.AmmendReport ( line);
                t.show ( "Bias sweep of the CBC3");

            }

            // first perform bias sweep
            if ( cBiasSweep )
            {
                LOG (INFO) << MAGENTA << "Performing sweep of bias settings on CBC3." ;
                t.start();
                //perform_BiasSweep(&cTool);
                t.stop();
                LOG (INFO) << MAGENTA << "Bias sweep finished." ;
                sprintf (line, "# %.3f s required to sweep bias settings on CBC3.", t.getElapsedTime() );
                cTool.AmmendReport ( line);
                t.show ( "Bias sweep of the CBC3");
            }

            tGlobal.stop();
            tGlobal.show ( "Complete irradiation measurement cycle." );
            sprintf (line, "# %.3f s required to complete one measurement cycle for a CBC3.", tGlobal.getElapsedTime() );
            cTool.AmmendReport ( line);

            // have to destroy the tool at the end of the program
            //cTool.SaveResults(); // if this is here then you end up with 2 copies of all histograms and canvases in the root file ... so I've removed it
            cTool.CloseResultFile();
            cTool.Destroy();

            if ( !batchMode ) cApp.Run();
        }
        else if (returnStatus == 1)
        {
            LOG (INFO) << "The child process terminated with an error!." ;
            exit (1);
        }
    }

    return 0;
}
