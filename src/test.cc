#include <cstring>
#include <stdint.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <inttypes.h>

#include "../Utils/Utilities.h"
#include "../Utils/Data.h"
#include "../Utils/Event.h"
#include "../Utils/Timer.h"
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

using namespace std;
INITIALIZE_EASYLOGGINGPP

int main ( int argc, char* argv[] )
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_ACF  miniDQM application" );
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "file", "Binary Data File", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "file", "f" );

    cmd.defineOption ( "nevt", "Specify number of events to be read from file at a time", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "nevt", "n" );


    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        LOG (INFO) << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    // now query the parsing results
    std::string rawFilename = ( cmd.foundOption ( "file" ) ) ? cmd.optionValue ( "file" ) : "";

    if ( rawFilename.empty() )
    {
        LOG (ERROR) << "Error, no binary file provided. Quitting" ;
        exit ( 2 );
    }

    // Check if the file can be found
    if ( ! boost::filesystem::exists ( rawFilename ) )
    {
        LOG (ERROR) << "Error!! binary file " << rawFilename << " not found, exiting!";
        exit ( 3 );
    }

    int maxevt     = ( cmd.foundOption ( "nevt" ) ) ? stoi (cmd.optionValue ( "nevt" ) ) : 10;
    BoardType t = BoardType::CBC3FC7;


    // Create the Histogrammer object
    //DQMHistogrammer* dqmh = new DQMHistogrammer (addTree, ncol, evtFilter, skipHist);
    SystemController dqmh;
    // Add File handler
    dqmh.addFileHandler ( rawFilename, 'r' );

    // Build the hardware setup
    std::string cHWFile = "settings/Cbc3HWDescription.xml";

    LOG (INFO) << "HWfile=" << cHWFile;
    //dqmh->parseHWxml ( cHWFile );
    std::stringstream outp;
    dqmh.InitializeHw ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");
    //dqmh->fParser.parseHW (cHWFile, fBeBoardFWMap, fBoardVector, os);
    const BeBoard* pBoard = dqmh.getBoard ( 0 );

    // Read the first event from the raw data file, needed to retrieve the event map
    std::vector<uint32_t> dataVec;
    //int eventSize = EVENT_HEADER_TDC_SIZE_32 + CBC_EVENT_SIZE_32 * cbcTypeEvtSizeMap[cbcType].first;
    int eventSize = 14;
    //dqmh.readFile (dataVec, 140000);
    dqmh.readFile (dataVec, 14000);

    // Now split the data buffer in events
    int nEvents = dataVec.size() / eventSize;

    Data d;
    d.Set ( pBoard, dataVec, nEvents, t);

    Timer timer;
    timer.start();
    const std::vector<Event*>& elist = d.GetEvents ( pBoard );

    for (auto ev : elist)
    {
        LOG (INFO) << *ev ;
        ev->GetSLinkEvent (pBoard);
    }

    timer.stop();
    timer.show ("Time to encode 1000 SLinkEvents: ");

    return 0;
}
