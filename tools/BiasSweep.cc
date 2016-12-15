#include "BiasSweep.h"

std::map<std::string, uint8_t> BiasSweep::fAmuxSettings =
{
    {"none", 0x00},
    {"Vplus", 0x01},
    {"VCth", 0x02},
    {"Vth", 0x02},
    {"Ihyst", 0x03},
    {"Icomp", 0x04},
    {"TestPulseChargeMirrCascodeVolt", 0x05},
    {"Ibias", 0x06},
    {"Bandgap", 0x07},
    {"TestPulseChargePumpCurrent", 0x08},
    {"Ipre1", 0x09},
    {"Ipre2", 0x0A},
    {"Vpc", 0x0B},
    {"Ipsf", 0x0C},
    {"Ipaos", 0x0D},
    {"Ipa", 0x0E},
    {"Vpasf", 0x0F},
    {"Vpafb", 0x10}
};

BiasSweep::BiasSweep()
{
}

BiasSweep::~BiasSweep()
{
}

void BiasSweep::SweepBias (std::string pBias, Cbc* pCbc)
{
    //since I want to have a simple class to just sweep a bias on 1 CBC, I create the Canvas and Graph inside the method
    //just create objects, sweep and fill and forget about them again!
    TString cName = Form ( "c_BiasSweep_Fe%dCbc%d", pCbc->getFeId(), pCbc->getCbcId() );
    TObject* cObj = gROOT->FindObject ( cName );

    if ( cObj ) delete cObj;

    fSweepCanvas = new TCanvas (cName, Form ("Bias Sweep %s", pBias.c_str() ), 10, 0, 500, 500 );
    fSweepCanvas->cd();

    std::time_t cTime = std::time (nullptr);
    cName = Form ("g_BiasSweep_%s_Fe%d_Cbc%d_TS%d", pBias.c_str(), pCbc->getFeId(), pCbc->getCbcId(), cTime );
    cObj = gROOT->FindObject (cName);

    if (cObj) delete cObj;

    TGraph* cGraph = new TGraph ();
    cGraph->SetName (cName);
    cGraph->SetTitle (Form ("Bias Sweep %s", pBias.c_str() ) );
    cGraph->SetLineWidth ( 2 );
    cGraph->GetXaxis()->SetTitle (pBias.c_str() );
    bookHistogram ( pCbc, pBias, cGraph );

    LOG (INFO) << "Created Canvas and Graph for Sweep of " << pBias;

#ifdef __USBINST__
    //create instance of Ke2110Controller
    std::string cLogFile = fDirectoryName + "/DMM_log.txt";
    Ke2110Controller* cKeController = new Ke2110Controller ();
    cKeController->SetLogFileName (cLogFile);
    cKeController->Reset();
    std::string cConfString = (pBias.find ("I") != std::string::npos) ? "CURRENT:DC" : "VOLTAGE:DC";
    //set up to either measure Current or Voltage, autorange, 10^-4 resolution and autozero
    cKeController->Configure (cConfString, 0, 0.0001);
    cKeController->Autozero();
#endif

    //ok, now set the Analogmux to the value required to see the bias there
    //in order to do this, read the current value and store it for later

    uint8_t cOriginalAmuxValue = fCbcInterface->ReadCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux");
    LOG (INFO) << "Analog mux set to: " << std::hex << (cOriginalAmuxValue & 0x1F) << std::dec << " (the Test pulse bits are not changed!)";

    auto cAmuxValue = fAmuxSettings.find (pBias);

    uint8_t cNewValue = cOriginalAmuxValue;

    if (cAmuxValue == std::end (fAmuxSettings) ) LOG (ERROR) << "Error: the bias " << pBias << " is not part of the known Amux settings - check spelling! - value will be left at the original";
    else
    {
        cNewValue = (cOriginalAmuxValue & 0xE0) | (cAmuxValue->second & 0x1F);
        fCbcInterface->WriteCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux", cNewValue);
        LOG (INFO) << "Analog MUX setting modified to connect " <<  pBias;
    }

    // here start sweeping the bias!
    // first, get the maximum necessary range (8 bits except for VTh of Cbc3)
    uint16_t cMaxValue = (pCbc->getChipType() == ChipType::CBC3 && pBias == "Vth") ? 1023 : 255;
    LOG (INFO) << "The selected Bias " << pBias << " requires a range of " << cMaxValue;

    ThresholdVisitor cThresholdVisitor (fCbcInterface, 0);

    for (uint16_t cBias = 0; cBias < cMaxValue; cBias++)
    {
        if (pBias == "VCth" || pBias == "Vth" )
        {
            cThresholdVisitor.setThreshold (cBias);
            pCbc->accept (cThresholdVisitor);
        }
        else
            fCbcInterface->WriteCbcReg (pCbc, pBias, cBias);

        double cReading = 0;
#ifdef __USBINST__
        cKeController->Measure();
        cReading = cKeController->GetLatestReadValue();
#endif

        //now I have the value I set and the reading from the DMM
        cGraph->SetPoint (cGraph->GetN(), cBias, cReading);
        LOG (INFO) << "Set bias to 0x" << std::hex << cBias << std::dec << " DAC units and read " << cReading << " on the DMM";

        //update the canvas
        if (cBias == 0) cGraph->Draw ("APL");
        else fSweepCanvas->Modified();

        fSweepCanvas->Update();
    }

    LOG (INFO) << "Finished sweeping " << pBias << " - now setting Amux settings back to original value of 0x" << std::hex << cOriginalAmuxValue << std::dec;

    //now set the Amux back to the original value
    fCbcInterface->WriteCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux", cOriginalAmuxValue);

    //to clean up, save everything
    this->writeResults();
    LOG (INFO) << "Bias Sweep finished, results saved!";
}

void BiasSweep::writeResults()
{
    //to clean up, just use Tool::SaveResults in here!
    this->SaveResults();
    //save the canvas too!
    fResultFile->cd();
    fSweepCanvas->Write ( fSweepCanvas->GetName(), TObject::kOverwrite );
}
