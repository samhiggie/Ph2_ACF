#include <iostream>
#include "argvparser.h"
#include "CommonVisitors.h"
#include "Timer.h"
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
const std::string rst ("\033[0m");

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

    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
    bool cVcthset = cmd.foundOption ("vcth");


    std::string cResultfile = "Cbc3RadiationCycle";
    cDirectory += "Cbc3RadiationCycle";
    cDirectory += currentDateTime();

    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );

    // commented out to make sure application does not close if the canvas is closed ( useful for running multiple tests with opening/closing canvases)
    else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    //prepare the LVClient and the KeController
#ifdef __USBINST__
    Ke2110Controller* cKeController = nullptr;
    HMP4040Client* cLVClient = nullptr;
#endif


    //Start server to communicate with HMP404 instrument via usbtmc and SCPI
    bool cPSStatus, cDMMStatus;
    cPSStatus = false;
    cDMMStatus = false;

    // get current working directory
    char buffer[256];
    std::string currentDir = getcwd (buffer, sizeof (buffer) );

    int cMonitoringInterval = 2;
    //Start the server to communicate with the LV power supply
    std::string cPowerSupplyOutputFile = currentDir + "/" + cDirectory + "/Current_log.txt";
    PortsInfo cPowerSupplyPortsInfo = std::make_pair (8081, 8080);
    cPSStatus = InitializeMonitoring ( cHostname, "HMP4040", cPowerSupplyPortsInfo, cMonitoringInterval, cPowerSupplyOutputFile );

    //Start server to communicate with Keithley DMM instrument via usbtmc and SCPI
    std::string cDMMOutputFile = currentDir + "/" + cDirectory  + "/Temperature_log.txt";
    PortsInfo cDMMPortsInfo = std::make_pair (8083, 8082);
    cDMMStatus = InitializeMonitoring ( cHostname, "Ke2110", cDMMPortsInfo, cMonitoringInterval, cDMMOutputFile);


    if ( cPSStatus && cDMMStatus)  // Verify child process terminated without error.
    {
        //at the beginning of the test start the monitoring and initalize the connection to the Arduino relay controller
#ifdef __USBINST__
        cLVClient = new HMP4040Client (cHostname, cPowerSupplyPortsInfo.first );
        LOG (INFO) << BOLDBLUE << "Starting monitoring of power supply currents on the HMP4040." << RESET ;
        cLVClient->StartMonitoring();
        //make sure power supply is switched  on before doing anything else
        LOG (INFO) << BLUE << "Making sure that the power supply is ON at the start of the test!" << RESET ;
        cLVClient->ToggleOutput (1);

        cKeController = new Ke2110Controller();
        cKeController->InitializeClient ( cHostname, cDMMPortsInfo.first );
        LOG (INFO) << BOLDBLUE << "Starting monitoring of the ambient temperature on the Ke2110 DMM." << RESET ;
        cKeController->SendMonitoringStart();
#endif
        // here the startup is complete

        std::stringstream outp;

        Tool cTool;
        cTool.InitializeHw ( cHWFile, outp );
        cTool.InitializeSettings ( cHWFile, outp );
        LOG (INFO) << outp.str();
        outp.str ("");
        cTool.CreateResultDirectory ( cDirectory, false, false );
        cTool.InitResultFile (cResultfile);
        //cTool.StartHttpServer (8084);
        cTool.ConfigureHw ();

        Timer t;
        t.start();
        //then sweep a bunch of biases
        BiasSweep cBiasSweep (cLVClient, cKeController);
        cBiasSweep.Inherit (&cTool);
        cBiasSweep.Initialize();
        std::vector<std::string> cBiases{"VCth", "CAL_Vcasc", "VPLUS1", "VPLUS2", "VBGbias", "VBG_LDO", "Vpafb", "VDDA", "Nc50", "Ipa", "Ipre1", "Ipre2", "CAL_I", "Ibias", "Ipsf", "Ipaos", "Icomp", "Ihyst"};

        for (auto cBoard : cBiasSweep.fBoardVector)
        {
            for (auto cFe : cBoard->fModuleVector)
            {
                for (auto cCbc : cFe->fCbcVector)
                {
                    for (auto cBias : cBiases)
                        cBiasSweep.SweepBias (cBias, cCbc);
                }
            }
        }

        t.stop();
        t.show ( "Time to sweep all biases" );

        ////first, run offset tuning
        Calibration cCalibration;
        cCalibration.Inherit (&cTool);
        cCalibration.Initialise (false);
        cCalibration.FindVplus();
        cCalibration.FindOffsets();
        cCalibration.writeObjects();
        cCalibration.dumpConfigFiles();

        ////now run a noise scan
        PedeNoise cPedeNoise;
        cPedeNoise.Inherit (&cTool);
        cPedeNoise.ConfigureHw();
        cPedeNoise.Initialise();
        cPedeNoise.measureNoise();
        //cPedeNoise.Validate();
        cPedeNoise.writeObjects();

        //sweep the stubs before the calibration, otherwise we'll have to adapt the threshold, just to be safe
        StubSweep cStubSweep;
        cStubSweep.Inherit (&cTool);
        cStubSweep.Initialize();
        cStubSweep.SweepStubs (1);

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

        LOG (INFO) << YELLOW << "Stopping the monitoring and exiting the server for the HMP4040!" << RESET;
        //maybe also want to switch off the power supply at the end of the test
        //cLVClient->ToggleOutput (0);
        cLVClient->StopMonitoring();
        LOG (INFO) << YELLOW << "Stopping the monitoring and exiting the server for the Ke2110!" << RESET;
        cKeController->SendMonitoringStop();

        cLVClient->Quit();
        cKeController->SendQuit();
#endif

    }
    else
        LOG (ERROR) << "Either PowerSupply or DMM server did not start up correctly!";

    LOG (INFO) << GREEN << "Irradiation Test exited normally!" << RESET;

    if ( !batchMode )
        cApp.Run();

    if (cLVClient) delete cLVClient;

    if (cKeController) delete cKeController;

    return 0;
}
