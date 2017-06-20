#include "SLinkEvent.h"

SLinkEvent::SLinkEvent (EventType pEventType, SLinkDebugMode pMode, ChipType pChipType, uint32_t& pLV1Id, uint16_t& pBXId, uint16_t& pSourceId) :
    fEventType (pEventType),
    fDebugMode (pMode),
    fChipType (pChipType),
    fSize (0),
    fCRCVal (0),
    fCondData (0),
    fFake (0),
    fComplete (false)
{
    this->generateDAQHeader (pLV1Id, pBXId, pSourceId);
}

void SLinkEvent::generateDAQHeader (uint32_t& pLV1Id, uint16_t& pBXId, uint16_t& pSourceId)
{
    uint64_t cWord = 0;
    //BOE_1 | EVENT_TYPE | L1 ID | BX ID | SOURCE ID | FOV
    cWord = (BOE_1 & 0xF) << 60 | (EVENT_TYPE & 0xF) << 56 | (pLV1Id & 0x00FFFFFF) << 32 | (pBXId & 0x0FFF) << 20 | (pSourceId & 0x0FFF) << 8 | (FOV & 0xF) << 4;

    fData.push_front (cWord);
    fSize += 1;
}

void SLinkEvent::generateTkHeader (uint32_t& pBeStatus, uint16_t& pNChips, std::bitset<72> pFeStatus&, bool pCondData, bool pFake)
{
    if (pFeStatus.size() != 72)
    {
        LOG (ERROR) << "ERROR: FeStatus needs to be 72 bit! - continuing with status 0 for all FE";
        pFeStatus = 0b0;
    }

    uint64_t cWord1 = 0;
    // version | format | event type | BeStatus | NChips | Fe Status (8)
    cWord1 = (0x2 & 0xF) << 60 | (fDebugMode & 0x03) << 58 | (fEventType & 0x03) << 56 | pCondData << 55 | !pFake << 54 | (pBeStatus & 0x3FFFFFFF) << 24 | pNChips << 8 | static_cast<uint8_t> ( (pFeStatus >> 64).to_ulong); // pFeStatus is the end of the
    cWord2 = pFeStatus & 0x00FFFFFFFFFFFFFFFF;

    fData.insert (fData.begin() + 1, cWord1);
    fData.insert (fData.begin() + 2, cWord2);
}

void SLinkEvent::generateStatus (std::vector<uint64_t>& pStatus)
{
    fData.insert (fData.begin() + 3, pStatus.begin(), pStatus.end() );
}

void SLinkEvent::generatePayload (std::vector<uint64_t>& pPayload)
{
    fData.insert (fData.end(), pPayload.begin(), pPayload.end() );
}

void SLinkEvent::generateStubs (std::vector<uint64_t>& pStubList)
{
    //if pStubList.size() == 4: CBC2, 16FE with 16 chips each, 1 bit per chip
    fData.insert (fData.end(), pStubList.begin(), pStubList.end() );
}

void SLinkEvent::generateConitionData (CondDataSet& pSet)
{}

void SLinkEvent::calulateCRC()
{}

void SLinkEvent::generateDAQTrailer()
{
    uint64_t cWord = 0;
    //EOE_1 | EvtLength | CRC | Event Stat | TTS
    cWord = (EOE_1 & 0xF) << 60 | (fSize & 0x00FFFFFF) << 32 | fCRCVal << 16 | (TTS_VALUE & 0xF) << 4;
    fData.push_back (cWord);
}
