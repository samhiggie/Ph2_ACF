/*

        FileName :                     Event.cc
        Content :                      Event handling from DAQ
        Programmer :                   Nicolas PIERRE
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/Event.h"

using namespace Ph2_HwDescription;


namespace Ph2_HwInterface {

    double Cluster::getBaricentre()
    {
        return fFirstStrip + double (fClusterWidth) / 2. - 0.5;
    }

    // Event implementation
    bool Event::operator== (const Event& pEvent) const
    {
        return fEventDataMap == pEvent.fEventDataMap;
    }

    bool Event::Bit ( uint8_t pFeId, uint8_t pCbcId, uint32_t pPosition ) const
    {
        uint32_t cWordP = pPosition / 32;
        uint32_t cBitP = pPosition % 32;

        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            if (cWordP >= cData->second.size() ) return false;

            return ( (cData->second.at (cWordP) >> (31 - cBitP) ) & 0x1);
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return false;
        }
    }


    std::string Event::BitString ( uint8_t pFeId, uint8_t pCbcId, uint32_t pOffset, uint32_t pWidth ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::ostringstream os;

            for ( uint32_t i = 0; i < pWidth; ++i )
            {
                uint32_t pos = i + pOffset;
                uint32_t cWordP = pos / 32;
                uint32_t cBitP = pos % 32;

                if ( cWordP >= cData->second.size() ) break;

                //os << ((cbcData[cByteP] & ( 1 << ( 7 - cBitP ) ))?"1":"0");
                os << ( ( cData->second[cWordP] >> ( 31 - cBitP ) ) & 0x1 );
            }

            return os.str();

        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return "";
        }
    }

    std::vector<bool> Event::BitVector ( uint8_t pFeId, uint8_t pCbcId, uint32_t pOffset, uint32_t pWidth ) const
    {
        std::vector<bool> blist;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::ostringstream os;

            for ( uint32_t i = 0; i < pWidth; ++i )
            {
                uint32_t pos = i + pOffset;
                uint32_t cWordP = pos / 32;
                uint32_t cBitP = pos % 32;

                if ( cWordP >= cData->second.size() ) break;

                blist.push_back ( ( cData->second[cWordP] >> ( 31 - cBitP ) ) & 0x1 );
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
    }


    //#if 0
    std::string Event::DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::stringbuf tmp;
        std::ostream os ( &tmp );
        os << std::hex << std::setfill ('0');

        //get the CBC event for pFeId and pCbcId into vector<32bit> cbcData
        std::vector< uint32_t > cbcData;
        GetCbcEvent (pFeId, pCbcId, cbcData);
        //for the first 32-bit word, use only the 22 LSBs
        //this unfortunately means that i have two leading bits that are always 0
        os << std::setw (6) << (cbcData.at (0) & 0x003FFFFF);

        //this is the body of 7 words that are full of data
        for ( uint32_t i = 1; i < 8; i++ )
            os << std::setw (8) << cbcData.at (i);

        //the last word with only 8 bits
        os << std::setw (2) << (cbcData.at (8) & 0xFF000000);


        //uint32_t cFirstByteP = OFFSET_CBCDATA / 8;
        //uint32_t cFirstBitP = OFFSET_CBCDATA % 8;
        //uint32_t cLastByteP = ( cFirstByteP + WIDTH_CBCDATA - 1 ) / 8;
        //uint32_t cLastBitP = ( cFirstByteP + WIDTH_CBCDATA - 1 ) % 8;

        //uint32_t cMask ( 0 );
        //uint32_t cMaskLastBit ( 0 );
        //uint32_t cMaskWidth ( 0 );

        ////First byte
        //cMaskLastBit = cFirstByteP < cLastByteP ? 7 : cLastBitP;
        //cMaskWidth = cMaskLastBit - cFirstBitP + 1;
        //cMask = ( 1 << ( 7 - cMaskLastBit ) );

        //for ( uint32_t i = 0; i < cMaskWidth; i++ )
        //{
        //cMask <<= 1;
        //cMask |= 1;
        //}

        //os << std::uppercase << std::setw ( 2 ) << std::setfill ( '0' ) << ( GetCbcEvent ( pFeId, pCbcId, cbcData ) [cFirstByteP]&cMask );

        //if ( cFirstByteP == cLastByteP )
        //return tmp.str();

        ////Second to the second last byte
        //if ( cFirstByteP != cLastByteP - 1 )
        //{
        //for ( uint32_t j = cFirstByteP + 1; j < cLastByteP; j++ )
        //os << std::uppercase << std::setw ( 2 ) << std::setfill ( '0' ) << ( GetCbcEvent ( pFeId, pCbcId, cbcData ) [j] & 0xFF );
        //}

        ////Last byte
        //cMaskLastBit = cLastBitP;
        //cMaskWidth = cMaskLastBit + 1;
        //cMask = ( 1 << ( 7 - cMaskLastBit ) );

        //for ( uint32_t i = 0; i < cMaskWidth; i++ )
        //{
        //cMask <<= 1;
        //cMask |= 1;
        //}

        //os << std::uppercase << std::setw ( 2 ) << std::setfill ( '0' ) << ( GetCbcEvent ( pFeId, pCbcId, cbcData ) [cFirstByteP]&cMask );

        return tmp.str();
    }
    //#endif

}
