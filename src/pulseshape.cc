#include <cstring>

#include "../Utils/Utilities.h"
#include "../tools/PulseShape.h"
#include <TApplication.h>
#include "../Utils/argvparser.h"
#include "TROOT.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace CommandLineProcessing;


int main( int argc, char* argv[] )
{
	PulseShape cPulseShape;
	ArgvParser cmd;

	// init
	cmd.setIntroductoryDescription( "CMS Ph2_ACF  PulseShape tool to perform the following procedures:\n-Print scan test pulse delay" );
	// error codes
	cmd.addErrorCode( 0, "Success" );
	cmd.addErrorCode( 1, "Error" );
	// options
	cmd.setHelpOption( "h", "help", "Print this help page" );

	cmd.defineOption( "file", "Hw Description File . Default value: settings/PulseShape.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "step", "Scan the delay with treshold scan step size, default value: 1", ArgvParser::OptionRequiresValue );
	cmd.defineOptionAlternative( "step", "s" );

	//cmd.defineOption("delay","Scan the delay ", ArgvParser::NoOptionAttribute);
	// cmd.defineOptionAlternative("delay","d");

	cmd.defineOption( "channel", "Scan the channel", ArgvParser::OptionRequiresValue );
	cmd.defineOptionAlternative( "channel", "c" );
	// cmd.defineOption( "latency", "scan the trigger latency", ArgvParser::NoOptionAttribute );
	// cmd.defineOptionAlternative( "latency", "l" );

	// cmd.defineOption( "threshold", "scan the CBC comparator threshold", ArgvParser::NoOptionAttribute );
	// cmd.defineOptionAlternative( "threshold", "t" );

	// cmd.defineOption( "stublatency", "scan the stub latency", ArgvParser::NoOptionAttribute );
	// cmd.defineOptionAlternative( "stublatency", "s" );

	// cmd.defineOption( "minimum", "minimum value for latency scan", ArgvParser::OptionRequiresValue );
	// cmd.defineOptionAlternative( "minimum", "m" );

	// cmd.defineOption( "range", "range in clock cycles for latency scan", ArgvParser::OptionRequiresValue );
	// cmd.defineOptionAlternative( "range", "r" );

	// cmd.defineOption( "pedestal", "scan the CBC pedestals in addition with the threshold scan", ArgvParser::NoOptionAttribute );
	// cmd.defineOptionAlternative( "pedestal", "p" );

	cmd.defineOption( "output", "Output Directory . Default value: Results/", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "output", "o" );

	cmd.defineOption( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative( "batch", "b" );

	// cmd.defineOption( "gui", "option only suitable when launching from gui", ArgvParser::NoOptionAttribute );
	// cmd.defineOptionAlternative( "gui", "g" );

	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	// now query the parsing results
	std::string cHWFile = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "settings/PulseShape.xml";
	//uint8_t cStep = (cmd.foundOption("step")) ? cmd.optionValue( "step" ) : 1;
	//uint8_t cDelay = (cmd.foundOption("delay")) ? cmd.optionValue( "delay" ) : false;
	//uint32_t cChannel = (cmd.foundOption("channel")) ? cmd.optionValue( "channel" ) : false;
	// bool cLatency = ( cmd.foundOption( "latency" ) ) ? true : false;
	// bool cStubLatency = ( cmd.foundOption( "stublatency" ) ) ? true : false;
	// bool cThreshold = ( cmd.foundOption( "threshold" ) ) ? true : false;
	// bool cScanPedestal = ( cmd.foundOption( "pedestal" ) ) ? true : false;
	std::string cDirectory = ( cmd.foundOption( "output" ) ) ? cmd.optionValue( "output" ) : "Results/";
	cDirectory += "PulseShape";
	bool batchMode = ( cmd.foundOption( "batch" ) ) ? true : false;
	// bool gui = ( cmd.foundOption( "gui" ) ) ? true : false;

	// uint8_t cStartLatency = ( cmd.foundOption( "minimum" ) ) ? convertAnyInt( cmd.optionValue( "minimum" ).c_str() ) :  0;
	// uint8_t cLatencyRange = ( cmd.foundOption( "range" ) ) ?  convertAnyInt( cmd.optionValue( "range" ).c_str() ) :  10;
	uint8_t cScanStep = ( cmd.foundOption( "step" ) ) ? convertAnyInt( cmd.optionValue( "step" ).c_str() ) : 1;
	uint8_t cSelectedChannel = ( cmd.foundOption( "channel" ) ) ? convertAnyInt( cmd.optionValue( "channel" ).c_str() ) : 1;

	TApplication cApp( "Root Application", &argc, argv );
	if ( batchMode ) gROOT->SetBatch( true );
	else TQObject::Connect( "TCanvas", "Closed()", "TApplication", &cApp, "Terminate()" );

	cPulseShape.InitializeHw( cHWFile );
	cPulseShape.InitializeSettings( cHWFile );
	cPulseShape.Initialize();
	cPulseShape.CreateResultDirectory( cDirectory );
	std::string cResultfile;
	cResultfile = "PulseShape";
	cPulseShape.InitResultFile( cResultfile );
	cPulseShape.ConfigureHw();

	// Here comes our Part:
	cPulseShape.ScanTestPulseDelay( cScanStep );

	//if ( cChannel ) .......

	// if ( cStubLatency ) cCommissioning.ScanStubLatency( cStartLatency, cLatencyRange );
	// if ( cThreshold ) cCommissioning.ScanThreshold( cScanPedestal );
	cPulseShape.SaveResults();


	if ( !batchMode ) cApp.Run();

	return 0;

}

