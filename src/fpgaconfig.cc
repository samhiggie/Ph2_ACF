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

	cmd.defineOption( "file", "FPGA Bitstream (*.mcs format)", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "image", "Load to image 1 or 2", ArgvParser::OptionRequiresValue);
	cmd.defineOptionAlternative("image","i");

	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	// now query the parsing results
	std::string cHWFile = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "settings/HWDescription_2CBC.xml";
	uint16_t cNImage = (cmd.foundOption("image")) ? cmd.optionValue("image") : 1;
	Timer t;
	t.start();

	cSystemController.InitializeHw( cHWFile );
	cSystemController.ConfigureHw( std::cout, cmd.foundOption( "ignoreI2c" ) );

	BeBoard* pBoard = cSystemController.fShelveVector.at(0)->fBoardVector.at(0);
		
	t.stop();
	t.show( "Time to Initialize/configure the system: " );

	bool cUploadDone = 0;	


	cSystemController.fBeBoardInterface->FlashProm(pBoard, cNImage, cHWFile);
 	
	while (cUploadDone == 0)
	{
		
                uint32_t progress= cSystemController.fBeBoardInterface->getConfiguringFpga(pBoard)->getProgressValue();
                
                std::cout << "\r" + progress;
         
               if(progress==100)
                cUploadDone = 1;
        
	}

		
	t.stop();
	t.show( "Time for changing VCth on all CBCs:" );
	}


