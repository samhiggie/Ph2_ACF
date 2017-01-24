#include <iostream>
//#include <unistd.h>
//#include <limits.h>
//#include <signal.h>
//#include "easylogging++.h"
#include "../Utils/argvparser.h"
//#include "../../Ph2_USBInstDriver/Utils/AppLock.cc"
#include "ArdNanoController.h"
//#include "boost/tokenizer.hpp"
//#include "Messages.h"
#ifdef __HTTP__
//#include "THttpServer.h"
#endif
#ifdef __ZMQ__
//#include <zmq.hpp>
//#include "../Utils/zmqutils.h"
#endif

using namespace Ph2_UsbInst;
using namespace CommandLineProcessing;

INITIALIZE_EASYLOGGINGPP

std::atomic<bool> fQuit;

int main (int argc, char** argv)
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

    //CMD line parsing goes here!
    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription ( "CMS Ph2_Arduino Driver and Monitoring library") ;
    // error codes
    cmd.addErrorCode ( 0, "Success" );
    cmd.addErrorCode ( 1, "Error" );
    // options
    cmd.setHelpOption ( "h", "help", "Print this help page" );

    cmd.defineOption ( "send", "Send single character to serial port", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
    cmd.defineOptionAlternative ( "send", "s" );

    cmd.defineOption ( "blink", "Test connection to ARDUINO board by making on-board LED blink on/off N times.", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "blink", "b" );

    cmd.defineOption ( "relay", "Control relay connected to Arduino board [on/off]", ArgvParser::OptionRequiresValue );
    cmd.defineOptionAlternative ( "relay", "r" );


    int result = cmd.parse ( argc, argv );

    if ( result != ArgvParser::NoParserError )
    {
        std::cout << cmd.parseErrorDescription ( result );
        exit ( 1 );
    }

    bool cBlink = ( cmd.foundOption ( "blink" ) ) ? true : false;
    bool cRelay = ( cmd.foundOption ( "relay" ) ) ? true : false;
    std::string cSerialCommand = ( cmd.foundOption ( "send" ) ) ? cmd.optionValue ( "send" ) : "";

    ArdNanoController* cController = new ArdNanoController();

    if ( !cSerialCommand.empty() )
        cController->Write (cSerialCommand.c_str() );

    if ( cBlink )
    {
        for ( unsigned int i = 0 ; i < atoi ( cmd.optionValue ("blink").c_str() ) * 2 ; i++ )
        {
            cController->ControlLED ( (i % 2) );
            std::this_thread::sleep_for (std::chrono::milliseconds (500) );
        }
    }

    if ( cRelay )
    {
        uint8_t cRelayState = ( cmd.optionValue ( "relay" ) == "on" || cmd.optionValue ( "relay" ) == "ON" || cmd.optionValue ( "relay" ) == "On") ? 1 : 0;
        LOG (INFO) << "Controling relay connected to Arduino nano .... switching it : " << BOLDGREEN << cmd.optionValue ( "relay" ) <<  RESET << " !!";
        cController->ControlRelay (cRelayState);

        cRelayState = cController->GetRelayState();
        std::string cReply = ( cRelayState == 1 ) ? "On" : "Off" ;
        LOG (INFO) << "Relay now.... " << cReply;
    }

    delete cController;
}
