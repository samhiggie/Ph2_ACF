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

#ifndef OldCalibration_h__
#define OldCalibration_h__

#include "SCurve.h"
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
typedef std::map<Cbc*, TGraphErrors*> GraphMap;
typedef std::map<Cbc*, TF1*> FitMap;
typedef std::map<Cbc*, TH1F*> HistMap;



class OldCalibration : public SCurve
{
  public:
	// C'tor
	OldCalibration( bool pAllChan ) {
		SCurve::MakeTestGroups( pAllChan );
		fVplusVec.push_back( 0x58 );
		fVplusVec.push_back( 0x78 );
		fVplusVec.push_back( 0x98 );
	}

	// D'tor
	~OldCalibration() {
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
		if ( pDumpConfigFiles ) SCurve::dumpConfigFiles();
	}

  protected:
	// Canvases
	TCanvas* fVplusCanvas;
	TCanvas* fVcthVplusCanvas;
	TCanvas* fOffsetCanvas;

	// Containers
	GraphMap fGraphMap;
	FitMap fFitMap;
	std::vector<uint8_t> fVplusVec;


	// Settings
	uint8_t fTargetVcth;

  protected:

	// SCurve related
	void processSCurves( TString pParameter, uint8_t pValue, bool pDraw, int  pTGrpId );

	// general stuff
	void findVplus( bool pDraw );

	void writeGraphs();
};


#endif
