#include <iostream>
#include "argvparser.h"
#include "CommonVisitors.h"
#include "Timer.h"
#include "Watchdog.h"
#include "UsbUtilities.h"

#include "Calibration.h"
#include "BiasSweep.h"
#include "StubSweep.h"
#include "PedeNoise.h"

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
//const std::string rst ("\033[0m");
void exitme()
{
    LOG (ERROR) << BOLDRED << "Quitting application due to watchdog timeout - something must have got stuck!" << RESET;
    exit (1);
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

    cmd.defineOption ( "events", "minimum Number of Events . Default value: 300k for this application", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "events", "e" );

    cmd.defineOption ( "vcth", "Threshold in VCth units (hex (including 0x) or decimal) . Default values from HW description .XML file", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "vcth", "v" );

    cmd.defineOption ( "dqm", "Print every i-th event.  ", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "dqm", "d" );

    cmd.defineOption ( "hostname", "Hostname of the machine that runs the LV and Temperature monitoring servers. Default: localhost", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "hostname", "H" );

    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );

    cmd.defineOption ( "standalone", "launch irradtest without spawning the monitoring servers from code", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "standalone", "s" );

    cmd.defineOption ( "skipbias", "skip the bias sweep - false by default!", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    //cmd.defineOptionAlternative ( "standalone", "s" );

    //cmd.defineOption ( "webmonitor", "start THttpServer on port - default: 8090", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
    //cmd.defineOptionAlternative ( "webmonitor", "w" );

    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/Cbc3HWDescription.xml";
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/Irrad_Xrays/";
    std::string cHostname = (cmd.foundOption ( "hostname" ) ) ? cmd.optionValue ("hostname") : "localhost";

    uint32_t pEventsperVcth = ( cmd.foundOption ( "events" ) ) ? convertAnyInt ( cmd.optionValue ( "events" ).c_str() ) : 300000;
    uint16_t cVcth = ( cmd.foundOption ( "vcth" ) ) ? convertAnyInt ( cmd.optionValue ( "vcth" ).c_str() ) : 500;
    //uint32_t cHttpPort = (cmd.foundOption ("webmonitor") ) ? convertAnyInt (cmd.optionValue ("webmonitor").c_str()) : 8090;

    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
    bool cStandalone = ( cmd.foundOption ( "standalone" ) ) ? true : false;
    bool cVcthset = cmd.foundOption ("vcth");
    bool cSkipbias = (cmd.foundOption ("skipbias") ) ? true : false;
    //bool cWebMon = (cmd.foundOption ("webmonitor")) ? true : false;

    Watchdog cDog;
    cDog.Start (5, &exitme);

    std::string cResultfile = "Cbc3RadiationCycle";
    cDirectory += "Cbc3RadiationCycle";
    cDirectory += currentDateTime();

    LOG (INFO)  << "Creating directory: " << cDirectory;
    std::string cCommand = "mkdir -p " + cDirectory;
    system ( cCommand.c_str() );

    //now make sure that the log file ends up where I want it
    std::string cLogFileName = cDirectory + "/CbcDAQ.log";
    el::Loggers::reconfigureAllLoggers (el::ConfigurationType::Filename, cLogFileName);
    LOG (INFO) << BLUE << "Changing log file to: " << MAGENTA << cLogFileName << RESET;

    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );
    else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    //prepare the LVClient and the KeController
#ifdef __USBINST__
    Ke2110Controller* cKeController = nullptr;
    HMP4040Client* cLVClient = nullptr;
#endif

    cDog.Reset (15);

    //Start server to communicate with HMP404 instrument via usbtmc and SCPI
    bool cPSStatus, cDMMStatus;

    // get current working directory
    char buffer[256];
    std::string currentDir = getcwd (buffer, sizeof (buffer) );

    std::string cPowerSupplyOutputFile = currentDir + "/" + cDirectory + "/Current_log.txt";
    PortsInfo cPowerSupplyPortsInfo = std::make_pair (8081, 8080);
    std::string cDMMOutputFile = currentDir + "/" + cDirectory  + "/Temperature_log.txt";
    PortsInfo cDMMPortsInfo = std::make_pair (8083, 8082);

    if (!cStandalone)
    {
        cPSStatus = false;
        cDMMStatus = false;

        int cMonitoringInterval = 2;
        //Start the server to communicate with the LV power supply
        cPSStatus = InitializeMonitoring ( cHostname, "HMP4040", cPowerSupplyPortsInfo, cMonitoringInterval );

        //Start server to communicate with Keithley DMM instrument via usbtmc and SCPI
        cDMMStatus = InitializeMonitoring ( cHostname, "Ke2110", cDMMPortsInfo, cMonitoringInterval);
    }
    else
    {
        LOG (INFO) << BOLDBLUE << "Standalone option set, thus requiring servers to be running already!" << RESET;
        cPSStatus = true;
        cDMMStatus = true;
    }

    if ( cPSStatus && cDMMStatus)  // Verify child process terminated without error.
    {
        cDog.Reset (15);
        //at the beginning of the test start the monitoring and initalize the connection to the Arduino relay controller
#ifdef __USBINST__
        cLVClient = new HMP4040Client (cHostname, cPowerSupplyPortsInfo.first );
        LOG (INFO) << BOLDBLUE << "Starting monitoring of power supply currents on the HMP4040." << RESET ;
        cLVClient->SetLogFileName (cPowerSupplyOutputFile);
        cLVClient->StartMonitoring();
        //make sure power supply is switched  on before doing anything else
        LOG (INFO) << BLUE << "Making sure that the power supply is ON at the start of the test!" << RESET ;
        cLVClient->ToggleOutput (1);

        cKeController = new Ke2110Controller();
        cKeController->InitializeClient ( cHostname, cDMMPortsInfo.first );
        LOG (INFO) << BOLDBLUE << "Starting monitoring of the ambient temperature on the Ke2110 DMM." << RESET ;
        cKeController->SendLogFileName (cDMMOutputFile);
        cKeController->SendMonitoringStart();
#endif
        // here the startup is complete

        cDog.Reset (5);
        std::stringstream outp;

        Tool cTool;
        cTool.InitializeHw ( cHWFile, outp );
        cTool.InitializeSettings ( cHWFile, outp );
        LOG (INFO) << outp.str();
        outp.str ("");
        cTool.CreateResultDirectory ( cDirectory, false, false );
        cTool.InitResultFile (cResultfile);

        //if (cWebMon) cTool.StartHttpServer (cHttpPort);
        //cTool.StartHttpServer (8090);

        cTool.ConfigureHw ();

        cDog.Reset (5);

        if (!cSkipbias)
        {
            Timer t;
            //then sweep a bunch of biases
            BiasSweep cBiasSweep (cLVClient, cKeController);
            cBiasSweep.Inherit (&cTool);
            cBiasSweep.Initialize();
            cDog.Reset (5);
            std::vector<std::string> cBiases{"VCth", "CAL_Vcasc", "VPLUS1", "VPLUS2", "VBGbias", "VBG_LDO", "Vpafb", "VDDA", "Nc50", "Ipa", "Ipre1", "Ipre2", "CAL_I", "Ibias", "Ipsf", "Ipaos", "Icomp", "Ihyst"};

            for (auto cBoard : cBiasSweep.fBoardVector)
            {
                for (auto cFe : cBoard->fModuleVector)
                {
                    for (auto cCbc : cFe->fCbcVector)
                    {
                        for (auto cBias : cBiases)
                        {
                            cDog.Reset (160);
                            t.start();
                            cBiasSweep.SweepBias (cBias, cCbc);
                            t.stop();
                            t.show ("Time for this bias");
                        }
                    }
                }
            }
        }

        cDog.Reset (50);

        //t.stop();
        //t.show ( "Time to sweep all biases" );

        ////first, run offset tuning
        Calibration cCalibration;
        cCalibration.Inherit (&cTool);
        cCalibration.Initialise (false);
        cCalibration.FindVplus();
        cCalibration.FindOffsets();
        cCalibration.writeObjects();
        cCalibration.dumpConfigFiles();

        cDog.Reset (70);

        ////now run a noise scan
        PedeNoise cPedeNoise;
        cPedeNoise.Inherit (&cTool);
        cPedeNoise.ConfigureHw();
        cPedeNoise.Initialise();
        cPedeNoise.measureNoise();
        //cPedeNoise.Validate();
        cPedeNoise.writeObjects();

        //sweep the stubs before the calibration, otherwise we'll have to adapt the threshold, just to be safe
        cDog.Reset (50);

        StubSweep cStubSweep;
        cStubSweep.Inherit (&cTool);
        cStubSweep.Initialize();
        cStubSweep.SweepStubs (1);

        cDog.Reset (100);

        //now take some data and save the binary files
        std::string cBinaryDataFileName = cDirectory + "/DAQ_data.raw";
        cTool.addFileHandler (cBinaryDataFileName, 'w');
        cTool.initializeFileHandler();
        cTool.ConfigureHw();
        BeBoard* pBoard = cTool.fBoardVector.at ( 0 );
        uint32_t cN = 1;
        uint32_t cNthAcq = 0;
        uint32_t count = 0;

        ThresholdVisitor cVisitor (cTool.fCbcInterface, 0);

        if (cVcthset)
        {
            cVisitor.setThreshold (cVcth);
            cTool.accept (cVisitor);
        }

        cTool.fBeBoardInterface->Start ( pBoard );

        while ( cN <= pEventsperVcth )
        {
            uint32_t cPacketSize = cTool.ReadData ( pBoard );

            if ( cN + cPacketSize >= pEventsperVcth )
                cTool.fBeBoardInterface->Stop ( pBoard );

            //if (cN % 10000 == 0)
            //{
            //cTool.fBeBoardInterface->Stop (pBoard);
            //cTool.fBeBoardInterface->FindPhase (pBoard);
            //cTool.fBeBoardInterface->Start (pBoard);
            //}

            const std::vector<Event*>& events = cTool.GetEvents ( pBoard );

            for ( auto& ev : events )
            {
                count++;
                cN++;

                if ( cmd.foundOption ( "dqm" ) )
                {
                    if ( count % atoi ( cmd.optionValue ( "dqm" ).c_str() ) == 0 )
                    {
                        LOG (INFO) << ">>> Event #" << count ;
                        outp.str ("");
                        outp << *ev << std::endl;
                        LOG (INFO) << outp.str();
                    }
                }

                if ( count % 1000  == 0 )
                    LOG (INFO) << ">>> Recorded Event #" << count ;

            }

            cNthAcq++;
        }

        cTool.SaveResults();
        cTool.CloseResultFile();
        cTool.Destroy();

        //finished everything we want to do, so turn everything off and be happy
#ifdef __USBINST__

        //LOG (INFO) << YELLOW << "Stopping the monitoring and exiting the server for the HMP4040!" << RESET;
        //maybe also want to switch off the power supply at the end of the test
        //cLVClient->ToggleOutput (0);
        //cLVClient->StopMonitoring();
        //LOG (INFO) << YELLOW << "Stopping the monitoring and exiting the server for the Ke2110!" << RESET;
        //cKeController->SendMonitoringStop();

        //cLVClient->Quit();
        //cKeController->SendQuit();
#endif

    }
    else
    {
        LOG (ERROR) << "Either PowerSupply or DMM server did not start up correctly!";
        cDog.Stop();
    }

    cDog.Stop();

    if ( !batchMode )
        cApp.Run();

    if (cLVClient) delete cLVClient;

    if (cKeController) delete cKeController;

    LOG (INFO) << GREEN << "Irradiation Test exited normally!" << RESET;

    return 0;
}
