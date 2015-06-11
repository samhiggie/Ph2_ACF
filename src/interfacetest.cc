

#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/Utilities.h"
#include "../System/SystemController.h"
//#include <TH1F.h>
//#include <TCanvas.h>
//#include <TStyle.h>
//#include <TApplication.h>
//#include <TROOT.h>

#include <sys/time.h>
#include <ctime>
#include <boost/format.hpp>
#include "../Utils/Visitor.h"
#include "../Utils/CommonVisitors.h"

#include "../Utils/Timer.h"
#include "../Utils/argvparser.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace CommandLineProcessing;


int main( int argc, char* argv[] )
{

	ArgvParser cmd;

	// init
	cmd.setIntroductoryDescription( "CMS Ph2_ACF  HWInterface performance Benchmark test" );
	// error codes
	cmd.addErrorCode( 0, "Success" );
	cmd.addErrorCode( 1, "Error" );
	// options
	cmd.setHelpOption( "h", "help", "Print this help page" );

	cmd.defineOption( "file", "Hw Description File . Default value: settings/HWDescription_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "configure", "test Configure HW", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "configure", "c" );

	cmd.defineOption( "single", "test single Register transaction", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "single", "s" );

	cmd.defineOption( "multi", "test multi-Register transaction", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "multi", "m" );

	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	// now query the parsing results
	std::string cHWFile = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "settings/HWDescription_2CBC.xml";
	bool cConfigure = ( cmd.foundOption( "configure" ) ) ? true : false;
	bool cSingle = ( cmd.foundOption( "single" ) ) ? true : false;
	bool cMulti = ( cmd.foundOption( "multi" ) ) ? true : false;

	SystemController cSystemController;
	Timer t;

	cSystemController.InitializeHw( cHWFile );
	cSystemController.InitializeSettings( cHWFile );

	if ( cConfigure )
	{
		t.start();

		cSystemController.ConfigureHw();

		t.stop();
		t.show( "Time to Initialize/configure the system: " );
	}

	struct CbcWriter : public HwDescriptionVisitor
	{
		CbcInterface* fInterface;
		CbcWriter( CbcInterface* pInterface ): fInterface( pInterface ) {
		}

		void visit( Cbc& pCbc ) {
			fInterface->WriteCbcReg( &pCbc , "VCth", 0x78 );
		}
	};


	if ( cSingle )
	{
		t.start();

		CbcWriter pWriter( cSystemController.fCbcInterface );
		cSystemController.accept( pWriter );

		t.stop();
		t.show( "Time to write a single Register on all CBC s" );
		CbcRegReader cReader( cSystemController.fCbcInterface, "VCth" );
		cSystemController.accept( cReader );
	}


	std::vector< std::pair< std::string, uint8_t > > cRegVec;

	uint8_t nChannels = 255;
	for ( uint8_t cChannel = 1; cChannel < nChannels; cChannel++ )
	{


		std::string cRegName = (boost::format("Channel%03d")%cChannel ).str();
		uint8_t cRegValue = 0x50;

		std::pair<std::string, uint8_t> cRegPair = { cRegName, cRegValue };
		cRegVec.push_back( cRegPair );
	}

	if ( cMulti )
	{

		t.start();

		cSystemController.fCbcInterface->WriteCbcMultReg( cSystemController.fShelveVector[0]->getBoard( 0 )->getModule( 0 )->getCbc( 0 ), cRegVec, true );
		cSystemController.fCbcInterface->WriteCbcMultReg( cSystemController.fShelveVector[0]->getBoard( 0 )->getModule( 0 )->getCbc( 1 ), cRegVec, false );

		t.stop();
		t.show( "Time for writing 254 registers to 2 CBCs" );
		CbcRegReader cReader( cSystemController.fCbcInterface, "Channel005" );
		cSystemController.accept( cReader );
	}
	return 0;
}
