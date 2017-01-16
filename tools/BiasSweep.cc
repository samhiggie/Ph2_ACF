#include "BiasSweep.h"

void BiasSweep::InitializeAmuxMap()
{
    fAmuxSettings.clear();

    if (fType == ChipType::CBC2)
    {
        // key(BiasSweep, reg name, amux code, bit mask, bit shift
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("none"), std::make_tuple ("", 0x00, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vplus"), std::make_tuple ( "Vplus", 0x01, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VCth"), std::make_tuple ("VCth", 0x02, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ihyst"), std::make_tuple ("FrontEndControl", 0x03, 0x3C, 2) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Icomp"), std::make_tuple ("Icomp", 0x04, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("CAL_Vcas"), std::make_tuple ("TestPulseChargeMirrCascodeVolt", 0x05, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ibias"), std::make_tuple ("", 0x06, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Bandgap"), std::make_tuple ("", 0x07, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("CAL_I"), std::make_tuple ("TestPulseChargePumpCurrent", 0x08, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipre1"), std::make_tuple ("Ipre1", 0x09, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipre2"), std::make_tuple ("Ipre2", 0x0A, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vpc"), std::make_tuple ("Vpc", 0x0B, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipsf"), std::make_tuple ("Ipsf", 0x0C, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipaos"), std::make_tuple ("Ipaos", 0x0D, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipa"), std::make_tuple ("Ipa", 0x0E, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vpasf"), std::make_tuple ("", 0x0F, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vpafb"), std::make_tuple ("Vpafb", 0x10, 0xFF, 0) );
    }
    else if (fType == ChipType::CBC3)
    {
        // key(BiasSweep, reg name, amux code, bit mask, bit shift
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("none"),   std::make_tuple ("", 0x00, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipa"),    std::make_tuple ("Ipa", 0x01, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipre2"),  std::make_tuple ("Ipre2", 0x02, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("CAL_I"),  std::make_tuple ("CALIbias", 0x03, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ibias"),  std::make_tuple ("Ibias", 0x04, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vth"),    std::make_tuple ("", 0x05, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VBGbias"), std::make_tuple ("BandgapFuse", 0x06, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VBG_LDO"), std::make_tuple ("", 0x07, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vpafb"),  std::make_tuple ("", 0x08, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Nc50"),   std::make_tuple ("", 0x09, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipre1"),  std::make_tuple ("Ipre1", 0x0A, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipsf"),   std::make_tuple ("Ipsf", 0x0B, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipaos"),  std::make_tuple ("Ipaos", 0x0C, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Icomp"),  std::make_tuple ("Icomp", 0x0D, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ihyst"),  std::make_tuple ("FeCtrl&TrgLat2", 0x0E, 0x3C, 2) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("CAL_Vcasc"), std::make_tuple ("CALVcasc", 0x0F, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VPLUS2"), std::make_tuple ("Vplus1&2", 0x10, 0xF0, 4) ) ;
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VPLUS1"), std::make_tuple ("Vplus1&2", 0x11, 0x0F, 0) ) ;
    }
}

BiasSweep::BiasSweep()
{
}

BiasSweep::~BiasSweep()
{
}

void BiasSweep::SweepBias (std::string pBias, Cbc* pCbc)
{
    fType = pCbc->getChipType();
    //initialize the Amux Setting map by calling the corresponding method
    this->InitializeAmuxMap();
    //since I want to have a simple class to just sweep a bias on 1 CBC, I create the Canvas and Graph inside the method
    //just create objects, sweep and fill and forget about them again!
    TString cName = Form ( "c_BiasSweep_Fe%dCbc%d", pCbc->getFeId(), pCbc->getCbcId() );
    TObject* cObj = gROOT->FindObject ( cName );

    if ( cObj ) delete cObj;

    fSweepCanvas = new TCanvas (cName, Form ("Bias Sweep %s", pBias.c_str() ), 10, 0, 500, 500 );
    fSweepCanvas->SetGrid();
    fSweepCanvas->cd();

    std::time_t cTime = std::time (nullptr);
    cName = Form ("g_BiasSweep_%s_Fe%d_Cbc%d_TS%d", pBias.c_str(), pCbc->getFeId(), pCbc->getCbcId(), cTime );
    cObj = gROOT->FindObject (cName);

    if (cObj) delete cObj;

    TGraph* cGraph = new TGraph ();
    cGraph->SetName (cName);
    std::string cAxis = (pBias.find ("I") != std::string::npos) ? "I" : "V";
    cGraph->SetTitle (Form ("Bias Sweep %s; %s ; %s", pBias.c_str(), pBias.c_str(), cAxis.c_str() ) );
    cGraph->SetLineWidth ( 2 );
    cGraph->SetLineColor (2);
    cGraph->SetMarkerColor (2);
    //cGraph->GetXaxis()->SetTitle (pBias.c_str() );
    bookHistogram ( pCbc, pBias, cGraph );

    LOG (INFO) << "Created Canvas and Graph for Sweep of " << pBias;

#ifdef __USBINST__
    //create instance of Ke2110Controller
    std::string cLogFile = fDirectoryName + "/DMM_log.txt";
    //create a controller, and immediately send a "pause" command to any running server applications
    Ke2110Controller* cKeController = new Ke2110Controller ();
    cKeController->InitializeClient ("localhost", 8083);
    cKeController->SendPause();
    //then set up for local operation
    cKeController->SetLogFileName (cLogFile);
    cKeController->openLogFile();
    cKeController->Reset();
    //are we measuring a voltage for the currents as well?
    //std::string cConfString = (pBias.find ("I") != std::string::npos) ? "CURRENT:DC" : "VOLTAGE:DC";
    std::string cConfString = "VOLTAGE:DC";
    //set up to either measure Current or Voltage, autorange, 10^-4 resolution and autozero
    cKeController->Configure (cConfString, 0, 0.0001, true);
    cKeController->Autozero();
    mypause();
    //now I am ready for a bias sweep
#endif

    //ok, now set the Analogmux to the value required to see the bias there
    //in order to do this, read the current value and store it for later

    uint8_t cOriginalAmuxValue = fCbcInterface->ReadCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux");
    LOG (INFO) << "Analog mux set to: " << std::hex << (cOriginalAmuxValue & 0x1F) << std::dec << " (full register is 0x" << std::hex << +cOriginalAmuxValue << std::dec << ") originally (the Test pulse bits are not changed!)";
    uint8_t cNewValue = cOriginalAmuxValue;

    auto cAmuxValue = fAmuxSettings.find (pBias);

    if (cAmuxValue == std::end (fAmuxSettings) )
    {
        LOG (ERROR) << "Error: the bias " << pBias << " is not part of the known Amux settings - check spelling! - value will be left at the original";
        exit (1);
    }
    else
    {
        cNewValue = (cOriginalAmuxValue & 0xE0) | (cAmuxValue->second.fAmuxCode & 0x1F);
        fCbcInterface->WriteCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux", cNewValue);
        LOG (INFO) << "Analog MUX setting modified to connect " <<  pBias << " (setting to 0x" << std::hex << +cNewValue << std::dec << ")";

        if (cAmuxValue->first == "Vth")
        {
            ThresholdVisitor cThresholdVisitor (fCbcInterface);
            pCbc->accept (cThresholdVisitor);
            uint16_t cOriginalThreshold = cThresholdVisitor.getThreshold();
            LOG (INFO) << "Original threshold set to " << cOriginalThreshold << " (0x" << std::hex << cOriginalThreshold << std::dec << ") - saving for later!";
            cThresholdVisitor.setOption ('w');
            mypause();

            for (uint16_t cThreshold = 0; cThreshold < 1023; cThreshold++)
            {
                cThresholdVisitor.setThreshold (cThreshold);
                pCbc->accept (cThresholdVisitor);
                //std::this_thread::sleep_for (std::chrono::milliseconds (300) );
                double cReading;
#ifdef __USBINST__
                cKeController->Measure();
                cReading = cKeController->GetLatestReadValue();
#endif

                //now I have the value I set and the reading from the DMM
                cGraph->SetPoint (cGraph->GetN(), cThreshold, cReading);

                if (cThreshold % 10 == 0) LOG (INFO) << "Set bias to " << cThreshold << " (0x" << std::hex << cThreshold << std::dec << ") DAC units and read " << cReading << " on the DMM";

                //update the canvas
                if (cThreshold == 1) cGraph->Draw ("APL");
                else if (cThreshold % 10 == 0)
                {
                    fSweepCanvas->Modified();
                    fSweepCanvas->Update();
                }
            }

            //now set back the original value
            LOG (INFO) << "Re-setting original Threshold value of " << cOriginalThreshold << "(0x" << std::hex << cOriginalThreshold << std::dec << ")";
            cThresholdVisitor.setThreshold (cOriginalThreshold);
            pCbc->accept (cThresholdVisitor);
            this->writeResults();
            LOG (INFO) << "Bias Sweep finished, results saved!";

        }

        else
        {
            // here start sweeping the bias!
            // now get the original bias value
            bool cChangeReg = (cAmuxValue->second.fBitMask != 0x00) ? true : false;
            // if the bias is sweepable, save the original value, do a full sweep, update the canvas etc;
            uint8_t cOriginalBiasValue;

            if (cChangeReg)
            {
                cOriginalBiasValue = fCbcInterface->ReadCbcReg (pCbc, cAmuxValue->second.fRegName);
                LOG (INFO) << "Origainal Register Value for bias " << cAmuxValue->first << "(" << cAmuxValue->second.fRegName << ") read to be 0x" << std::hex << +cOriginalBiasValue << std::dec << " - saving for later!";
                uint16_t cRange = (__builtin_popcount (cAmuxValue->second.fBitMask) == 4) ? 16 : 255;

                for (uint8_t cBias = 0; cBias < cRange; cBias++)
                {
                    //make map string, pair<string, uint8_t> and use the string in pair for the bias
                    uint8_t cRegValue = (cBias) << cAmuxValue->second.fBitShift;
                    //LOG (DEBUG) << +cBias << " " << std::hex <<  +cRegValue << std::dec << " " << std::bitset<8> (cRegValue);
                    fCbcInterface->WriteCbcReg (pCbc, cAmuxValue->second.fRegName, cRegValue );

                    //std::this_thread::sleep_for (std::chrono::milliseconds (300) );
                    double cReading = 0;
#ifdef __USBINST__
                    cKeController->Measure();
                    cReading = cKeController->GetLatestReadValue();
                    //cReading /= 10000;
#endif

                    //now I have the value I set and the reading from the DMM
                    cGraph->SetPoint (cGraph->GetN(), cBias, cReading);

                    if (cBias % 10 == 0) LOG (INFO) << "Set bias to 0x" << std::hex << +cBias << std::dec << " DAC units and read " << cReading << " on the DMM";

                    //update the canvas
                    if (cBias == 1) cGraph->Draw ("APL");
                    else if (cBias % 10 == 0)
                    {
                        fSweepCanvas->Modified();
                        fSweepCanvas->Update();
                    }
                }

                // set the bias back to the original value
                fCbcInterface->WriteCbcReg (pCbc, cAmuxValue->second.fRegName, cOriginalBiasValue );
                LOG (INFO) << "Re-setting " << cAmuxValue->second.fRegName << " to original value of 0x" << std::hex << +cOriginalBiasValue << std::dec;
                this->writeResults();
                LOG (INFO) << "Bias Sweep finished, results saved!";

            }
            //else, if the bias is not sweepable just measure what is on the amux output and do what with the result?
            else
            {
                LOG (INFO) << "Not an Amux setting that requires a sweep: " << cAmuxValue->first;
                double cReading = 0;
#ifdef __USBINST__
                cKeController->Measure();
                cReading = cKeController->GetLatestReadValue();
                //TODO
                LOG (INFO) << "Measured bias " << cAmuxValue->first << " to be " << cReading;
#endif

            }
        }

        LOG (INFO) << "Finished sweeping " << pBias << " - now setting Amux settings back to original value of 0x" << std::hex << +cOriginalAmuxValue << std::dec;

        //now set the Amux back to the original value
        fCbcInterface->WriteCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux", cOriginalAmuxValue);
        //to clean up, save everything
    }

#ifdef __USBINST__
    //close the log file
    cKeController->closeLogFile();
    //tell any server to resume the monitoring
    cKeController->SendResume();
#endif
}

void BiasSweep::writeResults()
{
    //to clean up, just use Tool::SaveResults in here!
    this->SaveResults();
    //save the canvas too!
    fResultFile->cd();
    fSweepCanvas->Write ( fSweepCanvas->GetName(), TObject::kOverwrite );
}
