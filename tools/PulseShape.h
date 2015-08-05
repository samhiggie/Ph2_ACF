/*!

        \file                  PulseShape.h
        \brief                 class to do reconstruct the pulse shape of the CBC
        \author              Andrea Massetti & Ali Imran
        \version                1.0
        \date                   20/01/15
        Support :               mail to : georg.auzinger@cern.ch

 */

#ifndef PULSESHAPE_H__
#define PULSESHAPE_H__

#include "Tool.h"
#include "Channel.h"
#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/CommonVisitors.h"


#include "TString.h"
#include "TMultiGraph.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2F.h"
#include <math.h>

using namespace Ph2_System;
using namespace Ph2_HwInterface;
using namespace Ph2_HwDescription;

/*!
 * \class PulseShape
 * \brief Class to reconstruct pulse shape
 */

typedef std::map<Cbc*, Channel*> ChannelMap;

class PulseShape : public Tool
{
  public:
	//initialize the istogram
	void Initialize();

	std::map<Cbc*, uint8_t> ScanVcth( uint32_t pDelay );

	// method to scan the test pulse delay with a given step size
	void ScanTestPulseDelay( uint8_t pStepSize );


  private:
	//find the test group of a given channel ID
	int findTestGroup( uint32_t pChannelId );


	void parseSettings();

	//set the system test pulse
	void setSystemTestPulse( uint8_t pTPAmplitude, uint8_t pChannelId );

	//update the Histogram
	void updateHists( std::string pHistName, bool pFinal );

	// return number of events process = vector.size()
	uint32_t fillVcthHist( BeBoard* pBoard, std::vector<Event*> pEventVector, uint32_t pVcth );
	//convert the delay before concat to the  test group number
	void setDelayAndTesGroup( uint32_t pDelay );


	void enableChannel( uint8_t pChannelId );

	void fitGraph( int pLow );


	//reverse the byte
	uint8_t reverse( uint8_t n ) {


		// Reverse the top and bottom nibble then swap them.
		return ( fLookup[n & 0b1111] << 4 ) | fLookup[n >> 4];
	}

	uint8_t to_reg( uint8_t pDelay, uint8_t pGroup ) {

		uint8_t cValue = ( ( reverse( pDelay ) ) & 0xF8 ) |
						 ( ( reverse( pGroup ) ) >> 5 );

		std::cout << std::bitset<8>( cValue ) << " cGroup " << +pGroup << " " << std::bitset<8>( pGroup ) << " pDelay " << +pDelay << " " << std::bitset<8>( pDelay ) << std::endl;
		return cValue;
	}

	ChannelMap fChannelMap;
	bool fFitHist;
	uint32_t fNevents;
	uint32_t fHoleMode;
	uint32_t fNCbc;
	uint8_t fVplus;
	uint8_t fTestGroup;
	uint8_t fTPAmplitude;
	uint32_t fChannel;
	uint8_t fOffset;
	uint32_t fStepSize;
	unsigned char fLookup[16] =
	{
		0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
		0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
	};



};
//fitting function
double pulseshape( double* x, double* par );

#endif
