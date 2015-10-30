/*

        FileName :                    BeBoardFWInterface.cc
        Content :                     BeBoardFWInterface base class of all type of boards
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            31/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */



#include "BeBoardFWInterface.h"
#include "FpgaConfig.h"

#define DEV_FLAG         0

namespace Ph2_HwInterface
{

	//Constructor, makes the board map
	BeBoardFWInterface::BeBoardFWInterface( const char* puHalConfigFileName, uint32_t pBoardId ) :
		RegManager( puHalConfigFileName, pBoardId ),
		fNTotalAcq( 0 ),
		runningAcquisition( false ),
		numAcq( 0 ),
		fSaveToFile( false ),
		fFileHandler( nullptr )
	{
	}




	std::string BeBoardFWInterface::getBoardType()
	{
		std::string cBoardTypeString;

		uhal::ValWord<uint32_t> cBoardType = ReadReg( BOARD_TYPE );

		char cChar = ( ( cBoardType & cMask4 ) >> 24 );
		cBoardTypeString.push_back( cChar );

		cChar = ( ( cBoardType & cMask3 ) >> 16 );
		cBoardTypeString.push_back( cChar );

		cChar = ( ( cBoardType & cMask2 ) >> 8 );
		cBoardTypeString.push_back( cChar );

		cChar = ( cBoardType & cMask1 );
		cBoardTypeString.push_back( cChar );

		return cBoardTypeString;

	}

	void BeBoardFWInterface::getBoardInfo()
	{
		std::cout << "FMC1 present : " << ReadReg( FMC1_PRESENT ) << std::endl;
		std::cout << "FMC2 present : " << ReadReg( FMC2_PRESENT ) << std::endl;
		std::cout << "FW version : " << ReadReg( FW_VERSION_MAJOR ) << "." << ReadReg( FW_VERSION_MINOR ) << "." << ReadReg( FW_VERSION_BUILD ) << std::endl;

		uhal::ValWord<uint32_t> cBoardType = ReadReg( BOARD_TYPE );

		std::cout << "BoardType : ";

		char cChar = ( ( cBoardType & cMask4 ) >> 24 );
		std::cout << cChar;

		cChar = ( ( cBoardType & cMask3 ) >> 16 );
		std::cout << cChar;

		cChar = ( ( cBoardType & cMask2 ) >> 8 );
		std::cout << cChar;

		cChar = ( cBoardType & cMask1 );
		std::cout << cChar << std::endl;

		std::cout << "FMC User Board ID : " << ReadReg( FMC_USER_BOARD_ID ) << std::endl;
		std::cout << "FMC User System ID : " << ReadReg( FMC_USER_SYS_ID ) << std::endl;
		std::cout << "FMC User Version : " << ReadReg( FMC_USER_VERSION ) << std::endl;

	}


	void BeBoardFWInterface::EncodeReg( const CbcRegItem& pRegItem, uint8_t pCbcId, std::vector<uint32_t>& pVecReq )
	{
		// temporary for 16CBC readout FW  (Beamtest NOV 15)
		// (pCbcId >> 3) becomes FE ID and is encoded starting from bit21 (not used so far)
		// (pCbcId & 7) restarts CbcIDs from 0 for FE 1 (if CbcID > 7)
		pVecReq.push_back( ( pCbcId >> 3 ) << 21 | ( pCbcId & 7 ) << 17 | pRegItem.fPage << 16 | pRegItem.fAddress << 8 | pRegItem.fValue );
	}


	void BeBoardFWInterface::DecodeReg( CbcRegItem& pRegItem, uint8_t pCbcId, uint32_t pWord )
	{
		// What does pCbcId do here??
		pCbcId = ( pWord & cMask5 ) >> 17;
		pRegItem.fPage = ( pWord & cMask6 ) >> 16;
		pRegItem.fAddress = ( pWord & cMask2 ) >> 8;
		pRegItem.fValue = pWord & cMask1;
	}



	void BeBoardFWInterface::StopThread()
	{
		if ( runningAcquisition )
		{
			runningAcquisition = false;
			try
			{
				thrAcq.join();
			}
			catch ( std::exception& e )
			{
				std::cerr << "Death to Stop in BeBoardFWInterface::StopThread()" << e.what() << std::endl;
			}
			catch ( ... )
			{
				std::cerr << "Death to Stop in BeBoardFWInterface::StopThread(). failed to perform thrAcq.join()" << std::endl;
			}
		}

	}

	int BeBoardFWInterface::getNumAcqThread()
	{
		return ( int )numAcq;
	}
}

