/*

    FileName :                     Data.cc
    Content :                      Data handling from DAQ
    Programmer :                   Nicolas PIERRE
    Version :                      1.0
    Date of creation :             10/07/14
    Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/Data.h"
#include <iostream>

namespace Ph2_HwInterface {
    //Data Class

    // copy constructor
    Data::Data ( const Data& pD ) :

        // Initialise( pD.fNevents );
        fNevents ( pD.fNevents ),
        fCurrentEvent ( pD.fCurrentEvent ),
        fNCbc ( pD.fNCbc ),
        fEventSize ( pD.fEventSize )
    {
    }


    void Data::Set ( const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, bool swapBits )
    {
        Reset();

        fNevents = static_cast<uint32_t> ( pNevents );
        fEventSize = static_cast<uint32_t> ( (pData.size() ) / fNevents );
        fNCbc = ( fEventSize - ( EVENT_HEADER_TDC_SIZE_32 ) ) / ( CBC_EVENT_SIZE_32 );
        std::vector<uint32_t> flist;

        //use and index to decide wether to swap a word or not
        uint32_t cIndex = 0;

        for ( auto word : pData )
        {
            //if the index is greater than 0 and a multiple of the event size in 32 bit words, reset index to 0
            if (cIndex > 0 && cIndex % fEventSize == 0) cIndex = 0;

            if (swapBits && is_channel_data (cIndex, fNCbc) ) word = reverse_bits (word);

            flist.push_back (  word );
            cIndex++;
        }

#ifdef __CBCDAQ_DEV__
        std::cout << "Initializing list with " << flist.size() << " 32 bit words, should be equal to " << pData.size()
                  << " 32 bit words containing data from "
                  << fNevents << "  Events with an eventbuffer size of " << fEventSize << " and " << fNCbc
                  << " CBCs each! " << EVENT_HEADER_TDC_SIZE_32 << " " << CBC_EVENT_SIZE_32 << std::endl;
#endif

        // Fill fEventList
        std::vector<uint32_t> lvec;

        for ( auto i = 0; i < flist.size(); ++i )
        {
             //std::cout << std::setw(3) <<  i << " ### " << std::bitset<32>(flist.at(i)) << std::endl;
             //if((i+1)%fEventSize == 0 && i >0 ) std::cout << std::endl << std::endl;

            lvec.push_back ( flist[i] );

            if ( i > 0 &&  (i+1) % fEventSize == 0 )
            { 
                fEventList.push_back ( new Event ( pBoard, fNCbc, lvec ) );
                lvec.clear();
            }
        }
    }

    void Data::Reset()
    {
        for ( auto& pevt : fEventList )
            delete pevt;

        fEventList.clear();
        fCurrentEvent = 0;
    }
}
