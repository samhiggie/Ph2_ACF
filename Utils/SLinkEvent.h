#ifndef __SLINKEVENT_H__
#define __SLINKEVENT_H__

#include <bitset>
#include <deque>
#include <iostream>
#include <vector>
#include <set>

#include "../HWDescription/Definition.h"
#include "ConsoleColor.h"
#include "ConditionDataSet.h"
#include "easylogging++.h"

#define BOE_1 0x5
#define EVENT_TYPE 0x01 //Physics Trigger
#define SOURCE_ID 0x33 //FedID = 51
#define FOV 0x00
#define EOE_1 0xA0
#define TTS_VALUE 0x7

class SLinkEvent
{
  public:
    SLinkEvent (EventType pEventType, SLinkDebugMode pMode, ChipType pChipType, uint32_t& pLV1Id, uint16_t& pBXId, int pSourceId = SOURCE_ID);

    ~SLinkEvent()
    {
        fData.clear();
    }

    void generateDAQHeader (uint32_t& pLV1Id, uint16_t& pBXId, int pSourceId);
    void generateTkHeader (uint32_t& pBeStatus, uint16_t& pNChips, std::set<uint8_t>& pEnabledFe, bool pCondData = false, bool pFake = false);
    // the following 4 are dumb methods in that they just insert a vector of 64 bit words
    // the actual event implementation will have to encode everything in there
    void generateStatus (std::vector<uint64_t>& pStatus);
    void generatePayload (std::vector<uint64_t>& pPayload);
    void generateStubs (std::vector<uint64_t>& pStubList);
    // kind of important
    void generateConitionData (ConditionDataSet& pSet);
    // sort of
    void calulateCRC();
    // everything either a define or a member variable so no need to pass anything
    void generateDAQTrailer();
    void print()
    {
        for (auto word : fData)
            LOG (INFO) << static_cast<std::bitset<64>> (word) << "   " << std::hex << word << std::dec;
    }

  private:
    //Enums defined in HWDescription/Definition.h
    ChipType fChipType;
    EventType fEventType;
    SLinkDebugMode fDebugMode;


    //using a std::deque as it is probably easier to push front and insert randomly
    std::deque<uint64_t> fData;
    //size of the complete event in 64 bit words
    uint32_t fSize;
    //to hold the cyclic redundancy check checksum
    uint16_t fCRCVal;
    //to signal that all the info is filled
    bool fComplete;
    //flags to signal presence of condition data and fake or real events
    bool fCondData, fFake;

    //for file IO and debugging
    friend std::ostream& operator<< ( std::ostream& out, const SLinkEvent& ev );
    //{
    //ev.print (out);
    //return out;
    //}
};

#endif
