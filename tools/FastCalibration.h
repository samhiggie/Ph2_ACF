/*!
*
* \file Calibration.h
* \brief Calibration class, calibration of the hardware
* \author Georg AUZINGER
* \date 16 / 10 / 14
*
* \Support : georg.auzinger@cern.ch
*
*/

#ifndef FastCalibration_h__
#define FastCalibration_h__

#include "Tool.h"
#include "Channel.h"
#include "../Utils/Visitor.h"
#include "../Utils/CommonVisitors.h"


#include <map>

#include "TCanvas.h"
#include "TProfile.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TText.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;


// Typedefs for Containers
typedef std::map<Cbc*, std::vector<Channel> > CbcChannelMap;
typedef std::map<Cbc*, TGraphErrors*> GraphMap;
typedef std::map<Cbc*, TF1*> FitMap;
typedef std::map<Cbc*, TH1F*> HistMap;
typedef std::vector<std::pair< std::string, uint8_t> > RegisterVector;
typedef std::map< int, std::vector<uint8_t> >  TestGroupChannelMap;

/*
Key=-1 to do calibration on all channels
Key=0-7 for the 8 Test Groups
*/

class FastCalibration : public Tool
{
  public:
	// C'tor
	FastCalibration( bool pbitwisetune , bool pAllChan ) {
		fVplusVec.push_back( 0x58 );
		fVplusVec.push_back( 0x78 );
		fVplusVec.push_back( 0x98 );
		fdoTGrpCalib = !pAllChan;
		fdoBitWisetuning = pbitwisetune;
		for ( int gid = -1; gid < 8; gid++ ) {
			std::vector<uint8_t> tempchannelVec;
			if ( gid > -1 ) {
				for ( int idx = 0; idx < 16; idx++ ) {
					int ctemp1 = idx * 16 + gid * 2;
					int ctemp2 = ctemp1 + 1;
					if ( ctemp1 < 254 ) tempchannelVec.push_back( ctemp1 );
					if ( ctemp2 < 254 )  tempchannelVec.push_back( ctemp2 );

				}
			}
			else {
				for ( int idx = 0; idx < 254; idx++ )
					tempchannelVec.push_back( idx );
			}
			fTestGroupChannelMap[gid] = tempchannelVec;
		}

	}

	// D'tor
	~FastCalibration() {
		if ( fResultFile ) {
			fResultFile->Write();
			fResultFile->Close();
		}
	}

	// methods
	void Initialise();  // wants to be called after SystemController::ReadHW, ReadSettings
	void ScanVplus();
	void ScanOffset();
	void SaveResults( bool pDumpConfigFiles = true ) {
		writeGraphs();
		if ( pDumpConfigFiles ) dumpConfigFiles();
	}

  protected:
	// Canvases
	TCanvas* fVplusCanvas;
	TCanvas* fVcthVplusCanvas;
	TCanvas* fOffsetCanvas;

	// Containers
	CbcChannelMap fCbcChannelMap;
	GraphMap fGraphMap;
	FitMap fFitMap;
	TestGroupChannelMap fTestGroupChannelMap;
	std::vector<uint8_t> fVplusVec;

	// Flags
	bool fdoTGrpCalib;
	bool fdoBitWisetuning;

	// Counters
	uint32_t fNCbc;
	uint32_t fNFe;

	// Settings
	bool fHoleMode;
	bool fTestPulse;
	uint8_t fTestPulseAmplitude;
	uint32_t fEventsPerPoint;
	uint8_t fTargetVcth;
	bool fFitted;



  protected:
	// for changing channel offsets
	void setOffset( uint8_t pOffset, int  pTGrpId );
	void toggleOffsetBit( uint8_t pBit, int  pTGrpId );

	// SCurve related
	void measureSCurves( bool pOffset, int  pTGrpId );
	uint32_t fillSCurves( BeBoard* pBoard,  const Event* pEvent, uint8_t pValue, int  pTGrpId, bool pDraw = false );
	void initializeSCurves( TString pParameter, uint8_t pValue, int  pTGrpId );
	void processSCurves( TString pParameter, uint8_t pValue, bool pDraw, int  pTGrpId );
	void processSCurvesOffset( TString pParameter, uint8_t pTargetBit, bool pDraw, int pTGrpId );

	// general stuff
	void findVplus( bool pDraw );
	void setSystemTestPulse( uint8_t pTPAmplitude, uint8_t pTestGroup );

	void writeGraphs();
	void dumpConfigFiles();

	uint8_t reverse( uint8_t n ) {
		// Reverse the top and bottom nibble then swap them.
		return ( fLookup[n & 0b1111] << 4 ) | fLookup[n >> 4];
	}

	/*!
	* \brief reverse the endianess before writing in to the register
	* \param pDelay: the actual delay
	* \param pGroup: the actual group number
	* \return the reversed endianness
	*/
	uint8_t to_reg( uint8_t pDelay, uint8_t pGroup ) {

		uint8_t cValue = ( ( reverse( pDelay ) ) & 0xF8 ) |
						 ( ( reverse( pGroup ) ) >> 5 );

		//std::cout << std::bitset<8>( cValue ) << " cGroup " << +pGroup << " " << std::bitset<8>( pGroup ) << " pDelay " << +pDelay << " " << std::bitset<8>( pDelay ) << std::endl;
		return cValue;
	}

	unsigned char fLookup[16] =
	{
		0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
		0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
	}; /*!< Lookup table for reverce the endianness */
};


#endif
