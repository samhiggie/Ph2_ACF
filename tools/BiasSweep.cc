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
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("CAL_Vcasc"), std::make_tuple ("CALVcasc", 0x0F, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VPLUS2"), std::make_tuple ("Vplus1&2", 0x10, 0xF0, 4) ) ;
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VPLUS1"), std::make_tuple ("Vplus1&2", 0x11, 0x0F, 0) ) ;
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VDDA"), std::make_tuple ("", 0x00, 0x00, 0) ) ;
    }
}

//cases: 1)Vth 2)Voltage & sweepable -> DMM 3) not sweepable: old code 4) current & sweepable: 1 reading on dmm with default setting, then sweep using PS

BiasSweep::BiasSweep()
{
#ifdef __USBINST__

    fKeController = nullptr;

    fArdNanoController = nullptr;

#endif
}

BiasSweep::~BiasSweep()
{
#ifdef __USBINST__

    if (fKeController) delete fKeController;

    if (fArdNanoController) delete fArdNanoController;

#endif
}

void BiasSweep::Initialize()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "SweepTimeout" );
    fSweepTimeout = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    cSetting = fSettingsMap.find ( "KePort" );
    fKePort = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 8083;
    cSetting = fSettingsMap.find ( "HMPPort" );
    fHMPPort = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 8082;

#ifdef __USBINST__
    //create a controller
    fKeController = new Ke2110Controller ();
    fKeController->InitializeClient ("localhost", fKePort);
    // first is async, second is multex??
    LOG (INFO) << BOLDRED << "Attempting to connect to arduino nano!" << RESET;
    fArdNanoController = new ArdNanoController (false, false);
    bool cArduinoReady = fArdNanoController->CheckArduinoState();

    if (!cArduinoReady)
    {
        fKeController->SendQuit();
        exit (1);
        //here quit the KeControler too
    }

    fArdNanoController->ControlLED (1);
#endif

    gROOT->ProcessLine ("#include <vector>");
    TString cName = "c_BiasSweep";
    TObject* cObj = gROOT->FindObject ( cName );

    if ( cObj ) delete cObj;

    fSweepCanvas = new TCanvas (cName, "Bias Sweep", 10, 0, 500, 500 );
    fSweepCanvas->SetGrid();
    fSweepCanvas->cd();
    LOG (INFO) << "Created Canvas for Bias sweeps";

    //initialize empty bias sweep object
    fData = new BiasSweepData();

    for (auto cBoard : fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            fType = cFe->getChipType();

            for (auto cCbc : cFe->fCbcVector)
            {
                cName = Form ("BiasSweep_Fe%d_Cbc%d", cCbc->getFeId(), cCbc->getCbcId() );
                cObj = gROOT->FindObject (cName);

                if (cObj) delete cObj;

                TTree* cTmpTree = new TTree (cName, cName);
                //cTmpTree->Branch (Form ("BiasSweepData_Fe%d_Cbc%d", cCbc->getFeId(), cCbc->getCbcId() ), "BiasSweepData", &fData);
                cTmpTree->Branch ("Bias", &fData->fBias);
                cTmpTree->Branch ("Fe", &fData->fFeId, "Fe/s" );
                cTmpTree->Branch ("Cbc", &fData->fCbcId, "Cbc/s" );
                cTmpTree->Branch ("Time", &fData->fTimestamp, "Time/l" );
                cTmpTree->Branch ("Unit", &fData->fUnit, "Unit/C" );
                cTmpTree->Branch ("InitialBiasValue", &fData->fInitialXValue, "InitialDAC/s");
                cTmpTree->Branch ("InitialDMMValue", &fData->fInitialYValue, "InitialDMM/F");
                cTmpTree->Branch ("BiasValues", &fData->fXValues);
                cTmpTree->Branch ("DMMValues", &fData->fYValues);

                this->bookHistogram (cCbc, "DataTree", cTmpTree);

                LOG (INFO) << "TTree for BiasSweep data for Fe " << +cCbc->getFeId() << " Cbc " << +cCbc->getCbcId() << " created!";
            }
        }

    }

    //initialize the Amux Setting map by calling the corresponding method
    this->InitializeAmuxMap();
}


void BiasSweep::SweepBias (std::string pBias, Cbc* pCbc)
{
    auto cAmuxValue = fAmuxSettings.find (pBias);

    if (cAmuxValue == std::end (fAmuxSettings) )
    {
        LOG (ERROR) << "Error: the bias " << pBias << " is not part of the known Amux settings - check spelling! - value will be left at the original";
        exit (1);
    }
    else
    {
        //boolean variable to find out if current or not
        bool cCurrent = (pBias.find ("I") != std::string::npos) ? true : false;
        //since I want to have a simple class to just sweep a bias on 1 CBC, I create the Graph inside the method
        //just create objects, sweep and fill and forget about them again!

        std::time_t cTime = std::time (nullptr);
        TString cName = Form ("g_BiasSweep_%s_Fe%d_Cbc%d_TS%d", pBias.c_str(), pCbc->getFeId(), pCbc->getCbcId(), cTime );

        TObject* cObj = gROOT->FindObject (cName);

        if (cObj) delete cObj;

        TGraph* cGraph = new TGraph ();
        cGraph->SetName (cName);
        std::string cYAxis = (cCurrent) ? "I [A]" : "V [V]";
        std::string cXAxis = pBias + " [DAC]";
        cGraph->SetTitle (Form ("Bias Sweep %s; %s ; %s", pBias.c_str(), cXAxis.c_str(), cYAxis.c_str() ) );
        cGraph->SetLineWidth ( 2 );
        cGraph->SetLineColor (2);
        cGraph->SetMarkerColor (2);
        //cGraph->GetXaxis()->SetTitle (pBias.c_str() );
        bookHistogram ( pCbc, pBias, cGraph );

        LOG (INFO) << "Created Graph for Sweep of " << pBias;

        //now get the TTree for this CBC and fill the already known fields
        TTree* cTmpTree = static_cast<TTree*> (getHist ( pCbc, "DataTree" ) );
        fData->fBias = pBias.c_str();
        fData->fTimestamp = static_cast<long int> (cTime);
        fData->fFeId = pCbc->getFeId();
        fData->fCbcId = pCbc->getCbcId();
        fData->fUnit[0] = (cCurrent) ? 'I' : 'V';
        fData->fUnit[1] = 0;
        fData->fInitialXValue = 0;
        fData->fInitialYValue = 0;
        fData->fXValues.clear();
        fData->fYValues.clear();

#ifdef __USBINST__
        std::string cLogFile = fDirectoryName + "/DMM_log.txt";
        //send pause command to any running Ke server
        LOG (INFO) << YELLOW << "Sending request to pause Temperature monitoring" << RESET;
        fKeController->SendPause();
        //then set up for local operation
        fKeController->SetLogFileName (cLogFile);
        fKeController->openLogFile();
        fKeController->Reset();
        std::string cConfString = "VOLTAGE:DC";
        //set up to either measure Voltage DC - params: configstring, range, resolution, autorange
        fKeController->Configure (cConfString, 0, 0.0001, true);
        fKeController->Autozero();

        if (cCurrent)
        {
            //initialize the HMP4040Client to connect to the server
            fHMPClient = new HMP4040Client ("localhost", fHMPPort);

            int cCounter = 0;
            LOG (INFO) << YELLOW <<  "Trying to pause monitoring with HMP4040!" << RESET;

            while (!fHMPClient->StopMonitoring() )
            {
                if (cCounter++ > 5)
                {
                    LOG (ERROR) << RED << "HMP4040 Monitoring pause failed!" << RESET;
                    exit (1);
                }
            }

            LOG (INFO) << YELLOW << "HMP4040 Monitoring Pause request sent successfully!" << RESET;
        }

#endif

        //ok, now set the Analogmux to the value required to see the bias there
        //in order to do this, read the current value and store it for later
        uint8_t cOriginalAmuxValue = this->configureAmux (cAmuxValue, pCbc, 1);

        if (cAmuxValue->first == "Vth")
            this->sweepVth (cGraph, pCbc);
        // the bias is not Vth
        else
        {
            // here start sweeping the bias!
            bool cChangeReg = (cAmuxValue->second.fBitMask != 0x00) ? true : false;

            // the bias is sweepable
            if (cChangeReg)
                // if the bias is sweepable, save the original value, do a full sweep, update the canvas etc;
                this->sweep8Bit (cAmuxValue, cGraph, pCbc, cCurrent);
            //else, the bias is not sweepable just measure what is on the amux output and put it in the initial value in the tree
            else
                this->measureSingle (cAmuxValue, pCbc);
        }

        //to clean up, save everything
        this->resetAmux (cOriginalAmuxValue, pCbc, 1);

#ifdef __USBINST__
        //close the log file
        fKeController->closeLogFile();
        //tell any server to resume the monitoring
        LOG (INFO) << YELLOW << "Sending request to resume Temperatue monitoring" << RESET;
        fKeController->SendResume();

        if (cCurrent)
        {
            int cCounter = 0;
            LOG (INFO) << YELLOW << "Trying to resume monitoring with HMP4040!" << RESET;

            while (!fHMPClient->StartMonitoring() )
            {
                if (cCounter++ > 5)
                {
                    LOG (ERROR) << RED <<  "HMP4040 Monitoring resume failed!" << RESET;
                    exit (1);
                }
            }

            LOG (INFO) << YELLOW << "HMP4040 Monitoring Resume request sent successfully!" << RESET;

            delete fHMPClient;
        }

#endif
        cTmpTree->Fill();
        this->writeResults();
        LOG (INFO) << "Bias Sweep finished, results saved!";
    }
}

uint8_t BiasSweep::configureAmux (std::map<std::string, AmuxSetting>::iterator pAmuxValue, Cbc* pCbc, double pSettlingTime_s  )
{
    // first read original value in Amux register and save for later
    uint8_t cOriginalAmuxValue = fCbcInterface->ReadCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux");
    LOG (INFO) << "Analog mux set to: " << std::hex << (cOriginalAmuxValue & 0x1F) << std::dec << " (full register is 0x" << std::hex << +cOriginalAmuxValue << std::dec << ") originally (the Test pulse bits are not changed!)";

    //ok, now set the Analogmux to the value required to see the bias there
    if (pAmuxValue->first == "VDDA")
    {
        LOG (INFO) << "Bias setting is " << pAmuxValue->first << " -this is not routed via the Amux, thus leaving settings at original value!";
        //need to switch the Arduino nano controller to VDDA
#ifdef __USBINST__
        fArdNanoController->ControlRelay (1);
        LOG (INFO) << "Setting Arduino Nano relay to " << pAmuxValue->first;
#endif
        return cOriginalAmuxValue;
    }
    else
    {
#ifdef __USBINST__
        fArdNanoController->ControlRelay (0);
        LOG (INFO) << "Setting Arduino Nano relay to Amux (default)";
#endif

        uint8_t cNewValue = (cOriginalAmuxValue & 0xE0) | (pAmuxValue->second.fAmuxCode & 0x1F);
        fCbcInterface->WriteCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux", cNewValue);
        LOG (INFO) << "Analog MUX setting modified to connect " <<  pAmuxValue->first << " (setting to 0x" << std::hex << +cNewValue << std::dec << ")";

        for ( unsigned int i = 0 ; i < (int) (pSettlingTime_s / 100e-3) ; i++)
            std::this_thread::sleep_for (std::chrono::milliseconds ( 100 ) );

        return cOriginalAmuxValue;
    }
}
void BiasSweep::resetAmux (uint8_t pAmuxValue, Cbc* pCbc, double pSettlingTime_s  )
{
#ifdef __USBINST__
    //fArdNanoController->ControlRelay (0);
    LOG (INFO) << "Setting Arduino Nano relay to Amux (default)";
#endif
    LOG (INFO) << "Reseting Amux settings back to original value of 0x" << std::hex << +pAmuxValue;
    fCbcInterface->WriteCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux", pAmuxValue);

    for ( unsigned int i = 0 ; i < (int) (pSettlingTime_s / 100e-3) ; i++)
        std::this_thread::sleep_for (std::chrono::milliseconds ( 100 ) );
}

void BiasSweep::sweep8Bit (std::map<std::string, AmuxSetting>::iterator pAmuxValue, TGraph* pGraph, Cbc* pCbc, bool pCurrent)
{

    uint8_t cOriginalBiasValue = fCbcInterface->ReadCbcReg (pCbc, pAmuxValue->second.fRegName);
    LOG (INFO) << "Origainal Register Value for bias " << pAmuxValue->first << "(" << pAmuxValue->second.fRegName << ") read to be 0x" << std::hex << +cOriginalBiasValue << std::dec << " - saving for later!";

    //take one initial reading on the dmm and save as the inital value in the tree
    float cInitialReading = 0;
#ifdef __USBINST__
    fKeController->Measure();
    cInitialReading = fKeController->GetLatestReadValue();
#endif
    fData->fInitialXValue = cOriginalBiasValue;
    fData->fInitialYValue = cInitialReading;

    uint16_t cRange = (__builtin_popcount (pAmuxValue->second.fBitMask) == 4) ? 16 : 255;

    for (uint8_t cBias = 0; cBias < cRange; cBias++)
    {
        //make map string, pair<string, uint8_t> and use the string in pair for the bias
        uint8_t cRegValue = (cBias) << pAmuxValue->second.fBitShift;
        //LOG (DEBUG) << +cBias << " " << std::hex <<  +cRegValue << std::dec << " " << std::bitset<8> (cRegValue);
        fCbcInterface->WriteCbcReg (pCbc, pAmuxValue->second.fRegName, cRegValue );

        std::this_thread::sleep_for (std::chrono::milliseconds (fSweepTimeout) );
        double cReading = 0;
#ifdef __USBINST__

        if (!pCurrent)
        {
            fKeController->Measure();
            cReading = fKeController->GetLatestReadValue();

            if (cBias % 10 == 0) LOG (INFO) << "Set bias to " << +cBias << " (0x" << std::hex << +cBias << std::dec << ") DAC units and read " << cReading << " V on the DMM";
        }
        else if (pCurrent)
        {
            //read the LV PS instead
            bool cSuccess = false;
            int cTimeoutCounter = 0;

            while (!cSuccess)
            {
                cSuccess = fHMPClient->MeasureValues();

                if (cTimeoutCounter++ > 5)
                    break;
            }

            if (cSuccess)
            {
                // request was successfull, so proceed
                cReading = fHMPClient->fValues.fCurrents.at (2);

                if (cBias % 10 == 0) LOG (INFO) << "Set bias to " << +cBias << " (0x" << std::hex << +cBias << std::dec << ") DAC units and read " << cReading << " A on the HMP4040";
            }
            else
                LOG (ERROR) << "Could not retreive the measurement values from the HMP4040!";
        }

#endif


        //now I have the value I set and the reading from the DMM
        pGraph->SetPoint (pGraph->GetN(), cBias, cReading);

        //update the canvas
        //update the canvas
        if (cBias == 1) pGraph->Draw ("APL");
        else if (cBias % 10 == 0)
        {
            fSweepCanvas->Modified();
            fSweepCanvas->Update();
        }

        //now set the values for the ttree
        fData->fXValues.push_back (cBias);
        fData->fYValues.push_back (cReading);
    }

    // set the bias back to the original value
    fCbcInterface->WriteCbcReg (pCbc, pAmuxValue->second.fRegName, cOriginalBiasValue );
    LOG (INFO) << "Re-setting " << pAmuxValue->second.fRegName << " to original value of 0x" << std::hex << +cOriginalBiasValue << std::dec;
}

void BiasSweep::measureSingle (std::map<std::string, AmuxSetting>::iterator pAmuxValue, Cbc* pCbc)
{
    LOG (INFO) << "Not an Amux setting that requires a sweep: " << pAmuxValue->first;
    double cReading = 0;
#ifdef __USBINST__
    fKeController->Measure();
    cReading = fKeController->GetLatestReadValue();
    LOG (INFO) << "Measured bias " << pAmuxValue->first << " to be " << cReading;
#endif
    //now set the values for the ttree
    fData->fInitialXValue = 0;
    fData->fInitialYValue = cReading;
}

void BiasSweep::sweepVth (TGraph* pGraph, Cbc* pCbc)
{
    ThresholdVisitor cThresholdVisitor (fCbcInterface);
    pCbc->accept (cThresholdVisitor);
    uint16_t cOriginalThreshold = cThresholdVisitor.getThreshold();
    LOG (INFO) << "Original threshold set to " << cOriginalThreshold << " (0x" << std::hex << cOriginalThreshold << std::dec << ") - saving for later!";
    cThresholdVisitor.setOption ('w');

    //take one initial reading on the dmm
    float cInitialReading = 0;
#ifdef __USBINST__
    fKeController->Measure();
    cInitialReading = fKeController->GetLatestReadValue();
#endif
    fData->fInitialXValue = cOriginalThreshold;
    fData->fInitialYValue = cInitialReading;

    for (uint16_t cThreshold = 0; cThreshold < 1023; cThreshold++)
    {
        cThresholdVisitor.setThreshold (cThreshold);
        pCbc->accept (cThresholdVisitor);
        //std::this_thread::sleep_for (std::chrono::milliseconds (fSweepTimeout) );
        double cReading = 0;
#ifdef __USBINST__
        fKeController->Measure();
        cReading = fKeController->GetLatestReadValue();
#endif

        //now I have the value I set and the reading from the DMM
        pGraph->SetPoint (pGraph->GetN(), cThreshold, cReading);

        if (cThreshold % 10 == 0) LOG (INFO) << "Set bias to " << cThreshold << " (0x" << std::hex << cThreshold << std::dec << ") DAC units and read " << cReading << " on the DMM";

        //update the canvas
        if (cThreshold == 1) pGraph->Draw ("APL");
        else if (cThreshold % 10 == 0)
        {
            fSweepCanvas->Modified();
            fSweepCanvas->Update();
        }

        //now set the values for the ttree
        fData->fXValues.push_back (cThreshold);
        fData->fYValues.push_back (cReading);
    }

    //now set back the original value
    LOG (INFO) << "Re-setting original Threshold value of " << cOriginalThreshold << "(0x" << std::hex << cOriginalThreshold << std::dec << ")";
    cThresholdVisitor.setThreshold (cOriginalThreshold);
    pCbc->accept (cThresholdVisitor);
}

void BiasSweep::writeResults()
{
    //to clean up, just use Tool::SaveResults in here!
    this->SaveResults();
    //save the canvas too!
    fResultFile->cd();
    fSweepCanvas->Write ( fSweepCanvas->GetName(), TObject::kOverwrite );
}
