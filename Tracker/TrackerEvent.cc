#define NB_STRIPS_CBC2			256
#define DAQ_TRAILER_SIZE		8
#define DAQ_HEADER_SIZE			8
#define IDX_DAQ_HEADER_FOV		0
#define IDX_DAQ_HEADER_SOURCE_LSB	1
#define IDX_DAQ_HEADER_SOURCE_MSB	2
#define IDX_DAQ_HEADER_BX		3
#define IDX_DAQ_HEADER_LV1_LSB		4
#define IDX_DAQ_HEADER_LV1_1		5
#define IDX_DAQ_HEADER_LV1_MSB		6
#define IDX_DAQ_HEADER_TYPE		7
#define IDX_DAQ_TRAILER_EOE		7
#define IDX_DAQ_TRAILER_LEN_MSB		6
#define IDX_DAQ_TRAILER_LEN_1		5
#define IDX_DAQ_TRAILER_LEN_LSB		4
#define IDX_DAQ_TRAILER_CRC_MSB		3
#define IDX_DAQ_TRAILER_CRC_LSB		2
#define IDX_DAQ_TRAILER_STAT		1
#define IDX_DAQ_TRAILER_TTS		0
#define BOE_1				0x5
#define EVENT_TYPE			0x01 //Physics trigger
#define SOURCE_FED_ID			0x33
#define FOV				0x00
#define EOE_1				0xA0
#define TTS_VALUE			0x70


#define SIZE_EVT			4
#define IDX_NUMBER_CBC			1
#define IDX_FORMAT			7
#define IDX_EVENT_TYPE			6
#define IDX_GLIB_STATUS 		3
#define IDX_FRONT_END_STATUS 		8
#define IDX_FRONT_END_STATUS_MSB 	0
//littleEndian8 index
#define IDX_CBC_STATUS 			16

#define FORMAT_VERSION 			2
#define ZERO_SUPPRESSION		1
#define VIRGIN_RAW			2
#define GLIB_STATUS_REGISTERS		0
#define STREAMER_ACQ_MODE		"acqMode"
#define STREAMER_ACQ_MODE_FULLDEBUG	1		
#define STREAMER_ACQ_MODE_CBCERROR	2		
#define STREAMER_ACQ_MODE_SUMMARYERROR	0
#define STREAMER_ACQ_MODE_OLD		3		

#define CONDITION_DATA_ENABLED		"enabled_%02d"
#define CONDITION_DATA_FE_ID		"FE_ID_%02d"
#define CONDITION_DATA_CBC		"CBC_number_%02d"
#define CONDITION_DATA_PAGE		"page_number_%02d"
#define CONDITION_DATA_REGISTER		"I2C_register_%02d"
#define CONDITION_DATA_TYPE		"data_type_%02d"
#define CONDITION_DATA_VALUE		"value_%02d"
#define CONDITION_DATA_NAME		"name_%02d"
#define NB_CONDITION_DATA		10

#include <boost/format.hpp>
#include "TrackerEvent.h"

using namespace std;
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

/** Construct one event from GLIB data. One fiber out of 12 is filled with data and the others are empty.
 * @param uAcqMode Acquisition mode (1:Full debug, 2:CBC error, 0:Summary error)
 * @param uFE Front Ends available (one bit per FE)
 * @param nbCBC Total number of CBC
 * @param uCBC Available CBCs (one bit per CBC) on each FE. temporary : only one CBC status
 * @param bZeroSuppr Sparsified mode
 * @param pPSetCondition pointer on the condition data parameter set
 * @param buffer GLIB data buffer
 * @param idxBuf index of the event data in buffer 
 */ 
TrackerEvent::TrackerEvent(  Event * pEvt, uint32_t nbCBC, uint32_t uFE,  uint32_t uCBC, ParamSet* pPSet){
	data_=NULL;
	size_=0;

	uint32_t uAcqMode = STREAMER_ACQ_MODE_FULLDEBUG;
	bool bZeroSuppr=false;
	bool bFakeData=false;

	uint32_t nbBitsPayload=0, nbBitsCondition=0, nbBitsHeader = 128;//at least 2 64-bits words
	uint32_t nbCondition = 0;
	
	switch (uAcqMode){
		case STREAMER_ACQ_MODE_FULLDEBUG:
			nbBitsHeader+= 10 * nbCBC;
			break;
		case STREAMER_ACQ_MODE_CBCERROR:
			nbBitsHeader+= nbCBC*2;
			break;
	}//+=0 if STREAMER_ACQ_MODE_SUMMARYERROR
	//cout << "nbBitsHeader = " << nbBitsHeader << endl;
	uint32_t nbFE=0, uFront=uFE;
	while (uFront>0){//Count number of FE
		if (uFront%2>0){
			nbFE++;
		}
		uFront>>=1;
	}
	uint32_t nbCbcFe=nbCBC/nbFE;//nb of CBCs per FE
	//cout << "nbFE = " << nbFE << endl;

	if (bZeroSuppr){
		for (uFront=0; uFront<nbFE;uFront++)
			nbBitsPayload+=	calcBitsForFE(pEvt, uFront, NULL, 0, nbCbcFe);
				
	} else
		nbBitsPayload=16 * nbFE + nbCBC*NB_STRIPS_CBC2;//Payload size (bits)
		
	//cout << "bZeroSuppr = " << bZeroSuppr << ", nbBitsPayload = " << nbBitsPayload << endl;
	nbBitsPayload+=64*4;//Stub data
	//cout << "nbBitsPayload = " << nbBitsPayload << ", nbCBC = " << nbCBC << endl;

	//Condition data
	nbCondition = countConditionData(pPSet);
	nbBitsCondition=(nbCondition+1)*64;
	bool bConditionData=(nbCondition>0);
	nbBitsHeader=nbBitsHeader/64*64 + (nbBitsHeader%64>0 ? 64 :0);//padded to 64 bits
	nbBitsPayload=nbBitsPayload/64*64 + (nbBitsPayload%64>0 ? 64 :0);//padded to 64 bits
	size_=(nbBitsHeader + nbBitsPayload + nbBitsCondition)/8;
	//cout<<"Tracker size (Bytes)= "<<nbBitsHeader/8<<"+"<<nbBitsPayload/8<<"= "<<size_<<endl;
	data_=new char[getDaqSize()];
	memset (data_, 0, getDaqSize());
	fillTrackerHeader(pEvt, uFE, nbCBC, nbCBC/nbFE, uAcqMode, bZeroSuppr, bConditionData, bFakeData);
	fillTrackerPayload(pEvt, nbFE, nbCBC, uCBC, nbBitsHeader, bZeroSuppr, bConditionData, nbCondition, pPSet);
	if (bConditionData)
		fillTrackerConditionData(pEvt, DAQ_HEADER_SIZE+(nbBitsHeader+nbBitsPayload)/8, nbFE, nbCBC, nbCondition, pPSet);

	fillDaqHeaderAndTrailer(pEvt);
}

TrackerEvent::~TrackerEvent(){
	delete data_;
}

uint32_t TrackerEvent::getDaqSize() const{
	return DAQ_TRAILER_SIZE+DAQ_HEADER_SIZE+size_;
}
							
///Fill the tracker header
void TrackerEvent::fillTrackerHeader( Event* pEvt, uint64_t uFE, uint32_t nbCBC, uint32_t nbCbcFe, uint32_t uAcqMode, bool bZeroSuppr, bool bConditionData, bool bFakeData){
	uint32_t uIdx=DAQ_HEADER_SIZE;
	data_[uIdx+IDX_FORMAT] = FORMAT_VERSION<<4 | uAcqMode<<2 | (bZeroSuppr ? ZERO_SUPPRESSION : VIRGIN_RAW);//Data format version, Header format, 
	data_[uIdx+IDX_EVENT_TYPE] = (bConditionData ? 1 : 0)<<7 | (bFakeData ? 0 : 1)<<6 | (GLIB_STATUS_REGISTERS&0x3F000000)>>24;//Event type, DTC status registers
	data_[uIdx+IDX_GLIB_STATUS+2]= (GLIB_STATUS_REGISTERS&0xFF0000)>>16;
	data_[uIdx+IDX_GLIB_STATUS+1]= (GLIB_STATUS_REGISTERS&0xFF00)>>8;
	data_[uIdx+IDX_GLIB_STATUS]	= (GLIB_STATUS_REGISTERS&0xFF);
	data_[uIdx+IDX_FRONT_END_STATUS_MSB] = 0;//(uFE>>64)&0xFF;//Front End status (72 bits but only 64 for now: temporary)
	for (uint32_t uIdx=0; uIdx<8; uIdx++)
		data_[uIdx+IDX_FRONT_END_STATUS+uIdx]=(uFE>>(uIdx*8))&0xFF;
		
	//Total number of CBC chips
	data_[uIdx+IDX_NUMBER_CBC+1]	= (nbCBC&0xFF00)>>8;
	data_[uIdx+IDX_NUMBER_CBC]	= (nbCBC&0xFF);
	
	uint32_t uChip, uStatus;
	for (uChip = 0; uChip<nbCBC; uChip++)//CBC status in header
		switch (uAcqMode){
			case STREAMER_ACQ_MODE_FULLDEBUG://10 bits always over 2 bytes
				uStatus = pEvt->PipelineAddress(uChip / nbCbcFe, uChip%nbCbcFe);
				data_[uIdx+littleEndian8(IDX_CBC_STATUS + uChip*10/8)] 		|= uStatus>>((uChip*2+2)%10) & 0xFF;
				data_[uIdx+littleEndian8(IDX_CBC_STATUS + uChip*10/8 + 1)] 	|= uStatus<<(8-(uChip*2+2)%10) & 0xFF;
				break;
			case STREAMER_ACQ_MODE_CBCERROR:
				data_[uIdx+littleEndian8(IDX_CBC_STATUS + uChip/4/* *2/8 */)] |= pEvt->Error(uChip/nbCbcFe, uChip%nbCbcFe)<<(8-(uChip*2)%8);
				break;
		}//nothing if STREAMER_ACQ_MODE_SUMMARYERROR
}

void TrackerEvent::fillTrackerPayload(Event* pEvt, uint32_t nbFE, uint32_t nbCBC, uint32_t uCBC, uint32_t nbBitsHeader, bool bZeroSuppr, bool bCondition, uint32_t nbCondition, ParamSet* pPSet){
	// Fill the tracker payload 	
	uint32_t uIdxCbc=0, uOct, idxPayload, bitPayload= nbBitsHeader, uFront, uChip;
	uint32_t nbCbcFe=nbCBC/nbFE;//nb of CBCs per FE
	vector< uint8_t > cbcData;
	for (uFront=0; uFront<nbFE; uFront++){
		//cout<<"FE="<<uFront<<" @"<<idxPayload<<endl;
		if (bZeroSuppr){
			bitPayload+=(calcBitsForFE(pEvt, uFront, data_, bitPayload, nbCbcFe)+7)/8;
			idxPayload=DAQ_HEADER_SIZE+(bitPayload+7)/8;
		} else {
			idxPayload=DAQ_HEADER_SIZE+nbBitsHeader/8+uFront*(nbCbcFe*NB_STRIPS_CBC2/8 + 2);
			data_[littleEndian8(idxPayload++)]=(uCBC&0xFF00)>>8;
			data_[littleEndian8(idxPayload++)]=(uCBC&0x00FF);
			for (uChip=0; uChip<nbCbcFe; uChip++){
				pEvt->GetCbcEvent(uFront, uChip, cbcData);
				for (uOct=0; uOct<NB_STRIPS_CBC2/8; uOct++){
					// Jonni: so we need to skip 2 CBC error bits and then 8 Pipeline Address Bits then we need to reconstruct nibbles from half nibbles
					data_[littleEndian8(idxPayload)] =  cbcData[uOct];
					reverseByte(data_[littleEndian8(idxPayload++)]);
				}
				//cout<<endl;
			}
		}//if zero suppressed
	}//for Front End
	//Stub Data
	idxPayload=(idxPayload+7)/8*8;
//	cout<<"idxStub="<<idxPayload<<endl;
	for (uFront=0; uFront<nbFE; uFront++){//Stub bits
		for (uChip=0; uChip<nbCbcFe; uChip++){
			if (pEvt->StubBit(uFront, uChip))
				data_[littleEndian8(idxPayload+7-uIdxCbc/8)] |= (1<<(uIdxCbc%8));

			uIdxCbc++;
		}
	}
//	idxPayload+=uChip/8 + 1;
}

void TrackerEvent::fillTrackerConditionData(Event* pEvt,  uint32_t idxPayload, uint32_t nbFE, uint32_t nbCBC, uint32_t nbCondition, ParamSet* pPSet)
{//Condition data
//	cout<<"idxNbCondition="<<idxPayload<<endl;
	uint32_t uOct, uCond, uVal=0, uFront, uChip;
	uint32_t nbCbcFe=nbCBC/nbFE;//nb of CBCs per FE
	idxPayload+=4;
	for (uOct=4; uOct<8; uOct++)//Nb of condition data in 64 bits
		data_[littleEndian8(idxPayload++)]= (nbCondition>>(56-uOct*8))&0xFF;
		
//	cout<<"idxCondition="<<idxPayload<<endl;
	for (uCond=0 ; uCond<NB_CONDITION_DATA; uCond++){//Key
		if (pPSet->getValue((boost::format(CONDITION_DATA_ENABLED)%uCond).str())==1){
			data_[littleEndian8(idxPayload++)]=pPSet->getValue((boost::format(CONDITION_DATA_FE_ID)%uCond).str())&0xFF;
			data_[littleEndian8(idxPayload++)]=((pPSet->getValue((boost::format(CONDITION_DATA_CBC)%uCond).str())&0x0F)) 
				| ((pPSet->getValue((boost::format(CONDITION_DATA_PAGE)%uCond).str())&0x0F)<<4);
			data_[littleEndian8(idxPayload++)]=pPSet->getValue((boost::format(CONDITION_DATA_REGISTER)%uCond).str())&0xFF;
			data_[littleEndian8(idxPayload++)]=pPSet->getValue((boost::format(CONDITION_DATA_TYPE)%uCond).str())&0xFF;
			switch(pPSet->getValue((boost::format(CONDITION_DATA_TYPE)%uCond).str())){//Value
				case 3://Trigger phase (TDC)
		//				uFront = pPSet->getValue((boost::format(CONDITION_DATA_FE_ID)%uCond).str());
		
					uVal=pEvt->GetTDC();
					break;
				case 6://Error bits
					uVal=0;
					for (uFront=0; uFront<nbFE; uFront++)
						for (uChip=0;uChip<nbCbcFe;uChip++)
							uVal |= pEvt->Error(uFront, uChip)<<(uChip*2);

					break;
				case 7://CBC Status bits
					uVal=0;
					if (nbCBC<=4){
						for (uFront=0; uFront<nbFE; uFront++)
							for (uChip=0;uChip<nbCbcFe;uChip++) //temporary: All CBC status bits in one 32-bits value
								uVal |= pEvt->PipelineAddress(uFront, uChip) << ((uFront*nbCbcFe+uChip)*8);
					} else {
						uFront=pPSet->getValue((boost::format(CONDITION_DATA_FE_ID)%uCond).str())&0xFF;
						uChip=pPSet->getValue((boost::format(CONDITION_DATA_CBC)%uCond).str())&0x0F; 
						uVal = pEvt->PipelineAddress(uFront, uChip);
					}
					break;
				default://Configuration parameter (I2C), Angle, High Voltage, Other Value
					uVal=pPSet->getValue((boost::format(CONDITION_DATA_VALUE)%uCond).str());
					break;
			}//switch
			for (uOct=0; uOct<4; uOct++)
				data_[littleEndian8(idxPayload++)]= (uVal>>(24-uOct*8))&0xFF;
		}
	}//for
}

	/**Data buffer construction and data size computing for one FE in sparsified mode for 2S modules. Two pass are necessary: <ul>
	 * <li> one to compute size with NULL as destination buffer</li>
	 * <li> one to construct destination buffer</li></ul>
	 * @return Data size in bits
	 * @param buffer GLIB data buffer
	 * @param index idxBuf of event data in buffer
	 * @param dest destination buffer. Set to NULL to only compute size.
	 * @param bitDest index to put data for this fiber into destination buffer
	 * @param nbCBC Number of CBCs in this FE
	 */
uint32_t TrackerEvent::calcBitsForFE(Event* pEvt, uint32_t uFront, char* dest, uint32_t bitDest, uint32_t nbCBC){
	uint32_t uChip, uBit, nbCluster=0, uClusterSize, nbMax=63, uPos;
	uint32_t nbBits=7;//FE header size for 2S modules
	bool bCluster;
	
	for (uChip=0; uChip<nbCBC; uChip++){
		uClusterSize=0;
		bCluster=false;
		if (nbCluster>=nbMax) break;
		for (uBit=0; uBit<NB_STRIPS_CBC2-2; uBit++){
			if (bCluster){
				if (!pEvt->DataBit(uFront, uChip, uBit) || uBit==NB_STRIPS_CBC2-3){//end of cluster
					bCluster=false;
					if (dest){//<chip ID 4b><position 8b><size 3b>
						setValueBits(dest, bitDest+nbBits   , 4, uChip);//Chip ID
						setValueBits(dest, bitDest+nbBits+ 4, 8, uPos);//position of cluster
						setValueBits(dest, bitDest+nbBits+12, 3, uClusterSize-1); //size of cluster
					}
					nbBits+=15;
					if (nbCluster>=nbMax) break;
				} else if (pEvt->DataBit(uFront, uChip, uBit)){// cluster continuation
					uClusterSize++;
					if (uClusterSize>8){
						if (dest){
							setValueBits(dest, bitDest+nbBits   , 4, uChip);//Chip ID
							setValueBits(dest, bitDest+nbBits+ 4, 8, uPos);//position of cluster
							setValueBits(dest, bitDest+nbBits+12, 3, 7);  //size of cluster
						}
						nbBits+=15;
						if (nbCluster>=nbMax) break;
						nbCluster++;
						uClusterSize=1;
						uPos=uBit;
					}
				}
			} else {//Beginning of  cluster
				if (pEvt->DataBit(uFront, uChip, uBit)){
					uPos=uBit;
					nbCluster++;
					uClusterSize=1;
					bCluster=true;
				}
			}
		}//for strips
	}//for CBC
	if (dest){
		setValueBits(dest, bitDest+1, 6, nbCluster);//Number of S clusters, module type bit is 0 (2S module)
	}
	return nbBits;
}
/** Set bit values over one or two bytes 
 * \param arrDest Destination array of bytes
 * \param bitDest position of Most Significant bit in arrDest
 * \param width field length in bits
 * \param uVal field value */
void TrackerEvent::setValueBits(char* arrDest, uint32_t bitDest, uint8_t width, uint8_t uVal){
	if (width>8-bitDest%8){//over two bytes
		uint32_t nbMsb= width - (8-bitDest%8);
		uint32_t nbLsb= width - nbMsb;
		arrDest[littleEndian8(bitDest/8)] 	|= (uVal >> nbLsb);
		arrDest[littleEndian8(bitDest/8 + 1)] 	|= (uVal&((1<<nbLsb)-1))<< (8-nbLsb);
	} else {//over one byte
		arrDest[littleEndian8(bitDest/8)] 	|= uVal << (8-bitDest%8-width);
	}
}

uint32_t TrackerEvent::countConditionData(ParamSet* pPSetCondition){
	uint32_t uRet=0;
	for (int iCond=0; iCond<NB_CONDITION_DATA; iCond++){
		if (pPSetCondition!=NULL && pPSetCondition->getValue((boost::format(CONDITION_DATA_ENABLED)%iCond).str())==1)
			uRet++;
	}
	return uRet;
}

uint32_t TrackerEvent::littleEndian8(uint32_t n){
	return n^7; //(n/8+1)*8 - n%8 -1;
}

void TrackerEvent::reverseByte(char & b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
}

void TrackerEvent::fillDaqHeaderAndTrailer(Event *pEvt){
//DAQ header	
//	globalDaqHeader_ = boe_ | eventType_ | l1aCounter_ | bxCounter_ | sourceFedID_ | fov_;
	data_[IDX_DAQ_HEADER_TYPE]		= (BOE_1<<4) | EVENT_TYPE;
	data_[IDX_DAQ_HEADER_LV1_MSB]		= (pEvt->GetEventCount()>>16)&255;
	data_[IDX_DAQ_HEADER_LV1_1]		= (pEvt->GetEventCountCBC()>>8)&255;
	data_[IDX_DAQ_HEADER_LV1_LSB]		= pEvt->GetEventCountCBC()&255;
	data_[IDX_DAQ_HEADER_BX]			= (pEvt->GetBunch()>>4)&255;
	data_[IDX_DAQ_HEADER_SOURCE_MSB]	= ((pEvt->GetBunch()&15)<<4) | ((SOURCE_FED_ID>>8)&255);
	data_[IDX_DAQ_HEADER_SOURCE_LSB]	= (SOURCE_FED_ID)&255;
	data_[IDX_DAQ_HEADER_FOV]			= (FOV<<4) | (0<<3) /*one word header*/ | 0; /*reserved*/
//DAQ trailer
	uint32_t len = size_ + 2;
	uint16_t crc = Crc16(data_, DAQ_HEADER_SIZE, 0xFFFF);
	crc = Crc16(data_ + DAQ_HEADER_SIZE, size_, crc);
	data_[DAQ_HEADER_SIZE+size_+IDX_DAQ_TRAILER_EOE]	= EOE_1;
	data_[DAQ_HEADER_SIZE+size_+IDX_DAQ_TRAILER_LEN_MSB]	= (len>>16)&255;
	data_[DAQ_HEADER_SIZE+size_+IDX_DAQ_TRAILER_LEN_1]	= (len>>8)&255; 
	data_[DAQ_HEADER_SIZE+size_+IDX_DAQ_TRAILER_LEN_LSB]	= (len)&255;
	data_[DAQ_HEADER_SIZE+size_+IDX_DAQ_TRAILER_CRC_MSB]	= (crc>>8)&255; 
	data_[DAQ_HEADER_SIZE+size_+IDX_DAQ_TRAILER_CRC_LSB]	= (crc)&255;
	data_[DAQ_HEADER_SIZE+size_+IDX_DAQ_TRAILER_STAT]	= 0;
	data_[DAQ_HEADER_SIZE+size_+IDX_DAQ_TRAILER_TTS]	= TTS_VALUE;
}

uint16_t TrackerEvent::Crc16(const char *Adresse_tab , uint32_t Taille_max, uint16_t Crc=0xFFFF) const
{
	uint16_t Polynome = 0xA001;// Polynome 2^16 + 2^15 + 2^2 + 2^0 = 0x8005.
	uint32_t CptOctet;
	unsigned char CptBit, Parity;

	for ( CptOctet= 0 ; CptOctet < Taille_max ; CptOctet++)
	{
		Crc ^= *( Adresse_tab + CptOctet); //Ou exculsif entre octet message et CRC
		for ( CptBit = 0; CptBit <= 7 ; CptBit++) /* Mise a 0 du compteur nombre de bits */
		{
			Parity= Crc%2;
			Crc >>= 1; // Decalage a droite du crc
			if (Parity) 
				Crc ^= Polynome; // Test si nombre impair -> Apres decalage a droite il y aura une retenue
		} // "ou exclusif" entre le CRC et le polynome generateur.
	}
	return(Crc);
}

