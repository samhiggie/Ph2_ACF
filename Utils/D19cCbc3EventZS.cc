/*

        FileName :                     Event.cc
        Content :                      Event handling from DAQ
        Programmer :                   Nicolas PIERRE
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/D19cCbc3EventZS.h"

using namespace Ph2_HwDescription;


namespace Ph2_HwInterface {

    // Event implementation
    D19cCbc3EventZS::D19cCbc3EventZS ( const BeBoard* pBoard,  uint32_t pZSEventSize, const std::vector<uint32_t>& list )
    {
        SetEvent ( pBoard, pZSEventSize, list );
    }

    void D19cCbc3EventZS::SetEvent( const BeBoard* pBoard, uint32_t pZSEventSize, const std::vector<uint32_t>& list)
    {
        //LOG(INFO) << "event size: " << +pZSEventSize;

        // these two values come from width of the hybrid/cbc enabled mask
        uint8_t fMaxHybrids = 8;
        uint8_t fMaxCBCs = 8;

        fEventSize = 0x0000FFFF & list.at (0);

        if (fEventSize != list.size() || fEventSize != pZSEventSize )
            LOG (ERROR) << "Vector size doesnt match the BLOCK_SIZE in Header1";

        uint8_t header1_size = (0xFF000000 & list.at (0) ) >> 24;

        if (header1_size != D19C_EVENT_HEADER1_SIZE_32_CBC3)
            LOG (ERROR) << "Misaligned data: Header1 size doesnt correspond to the one sent from firmware";

        fNFe_software = static_cast<uint8_t> (pBoard->getNFe() );
        fNFe_event = 0;
        fFeMask_software = 0;
        fFeMask_event = static_cast<uint8_t> ( (0x00FF0000 & list.at (0) ) >> 16);

        for (uint8_t bit = 0; bit < fMaxHybrids; bit++)
        {
            if ( (fFeMask_event >> bit) & 1)
                fNFe_event ++;
        }
        for (uint8_t cFe = 0; cFe < fNFe_software; cFe++)
        {
            uint8_t cFeId = pBoard->fModuleVector.at(cFe)->getFeId();
            fFeMask_software |= 1 << cFeId;
        }

        fDummySize = 0x000000FF & list.at (1);
        fEventCount = 0x00FFFFFF &  list.at (2);
        fBunch = 0xFFFFFFFF & list.at (3);
        fTDC = 0x000000FF & list.at (4);
        fTLUTriggerID = (0x00FFFF00 & list.at (4) ) >> 8;

        fBeId = pBoard->getBeId();
        fBeFWType = 0;
        fCBCDataType = (0x0000FF00 & list.at(1)) >> 8;
        fBeStatus = 0;
        fNCbc = 0;
        fEventDataSize = fEventSize;

        // now iterate through modules
        uint32_t address_offset = D19C_EVENT_HEADER1_SIZE_32_CBC3;

        for (uint8_t cFe = 0; cFe < fNFe_software; cFe++)
        {
            uint8_t cFeId = pBoard->fModuleVector.at(cFe)->getFeId();
            if (((fFeMask_software >> cFeId) & 1) && ((fFeMask_event >> cFeId) & 1))
            {
                uint8_t chip_data_mask = static_cast<uint8_t> ( ( (0xFF000000) & list.at (address_offset + 0) ) >> 24);
                //LOG(INFO) << "Chip data mask: " << std::hex << +chip_data_mask << std::dec;

                uint16_t fe_data_size = static_cast<uint16_t> ( ( (0x0000FFFF) & list.at (address_offset + 0) ) >> 0);
                uint8_t header2_size = (0x00FF0000 & list.at (address_offset + 0) ) >> 16;
                //LOG(INFO) << "FE Data Size: " << +fe_data_size;

                if (header2_size != D19C_EVENT_HEADER2_SIZE_32_CBC3)
                    LOG (ERROR) << "Header2 size doesnt correspond to the one sent from firmware";

                // iterating through the hybrid words
                address_offset += D19C_EVENT_HEADER2_SIZE_32_CBC3;
                uint32_t begin = address_offset;
                uint32_t end = begin+1;
                bool cLastWord = false;
                for (uint32_t word_id = address_offset; word_id < address_offset + (fe_data_size - 1); word_id++) {

                    uint8_t cChipID = (0xE0000000 & list.at(word_id)) >> 29;
                    //LOG(INFO) << "ChipId: " << +cChipID << ", Word ID: " << +word_id;
                    uint8_t cDataType = (0x18000000 & list.at(word_id)) >> 27;

                    // checks the last word
                    if (word_id == (address_offset + fe_data_size - 2)) cLastWord = true;
                    else {
                        uint8_t cNextChipID = (0xE0000000 & list.at(word_id+1)) >> 29;
                        if (cNextChipID != cChipID) cLastWord = true;
                        else cLastWord = false;
                    }

                    if( cLastWord ) {
                        // now store the ZS event
                        uint16_t cKey = encodeId (cFeId, cChipID);
                        std::vector<uint32_t> cCbcDataZS (std::next (std::begin (list), begin), std::next (std::begin (list), end) );
                        fEventDataMap[cKey] = cCbcDataZS;

                        begin = end;
                    }

                    end++;

                }

                address_offset = address_offset + fe_data_size;
            }

        }

    }

    std::string D19cCbc3EventZS::HexString() const
    {
        return "";
    }

    std::string D19cCbc3EventZS::DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::stringbuf tmp;
        std::ostream os ( &tmp );
        std::ios oldState (nullptr);
        oldState.copyfmt (os);
        os << std::hex << std::setfill ('0');

        //get the CBC event for pFeId and pCbcId into vector<32bit> cbcData
        std::vector< uint32_t > cbcData;
        GetCbcEvent (pFeId, pCbcId, cbcData);

        // trigdata
        os << std::endl;
        for (auto word : cbcData)
            os << std::setw (8) << word << std::endl;

        os.copyfmt (oldState);

        return tmp.str();
    }

    bool D19cCbc3EventZS::Error ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 2) {
                    // buf overflow and lat error
                    if (i == 0) return ((0x00000001 & word) >> 0); // lat err
                    if (i == 1) return ((0x00000002 & word) >> 1); // buf ovf
                }
            }
            return true;
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return true;
        }
    }

    uint32_t D19cCbc3EventZS::Error ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint32_t cError = 0;
            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 2) {
                    // buf overflow and lat error
                    cError = (0x00000003 & word) >> 0;
                }
            }
            return cError;
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return 0;
        }
    }

    uint32_t D19cCbc3EventZS::PipelineAddress ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint32_t cPipelineAddress = 0;
            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 2) {
                    // pipe address
                    cPipelineAddress = (0x00001ff0 & word) >> 4;
                }
            }
            return cPipelineAddress;
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return 0;
        }
    }

    bool D19cCbc3EventZS::DataBit ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 0) {
                    uint8_t cDataMask = (0x03000000 & word) >> 24;
                    if ( (cDataMask >> 0) & 1 ) {
                        uint8_t cClusterAddress = (0x000007f8 & word) >> 3;
                        uint8_t cClusterWidth = ( (0x00000007 & word) >> 0 ) + 1;

                        // check parity
                        if( (cClusterAddress-i)%2 == 0 ) {
                            // check if it's in cluster
                            if ( (i >= cClusterAddress) && (i < (cClusterAddress+2*cClusterWidth)) ) return 1;
                        }
                    }
                    if ( (cDataMask >> 1) & 1 ) {
                        uint8_t cClusterAddress = (0x003fc000 & word) >> 14;
                        uint8_t cClusterWidth = ( (0x00003800 & word) >> 11 ) + 1;

                        // check parity
                        if( (cClusterAddress-i)%2 == 0 ) {
                            // check if it's in cluster
                            if ( (i >= cClusterAddress) && (i < (cClusterAddress+2*cClusterWidth)) ) return 1;
                        }
                    }
                }
            }
            // not found
            return 0;
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return 0;
        }
    }

    std::string D19cCbc3EventZS::DataBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            // i didn't find way better than this one, because of the fact that clusters separated for sensors, and also randomly stored in the event
            bool *hit_bits = new bool[NCHANNELS];
            for(uint32_t i=0; i < NCHANNELS; i++) hit_bits[i] = false;

            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 0) {
                    uint8_t cDataMask = (0x03000000 & word) >> 24;
                    if ( (cDataMask >> 0) & 1 ) {
                        uint8_t cClusterAddress = (0x000007f8 & word) >> 3;
                        uint8_t cClusterWidth = ( (0x00000007 & word) >> 0 ) + 1;

                        for(uint8_t i = 0; i < cClusterWidth; i++)
                            hit_bits[cClusterAddress + 2*i] = true;
                    }
                    if ( (cDataMask >> 1) & 1 ) {
                        uint8_t cClusterAddress = (0x003fc000 & word) >> 14;
                        uint8_t cClusterWidth = ( (0x00003800 & word) >> 11 ) + 1;

                        for(uint8_t i = 0; i < cClusterWidth; i++)
                            hit_bits[cClusterAddress + 2*i] = true;
                    }
                }
            }

            std::ostringstream os;
            for(uint32_t i=0; i < NCHANNELS; i++) os << hit_bits[i];
            delete hit_bits;

            return os.str();
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return "";
        }

    }

    std::vector<bool> D19cCbc3EventZS::DataBitVector ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::vector<bool> blist;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            // i didn't find way better than this one, because of the fact that clusters separated for sensors, and also randomly stored in the event
            bool *hit_bits = new bool[NCHANNELS];
            for(uint32_t i=0; i < NCHANNELS; i++) hit_bits[i] = false;

            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 0) {
                    uint8_t cDataMask = (0x03000000 & word) >> 24;
                    if ( (cDataMask >> 0) & 1 ) {
                        uint8_t cClusterAddress = (0x000007f8 & word) >> 3;
                        uint8_t cClusterWidth = ( (0x00000007 & word) >> 0 ) + 1;

                        for(uint8_t i = 0; i < cClusterWidth; i++)
                            hit_bits[cClusterAddress + 2*i] = true;
                    }
                    if ( (cDataMask >> 1) & 1 ) {
                        uint8_t cClusterAddress = (0x003fc000 & word) >> 14;
                        uint8_t cClusterWidth = ( (0x00003800 & word) >> 11 ) + 1;

                        for(uint8_t i = 0; i < cClusterWidth; i++)
                            hit_bits[cClusterAddress + 2*i] = true;
                    }
                }
            }

            for(uint32_t i=0; i < NCHANNELS; i++) blist.push_back(hit_bits[i]);
            delete hit_bits;

        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
    }

    std::vector<bool> D19cCbc3EventZS::DataBitVector ( uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList ) const
    {
        std::vector<bool> blist;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::vector<uint32_t> cHitsVector;

            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 0) {
                    uint8_t cDataMask = (0x03000000 & word) >> 24;
                    if ( (cDataMask >> 0) & 1 ) {
                        uint8_t cClusterAddress = (0x000007f8 & word) >> 3;
                        uint8_t cClusterWidth = ( (0x00000007 & word) >> 0 ) + 1;

                        for(uint8_t i = 0; i < cClusterWidth; i++)
                            cHitsVector.push_back(cClusterAddress + 2*i);
                    }
                    if ( (cDataMask >> 1) & 1 ) {
                        uint8_t cClusterAddress = (0x003fc000 & word) >> 14;
                        uint8_t cClusterWidth = ( (0x00003800 & word) >> 11 ) + 1;

                        for(uint8_t i = 0; i < cClusterWidth; i++)
                            cHitsVector.push_back(cClusterAddress + 2*i);
                    }
                }
            }

            // checks if channel is in the hits vector.
            for(auto cChannel: channelList) blist.push_back(std::find(cHitsVector.begin(),cHitsVector.end(),cChannel) != cHitsVector.end());

        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
    }

    std::string D19cCbc3EventZS::GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return "";
    }


    std::string D19cCbc3EventZS::StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::ostringstream os;

        std::vector<Stub> cStubVector = this->StubVector (pFeId, pCbcId);

        for (auto cStub : cStubVector)
            os << std::bitset<8> (cStub.getPosition() ) << " " << std::bitset<4> (cStub.getBend() ) << " ";

        return os.str();


        //return BitString ( pFeId, pCbcId, OFFSET_CBCSTUBDATA, WIDTH_CBCSTUBDATA );
    }

    bool D19cCbc3EventZS::StubBit ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::vector<Stub> cStubVector = this->StubVector (pFeId, pCbcId);
        return !cStubVector.empty();
    }

    std::vector<Stub> D19cCbc3EventZS::StubVector (uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<Stub> cStubVec;
        //here create stubs and return the vector
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 1) {
                    uint8_t cStubAddress = (0x001fe000 & word) >> 13;
                    uint8_t cStubBend = (0x000003c0 & word) >> 6 ;
                    cStubVec.emplace_back (cStubAddress, cStubBend);
                }
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return cStubVec;
    }

    uint32_t D19cCbc3EventZS::GetNHits (uint8_t pFeId, uint8_t pCbcId) const
    {
        uint32_t cNHits = 0;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 0) {
                    uint8_t cDataMask = (0x03000000 & word) >> 24;
                    if ( (cDataMask >> 0) & 1 ) {
                        //uint8_t cClusterAddress = (0x000007f8 & word) >> 3;
                        uint8_t cClusterWidth = ( (0x00000007 & word) >> 0 ) + 1;
                        cNHits += cClusterWidth;
                    }
                    if ( (cDataMask >> 1) & 1 ) {
                        //uint8_t cClusterAddress = (0x003fc000 & word) >> 14;
                        uint8_t cClusterWidth = ( (0x00003800 & word) >> 11 ) + 1;
                        cNHits += cClusterWidth;
                    }
                }
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return cNHits;
    }

    std::vector<uint32_t> D19cCbc3EventZS::GetHits (uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<uint32_t> cHits;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 0) {
                    uint8_t cDataMask = (0x03000000 & word) >> 24;
                    if ( (cDataMask >> 0) & 1 ) {
                        uint8_t cClusterAddress = (0x000007f8 & word) >> 3;
                        uint8_t cClusterWidth = ( (0x00000007 & word) >> 0 ) + 1;

                        for(uint8_t i = 0; i < cClusterWidth; i++)
                            cHits.push_back(cClusterAddress + 2*i);
                    }
                    if ( (cDataMask >> 1) & 1 ) {
                        uint8_t cClusterAddress = (0x003fc000 & word) >> 14;
                        uint8_t cClusterWidth = ( (0x00003800 & word) >> 11 ) + 1;

                        for(uint8_t i = 0; i < cClusterWidth; i++)
                            cHits.push_back(cClusterAddress + 2*i);
                    }
                }
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;


        return cHits;
    }

    void D19cCbc3EventZS::printCbcHeader (std::ostream& os, uint8_t pFeId, uint8_t pCbcId) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint8_t cBeId =  0;
            uint8_t cFeId =  pFeId;
            uint8_t cCbcId = pCbcId;
            uint16_t cCbcDataSize = 0xFF;
            os << GREEN << "CBC Header:" << std::endl;
            os << "BeId: " << +cBeId << " FeId: " << +cFeId << " CbcId: " << +cCbcId << " DataSize: " << cCbcDataSize << RESET << std::endl;
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

    }

    void D19cCbc3EventZS::print ( std::ostream& os) const
    {
        os << BOLDGREEN << "EventType: d19c CBC3" << RESET << std::endl;
        os << BOLDBLUE <<  "L1A Counter: " << this->GetEventCount() << RESET << std::endl;
        os << "          Be Id: " << +this->GetBeId() << std::endl;
        //os << "          Be FW: " << +this->GetFWType() << std::endl;
        //os << "      Be Status: " << +this->GetBeStatus() << std::endl;
        //os << "  Cbc Data type: " << +this->GetCbcDataType() << std::endl;
        //os << "          N Cbc: " << +this->GetNCbc() << std::endl;
        os << "Event Data size: " << +this->GetEventDataSize() << std::endl;
        //os << "  CBC Counter: " << this->GetEventCountCBC() << RESET << std::endl;
        os << "Bunch Counter: " << this->GetBunch() << std::endl;
        //os << "Orbit Counter: " << this->GetOrbit() << std::endl;
        //os << " Lumi Section: " << this->GetLumi() << std::endl;
        os << BOLDRED << "    TDC Counter: " << +this->GetTDC() << RESET << std::endl;

        const int FIRST_LINE_WIDTH = 22;
        const int LINE_WIDTH = 32;
        const int LAST_LINE_WIDTH = 8;


        for (auto const& cKey : this->fEventDataMap)
        {
            uint8_t cFeId;
            uint8_t cCbcId;
            this->decodeId (cKey.first, cFeId, cCbcId);

            //here display the Cbc Header manually
            this->printCbcHeader (os, cFeId, cCbcId);

            // here print a list of stubs
            uint8_t cCounter = 1;

            if (this->StubBit (cFeId, cCbcId) )
            {
                os << BOLDCYAN << "List of Stubs: " << RESET << std::endl;

                for (auto& cStub : this->StubVector (cFeId, cCbcId) )
                {
                    os << CYAN << "Stub: " << +cCounter << " Position: " << +cStub.getPosition() << " Bend: " << +cStub.getBend() << " Strip: " << cStub.getCenter() << RESET << std::endl;
                    cCounter++;
                }
            }

            // here list other bits in the stub stream

            std::string data ( this->DataBitString ( cFeId, cCbcId ) );
            os << GREEN << "FEId = " << +cFeId << " CBCId = " << +cCbcId << RESET << " len(data) = " << data.size() << std::endl;
            os << YELLOW << "PipelineAddress: " << this->PipelineAddress (cFeId, cCbcId) << RESET << std::endl;
            os << RED << "Error: " << static_cast<std::bitset<2>> ( this->Error ( cFeId, cCbcId ) ) << RESET << std::endl;
            os << CYAN << "Total number of hits: " << this->GetNHits ( cFeId, cCbcId ) << RESET << std::endl;
            os << BLUE << "List of hits: " << RESET << std::endl;
            std::vector<uint32_t> cHits = this->GetHits (cFeId, cCbcId);

            if (cHits.size() == 254) os << "All channels firing!" << std::endl;
            else
            {
                int cCounter = 0;

                for (auto& cHit : cHits )
                {
                    os << std::setw (3) << cHit << " ";
                    cCounter++;

                    if (cCounter == 10)
                    {
                        os << std::endl;
                        cCounter = 0;
                    }

                }

                os << RESET << std::endl;
            }

            os << "Ch. Data:      ";

            for (int i = 0; i < FIRST_LINE_WIDTH; i += 2)
                os << data.substr ( i, 2 ) << " ";

            os << std::endl;

            for ( int i = 0; i < 7; ++i )
            {
                for (int j = 0; j < LINE_WIDTH; j += 2)
                    //os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * i, LINE_WIDTH ) << std::endl;
                    os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * i + j, 2 ) << " ";

                os << std::endl;
            }

            for (int i = 0; i < LAST_LINE_WIDTH; i += 2)
                os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * 7 + i, 2 ) << " ";

            os << std::endl;

            os << BLUE << "Stubs: " << this->StubBitString ( cFeId, cCbcId ).c_str() << RESET << std::endl;
        }

        os << std::endl;
    }

    std::vector<Cluster> D19cCbc3EventZS::getClusters ( uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<Cluster> cClusterVec;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for(auto word : cData->second) {
                uint8_t cDataType = (0x18000000 & word) >> 27;
                if (cDataType == 0) {
                    uint8_t cDataMask = (0x03000000 & word) >> 24;
                    if ( (cDataMask >> 0) & 1 ) {
                        uint8_t cClusterAddress = (0x000007f8 & word) >> 3;
                        uint8_t cClusterWidth = ( (0x00000007 & word) >> 0 ) + 1;

                        Cluster aCluster;
                        aCluster.fSensor = cClusterAddress % 2;
                        aCluster.fFirstStrip = cClusterAddress;
                        aCluster.fClusterWidth = cClusterWidth;
                        cClusterVec.push_back(aCluster);
                    }
                    if ( (cDataMask >> 1) & 1 ) {
                        uint8_t cClusterAddress = (0x003fc000 & word) >> 14;
                        uint8_t cClusterWidth = ( (0x00003800 & word) >> 11 ) + 1;

                        Cluster aCluster;
                        aCluster.fSensor = cClusterAddress % 2;
                        aCluster.fFirstStrip = cClusterAddress;
                        aCluster.fClusterWidth = cClusterWidth;
                        cClusterVec.push_back(aCluster);
                    }
                }
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return cClusterVec;
    }

    SLinkEvent D19cCbc3EventZS::GetSLinkEvent ( const BeBoard* pBoard) const
    {
        ;
    }
}
