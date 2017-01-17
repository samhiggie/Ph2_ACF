#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/argvparser.h"
#include "../Utils/Timer.h"
#include "../tools/BiasSweep.h"
#include "TROOT.h"
#include "TApplication.h"

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
    //cmd.defineOption ( "configure", "Configure HW", ArgvParser::NoOptionAttribute );
    //cmd.defineOptionAlternative ( "configure", "c" );
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

    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );

    //else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

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

        if ( !batchMode ) cApp.Run();
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

        //for (uint16_t cVcth = 1023; cVcth > 0; cVcth-- )
        //{
        if (cVcthset)
        {
            cVisitor.setThreshold (cVcth);
            cSystemController.accept (cVisitor);
        }

        //LOG (DEBUG) << "I get here!";

        cSystemController.ReadNEvents (pBoard, pEventsperVcth);
        //LOG (DEBUG) << "I get here too!";

        //LOG (DEBUG) << "Read " << cPacketSize << " events in this acquistion";


        //if ( cN + cPacketSize > pEventsperVcth )
        //cSystemController.fBeBoardInterface->Stop ( pBoard );

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

        //}

        //Timer t;
        //t.start();

        //t.stop();
        //t.show("Time to loop VCth from 0 to ff with broadcast:");

        LOG (INFO) << "*** End of the System test ***" ;
        cSystemController.Destroy();
    }

    return 0;
}
