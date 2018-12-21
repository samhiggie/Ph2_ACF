#include "../tools/StubTool.h"
#include "../Utils/argvparser.h"
#include "TROOT.h"
#include "TApplication.h"
#include "../Utils/Timer.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

INITIALIZE_EASYLOGGINGPP

int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2 DAQ Workshop test application" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Hw Description File . Default value: settings/Calibration8CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "batch", "b" );

    cmd.defineOption ( "offset","Run the stub bend test application using correlation offset", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "offset", "o" );

    cmd.defineOption ( "noise", "Run the stub bend testt application using noises", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "noise", "n" );

    //actually parse
    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/Calibration8CBC.xml";
    //std::string cDirectory = "Results/mytest";
    bool batchMode = ( cmd.foundOption ( "batch" ) ) ? true : false;
    bool offset    = ( cmd.foundOption ( "offset" ) ) ? true : false;
    bool noise     = ( cmd.foundOption ( "noise" ) ) ? true : false;

    TApplication cApp ( "Root Application", &argc, argv );

    if ( batchMode ) gROOT->SetBatch ( true );
    else TQObject::Connect ( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

    Timer t;

    //create a genereic Tool Object, I can then construct all other tools from that using the Inherit() method
    //this tool stays on the stack and lives until main finishes - all other tools will update the HWStructure from cTool
    Tool cTool;
    std::stringstream outp;
    cTool.InitializeHw ( cHWFile, outp );
    cTool.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");
    std::string cDirectory = "Results/StubScan";
    if (offset) cDirectory = "Results/StubScan_correlation_offset";
    if (noise)  cDirectory = "Results/StubScan_noises";
    cTool.CreateResultDirectory ( cDirectory);
    cTool.InitResultFile ( "mytestfile" );
    cTool.ConfigureHw ();
    std::string::size_type sz;
    t.start();
    StubTool cStubTool;
    cStubTool.Inherit (&cTool);
    cStubTool.Initialize();
    if (offset) cStubTool.scanStubs();
    if (noise)  cStubTool.scanStubs_wNoise();

    t.stop();
    t.show ( "Time to run our code: " );
    
    //clean up and exit cleanly
    cTool.SaveResults();
    cTool.CloseResultFile();
//    cTool.Destroy();

    if ( !batchMode ) cApp.Run();

    return 0;
}
