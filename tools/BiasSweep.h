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
#include <map>
#include "TGraph.h"
#include "TAxis.h"
#include "TString.h"
#include "Utils/CommonVisitors.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

#ifdef __USBINST__
#include "Ke2110Controller.h"
using namespace Ph2_UsbInst;
#endif

class AmuxSetting
{
  public:
    std::string fRegName;
    uint8_t fAmuxCode;
    uint8_t fBitMask;
    uint8_t fBitShift;

    AmuxSetting (std::string pRegName, uint8_t pAmuxCode, uint8_t pBitMask, uint8_t pBitShift) : fRegName (pRegName), fAmuxCode (pAmuxCode), fBitMask (pBitMask), fBitShift (pBitShift) {}
};

class BiasSweep : public Tool
{
  public:
    BiasSweep();
    ~BiasSweep();
    void SweepBias (std::string pBias, Cbc* pCbc);

  private:
    void InitializeAmuxMap();
    //ChipType fChipType;
    TCanvas* fSweepCanvas;

    ChipType fType;
    std::map<std::string, AmuxSetting> fAmuxSettings;

    void writeResults();

};




#endif
