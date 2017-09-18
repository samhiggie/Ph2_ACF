
//Simple bare bones daq to be used as a template for the 
//relevant sections of usercode in order to test the middleware


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
#include "../HWDescription/FrontEndDescription.h"
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

	SystemController mysyscontroller;
	std::cout << "\nInitHW";
	mysyscontroller.InitializeHw( cHWFile );
	std::cout << "\nMPAI";
        MPAInterface* fMPAInterface = mysyscontroller.fMPAInterface; 
	std::cout << "\nBOARD"<<std::endl;
	BeBoard* pBoard = mysyscontroller.fBoardVector.at( 0 );




	
	uint8_t pBeId = 0;
	uint8_t pFMCId = 0;

	//One BE board, multiple FE's for module.  Cside identifies whether the firmware accesses the left
	//or right six MPAs

	//Left
	uint8_t pFeId = 0;
	int cside=1;
	Module* MAPSAR = new Module(); 
	for (int i=0;i<6;i++)
		{
		MAPSAR->addMPA(new MPA(pBeId, pFMCId, pFeId, i, cside));
		}
	uint8_t nummpaR =MAPSAR->getNMPA();

	//Right
	pFeId = 1;
	cside=0;
	Module* MAPSAL = new Module(); 
	for (int i=0;i<6;i++)
		{
		MAPSAL->addMPA(new MPA(pBeId, pFMCId, pFeId, i, cside));
		}
	uint8_t nummpaL =MAPSAL->getNMPA();


	std::cout<<"Number of MPAs in Left MAPSA "<<int(nummpaL)<<std::endl;
	std::cout<<"Number of MPAs in Right MAPSA "<<int(nummpaR)<<std::endl;

	//Add both modules to the BE board
	pBoard->addModule(MAPSAL);
	pBoard->addModule(MAPSAR);
	


	//Power on and check FW version.  These do not need to be called every time datataking begins. 
	std::cout << "\nExecuting POWER ON...";
	mysyscontroller.fBeBoardInterface->PowerOn(pBoard);
	std::cout << "\nFirmware version: "; 
	mysyscontroller.fBeBoardInterface->ReadVer(pBoard);
	std::chrono::milliseconds cWait( 10 );


	//Initialize configuration data 
	std::vector<std::vector< uint32_t >> confsR;
	std::vector<std::vector< uint32_t >> confsL;

	//Manually set some configuration values for testing
	int threshmod = 70;
	int opmodemod = 3;
	std::pair < std::vector< std::string > ,std::vector< uint32_t >> mod1({"OM","THDAC"},{opmodemod,threshmod});


	//Load up configuration for right and left MAPSAs into buffers
	int curside;
	for(int i=0;i<nummpaR;i++)
	{
		MPA* curmpa = (MAPSAR->getMPA(i));
		curside =int(curmpa->getMPASide());
		std::this_thread::sleep_for( cWait );
		confsR.push_back(fMPAInterface->ReadConfig("calibratedRight", i+1, 1));
		fMPAInterface->ModifyPerif(mod1,&confsR.back());
		fMPAInterface->ConfigureMPA(&confsR.back(), 1 , i+1 , curside);

	}


	for(int i=0;i<nummpaL;i++)
	{
		MPA* curmpa = (MAPSAL->getMPA(i));
		curside =int(curmpa->getMPASide());
		std::this_thread::sleep_for( cWait );
		confsL.push_back(fMPAInterface->ReadConfig("calibratedLeft", i+1, 1));
		fMPAInterface->ModifyPerif(mod1,&confsL.back());
		fMPAInterface->ConfigureMPA(&confsL.back(), 1 , i+1 , curside);
	}



	//Transfer configuration information to MPAs after loading 
	fMPAInterface->SendConfig(nummpaL,nummpaR);
	std::chrono::milliseconds cWait1( 100 );//

	int ibuffer = 1;



	//Option 1 just takes data continuously, option 2 waits for triggers
	fMPAInterface->SequencerInit(1,200000,1,0);
	//fMPAInterface->TestbeamInit(500000,0, 0);


	//Always four buffers in current FW
	int nbuffers = 4;


	//Release all data currently stored before taking new data
	for(int i=0;i<nummpaR;i++)
	{
		MPA* curmpa = (MAPSAR->getMPA(i));
		curside =int(curmpa->getMPASide());
		std::this_thread::sleep_for( cWait );

		for(int k=1;k<=nbuffers;k++)
			{
			fMPAInterface->HeaderInitMPA(i+1,curside);
			std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  returndata = fMPAInterface->ReadMPAData(k,i+1,curside);
			fMPAInterface->ReadTrig(k);
			}
	}
	for(int i=0;i<nummpaL;i++)
	{
		MPA* curmpa = (MAPSAL->getMPA(i));
		curside =int(curmpa->getMPASide());
		std::this_thread::sleep_for( cWait );

		for(int k=1;k<=nbuffers;k++)
			{
			fMPAInterface->HeaderInitMPA(i+1,curside);
			std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  returndata = fMPAInterface->ReadMPAData(k,i+1,curside);
			fMPAInterface->ReadTrig(k);
			}
	}
	fMPAInterface->Cleardata();



	int spill = 0;
	int tempspill = 0;
	int nev = 0;




	std::ofstream outFile_;
  	outFile_.open ("output.dat");


	bool Kill=false;
	while (not Kill)
	{

		tempspill=spill;
		spill+=fMPAInterface->WaitTestbeam();
		if (tempspill!=spill) std::cout<<"Starting Spill "<<spill<<std::endl;

		fMPAInterface->Cleardata();
		fMPAInterface->ReadTrig(ibuffer);


		for(int i=0;i<nummpaL;i++)
		{
			MPA* curmpa = (MAPSAL->getMPA(i));
			curside =int(curmpa->getMPASide());
			std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  returndata = fMPAInterface->ReadMPAData(ibuffer,i+1,curside);
		}

		for(int i=0;i<nummpaR;i++)
		{
			MPA* curmpa = (MAPSAR->getMPA(i));
			curside =int(curmpa->getMPASide());
			std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  returndata = fMPAInterface->ReadMPAData(ibuffer,i+1,curside);
		}



		ibuffer+=1;
		if (ibuffer >nbuffers) ibuffer=1 ;

		nev+=1;
		if (nev%100==0)	std::cout<<nev<<" Events"<<std::endl;
	
		//For testing puroses, output all collected data

		std::vector<uint32_t> cData = *(fMPAInterface->GetcurData());

		int iic1 = 0;
		for( unsigned int iic1=0;iic1<cData.size();iic1++)
		{
			outFile_.write( (char*)&cData.at(iic1), sizeof(uint32_t));
	
		}





	}//while (not Kill)
	outFile_.close();
}//int main

