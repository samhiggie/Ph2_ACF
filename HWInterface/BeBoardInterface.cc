/*

        FileName :                    BeBoardInterface.cc
        Content :                     User Interface to the Boards
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            31/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com nico.pierre@icloud.com

 */

#include "BeBoardInterface.h"

namespace Ph2_HwInterface
{

	BeBoardInterface::BeBoardInterface( const BeBoardFWMap& pBoardMap ) :
		fBoardMap( pBoardMap ),
		fBoardFW( nullptr ),
		prevBoardIdentifier( 65535 )
	{
	}

	BeBoardInterface::~BeBoardInterface()
	{

	}

	void BeBoardInterface::setBoard( uint16_t pBoardIdentifier )
	{
		if ( prevBoardIdentifier != pBoardIdentifier )
		{
			BeBoardFWMap::iterator i = fBoardMap.find( pBoardIdentifier );
			if ( i == fBoardMap.end() )
				std::cout << "The Board: " << +( pBoardIdentifier >> 8 ) <<  " in Shelve " << +( static_cast<uint8_t>( pBoardIdentifier ) ) << "  doesn't exist" << std::endl;
			else
			{
				fBoardFW = i->second;
				prevBoardIdentifier = pBoardIdentifier;
			}
		}
	}

	void BeBoardInterface::WriteBoardReg( BeBoard* pBoard, const std::string& pRegNode, const uint32_t& pVal )
	{
		setBoard( pBoard->getBeBoardIdentifier() );

		fBoardFW->WriteReg( pRegNode, pVal );
		pBoard->setReg( pRegNode, pVal );
	}

	void BeBoardInterface::WriteBlockBoardReg( BeBoard* pBoard, const std::string& pRegNode, const std::vector<uint32_t>& pValVec )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->WriteBlockReg( pRegNode, pValVec );
	}


	void BeBoardInterface::WriteBoardMultReg( BeBoard* pBoard, const std::vector < std::pair< std::string , uint32_t > >& pRegVec )
	{
		setBoard( pBoard->getBeBoardIdentifier() );

		fBoardFW->WriteStackReg( pRegVec );

		for ( const auto& cReg : pRegVec )
		{
			// fBoardFW->WriteReg( cReg.first, cReg.second );
			pBoard->setReg( cReg.first, cReg.second );
		}
	}


	uint32_t BeBoardInterface::ReadBoardReg( BeBoard* pBoard, const std::string& pRegNode )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		uint32_t cRegValue = static_cast<uint32_t>( fBoardFW->ReadReg( pRegNode ) );
		pBoard->setReg( pRegNode,  cRegValue );
		return cRegValue;
	}

	void BeBoardInterface::ReadBoardMultReg( BeBoard* pBoard, std::vector < std::pair< std::string , uint32_t > >& pRegVec )
	{
		setBoard( pBoard->getBeBoardIdentifier() );

		for ( auto& cReg : pRegVec )
		{
			cReg.second = static_cast<uint32_t>( fBoardFW->ReadReg( cReg.first ) );
			pBoard->setReg( cReg.first, cReg.second );
		}
	}

	std::vector<uint32_t> BeBoardInterface::ReadBlockBoardReg( BeBoard* pBoard, const std::string& pRegNode, uint32_t pSize )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		return fBoardFW->ReadBlockRegValue( pRegNode, pSize );
	}

	void BeBoardInterface::getBoardInfo( const BeBoard* pBoard )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->getBoardInfo();
	}

	void BeBoardInterface::ConfigureBoard( const BeBoard* pBoard )
	{
		setBoard( pBoard->getBeId() );
		fBoardFW->ConfigureBoard( pBoard );
	}


	void BeBoardInterface::StartThread( BeBoard* pBoard , uint32_t uNbAcq, HwInterfaceVisitor* visitor)
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->StartThread( pBoard, uNbAcq, visitor );
	}

	void BeBoardInterface::StopThread( BeBoard* pBoard )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->StopThread();
	}

	int BeBoardInterface::getNumAcqThread( BeBoard* pBoard )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		return fBoardFW->getNumAcqThread();
	}

	bool BeBoardInterface::isRunningThread( BeBoard* pBoard )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		return fBoardFW->isRunningThread();
	}

	void BeBoardInterface::Start( BeBoard* pBoard )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->Start();
	}


	void BeBoardInterface::Stop( BeBoard* pBoard, uint32_t pNthAcq )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->Stop( pNthAcq );
	}


	void BeBoardInterface::Pause( BeBoard* pBoard )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->Pause();
	}


	void BeBoardInterface::Resume( BeBoard* pBoard )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->Resume();
	}


	uint32_t BeBoardInterface::ReadData( BeBoard* pBoard, uint32_t pNthAcq, bool pBreakTrigger )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->ReadData( pBoard, pNthAcq, pBreakTrigger );
	}

	const Event* BeBoardInterface::GetNextEvent( const BeBoard* pBoard )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->GetNextEvent( pBoard );
	}

	const char* BeBoardInterface::GetBuffer( const BeBoard* pBoard, uint32_t& pBufSize )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->GetBuffer( pBufSize );
	}

	const uhal::Node& BeBoardInterface::getUhalNode( const BeBoard* pBoard, const std::string& pStrPath )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		return fBoardFW->getUhalNode( pStrPath );
	}

	uhal::HwInterface* BeBoardInterface::getHardwareInterface( const BeBoard* pBoard )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		return fBoardFW->getHardwareInterface();
	}


	void BeBoardInterface::FlashProm( BeBoard* pBoard, uint16_t numConfig, const char* pstrFile )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		fBoardFW->FlashProm( numConfig, pstrFile );
	}

	const FpgaConfig* BeBoardInterface::getConfiguringFpga( BeBoard* pBoard )
	{
		setBoard( pBoard->getBeBoardIdentifier() );
		return fBoardFW->getConfiguringFpga();
	}
}
