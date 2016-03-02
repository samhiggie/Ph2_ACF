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

namespace Ph2_HwInterface {

    CbcInterface::CbcInterface ( const BeBoardFWMap& pBoardMap ) :
        fBoardMap ( pBoardMap ),
        fBoardFW ( nullptr ),
        prevBoardIdentifier ( 65535 ),
        fRegisterCount ( 0 ),
        fTransactionCount ( 0 )
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

    void CbcInterface::setBoard ( uint16_t pBoardIdentifier )
    {
        if ( prevBoardIdentifier != pBoardIdentifier )
        {
            BeBoardFWMap::iterator i = fBoardMap.find ( pBoardIdentifier );

            if ( i == fBoardMap.end() )
                std::cout << "The Board: " << + ( pBoardIdentifier >> 8 ) << "  doesn't exist" << std::endl;
            else
            {
                fBoardFW = i->second;
                prevBoardIdentifier = pBoardIdentifier;
            }
        }
    }


    bool CbcInterface::ConfigureCbc ( const Cbc* pCbc, bool pVerifLoop, uint32_t pBlockSize )
    {
        //first, identify the correct BeBoardFWInterface
        setBoard ( pCbc->getBeBoardIdentifier() );
        // then send a CBC Hard Reset
        this->CbcFastReset ( pCbc );

        //vector to encode all the registers into
        std::vector<uint32_t> cVec;

        //Deal with the CbcRegItems and encode them

        CbcRegMap cCbcRegMap = pCbc->getRegMap();

        for ( auto& cRegItem : cCbcRegMap )
        {
            fBoardFW->EncodeReg (cRegItem, pCbc->getCbcId(), cVec, pVerifLoop, true);
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }

        // write the registers, the answer will be in the same cVec
        bool cSuccess = fBoardFW->WriteCbcBlockReg (pCbc->getFeId(), cVec, pVerifLoop);

#ifdef COUNT_FLAG
        fTransactionCount++;
#endif

        return cSuccess;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    setBoard ( pCbc->getBeBoardIdentifier() );

    this->CbcFastReset ( pCbc );
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

            EncodeReg ( cIt->second, pCbc->getCbcId(), cVecWrite );

            if ( pVerifLoop )
            {
                CbcRegItem cItem = cIt->second;
                cItem.fValue = 0;

                EncodeReg ( cItem, pCbc->getCbcId(), cVecRead );
            }

#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
            cCounter++;
        }

        fBoardFW->WriteCbcBlockReg ( pCbc->getFeId(), cVecWrite );

#ifdef COUNT_FLAG
        fTransactionCount++;
#endif

        if ( pVerifLoop )
        {
            uint8_t cCbcId = pCbc->getCbcId();

            fBoardFW->ReadCbcBlockReg ( pCbc->getFeId(), cVecRead );


            // for ( int i = 0; i < cVecWrite.size(); i++ )
            // {
            //  std::cout << "         feididpaaaaaaaavvvvvvvv" << std::endl;
            //  std::cout << static_cast<std::bitset<32> >( cVecWrite.at( i ) ) << std::endl << static_cast<std::bitset<32> >( cVecRead.at( i ) ) << std::endl << std::endl;
            // }

            // only if I have a mismatch will i decode word by word and compare
            if ( cVecWrite != cVecRead )
            {
                bool cAllgood = false;
                int cIterationCounter = 1;

                while ( !cAllgood )
                {
                    if ( cAllgood ) break;

                    std::vector<uint32_t> cWrite_again;
                    std::vector<uint32_t> cRead_again;

                    auto cMismatchWord = std::mismatch ( cVecWrite.begin(), cVecWrite.end(), cVecRead.begin() );

                    while ( cMismatchWord.first != cVecWrite.end() )
                    {
                        CbcRegItem cRegItemWrite;
                        DecodeReg ( cRegItemWrite, cCbcId, *cMismatchWord.first );
                        CbcRegItem cRegItemRead;
                        DecodeReg ( cRegItemRead, cCbcId, *cMismatchWord.second );

                        // uint32_t index = std::distance( cVecWrite.end(), cMismatchWord.first );
                        // std::advance( cIt, index );
                        // std::string cMismatchName = cIt->first;

                        if ( cIterationCounter == 5 )
                        {
                            std::cout << RED << "\nERROR !!!\nReadback value not the same after 5 Iteration for Register @ Page: "  << int ( cRegItemWrite.fPage ) << " Address: " << int ( cRegItemWrite.fAddress ) << "\n" << std::hex << "Written Value : 0x" << int ( cRegItemWrite.fValue ) << "\nReadback Value : 0x" << int ( cRegItemRead.fValue ) << std::dec << std::endl;
                            std::cout << "Cbc Id : " << int ( pCbc->getCbcId() ) << RESET << std::endl << std::endl;
                            std::cout << BOLDRED << "Failed to write register in " << cIterationCounter << " trys! Giving up!" << RESET << std::endl;
                        }

                        cMismatchWord = std::mismatch ( ++cMismatchWord.first, cVecWrite.end(), ++cMismatchWord.second );
                        // mypause();

                        // cWrite_again.push_back( *cMismatchWord.first );
                        EncodeReg ( cRegItemWrite, cCbcId, cWrite_again );
                        cRegItemRead.fValue = 0;
                        EncodeReg ( cRegItemRead, cCbcId, cRead_again );
                    }

                    fBoardFW->WriteCbcBlockReg ( pCbc->getFeId(), cWrite_again );
                    fBoardFW->ReadCbcBlockReg ( pCbc->getFeId(), cRead_again );

                    if ( cWrite_again != cRead_again )
                    {
                        if ( cIterationCounter == 5 ) break;

                        cVecWrite.clear();
                        cVecWrite = cWrite_again;
                        cVecRead.clear();
                        cVecRead = cRead_again;
                        cIterationCounter++;
                    }
                    else
                    {
                        std::cout << BLUE << "Managed to write all registers correctly in " << cIterationCounter << " Iteration(s)!" << RESET << std::endl;
                        cAllgood = true;
                    }
                }
            }
        }
    }
}



bool CbcInterface::WriteCbcReg ( Cbc* pCbc, const std::string& pRegNode, uint8_t pValue, bool pVerifLoop )
{
    //first, identify the correct BeBoardFWInterface
    setBoard ( pCbc->getBeBoardIdentifier() );

    //next, get the reg item
    CbcRegItem cRegItem = pCbc->getRegItem ( pRegNode );
    cRegItem.fValue = pValue;

    //vector for transaction
    std::vector<uint32_t> cVec;

    // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
    fBoardFW->EncodeReg ( cRegItem, pCbc->getCbcId(), cVec, pVerifLoop, true );
    // write the register, the answer is in the same cVec
    bool cSuccess = fBoardFW->WriteCbcBlockReg ( pCbc->getFeId(), cVec, pVerifLoop );

    //update the HWDescription object
    if (cSuccess)
        pCbc->setReg ( pRegNode, pValue );

#ifdef COUNT_FLAG
    fRegisterCount++;
    fTransactionCount++;
#endif

    return cSuccess;
}

bool CbcInterface::WriteCbcMultReg ( Cbc* pCbc, const std::vector< std::pair<std::string, uint8_t> >& pVecReq, bool pVerifLoop )
{
    //first, identify the correct BeBoardFWInterface
    setBoard ( pCbc->getBeBoardIdentifier() );

    std::vector<uint32_t> cVec;

    //Deal with the CbcRegItems and encode them
    CbcRegItem cRegItem;

    for ( const auto& cReg : pVecReq )
    {
        cRegItem = pCbc->getRegItem ( cReg.first );
        cRegItem.fValue = cReg.second;

        fBoardFW->EncodeReg ( cRegItem, pCbc->getCbcId(), cVec, pVerifLoop, true );
#ifdef COUNT_FLAG
        fRegisterCount++;
#endif
    }

    // write the registerss, the answer will be in the same cVec
    bool cSuccess = fBoardFW->WriteCbcBlockReg (pCbc->getFeId(), cVec, pVerifLoop);

#ifdef COUNT_FLAG
    fTransactionCount++;
#endif

    // if the transaction is successfull, update the HWDescription object
    if (cSuccess)
    {
        for ( const auto& cReg : pVecReq )
        {
            cRegItem = pCbc->getRegItem ( cReg.first );
            pCbc->setReg ( cReg.first, cReg.second );
        }
    }

    return cSuccess;
}



uint8_t CbcInterface::ReadCbcReg ( Cbc* pCbc, const std::string& pRegNode )
{
    setBoard ( pCbc->getBeBoardIdentifier() );

    CbcRegItem cRegItem = pCbc->getRegItem ( pRegNode );

    std::vector<uint32_t> cVecReq;

    fBoardFW->EncodeReg ( cRegItem, pCbc->getCbcId(), cVecReq, true, false );
    fBoardFW->ReadCbcBlockReg ( pCbc->getFeId(), cVecReq );

    //bools to find the values of failed and read
    bool cFailed;
    bool cRead;
    fBoardFW->DecodeReg ( cRegItem, cCbcId, cVecReq[0], cRead, cFailed );

    if (!cFailed) pCbc->setReg ( pRegNode, cRegItem.fValue );

    return cRegItem.fValue;
}


void CbcInterface::ReadCbcMultReg ( Cbc* pCbc, const std::vector<std::string>& pVecReg )
{
    //first, identify the correct BeBoardFWInterface
    setBoard ( pCbc->getBeBoardIdentifier() );

    std::vector<uint32_t> cVec;

    //Deal with the CbcRegItems and encode them
    CbcRegItem cRegItem;

    for ( const auto& cReg : pVecReq )
    {
        cRegItem = pCbc->getRegItem ( cReg.first );

        fBoardFW->EncodeReg ( cRegItem, pCbc->getCbcId(), cVec, true, false );
#ifdef COUNT_FLAG
        fRegisterCount++;
#endif
    }

    // write the registers, the answer will be in the same cVec
    fBoardFW->ReadCbcBlockReg (pCbc->getFeId(), cVec);

#ifdef COUNT_FLAG
    fTransactionCount++;
#endif

    bool cFailed;
    bool cRead;

    //update the HWDescription object with the value I just read
    for ( const auto& cReadWord : cVec )
    {
        DecodeReg ( cRegItem, pCbc->getFeId(), cReadWord, cRead, cFailed );

        if (!cFailed) pCbc->setReg ( cReadWord, cRegItem.fValue );
    }
}


//void CbcInterface::ReadAllCbc ( const Module* pModule )
//{
//CbcRegItem cRegItem;
//uint8_t cCbcId;
//std::vector<uint32_t> cVecReq;
//std::vector<std::string> cVecRegNode;

//int cMissed = 0;

//setBoard ( pModule->getBeBoardIdentifier() );

//for ( uint8_t i = 0; i < pModule->getNCbc(); i++ )
//{

//if ( pModule->getCbc ( i + cMissed ) == nullptr )
//{
//i--;
//cMissed++;
//}

//else
//{

//Cbc* cCbc = pModule->getCbc ( i + cMissed );

//const CbcRegMap& cCbcRegMap = cCbc->getRegMap();

//for ( auto& it : cCbcRegMap )
//{
//EncodeReg ( it.second, cCbc->getCbcId(), cVecReq );
//cVecRegNode.push_back ( it.first );
//}

//fBoardFW->ReadCbcBlockReg ( cCbc->getFeId(), cVecReq );

//for ( uint32_t j = 0; j < cVecReq.size(); j++ )
//{
//DecodeReg ( cRegItem, cCbcId, cVecReq[j] );

//cCbc->setReg ( cVecRegNode.at ( j ), cRegItem.fValue );
//}
//}
//}
//}


void CbcInterface::WriteBroadcast ( const Module* pModule, const std::string& pRegNode, uint32_t pValue )
{
    //first set the correct BeBoard
    setBoard ( pModule->getBeBoardIdentifier() );

    CbcRegItem cRegItem = pModule->fCbcVector.at (0)->getRegItem ( pRegNode );
    cRegItem.fValue = pValue;

    //vector for transaction
    std::vector<uint32_t> cVec;

    // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
    // the 1st boolean could be true if I acually wanted to read back from each CBC but this somehow does not make sense!
    fBoardFW->BCEncodeReg ( cRegItem, pModule->fCbcVector.size(), cVec, false, true );

    bool cSuccess = fBoardFW->BCWriteCbcBlockReg ( pCbc->getFeId(), cVec, false );

#ifdef COUNT_FLAG
    fRegisterCount++;
    fTransactionCount++;
#endif

    //update the HWDescription object -- not sure if the transaction was successfull
    if (cSuccess)
        for (auto& cCbc : pModule->fCbcVector)
            cCbc->setReg ( pRegNode, pValue );
}

void CbcInterface::WriteBroadcastMultReg (const Module* pModule, const std::vector<std::pair<std::string, uint8_t>> pVecReg)
{
    //first set the correct BeBoard
    setBoard ( pModule->getBeBoardIdentifier() );

    std::vector<uint32_t> cVec;

    //Deal with the CbcRegItems and encode them
    CbcRegItem cRegItem;

    for ( const auto& cReg : pVecReq )
    {
        cRegItem = pModule->fCbcVector.at (0)->getRegItem ( cReg.first );
        cRegItem.fValue = cReg.second;

        fBoardFW->BCEncodeReg ( cRegItem, pModule->fCbcVector.size(), cVec, false, true );
#ifdef COUNT_FLAG
        fRegisterCount++;
#endif
    }

    // write the registerss, the answer will be in the same cVec
    bool cSuccess = fBoardFW->BCWriteCbcBlockReg (pCbc->getFeId(), cVec, false);

#ifdef COUNT_FLAG
    fTransactionCount++;
#endif

    if (cSuccess)
        for (auto& cCbc : pModule->fCbcVector)
            for (auto& cReg : pVecReg)
            {
                cRegItem = cCbc->getRegItem ( cReg.first );
                cCbc->setReg ( cReg.first, cReg.second );
            }
}

void CbcInterface::CbcHardReset ( const Cbc* pCbc )
{
    setBoard ( pCbc->getBeBoardIdentifier() );

    fBoardFW->CbcHardReset();
}

void CbcInterface::CbcFastReset ( const Cbc* pCbc )
{
    setBoard ( pCbc->getBeBoardIdentifier() );

    fBoardFW->CbcFastReset();
}

}
