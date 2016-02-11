#ifndef _TRACKER_EVENT_H_
#define _TRACKER_EVENT_H_

#include "../Utils/Event.h"
#include "../HWDescription/BeBoard.h"
#include "ParamSet.h"
/*!\class TrackerEvent
 * \brief Generation of Phase-2 tracker format event like described at https://cms-docdb.cern.ch/cgi-bin/DocDB/ShowDocument?docid=12091 */
class TrackerEvent{
public:
	/*! \brief Create one DAQ event from a Ph2_ACF one 
 * \param nbCBC Number of CBC data
 * \param Mask of enabled Front Ends
 * \param Mask of enabled CBCs
 * \param bFakeData True if data is coming from a file instead of a physical board
 * \param pPSet pointer to a parameter set containing configuration of condition data and acquisition mode and event type*/
	TrackerEvent( Ph2_HwInterface::Event * pEvt, uint32_t nbCBC, uint32_t uFE,  uint32_t uCBC, bool bFakeData, ParamSet* pPSet);
	virtual ~TrackerEvent();
	/*! \brief size of generated event in bytes including DAQ header and trailer */
	uint32_t getDaqSize() const;
	/*! \brief Size in bytes of the Tracker payload of the event */
	uint32_t getSize() const{ return size_;}
	/*! \brief pointer to event data */
	char* getData() const { return data_;}
	/*! \brief Fill an array of 4 bytes with the full size (with DAQ header and trailer) in bytes */
	void fillArrayWithSize(char *arrSize);
	/// Write CBC I2C register values into the parameter set
	static void setI2CValuesForConditionData(BeBoard *beBoard, ParamSet* pPSet);
private : 
	char *data_;
	uint32_t size_;

	/*! \brief Fill data in the event Tracker header */
	void fillTrackerHeader( Ph2_HwInterface::Event* pEvt, uint64_t uFE, uint32_t nbCBC, uint32_t nbCbcFe, uint32_t uAcqMode, bool bZeroSuppr, bool bConditionData, bool bFakeData) ;
	/*! \brief Fill data in the event payload */
	void fillTrackerPayload(Ph2_HwInterface::Event* pEvt, uint32_t nbFE, uint32_t nbCBC, uint32_t uCBC, uint32_t nbBitsHeader, bool bZeroSuppr, bool bCondition, uint32_t nbCondition, ParamSet* pPSet);
	/*! \brief Fill data in the event condition data */
	void fillTrackerConditionData(Ph2_HwInterface::Event* pEvt, uint32_t idxPayload, uint32_t nbFE, uint32_t nbCBC, uint32_t nbCondition, ParamSet* pPSet);
	/*! \brief Data buffer construction and data size computing for one FE in sparsified mode for 2S modules. 	
 	* Two pass are necessary: <ul>
	 * <li> one to compute size with NULL as destination buffer</li>
	 * <li> one to construct destination buffer</li></ul>
	 * \return Data size in bits
	 * \param pEvt PH2_ACF event
	 * \param uFront Mask of enabled Front Ends
	 * \param dest destination buffer. Set to NULL to only compute size.
	 * \param bitDest bit index to put data into destination buffer
	 * \param nbCBC Number of CBCs in this FE
	 */
	uint32_t calcBitsForFE(Ph2_HwInterface::Event* pEvt, uint32_t uFront, char* dest, uint32_t bitDest, uint32_t nbCBC);
	/*! \brief Set some bit values in the destination buffer */
	void setValueBits(char* arrDest, uint32_t bitDest, uint8_t width, uint8_t uVal);
	/*! \brief Counts the number of enabled condition data in configuration parameter set */
	uint32_t countConditionData(ParamSet* pPSetCondition);
	/**Calculate byte index in data payload from 'normal' index. Bytes are not in the same order in destination buffer as they are constructed.
	 * They are put in 64 bits words (8 bytes) in little endian order (from right to left) : 7 6 5 4 3 2 1 0 8 9 ...
	 * \return index in destination buffer
	 */
	uint32_t littleEndian8(uint32_t n);
	/*! \brief reverse bits of a byte */
	void reverseByte(char & b);
	void fillDaqHeaderAndTrailer(Ph2_HwInterface::Event *pEvt);
	/** \brief Compute Cyclic Redundancy Check code 
	 * \param Adresse_tab data address
	 * \param Taille_max data size
	 * \param Crc previous code (cyclic)
	 */
	uint16_t Crc16(const char *Adresse_tab , uint32_t Taille_max, uint16_t Crc) const;
};
#endif
