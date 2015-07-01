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



class PulseShape : public Tool
{
  public:
	void Initialize();
	void SaveResults();
  private:
	void parseSettings();


	uint32_t fNevents;
	uint32_t fHoleMode;
	uint32_t fNCbc;
};
#endif