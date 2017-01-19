#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/argvparser.h"
#include "../Utils/Timer.h"
#include "../Utils/UsbUtilities.h"

#include "../tools/BiasSweep.h"
#include "TROOT.h"
#include "TApplication.h"
#include <sys/wait.h>

#ifdef __USBINST__
#include <zmq.hpp>
//#include "../../Ph2_USBInstDriver/Utils/zmqutils.h"
#include "AppLock.cc"
#include "HMP4040Controller.h"
#include "HMP4040Client.h"
#include "Ke2110Controller.h"
using namespace Ph2_UsbInst;
#endif
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP



int main ( int argc, char** argv )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  system test application" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/HWDescription_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "output", "Output Directory . Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );

    cmd.defineOption ( "sweep", "test the bias sweep tool", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "sweep", "s" );

    cmd.defineOption ( "events", "Number of Events . Default value: 10", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "events", "e" );

    cmd.defineOption ( "vcth", "Threshold in VCth units (hex (including 0x) or decimal) . Default values from HW description .XML file", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "vcth", "v" );

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
    //bool cConfigure = ( cmd.foundOption ( "configure" ) ) ? true : false;
    uint32_t pEventsperVcth = ( cmd.foundOption ( "events" ) ) ? convertAnyInt ( cmd.optionValue ( "events" ).c_str() ) : 10;
    bool cVcthset = cmd.foundOption ("vcth");
    uint16_t cVcth = ( cmd.foundOption ( "vcth" ) ) ? convertAnyInt ( cmd.optionValue ( "vcth" ).c_str() ) : 200;
    std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
    cDirectory += "BiasSweep";

    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
    bool cSweep = ( cmd.foundOption ( "sweep" ) ) ? true : false;
    bool cCurrents = true;

    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );

    //else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    //Start server to communicate with HMP404 instrument via usbtmc and SCPI
    std::string cHostname = "localhost";
    int cPowerSupplyHttpPortNumber = 8080;
    int cPowerSupplyZmqPortNumber = 8090;
    std::string cPowerSupplyHWFile = "HMP4040_cbc3.xml";
    std::string cPowerSupplyOutputFile = cDirectory + "/Current_log.txt";
    int cPowerSupplyInterval = 5;
    std::pair<int, int> cPowerSupplyPortsInfo = std::pair<int, int> (cPowerSupplyZmqPortNumber, cPowerSupplyHttpPortNumber);

    pid_t childPid;  // the child process that the execution will soon run inside of.
    childPid = fork();

    if (childPid < 0) // fork failed
    {
        // log the error
        exit (1);
    }
    else if (childPid == 0 && cCurrents) // fork succeeded
    {
        // launch HMP4040 server
        LOG (INFO) << BOLDBLUE << "Trying to launch server to monitor currents on the HMP4040" << RESET ;
        launch_Server ( cPowerSupplyHWFile, cHostname, cPowerSupplyPortsInfo, cPowerSupplyInterval );
        exit (0);
    }
    else  // Main (parent) process after fork succeeds
    {
        int returnStatus = -1 ;
        waitpid (childPid, &returnStatus, 0); // Parent process waits here for child to terminate.

        if (returnStatus == 0 && cCurrents)  // Verify child process terminated without error.
        {
#ifdef __USBINST__
            LOG (INFO) << BOLDBLUE << "Starting monitoring of power supply currents on the HMP4040" << RESET ;
            HMP4040Client* cLVClient = new HMP4040Client (cHostname, cPowerSupplyZmqPortNumber);
            cLVClient->StartMonitoring();

            // make sure power supply is switched  on before doing anything else
            LOG (INFO) << BOLDBLUE << "Switching on the power supply" << RESET ;
            cLVClient->ToggleOutput (1);
#endif
            std::stringstream outp;

            if (cSweep)
            {
                Tool cTool;
                cTool.InitializeHw ( cHWFile, outp );
                cTool.InitializeSettings ( cHWFile, outp );
                LOG (INFO) << outp.str();
                outp.str ("");
                cTool.CreateResultDirectory ( cDirectory );
                cTool.InitResultFile ( "BiasSweeps" );
                cTool.StartHttpServer();
                cTool.ConfigureHw ();

                BiasSweep cSweep;
                cSweep.Inherit (&cTool);
                cSweep.Initialize();

                for (auto cBoard : cSweep.fBoardVector)
                {
                    for (auto cFe : cBoard->fModuleVector)
                    {
                        for (auto cCbc : cFe->fCbcVector)
                        {
                            cSweep.SweepBias ("Ipa", cCbc);
                            cSweep.SweepBias ("Vth", cCbc);
                        }
                    }
                }

                cTool.SaveResults();
                cTool.CloseResultFile();
                cTool.Destroy();

            }
            else
            {

                // from here
                SystemController cSystemController;
                cSystemController.InitializeHw ( cHWFile, outp );
                cSystemController.InitializeSettings ( cHWFile, outp );

                LOG (INFO) << outp.str();
                outp.str ("");

                cSystemController.ConfigureHw ();
                mypause();

                BeBoard* pBoard = cSystemController.fBoardVector.at ( 0 );
                uint32_t cN = 1;
                uint32_t cNthAcq = 0;

                const std::vector<Event*>* pEvents ;

                //cSystemController.fBeBoardInterface->Start ( pBoard );

                //while ( cN <= pEventsperVcth )
                //{
                //uint32_t cPacketSize = cSystemController.ReadData ( pBoard );

                ThresholdVisitor cVisitor (cSystemController.fCbcInterface, 0);

                if (cVcthset)
                {
                    cVisitor.setThreshold (cVcth);
                    cSystemController.accept (cVisitor);
                }

                cSystemController.ReadNEvents (pBoard, pEventsperVcth);

                pEvents = &cSystemController.GetEvents ( pBoard );

                for ( auto& ev : *pEvents )
                {
                    LOG (INFO) << ">>> Event #" << cN++ << " Threshold: " << cVcth;
                    outp.str ("");
                    outp << *ev;
                    LOG (INFO) << outp.str();
                }

                cNthAcq++;
                //}
                cSystemController.Destroy();
            }

#ifdef __USBINST__
            LOG (INFO) << "Toggling Output off again, stopping the monitoring and exiting the server!";
            cLVClient->ToggleOutput (0);
            cLVClient->StopMonitoring();
            cLVClient->Quit();
            delete cLVClient;
#endif
        }
        else if (returnStatus == 1)
        {
            LOG (INFO) << "The child process terminated with an error!." ;
            exit (1);
        }
    }

    LOG (INFO) << "*** End of the System test ***" ;

    if ( !batchMode ) cApp.Run();

    return 0;
}