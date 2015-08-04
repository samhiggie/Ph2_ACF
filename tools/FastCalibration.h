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
	FastCalibration( bool pbitwisetune , bool pAllChan ) {
		fVplusVec.push_back( 0x14 );
		fVplusVec.push_back( 0x64 );
		fVplusVec.push_back( 0xA4 );
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

	void SaveResults() {
		writeGraphs();
		dumpConfigFiles();
	}

	void Validate();

  private:
	CbcChannelMap fCbcChannelMap;
	GraphMap fGraphMap;
	FitMap fFitMap;
	HistMap fHistMap;
	TestGroupChannelMap fTestGroupChannelMap;
	bool fdoTGrpCalib;
	bool fdoBitWisetuning;
	bool fHoleMode;
	uint32_t fEventsPerPoint;
	uint32_t fNCbc;
	uint8_t fTargetVcth;
	bool fFit;

	std::vector<uint8_t> fVplusVec;


  protected:
	void measureSCurves( bool pOffset, int  pTGrpId );
	//void measureSCurves( bool pOffset );
	void setOffset( uint8_t pOffset, int  pTGrpId );
	//void setOffset( uint8_t pOffset );
	void toggleOffsetBit( uint8_t pBit, int  pTGrpId );
	//void toggleOffsetBit( uint8_t pBit );
	uint32_t fillSCurves( BeBoard* pBoard,  const Event* pEvent, uint8_t pValue, int  pTGrpId, bool pDraw = false );
	//uint32_t fillSCurves( BeBoard* pBoard,  const Event* pEvent, uint8_t pValue, bool pDraw = false );
	void initializeSCurves( TString pParameter, uint8_t pValue, int  pTGrpId );
	//void initializeSCurves( TString pParameter, uint8_t pValue );
	void processSCurves( TString pParameter, uint8_t pValue, bool pDraw, int  pTGrpId );
	void processSCurvesOffset( TString pParameter, uint8_t pTargetBit, bool pDraw, int pTGrpId );
	// void fitVplusVcthGraph();
	void findVplus( bool pDraw );
	void writeGraphs();
	void dumpConfigFiles();

};


#endif
