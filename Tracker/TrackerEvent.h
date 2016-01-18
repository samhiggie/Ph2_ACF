#ifndef _TRACKER_EVENT_H_
#define _TRACKER_EVENT_H_

#include "../Utils/Event.h"
#include "ParamSet.h"

class TrackerEvent{
public:
	TrackerEvent( Ph2_HwInterface::Event * pEvt, uint32_t nbCBC, uint32_t uFE,  uint32_t uCBC, ParamSet* pPSet);
	virtual ~TrackerEvent();
	uint32_t getDaqSize() const;
	uint32_t getSize() const{ return size_;}
	char* getData() const { return data_;}
private : 
	char *data_;
	uint32_t size_;

	void fillTrackerHeader( Ph2_HwInterface::Event* pEvt, uint64_t uFE, uint32_t nbCBC, uint32_t nbCbcFe, uint32_t uAcqMode, bool bZeroSuppr, bool bConditionData, bool bFakeData) ;
	void fillTrackerPayload(Ph2_HwInterface::Event* pEvt, uint32_t nbFE, uint32_t nbCBC, uint32_t uCBC, uint32_t nbBitsHeader, bool bZeroSuppr, bool bCondition, uint32_t nbCondition, ParamSet* pPSet);
	void fillTrackerConditionData(Ph2_HwInterface::Event* pEvt, uint32_t idxPayload, uint32_t nbFE, uint32_t nbCBC, uint32_t nbCondition, ParamSet* pPSet);
	uint32_t calcBitsForFE(Ph2_HwInterface::Event* pEvt, uint32_t uFront, char* dest, uint32_t bitDest, uint32_t nbCBC);
	void setValueBits(char* arrDest, uint32_t bitDest, uint8_t width, uint8_t uVal);
	uint32_t countConditionData(ParamSet* pPSetCondition);

	uint32_t littleEndian8(uint32_t n);
	void reverseByte(char & b);
	void fillDaqHeaderAndTrailer(Ph2_HwInterface::Event *pEvt);
	uint16_t Crc16(const char *Adresse_tab , uint32_t Taille_max, uint16_t Crc) const;
};
#endif
