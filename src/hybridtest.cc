#include <cstring>
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../tools/HybridTester.h"
#include <TApplication.h>
#include "../Utils/argvparser.h"
#include "TROOT.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace CommandLineProcessing;


int main( int argc, char* argv[] )
{

	ArgvParser cmd;

	// init
	cmd.setIntroductoryDescription( "CMS Ph2_ACF  Hybrid validation test performs the following actions:\n-Test individual CBC registers one by one by writing complimentary bit patterns 0x55 and 0xAA\n-scan the threshold range and measure the noise occupancy to determine the pedestal and identify a threshold with ~0 noise occupancy\n-measure the single-strip efficiency under the influence of an external signal source to identify bad connections" );
	// error codes
	cmd.addErrorCode( 0, "Success" );
	cmd.addErrorCode( 1, "Error" );
	// options
	cmd.setHelpOption( "h", "help", "Print this help page" );

	cmd.defineOption( "file", "Hw Description File . Default value: settings/HybridTest2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "registers", "test registers", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "registers", "r" );

	cmd.defineOption( "scan", "scan noise occupancy, if not set, the threshold from the .XML will be used", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "scan", "s" );

	cmd.defineOption( "output", "Output Directory . Default value: Results/", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "output", "o" );

	cmd.defineOption( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "batch", "b" );


	// only relevant when in GUI mode
	cmd.defineOption( "gui", "option only suitable when launching from gui", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "gui", "g" );

	cmd.defineOption( "vcth", "option on suitable when launching from gui", ArgvParser::OptionRequiresValue );
	cmd.defineOptionAlternative( "vcth", "v" );

	cmd.defineOption( "nEvents", "option on suitable when launching from gui", ArgvParser::OptionRequiresValue );
	cmd.defineOptionAlternative( "nEvents", "n" );

	cmd.defineOption( "holemode", "option on suitable when launching from gui", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "holemode", "hm" );

	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	bool isGui = ( cmd.foundOption( "gui" ) ) ? true : false;
	// now query the parsing results
	std::string cHWFile = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "settings/HybridTest2CBC.xml";
	bool batchMode = ( cmd.foundOption( "batch" ) ) ? true : false;
	bool cRegisters = ( cmd.foundOption( "registers" ) ) ? true : false;
	bool cScan = ( cmd.foundOption( "scan" ) ) ? true : false;
	std::string cDirectory = ( cmd.foundOption( "output" ) ) ? cmd.optionValue( "output" ) : "Results/";
	cDirectory += "HybridTest";


	TApplication cApp( "Root Application", &argc, argv );
	if ( batchMode ) gROOT->SetBatch( true );
	else TQObject::Connect( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

	HybridTester cHybridTester;

	if ( !isGui )
	{
		cHybridTester.InitializeHw( cHWFile );
		cHybridTester.InitializeSettings( cHWFile );
		cHybridTester.Initialize( cScan );
		cHybridTester.CreateResultDirectory( cDirectory );
		cHybridTester.InitResultFile( "HybridTest" );
		cHybridTester.ConfigureHw();

		// Here comes our Part:
		cHybridTester.Measure();
		cHybridTester.TestChannels();
		if ( cRegisters ) cHybridTester.TestRegisters();
		if ( cScan )
		{
			cHybridTester.ScanThreshold();
			// Wait for user to acknowledge and turn on external Source!
			std::cout << "Identified the threshold for 0 noise occupancy - Start external Signal source!" << std::endl;
			mypause();
		}

		cHybridTester.SaveResults();

	}
	else
	{
		std::cout << "GUI mode triggered!" << std::endl;
		cHybridTester.InitializeHw( cHWFile );
		int cVcth = ( cmd.foundOption( "vcth" ) ) ? std::stoi( cmd.optionValue( "vcth" ) ) : 70;
		int nEvents = ( cmd.foundOption( "nEvents" ) ) ? std::stoi( cmd.optionValue( "nEvents" ) ) : 100;
		bool cHoleMode = ( cmd.foundOption( "holemode" ) ) ? true : false;

		cHybridTester.InitializeSettings( cHWFile );
		cHybridTester.InitialiseGUI( cVcth, nEvents, cRegisters, cScan, cHoleMode );
		cHybridTester.CreateResultDirectory( cDirectory );
		cHybridTester.InitResultFile( "HybridTest" );

		cHybridTester.Measure();
		cHybridTester.TestChannels();
		// Here comes our Part:
		if ( cRegisters ) cHybridTester.TestRegisters();
		if ( cScan )  cHybridTester.ScanThreshold();
		cHybridTester.SaveResults();

	}



	if ( !batchMode ) cApp.Run();

	return 0;

}

