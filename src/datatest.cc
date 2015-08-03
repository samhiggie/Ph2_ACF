#include <cstring>
#include "../Utils/Utilities.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
//#include "../tools/Calibration.h"
#include "../Utils/Timer.h"
//#include <TApplication.h>
#include <inttypes.h>
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace CommandLineProcessing;

//Class used to process events acquired by a parallel acquisition
class AcqVisitor: public HwInterfaceVisitor
{
	int cN;
  public:
	AcqVisitor() {
		cN = 0;
	}
	//void init(std::ofstream* pfSave, bool bText);
	virtual void visit( const Ph2_HwInterface::Event& pEvent ) {
		cN++;
		std::cout << ">>> Event #" << cN << std::endl;
		std::cout << pEvent << std::endl;
	}
};

void syntax( int argc )
{
	if ( argc > 4 ) std::cerr << RED << "ERROR: Syntax: calibrationtest VCth NEvents (HWDescriptionFile)" << std::endl;
	else if ( argc < 3 ) std::cerr << RED << "ERROR: Syntax: calibrationtest VCth NEvents (HWDescriptionFile)" << std::endl;
	else return;
}

int main( int argc, char* argv[] )
{

	int pEventsperVcth;
	int cVcth;

	SystemController cSystemController;
	ArgvParser cmd;

	// init
	cmd.setIntroductoryDescription( "CMS Ph2_ACF  Data acquisition test and Data dump" );
	// error codes
	cmd.addErrorCode( 0, "Success" );
	cmd.addErrorCode( 1, "Error" );
	// options
	cmd.setHelpOption( "h", "help", "Print this help page" );

	cmd.defineOption( "ignoreI2c", "Ignore I2C configuration of CBCs. Allows to run acquisition on a bare board without CBC." );
	cmd.defineOptionAlternative( "ignoreI2c", "i" );

	cmd.defineOption( "file", "Hw Description File . Default value: settings/HWDescription_2CBC.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "vcth", "Threshold in VCth units (hex (including 0x) or decimal) . Default values from HW description .XML file", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "vcth", "v" );

	cmd.defineOption( "events", "Number of Events . Default value: 10", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "events", "e" );

	cmd.defineOption( "parallel", "Acquisition running in parallel in a separate thread" );
	cmd.defineOptionAlternative( "parallel", "p" );

	cmd.defineOption( "save", "Save the data to a raw file.  ", ArgvParser::OptionRequiresValue );
	cmd.defineOptionAlternative( "save", "s" );

	cmd.defineOption( "option", "Define file access mode: w : write , a : append, w+ : write/update", ArgvParser::OptionRequiresValue );
	cmd.defineOptionAlternative( "option", "o" );


	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	bool cSaveToFile = false;
	std::string cOutputFile;
	// now query the parsing results
	std::string cHWFile = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "settings/HWDescription_2CBC.xml";

	if ( cmd.foundOption( "save" ) )
		cSaveToFile = true ;
	if ( cSaveToFile )
		cOutputFile =  cmd.optionValue( "save" );


	std::cout << "save:   " << cOutputFile << std::endl;
	std::string cOptionWrite = ( cmd.foundOption( "option" ) ) ? cmd.optionValue( "option" ) : "w+";
	cVcth = ( cmd.foundOption( "vcth" ) ) ? convertAnyInt( cmd.optionValue( "vcth" ).c_str() ) : 0;
	pEventsperVcth = ( cmd.foundOption( "events" ) ) ? convertAnyInt( cmd.optionValue( "events" ).c_str() ) : 10;

	Timer t;
	t.start();
	cSystemController.addFileHandler( cOutputFile, cOptionWrite );

	cSystemController.InitializeHw( cHWFile );
	cSystemController.ConfigureHw( std::cout, cmd.foundOption( "ignoreI2c" ) );

	t.stop();
	t.show( "Time to Initialize/configure the system: " );

	if ( cVcth != 0 )
	{
		t.start();

		for ( auto cShelve : cSystemController.fShelveVector )
		{
			for ( auto cShelve : cSystemController.fShelveVector )
			{
				for ( auto cBoard : ( cShelve )->fBoardVector )
				{
					for ( auto cFe : cBoard->fModuleVector )
					{
						for ( auto cCbc : cFe->fCbcVector )
							cSystemController.fCbcInterface->WriteCbcReg( cCbc, "VCth", uint8_t( cVcth ) );
					}
				}
			}
		}

		t.stop();
		t.show( "Time for changing VCth on all CBCs:" );
	}

	BeBoard* pBoard = cSystemController.fShelveVector.at( 0 )->fBoardVector.at( 0 );
	if ( cmd.foundOption( "parallel" ) )
	{
		uint32_t nbPacket = pBoard->getReg( CBC_PACKET_NB ), nbAcq = pEventsperVcth / ( nbPacket + 1 ) + ( pEventsperVcth % ( nbPacket + 1 ) != 0 ? 1 : 0 );
		std::cout << "Packet number=" << nbPacket << ", Nb events=" << pEventsperVcth << " -> Nb acquisition iterations=" << nbAcq << std::endl;

		AcqVisitor visitor;
		std::cout << "Press Enter to start the acquisition, press Enter again to stop it." << std::endl;
		std::cin.ignore();
		cSystemController.fBeBoardInterface->StartThread( pBoard, nbAcq, &visitor );
		std::cin.ignore();
		cSystemController.fBeBoardInterface->StopThread( pBoard );
	}
	else
	{
		t.start();
		// make event counter start at 1 as does the L1A counter
		uint32_t cN = 1;
		uint32_t cNthAcq = 0;

		cSystemController.fBeBoardInterface->Start( pBoard );
		while ( cN <= pEventsperVcth )
		{
			uint32_t cPacketSize = cSystemController.fBeBoardInterface->ReadData( pBoard, cNthAcq, false );

			if ( cN + cPacketSize >= pEventsperVcth ) cSystemController.fBeBoardInterface->Stop( pBoard, cNthAcq );
			const std::vector<Event*>& events = cSystemController.GetEvents( pBoard );

			for ( auto& ev : events )
			{
				std::cout << ">>> Event #" << cN++ << std::endl;
				std::cout << *ev << std::endl;
			}
			cNthAcq++;
		}
		t.stop();
		t.show( "Time to take data:" );
	}
}

