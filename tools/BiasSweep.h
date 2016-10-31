/*!

        \file                   BiasSweep.h
        \brief                  Class to sweep one of the CBC2/3 biases and perform an analog measurement via the AnalogMux and Ke2110 DMM
        \author                 Georg AUZINGER
        \version                1.0
        \date                   31/10/16
        Support :               mail to : georg.auzinger@SPAMNOT.cern.ch

 */


#ifndef __BIASSWEEP_H__
#define __BIASSWEEP_H__
#include "Tool.h"
#include "Ke2110Controller.h"
#include <map>
#include "TGraph.h"
#include "TString.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace Ph2_UsbInst;

class BiasSweep : public Tool
{
  public:
    BiasSweep();
    ~BiasSweep();
    void SweepBias (std::string pBias, Cbc* pCbc);

  private:
    int fCbcVersion;
    int fCounter;
    TCanvas* fSweepCanvas;

    static  std::map<std::string, uint8_t> fAmuxSettings;

    void writeResults();


};




#endif
