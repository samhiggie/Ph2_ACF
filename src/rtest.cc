
#include <cstring>
#include <fstream>
#include "../Utils/Utilities.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/MPAInterface.h"
#include "../HWInterface/MPAGlibFWInterface.h"
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
#include "../Utils/CommonVisitors.h"
#include "../Tracker/TrackerEvent.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

using namespace std;
INITIALIZE_EASYLOGGINGPP

int main( int argc, char* argv[] )
{

	ArgvParser cmd;

	cmd.defineOption( "file", "Hw Description File . Default value: settings/HWDescription_MAPSA.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "output", "Output Directory . Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "output", "o" );

	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	std::string cHWFile = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "settings/HWDescription_MAPSA.xml";
	std::string cDirectory = ( cmd.foundOption( "output" ) ) ? cmd.optionValue( "output" ) : "Results/";

	std::cout << "\ndefsys...";
	SystemController mysyscontroller;
	std::cout << "\nINITHW...";
	mysyscontroller.InitializeHw( cHWFile );
	std::cout << "\nINITHWdone...";
        MPAInterface* fMPAInterface = mysyscontroller.fMPAInterface; 
	BeBoard* pBoard = mysyscontroller.fBoardVector.at( 0 );
	std::cout << "\nExecuting POWER ON...";
	mysyscontroller.fBeBoardInterface->PowerOn(pBoard);
	std::cout << "\nFirmware version: "; 
	mysyscontroller.fBeBoardInterface->ReadVer(pBoard);
	std::chrono::milliseconds cWait( 10 );
	std::vector<std::vector< uint32_t >> confs;
	for(int i=0;i<=5;i++)
	{

		std::this_thread::sleep_for( cWait );

		confs.push_back(fMPAInterface->ReadConfig("testing", i+1, 1));
		fMPAInterface->HeaderInitMPA(i+1);

		std::pair < std::vector< std::string > ,std::vector< uint32_t >> mod1({"THDAC"},{60});
		fMPAInterface->ModifyPerif(mod1,&confs[i]);
		for (int j=1;j<=24;j++)
			{
			std::pair < std::vector< std::string > ,std::vector< uint32_t >> mod2({"PML"},{1});
			fMPAInterface->ModifyPix(mod2,&confs[i],j);
			}

		fMPAInterface->ConfigureMPA(&confs[i], 1 , i+1);
	}
	std::chrono::milliseconds cWait1( 100 );//



	for(int q=0;q<=140;q++)
	{
	std::cout<<"\n--------------------------------------\n "<<std::endl;

	std::cout<<"Threshold: "<<q<<std::endl;

	std::this_thread::sleep_for( cWait );
	fMPAInterface->SequencerInit(0,300000,1,0);


	for(int i=0;i<=5;i++)
	{

		std::cout<<"\nReading MPA: "<<i<<std::endl;
		std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  returndata = mysyscontroller.fBeBoardInterface->ReadData(pBoard, 1,i+1);

		std::pair<std::vector<uint32_t>, std::vector<std::string>> formdata = fMPAInterface->FormatData(returndata);
		std::vector<uint32_t> counts = formdata.first;

		int totalcount = 0;
		for (int j=0;j<counts.size();j++)	totalcount+=counts[j];

		std::cout<<"Total Counts: "<<totalcount<<std::endl;
		std::pair<std::vector<uint32_t>, std::vector<uint64_t>> readmem = fMPAInterface->ReadMemory(formdata.second, 3);

		std::vector<uint32_t> bx = readmem.first;
		std::vector<uint64_t> pixmem = readmem.second;
		for (int j=0;j<pixmem.size();j++)
			{
				if (j>3) break;
				std::cout<<"BX: "<<bx[j]<<std::endl;
		    		std::bitset<48> p(pixmem[j]);
				std::cout<<"PIX: "<<p.to_string()<<std::endl;
			}

		std::pair < std::vector< std::string > ,std::vector< uint32_t >> mod1({"THDAC"},{q});
		fMPAInterface->ModifyPerif(mod1,&confs[i]);
		fMPAInterface->ConfigureMPA(&confs[i],1, i+1);
	}


	}

}

