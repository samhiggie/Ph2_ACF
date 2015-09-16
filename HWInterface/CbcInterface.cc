/*

        FileName :                     CbcInterface.cc
        Content :                      User Interface to the Cbcs
        Programmer :                   Lorenzo BIDEGAIN, Nicolas PIERRE, Georg AUZINGER
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#include "CbcInterface.h"
#include "../Utils/ConsoleColor.h"

#define DEV_FLAG 0
// #define COUNT_FLAG 0

namespace Ph2_HwInterface
{

	CbcInterface::CbcInterface( const BeBoardFWMap& pBoardMap ) :
		fBoardMap( pBoardMap ),
		fBoardFW( nullptr ),
		prevBoardIdentifier( 65535 ),
		fRegisterCount( 0 ),
		fTransactionCount( 0 )
	{
#ifdef COUNT_FLAG
		std::cout << "Counting number of Transactions!" << std::endl;
#endif
	}

	CbcInterface::~CbcInterface()
	{
	}

	void CbcInterface::output()
	{
#ifdef COUNT_FLAG
		std::cout << "This instance of HWInterface::CbcInterface wrote (only write!) " << fRegisterCount << " Registers in " << fTransactionCount << " Transactions (only write!)! " << std::endl;
#endif
	}

	void CbcInterface::setBoard( uint16_t pBoardIdentifier )
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


	void CbcInterface::ConfigureCbc( const Cbc* pCbc, bool pVerifLoop, uint32_t pBlockSize )
	{
		this->CbcFastReset( pCbc );

		setBoard( pCbc->getBeBoardIdentifier() );

		CbcRegMap cCbcRegMap = pCbc->getRegMap();
		CbcRegMap::iterator cIt = cCbcRegMap.begin();

		while ( cIt != cCbcRegMap.end() )
		{
			std::vector<uint32_t> cVecWrite;
			std::vector<uint32_t> cVecRead;

			uint32_t cCounter = 0;

			for ( cIt; cIt != cCbcRegMap.end(); cIt++ )
			{

				if ( cCounter >= pBlockSize ) break;

				EncodeReg( cIt->second, pCbc->getCbcId(), cVecWrite );

				if ( pVerifLoop )
				{
					CbcRegItem cItem = cIt->second;
					cItem.fValue = 0;

					EncodeReg( cItem, pCbc->getCbcId(), cVecRead );
				}
#ifdef COUNT_FLAG
				fRegisterCount++;
#endif
				cCounter++;
			}

			fBoardFW->WriteCbcBlockReg( pCbc->getFeId(), cVecWrite );

#ifdef COUNT_FLAG
			fTransactionCount++;
#endif
			if ( pVerifLoop )
			{
				uint8_t cCbcId = pCbc->getCbcId();

				fBoardFW->ReadCbcBlockReg( pCbc->getFeId(), cVecRead );

                
    //             for(int i = 0; i < cVecWrite.size(); i++)
    //             {
    //             	std::cout << "           <-idpaaaaaaaavvvvvvvv" << std::endl;
    //                 std::cout << static_cast<std::bitset<32> >(cVecWrite.at(i)) << std::endl << static_cast<std::bitset<32> > (cVecRead.at(i)) << std::endl << std::endl;
				// }
				
                // only if I have a mismatch will i decode word by word and compare
				if ( cVecWrite != cVecRead )
				{

					auto cMismatchWord = std::mismatch( cVecWrite.begin(), cVecWrite.end(), cVecRead.begin() );
					while ( cMismatchWord.first != cVecWrite.end() )
					{
						CbcRegItem cRegItemWrite;
						DecodeReg( cRegItemWrite, cCbcId, *cMismatchWord.first );
						CbcRegItem cRegItemRead;
						DecodeReg( cRegItemRead, cCbcId, *cMismatchWord.second );

						// uint32_t index = std::distance( cVecWrite.end(), cMismatchWord.first );
						// std::advance( cIt, index );
						// std::string cMismatchName = cIt->first;

						std::cout << RED << "\nERROR !!!\nReadback value not the same for Register @ Page: "  << int( cRegItemWrite.fPage ) << " Address: " << int( cRegItemWrite.fAddress ) << "\n" << std::hex << "Written Value : 0x" << int( cRegItemWrite.fValue ) << "\nReadback Value : 0x" << int( cRegItemRead.fValue ) << std::dec << std::endl;
						std::cout << "Cbc Id : " << int( pCbc->getCbcId() ) << RESET << std::endl << std::endl;
						cMismatchWord = std::mismatch( ++cMismatchWord.first, cVecWrite.end(), ++cMismatchWord.second );
						// mypause();
					}
				}

			}
		}
	}



	bool CbcInterface::WriteCbcReg( Cbc* pCbc, const std::string& pRegNode, uint8_t pValue, bool pVerifLoop )
	{
		CbcRegItem cRegItem = ( pCbc->getRegMap() )[pRegNode];
		std::vector<uint32_t> cVecWrite;
		std::vector<uint32_t> cVecRead;

		cRegItem.fValue = pValue;

		setBoard( pCbc->getBeBoardIdentifier() );

		EncodeReg( cRegItem, pCbc->getCbcId(), cVecWrite );

		fBoardFW->WriteCbcBlockReg( pCbc->getFeId(), cVecWrite );

#ifdef COUNT_FLAG
		fRegisterCount++;
		fTransactionCount++;
#endif

		pCbc->setReg( pRegNode, pValue );

		if ( pVerifLoop )
		{
			uint8_t cCbcId = pCbc->getCbcId();

			cRegItem.fValue = 0;

			EncodeReg( cRegItem, pCbc->getCbcId(), cVecRead );

			fBoardFW->ReadCbcBlockReg( pCbc->getFeId(), cVecRead );

			if ( cVecWrite != cVecRead )
			{

				CbcRegItem cReadItem;
				CbcRegItem cWriteItem;
				DecodeReg( cWriteItem, cCbcId, cVecWrite.at( 0 ) );
				DecodeReg( cReadItem, cCbcId, cVecRead.at( 0 ) );
				pCbc->setReg( pRegNode, cReadItem.fValue );

				std::cout << RED <<  "ERROR !!!\nReadback Value different for Register : " << pRegNode << "\n" << std::hex << "Written Value : 0x" << +pValue << "\nReadback Value : 0x" << int( cRegItem.fValue ) << std::dec << std::endl;
				std::cout << "Register Adress : " << int( cRegItem.fAddress ) << std::endl;
				std::cout << "Cbc Id : " << +cCbcId << RESET << std::endl << std::endl;
				// mypause();
				return false;
			}

			else return true;
		}
	}

	void CbcInterface::WriteCbcMultReg( Cbc* pCbc, const std::vector< std::pair<std::string, uint8_t> >& pVecReq, bool pVerifLoop )
	{
		setBoard( pCbc->getBeBoardIdentifier() );

		std::vector<uint32_t> cVecWrite;
		std::vector<uint32_t> cVecRead;

		CbcRegItem cRegItemWrite;
		CbcRegItem cRegItemRead;

		for ( const auto& v : pVecReq )
		{
			cRegItemWrite = ( pCbc->getRegMap() )[v.first];
			cRegItemWrite.fValue = v.second;

			EncodeReg( cRegItemWrite, pCbc->getCbcId(), cVecWrite );

			pCbc->setReg( v.first, cRegItemWrite.fValue );

			if ( pVerifLoop )
			{
				cRegItemRead = ( pCbc->getRegMap() )[v.first];
				cRegItemRead.fValue = 0;

				EncodeReg( cRegItemRead, pCbc->getCbcId(), cVecRead );
			}

#ifdef COUNT_FLAG
			fRegisterCount++;
#endif
		}

		fBoardFW->WriteCbcBlockReg( pCbc->getFeId(), cVecWrite );

#ifdef COUNT_FLAG
		fRegisterCount++;
#endif


		if ( pVerifLoop )
		{
			uint8_t cCbcId = pCbc->getCbcId();

			fBoardFW->ReadCbcBlockReg( pCbc->getFeId(), cVecRead );

			// only if I have a mismatch will i decode word by word and compare
			if ( cVecWrite != cVecRead )
			{

				auto cMismatchWord = std::mismatch( cVecWrite.begin(), cVecWrite.end(), cVecRead.begin() );
				while ( cMismatchWord.first != cVecWrite.end() )
				{
					CbcRegItem cRegItemWrite;
					DecodeReg( cRegItemWrite, cCbcId, *cMismatchWord.first );
					CbcRegItem cRegItemRead;
					DecodeReg( cRegItemRead, cCbcId, *cMismatchWord.second );

					uint32_t index = std::distance( cVecWrite.begin(), cMismatchWord.first );
					std::string cMismatchName = pVecReq.at( index ).first;

					std::cout << RED << "\nERROR !!!\nReadback value not the same for Register : " << cMismatchName << " @ Page: " << int( cRegItemWrite.fPage ) << " Address: " << int( cRegItemWrite.fAddress ) << "\n"  << std::hex << "Written Value : 0x" << int( cRegItemWrite.fValue ) << "\nReadback Value : 0x" << int( cRegItemRead.fValue ) << std::dec << std::endl;
					std::cout << "Cbc Id : " << int( pCbc->getCbcId() ) << RESET << std::endl << std::endl;
					cMismatchWord = std::mismatch( ++cMismatchWord.first, cVecWrite.end(), ++cMismatchWord.second );
					// mypause();
				}
			}
		}
	}

	uint8_t CbcInterface::ReadCbcReg( Cbc* pCbc, const std::string& pRegNode )
	{

		uint8_t cCbcId;
		CbcRegItem cRegItem = ( pCbc->getRegMap() )[pRegNode];
		std::vector<uint32_t> cVecReq;

		setBoard( pCbc->getBeBoardIdentifier() );

		EncodeReg( cRegItem, pCbc->getCbcId(), cVecReq );

		fBoardFW->ReadCbcBlockReg( pCbc->getFeId(), cVecReq );

		DecodeReg( cRegItem, cCbcId, cVecReq[0] );

		pCbc->setReg( pRegNode, cRegItem.fValue );
		return cRegItem.fValue;
	}


	void CbcInterface::ReadCbcMultReg( Cbc* pCbc, const std::vector<std::string>& pVecReg )
	{

		CbcRegItem cRegItem;
		std::vector<uint32_t> cVecReq;

		setBoard( pCbc->getBeBoardIdentifier() );

		for ( const auto& v : pVecReg )
			EncodeReg( pCbc->getRegMap()[v], pCbc->getCbcId(), cVecReq );

		fBoardFW->ReadCbcBlockReg( pCbc->getFeId(), cVecReq );

		int iReq=0;
		for ( const auto& v : pVecReg ){
			DecodeReg( cRegItem, 0, cVecReq[iReq++] ); 
			pCbc->setReg( v, cRegItem.fValue );
		}
	}


	void CbcInterface::ReadAllCbc( const Module* pModule )
	{
		CbcRegItem cRegItem;
		uint8_t cCbcId;
		std::vector<uint32_t> cVecReq;
		std::vector<std::string> cVecRegNode;

		int cMissed = 0;

		setBoard( pModule->getBeBoardIdentifier() );

		for ( uint8_t i = 0; i < pModule->getNCbc(); i++ )
		{

			if ( pModule->getCbc( i + cMissed ) == nullptr )
			{
				i--;
				cMissed++;
			}

			else
			{

				Cbc* cCbc = pModule->getCbc( i + cMissed );

				const CbcRegMap& cCbcRegMap = cCbc->getRegMap();

				for ( auto& it : cCbcRegMap )
				{
					EncodeReg( it.second, cCbc->getCbcId(), cVecReq );
					cVecRegNode.push_back( it.first );
				}

				fBoardFW->ReadCbcBlockReg( cCbc->getFeId(), cVecReq );

				for ( uint32_t j = 0; j < cVecReq.size(); j++ )
				{
					DecodeReg( cRegItem, cCbcId, cVecReq[j] );

					cCbc->setReg( cVecRegNode.at( j ), cRegItem.fValue );
				}
			}
		}
	}


	void CbcInterface::WriteBroadcast( const Module* pModule, const std::string& pRegNode, uint32_t pValue )
	{
		setBoard( pModule->getBeBoardIdentifier() );

		uint8_t cCbcId = 0xFF;
		std::vector<uint32_t> cVecReq;

		int cMissed = 0;

		for ( uint8_t i = 0; i < pModule->getNCbc(); i++ )
		{
			if ( pModule->getCbc( i + cMissed ) == nullptr )
			{
				i--;
				cMissed++;
			}

			/*
			   It makes the broadcast only the first time it finds an
			   existing Cbc and then update all the other Cbcs.
			 */
			else if ( i == 0 && pModule->getCbc( i + cMissed ) != nullptr )
			{
				Cbc* cCbc = pModule->getCbc( i + cMissed );
				CbcRegItem cRegItem = ( cCbc->getRegMap() )[pRegNode];
				cRegItem.fValue = pValue;

				EncodeReg( cRegItem, cCbcId, cVecReq );

				fBoardFW->WriteCbcBlockReg( cCbc->getFeId(), cVecReq );

				ReadCbcReg( pModule->getCbc( i + cMissed ), pRegNode );
			}

			else
				ReadCbcReg( pModule->getCbc( i + cMissed ), pRegNode );
		}
	}

	void CbcInterface::CbcHardReset( const Cbc* pCbc )
	{
		setBoard( pCbc->getBeBoardIdentifier() );

		fBoardFW->WriteReg( CBC_HARD_RESET, 1 );

		usleep( 200000 );

		fBoardFW->WriteReg( CBC_HARD_RESET, 0 );

		usleep( 200000 );
	}

	void CbcInterface::CbcFastReset( const Cbc* pCbc )
	{
		setBoard( pCbc->getBeBoardIdentifier() );

		fBoardFW->WriteReg( CBC_FAST_RESET, 1 );

		usleep( 200000 );

		fBoardFW->WriteReg( CBC_FAST_RESET, 0 );
	}


	void CbcInterface::EncodeReg( const CbcRegItem& pRegItem, uint8_t pCbcId, std::vector<uint32_t>& pVecReq )
	{
		fBoardFW->EncodeReg( pRegItem, pCbcId, pVecReq );
	}


	void CbcInterface::DecodeReg( CbcRegItem& pRegItem, uint8_t pCbcId, uint32_t pWord )
	{
		fBoardFW->DecodeReg( pRegItem, pCbcId, pWord );
	}

}
