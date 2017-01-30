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

#include "Calibration.h"
#include "BiasSweep.h"
#include "PedeNoise.h"
#include "argvparser.h"
#include "UsbUtilities.h"
#include "Timer.h"

#include "TROOT.h"
#include "TApplication.h"

#ifdef __USBINST__
#include <zmq.hpp>
#include "AppLock.cc"
#include "HMP4040Client.h"
#include "Ke2110Controller.h"
#include "ArdNanoController.h"
using namespace Ph2_UsbInst;
#endif

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

INITIALIZE_EASYLOGGINGPP

// need this to reset terminal output
const std::string rst ("\033[0m");

// sweep bias settings on CBC3
void sweepBias (Tool* pTool)
{
    std::vector<std::string> RegToSweep{"Icomp", "Ipa", "Ipaos", "Ipre1", "Ipre2", "Ipsf", "CAL_Vcasc", "Vth", "Vplus1"};
}
// perform the CBC Vplus, Voffset calibration
void calibrate (Tool* pTool)
{
}
// take data 
void takeData (Tool* pTool , uint32_t pNevents )
{

}
// check stub and bend information on CBC3 
void sweepStubs(Tool* pTool)
{

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

    cmd.defineOption ( "events", "Number of Events . Default value: 10", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "events", "e" );

    cmd.defineOption ( "sweep", "test the bias sweep tool", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "sweep", "s" );

    cmd.defineOption ( "stubs", "test the stub sweep tool", ArgvParser::NoOptionAttribute );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/Cbc3HWDescription.xml";
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/Irrad_Xrays";
    std::string cHostname = "localhost";


    // check for AMUX monitor tester
    std::string cAmuxRegName = (cmd.foundOption ("AMUX") ) ? cmd.optionValue ("AMUX") : "";
    bool cBiasSweep = ( cmd.foundOption ( "sweep" ) ) ? true : false;
    bool cStubSweep = ( cmd.foundOption ( "stubs" ) ) ? true : false;
    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
    
    
    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );
    // commented out to make sure application does not close if the canvas is closed ( useful for running multiple tests with opening/closing canvases)
    //else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );


    //Start server to communicate with HMP404 instrument via usbtmc and SCPI
    bool cPSStatus, cDMMStatus;
    cPSStatus = false; 
    cDMMStatus = false; 

    // get current working directory
    char buffer[256];
    std::string currentDir = getcwd (buffer, sizeof (buffer) );

    // should we monitor everything with the same interval? maybe easier 
    int cMonitoringInterval = 2; 
    std::string cPowerSupplyOutputFile = currentDir + "/" + cDirectory + "/Current_log.txt";
    int cPowerSupplyInterval = cMonitoringInterval;
    PortsInfo cPowerSupplyPortsInfo;
    cPSStatus = InitializeMonitoring( cHostname , "HMP4040" , cPowerSupplyPortsInfo , cPowerSupplyInterval , cPowerSupplyOutputFile );
    
    //Start server to communicate with Keithley DMM instrument via usbtmc and SCPI
    std::string cDMMOutputFile = currentDir + "/" + cDirectory  + "/Temperature_log.txt";
    int cDMMInterval = cMonitoringInterval ;
    PortsInfo cDMMPortsInfo;
    cDMMStatus = InitializeMonitoring( cHostname , "Ke2110" , cDMMPortsInfo  , cDMMInterval , cDMMOutputFile);

    // so this seems to work
    // but funny output from HMP4040 client 
    // and still WEIRD problem with root 
    //Start connection to Arduino ...
    bool cAsync = true;
    bool cMultex = false; 
    ArdNanoController* cArduinoController = new ArdNanoController(cAsync,cMultex);
    bool cState = cArduinoController->CheckArduinoState();

    if ( cPSStatus && cDMMStatus && cState )  // Verify child process terminated without error.
    {
        //at the beginning of the test start the monitoring and initalize the connection to the Arduino relay controller
        #ifdef __USBINST__
            LOG (INFO) << BOLDBLUE << "Arduino test - turning on LED" << RESET ;
            cArduinoController->ControlLED(1);
            
            HMP4040Client* cLVClient = new HMP4040Client (cHostname, cPowerSupplyPortsInfo.first );
            LOG (INFO) << BOLDBLUE << "Starting monitoring of power supply currents on the HMP4040." << RESET ;
            cLVClient->StartMonitoring();
            //make sure power supply is switched  on before doing anything else
            LOG (INFO) << BLUE << "Making sure that the power supply is ON at the start of the test!" << RESET ;
            cLVClient->ToggleOutput (1);
           
            Ke2110Controller* cKeController = new Ke2110Controller();
            cKeController->InitializeClient( cHostname , cDMMPortsInfo.first ); 
            LOG (INFO) << BOLDBLUE << "Starting monitoring of the ambient temperature on the Ke2110 DMM." << RESET ;
            cKeController->SendMonitoringStart();
            
            std::this_thread::sleep_for (std::chrono::seconds (2*cMonitoringInterval) );
        #endif

        LOG (DEBUG) << "Waiting here for 5x monitoring interval...."; 
        std::this_thread::sleep_for (std::chrono::seconds (2*cMonitoringInterval) );

        // at the end of the test stop the monitoring and close the connection to the Arduino relay controller 
        #ifdef __USBINST__
            LOG (INFO) << YELLOW << "Arduino test - turning off LED" << RESET ;
            cArduinoController->ControlLED(0);
            
            LOG (INFO) << YELLOW << "Stopping the monitoring and exiting the server for the HMP4040!" << RESET;
            //maybe also want to switch off the power supply at the end of the test
            //cLVClient->ToggleOutput (0);
            cLVClient->StopMonitoring();
            LOG (INFO) << YELLOW << "Stopping the monitoring and exiting the server for the Ke2110!" << RESET;
            cKeController->SendMonitoringStop();
            
            std::this_thread::sleep_for (std::chrono::seconds (2*cMonitoringInterval) );
            cLVClient->Quit();
            cKeController->SendQuit();
        #endif

    }
    else
         LOG (ERROR) << "Either PowerSupply or DMM server did not start up correctly!";


    //if ( !batchMode ) cApp.Run();
    return 0;
}
