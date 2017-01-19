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
#include <vector>
#include "TGraph.h"
#include "TObject.h"
#include "TAxis.h"
#include "TTree.h"
#include "TString.h"
#include "Utils/CommonVisitors.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

#ifdef __USBINST__
#include "Ke2110Controller.h"
using namespace Ph2_UsbInst;
#endif
#ifdef __MAKECINT__
#pragma link C++ class vector<float>+;
#pragma link C++ class vector<uint16_t>+;
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

class BiasSweepData : public TObject
{
  public:
    uint16_t fFeId;
    uint16_t fCbcId;
    std::string fBias;
    long int fTimestamp;
    std::vector<uint16_t> fXValues;
    std::vector<float> fYValues;

    BiasSweepData() : fFeId (0), fCbcId (0), fBias (""), fTimestamp (0)
    {
    }
    ~BiasSweepData() {}

    //ClassDef (BiasSweepData, 1); //The class title
};

class BiasSweep : public Tool
{
  public:
    BiasSweep();
    ~BiasSweep();
    void Initialize();
    // added for debugging - S.
    // ******
    uint8_t ConfigureAMUX  (std::string pBias , Cbc* pCbc , double pSettlingTime_s = 100.0e-3);
    void ResetAMUX (uint8_t pAmuxValue, Cbc* pCbc, double pSettlingTime_s = 100.0e-3);
    void WriteRegister(std::string pBias , uint8_t cRegValue ,  Cbc* pCbc , double pSettlingTime_s = 100.0e-3);
    uint8_t ReadRegister(std::string pBias , Cbc* pCbc );
    // *******
    void SweepBias (std::string pBias, Cbc* pCbc);



  private:
    void InitializeAmuxMap();
    TCanvas* fSweepCanvas;

    ChipType fType;
    std::map<std::string, AmuxSetting> fAmuxSettings;
    //for the TTree
    BiasSweepData* fData;

    void writeResults();

};




#endif
