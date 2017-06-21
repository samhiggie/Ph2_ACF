#include "SLinkEvent.h"

SLinkEvent::SLinkEvent (EventType pEventType, SLinkDebugMode pMode, ChipType pChipType, uint32_t& pLV1Id, uint16_t& pBXId, int pSourceId) :
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

void SLinkEvent::generateDAQHeader (uint32_t& pLV1Id, uint16_t& pBXId, int pSourceId)
{
    uint64_t cWord = 0;
    //BOE_1 | EVENT_TYPE | L1 ID | BX ID | SOURCE ID | FOV
    cWord |= ( (uint64_t) BOE_1 & 0xF) << 60 | ( (uint64_t) EVENT_TYPE & 0xF) << 56 | ( (uint64_t) pLV1Id & 0x00FFFFFF) << 32 | ( (uint64_t) pBXId & 0x0FFF) << 20 | (pSourceId & 0x0FFF) << 8 | (FOV & 0xF) << 4;

    fData.push_front (cWord);
    fSize += 1;
}

void SLinkEvent::generateTkHeader (uint32_t& pBeStatus, uint16_t& pNChips, std::set<uint8_t>& pEnabledFe, bool pCondData, bool pFake)
{
    uint64_t cWord1 = 0;
    uint64_t cWord2 = 0;
    // version | format | event type | BeStatus | NChips | Fe Status (8)
    cWord1 = ( (uint64_t) 0x2 & 0xF) << 60 | ( (uint64_t) fDebugMode & 0x03) << 58 |  ( (uint64_t) fEventType & 0x03) << 56 | (uint64_t) pCondData << 55 | (uint64_t) !pFake << 54 | ( (uint64_t) pBeStatus & 0x3FFFFFFF) << 24 | pNChips << 8 ; // pFeStatus is the end of the

    for (auto& cFe : pEnabledFe)
    {
        if (cFe < 64)
            cWord2 |= (uint64_t) 1 << cFe;
        else
            cWord1 |= (uint64_t) 1 << cFe - 64;
    }

    fData.insert (fData.begin() + 1, cWord1);
    fData.insert (fData.begin() + 2, cWord2);
    fSize += 2;
}

void SLinkEvent::generateStatus (std::vector<uint64_t>& pStatus)
{
    if (fDebugMode != SLinkDebugMode::SUMMARY)
        fData.insert (fData.begin() + 3, pStatus.begin(), pStatus.end() );

    fSize += pStatus.size();
}

void SLinkEvent::generatePayload (std::vector<uint64_t>& pPayload)
{
    fData.insert (fData.end(), pPayload.begin(), pPayload.end() );
    fSize += pPayload.size();
}

void SLinkEvent::generateStubs (std::vector<uint64_t>& pStubList)
{
    //if pStubList.size() == 4: CBC2, 16FE with 16 chips each, 1 bit per chip
    fData.insert (fData.end(), pStubList.begin(), pStubList.end() );
    fSize += pStubList.size();
}

void SLinkEvent::generateConitionData (ConditionDataSet& pSet)
{}

void SLinkEvent::calulateCRC()
{}

void SLinkEvent::generateDAQTrailer()
{
    uint64_t cWord = 0;
    //EOE_1 | EvtLength | CRC | Event Stat | TTS
    cWord = ( (uint64_t) EOE_1 & 0xFF) << 56 | ( (uint64_t) fSize & 0x00FFFFFF) << 32 | fCRCVal << 16 | (TTS_VALUE & 0xF) << 4;
    fData.push_back (cWord);
    fSize += 1;
}
