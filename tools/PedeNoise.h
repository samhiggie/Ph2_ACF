/*!
*
* \file PedeNoise.h
* \brief Calibration class, calibration of the hardware
* \author Georg AUZINGER
* \date 12 / 11 / 15
*
* \Support : georg.auzinger@cern.ch
*
*/

#ifndef PedeNoise_h__
#define PedeNoise_h__

#include "Tool.h"
#include "Channel.h"
#include "FastCalibration.h"
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


typedef std::map<Cbc*, TH1F*> HistMap;

// experimental for Noise Measurement
typedef std::map<Cbc*, std::map<uint8_t, uint8_t>> OffsetMap;
/*
Key=-1 to do calibration on all channels
Key=0-7 for the 8 Test Groups
*/

class PedeNoise : public FastCalibration
{

  public:
	PedeNoise( bool pPedeNoise = true ) :
		FastCalibration( false, false ),
		fPedeNoise( pPedeNoise ) {
	}

	void Initialise();
	void measureNoise();
	void Validate();
	void SaveResults() {
		writeGraphs();
	}

  private:
	// Canvases for Pede/Noise Plots
	TCanvas* fNoiseCanvas;
	TCanvas* fPedestalCanvas;
	TCanvas* fFeSummaryCanvas;

	// Canvas for Occupancy plot
	TCanvas* fValidationCanvas;


	// HistMaps for Noise Histos
	HistMap fNoiseMap;
	HistMap fSensorNoiseMapEven;
	HistMap fSensorNoiseMapOdd;
	HistMap fNoiseStripMap;
	HistMap fPedestalMap;

	// Map for Occupancy Measurement
	HistMap fHistMap;

	bool fPedeNoise;


	// Map for initial Offsets
	OffsetMap fOffsetMap;

  protected:
	void saveInitialOffsets();
	void setOffset( uint8_t pOffset, int  pTGrpId );
	void enableTestGroupforNoise( int  pTGrpId );
	void processSCurvesNoise( TString pParameter, uint8_t pValue, bool pDraw, int  pTGrpId );
	void writeGraphs();

};



#endif