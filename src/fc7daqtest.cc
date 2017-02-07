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
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF FC7 DAQ Test Application" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );
    cmd.defineOption ( "file", "Hw Description File . Default value: settings/FC7DAQHWDescription.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );
    cmd.defineOption ( "output", "Output Directory . Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "output", "o" );
    cmd.defineOption ( "configure", "Configure HW", ArgvParser::NoOptionAttribute );
    cmd.defineOptionAlternative ( "configure", "c" );

    int result = cmd.parse ( argc, argv );
    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }
    // now query the parsing results
    std::string cHWFile = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "settings/FC7DAQHWDescription.xml";

    std::stringstream outp;

    // from here
    SystemController cSystemController;
    cSystemController.InitializeHw ( cHWFile, outp );
    cSystemController.InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");

    bool cToBeConfigured = ( cmd.foundOption ( "configure" ) ) ? true : false;
    if (cToBeConfigured) cSystemController.ConfigureHw();

    BeBoard* pBoard = cSystemController.fBoardVector.at(0);
    //cSystemController.fBeBoardInterface->ConfigureBoard (pBoard);

    //Trigger Test
    /*cSystemController.fBeBoardInterface->Start(pBoard);
    LOG (INFO) << BOLDCYAN << "Trigger Started" << RESET;
    cSystemController.fBeBoardInterface->getBoardInfo(pBoard);
    cSystemController.fBeBoardInterface->Stop(pBoard);
    LOG (INFO) << BOLDCYAN << "Trigger Stopped" << RESET;*/
    cSystemController.fBeBoardInterface->getBoardInfo(pBoard);


    LOG (INFO) << "*** End of the DAQ test ***" ;
    cSystemController.Destroy();

    return 0;
}
