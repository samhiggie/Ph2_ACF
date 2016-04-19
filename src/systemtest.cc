#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/argvparser.h"
#include "../Utils/Timer.h"


using namespace Ph2_System;
using namespace CommandLineProcessing;

int main( int argc, char** argv )
{

	ArgvParser cmd;

	// init
	cmd.setIntroductoryDescription( "CMS Ph2_ACF  system test application" );
	// error codes
	cmd.addErrorCode( 0, "Success" );
	cmd.addErrorCode( 1, "Error" );
	// options
	cmd.setHelpOption( "h", "help", "Print this help page" );

	cmd.defineOption( "file", "Hw Description File . Default value: settings/HWDescription_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "configure", "Configure HW", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "configure", "c" );

	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	// now query the parsing results
	std::string cHWFile = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "settings/HWDescription_2CBC.xml";
	bool cConfigure = ( cmd.foundOption( "configure" ) ) ? true : false;


	SystemController cSystemController;
	cSystemController.InitializeHw( cHWFile );
	cSystemController.InitializeSettings( cHWFile );
	if ( cConfigure ) cSystemController.ConfigureHw();

    //Timer t;
    //t.start();

    //for(uint8_t cVcth = 0x00; cVcth < 0xFF; cVcth++)
    //{
        //std::cout << "Writing " << +cVcth << std::endl;
         //cSystemController.fCbcInterface->WriteBroadcast(cSystemController.fBoardVector.at(0)->fModuleVector.at(0), "VCth", cVcth);
         //CbcRegReader cReader(cSystemController.fCbcInterface, "VCth");
         //cSystemController.accept(cReader);
    //}
    //t.stop();
    //t.show("Time to loop VCth from 0 to ff with broadcast:");

	std::cout << "*** End of the System test ***" << std::endl;
    cSystemController.Destroy();
	return 0;
}
