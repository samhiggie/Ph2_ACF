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
	void Initialize();
	// method to scan the test pulse delay 
	std::map<Cbc*,std::pair<uint8_t, uint8_t>> ScanTestPulseDelay(uint8_t pVcth);
	//method to print the step pulse delay where pStepSize are the steps of Vcth
	void printScanTestPulseDelay(uint8_t pStepSize);

  private:
	void parseSettings();
	void setSystemTestPulse(uint8_t pTPAmplitude, uint8_t pChannelId);
	void updateHists( std::string pHistName, bool pFinal );
	// return number of events process = vector.size()
	uint32_t fillDelayHist(BeBoard* pBoard, std::vector<Event*> pEventVector, uint32_t pTPDelay);

	ChannelMap fChannelMap;
	uint32_t fNevents;
	uint32_t fHoleMode;
	uint32_t fNCbc;

};

#endif