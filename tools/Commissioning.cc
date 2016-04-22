#include "Commissioning.h"

void Commissioning::Initialize (uint32_t pStartLatency, uint32_t pLatencyRange)
{
    for ( auto& cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto& cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            TCanvas* ctmpCanvas = new TCanvas ( Form ( "c_online_canvas_fe%d", cFeId ), Form ( "FE%d  Online Canvas", cFeId ) );
            // ctmpCanvas->Divide( 2, 2 );
            fCanvasMap[cFe] = ctmpCanvas;

            fNCbc = cFe->getNCbc();

            // 1D Hist forlatency scan
            TString cName =  Form ( "h_module_latency_Fe%d", cFeId );
            TObject* cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH1F* cLatHist = new TH1F ( cName, Form ( "Latency FE%d; Latency; # of Hits", cFeId ), (pLatencyRange ) * fTDCBins, pStartLatency ,  pStartLatency + (pLatencyRange )  * fTDCBins );
            //modify the axis labels
            uint32_t pLabel = pStartLatency;

            for (uint32_t cBin = 0; cBin < cLatHist->GetNbinsX(); cBin++)
            {
                if ( cBin % fTDCBins == 1)
                {
                    cLatHist->GetXaxis()->SetBinLabel (cBin, std::to_string (pLabel).c_str() );
                    pLabel++;
                }
            }

            cLatHist->GetXaxis()->SetTitle ("Trigger Latency [cc]");
            cLatHist->SetFillColor ( 4 );
            cLatHist->SetFillStyle ( 3001 );
            bookHistogram ( cFe, "module_latency", cLatHist );

            cName =  Form ( "h_module_stub_latency_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH1F* cStubHist = new TH1F ( cName, Form ( "Stub Lateny FE%d; Stub Lateny; # of Stubs", cFeId ), pLatencyRange, pStartLatency, pStartLatency + pLatencyRange);
            cStubHist->SetMarkerStyle ( 2 );
            bookHistogram ( cFe, "module_stub_latency", cStubHist );
        }
    }

    parseSettings();
    std::cout << "Histograms and Settings initialised." << std::endl;
}

std::map<Module*, uint8_t> Commissioning::ScanLatency ( uint8_t pStartLatency, uint8_t pLatencyRange )
{
    // This is not super clean but should work
    // Take the default VCth which should correspond to the pedestal and add 8 depending on the mode to exclude noise
    CbcRegReader cReader ( fCbcInterface, "VCth" );
    this->accept ( cReader );
    uint8_t cVcth = cReader.fRegValue;

    int cVcthStep = ( fHoleMode == 1 ) ? +15 : -15;
    std::cout << "VCth value from config file is: " << +cVcth << " ;  changing by " << cVcthStep << "  to " << + ( cVcth + cVcthStep ) << " supress noise hits for crude latency scan!" << std::endl;
    cVcth += cVcthStep;

    //  Set that VCth Value on all FEs
    CbcRegWriter cWriter ( fCbcInterface, "VCth", cVcth );
    this->accept ( cWriter );
    this->accept ( cReader );

    // Now the actual scan
    std::cout << "Scanning Latency ... " << std::endl;
    uint32_t cIterationCount = 0;

    for ( uint8_t cLat = pStartLatency; cLat < pStartLatency + pLatencyRange; cLat++ )
    {
        //  Set a Latency Value on all FEs
        cWriter.setRegister ( "TriggerLatency", cLat );
        this->accept ( cWriter );


        // Take Data for all Modules
        for ( BeBoard* pBoard : fBoardVector )
        {
            // I need this to normalize the TDC values I get from the Strasbourg FW
            bool pStrasbourgFW = false;

            if (pBoard->getBoardType() == "GLIB") pStrasbourgFW = true;

            fBeBoardInterface->ReadNEvents ( pBoard, fNevents );
            const std::vector<Event*>& events = fBeBoardInterface->GetEvents ( pBoard );

            // Loop over Events from this Acquisition
            for ( auto cFe : pBoard->fModuleVector )
            {
                int cNHits = 0;
                cNHits += countHitsLat ( cFe, events, "module_latency", cLat, cIterationCount, pStrasbourgFW);

                std::cout << "FE: " << +cFe->getFeId() << " Latency " << +cLat << " Hits " << cNHits  << " Events " << fNevents << std::endl;
            }
        }

        // done counting hits for all FE's, now update the Histograms
        updateHists ( "module_latency", false );
        cIterationCount++;
    }


    // analyze the Histograms
    std::map<Module*, uint8_t> cLatencyMap;

    //std::cout << "Identified the Latency with the maximum number of Hits at: " << std::endl;

    //for ( auto cFe : fModuleHistMap )
    //{
    //TH1F* cTmpHist = ( TH1F* ) getHist ( static_cast<Ph2_HwDescription::Module*> ( cFe.first ), "module_latency" );
    ////the true latency now is the floor(iBin/8)
    //uint8_t cLatency =  static_cast<uint8_t> ( floor ( (cTmpHist->GetMaximumBin() - 1 ) / 8) );
    //cLatencyMap[cFe.first] = cLatency;
    //cWriter.setRegister ( "TriggerLatency", cLatency );
    //this->accept ( cWriter );

    //std::cout << "    FE " << +cFe.first->getModuleId()  << ": " << +cLatency << " clock cycles!" << std::endl;
    //}
    updateHists ( "module_latency", true );

    return cLatencyMap;
}

std::map<Module*, uint8_t> Commissioning::ScanStubLatency ( uint8_t pStartLatency, uint8_t pLatencyRange )
{
    // This is not super clean but should work
    // Take the default VCth which should correspond to the pedestal and add 8 depending on the mode to exclude noise
    CbcRegReader cReader ( fCbcInterface, "VCth" );
    this->accept ( cReader );
    uint8_t cVcth = cReader.fRegValue;

    int cVcthStep = ( fHoleMode == 1 ) ? +20 : -20;
    std::cout << "VCth value from config file is: " << +cVcth << " ;  changing by " << cVcthStep << "  to " << + ( cVcth + cVcthStep ) << " supress noise hits for crude latency scan!" << std::endl;
    cVcth += cVcthStep;

    //  Set that VCth Value on all FEs
    CbcRegWriter cVcthWriter ( fCbcInterface, "VCth", cVcth );
    this->accept ( cVcthWriter );
    this->accept ( cReader );

    // Now the actual scan
    std::cout << "Scanning Stub Latency ... " << std::endl;

    for ( uint8_t cLat = pStartLatency; cLat < pStartLatency + pLatencyRange; cLat++ )
    {
        uint32_t cN = 1;
        uint32_t cNthAcq = 0;
        int cNStubs = 0;

        // Take Data for all Modules
        for ( BeBoard* pBoard : fBoardVector )
        {
            //here set the stub latency
            std::string cBoardType = pBoard->getBoardType();
            std::vector<std::pair<std::string, uint32_t>> cRegVec;

            if (cBoardType == "GLIB")
            {
                cRegVec.push_back ({"cbc_stubdata_latency_adjust_fe1", cLat});
                cRegVec.push_back ({"cbc_stubdata_latency_adjust_fe2", cLat});
            }
            else if (cBoardType == "ICGLIB")
            {
                cRegVec.push_back ({"cbc_daq_ctrl.latencies.stub_latency", cLat});
            }

            fBeBoardInterface->WriteBoardMultReg (pBoard, cRegVec);

            fBeBoardInterface->ReadNEvents ( pBoard, fNevents );
            const std::vector<Event*>& events = fBeBoardInterface->GetEvents ( pBoard );

            // if(cN <3 ) std::cout << *cEvent << std::endl;

            // Loop over Events from this Acquisition
            for ( auto& cEvent : events )
            {
                for ( auto cFe : pBoard->fModuleVector )
                    cNStubs += countStubs ( cFe, cEvent, "module_stub_latency", cLat );

                cN++;
            }

            cNthAcq++;
            std::cout << "Stub Latency " << +cLat << " Stubs " << cNStubs  << " Events " << cN << std::endl;

        }

        // done counting hits for all FE's, now update the Histograms
        updateHists ( "module_stub_latency", false );
    }

    // analyze the Histograms
    std::map<Module*, uint8_t> cStubLatencyMap;

    std::cout << "Identified the Latency with the maximum number of Stubs at: " << std::endl;

    for ( auto cFe : fModuleHistMap )
    {
        TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( cFe.first, "module_stub_latency" ) );
        uint8_t cStubLatency =  static_cast<uint8_t> ( cTmpHist->GetMaximumBin() - 1 );
        cStubLatencyMap[cFe.first] = cStubLatency;

        //BeBoardRegWriter cLatWriter ( fBeBoardInterface, "", 0 );

        //if ( cFe.first->getFeId() == 0 ) cLatWriter.setRegister ( "cbc_stubdata_latency_adjust_fe1", cStubLatency );
        //else if ( cFe.first->getFeId() == 1 ) cLatWriter.setRegister ( "cbc_stubdata_latency_adjust_fe2", cStubLatency );

        //this->accept ( cLatWriter );

        std::cout << "Stub Latency FE " << +cFe.first->getModuleId()  << ": " << +cStubLatency << " clock cycles!" << std::endl;
    }

    return cStubLatencyMap;
}


//////////////////////////////////////          PRIVATE METHODS             //////////////////////////////////////


int Commissioning::countHitsLat ( Module* pFe,  const std::vector<Event*> pEventVec, std::string pHistName, uint8_t pParameter, uint32_t pIterationCount, bool pStrasbourgFW)
{
    int cHitSum = 0;
    //  get histogram to fill
    TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( pFe, pHistName ) );

    for (auto& cEvent : pEventVec)
    {
        //first, reset the hit counter - I need separate counters for each event
        int cHitCounter = 0;
        //get TDC value for this particular event
        uint8_t cTDCVal = cEvent->GetTDC();

        if (cTDCVal != 0 && pStrasbourgFW) cTDCVal -= 5;

        if (cTDCVal > 8 ) std::cout << "ERROR, TDC value not within expected range - normalized value is " << +cTDCVal << " - original Value was " << +cEvent->GetTDC() << "; not considering this Event!" <<  std::endl;

        else
        {
            for ( auto cCbc : pFe->fCbcVector )
            {
                //now loop the channels for this particular event and increment a counter
                for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
                {
                    if ( cEvent->DataBit ( cCbc->getFeId(), cCbc->getCbcId(), cId ) )
                        cHitCounter++;
                }
            }

            //now I have the number of hits in this particular event for all CBCs and the TDC value
            //if this is a GLIB with Strasbourg FW, the TDC values are always between 5 and 12 which means that I have to subtract 5 from the TDC value to have it normalized between 0 and 7

            uint32_t iBin = pParameter + pIterationCount * (fTDCBins - 1) + cTDCVal;
            cTmpHist->Fill ( iBin , cHitCounter);
            //std::cout << "Latency " << +pParameter << " TDC Value " << +cTDCVal << " NHits: " << cHitCounter << " iteration count " << pIterationCount << " Value " << iBin << " iBin " << cTmpHist->FindBin(iBin) << std::endl;

            cHitSum += cHitCounter;
        }
    }

    return cHitSum;
}

int Commissioning::countHits ( Module* pFe,  const Event* pEvent, std::string pHistName, uint8_t pParameter )
{
    // loop over Modules & Cbcs and count hits separately
    int cHitCounter = 0;

    //  get histogram to fill
    TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( pFe, pHistName ) );

    for ( auto cCbc : pFe->fCbcVector )
    {
        for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
        {
            if ( pEvent->DataBit ( cCbc->getFeId(), cCbc->getCbcId(), cId ) )
            {
                cTmpHist->Fill ( pParameter );
                cHitCounter++;
            }
        }
    }

    return cHitCounter;
}

int Commissioning::countStubs ( Module* pFe,  const Event* pEvent, std::string pHistName, uint8_t pParameter )
{
    // loop over Modules & Cbcs and count hits separately
    int cStubCounter = 0;

    //  get histogram to fill
    TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( pFe, pHistName ) );

    for ( auto cCbc : pFe->fCbcVector )
    {
        if ( pEvent->StubBit ( cCbc->getFeId(), cCbc->getCbcId() ) )
        {
            cTmpHist->Fill ( pParameter );
            cStubCounter++;
        }
    }

    return cStubCounter;
}

void Commissioning::updateHists ( std::string pHistName, bool pFinal )
{
    for ( auto& cCanvas : fCanvasMap )
    {
        // maybe need to declare temporary pointers outside the if condition?
        if ( pHistName == "module_latency" )
        {
            cCanvas.second->cd();
            TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->DrawCopy ( );
            cCanvas.second->Update();

        }
        else if ( pHistName == "module_stub_latency" )
        {
            cCanvas.second->cd();
            TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->DrawCopy ( );
            cCanvas.second->Update();
        }

#ifdef __HTTP__
        fHttpServer->ProcessRequests();
#endif
    }
}


void Commissioning::parseSettings()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );

    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 2000;

    cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )  fHoleMode = cSetting->second;
    else fHoleMode = 1;


    std::cout << "Parsed the following settings:" << std::endl;
    std::cout << "	Nevents = " << fNevents << std::endl;
    std::cout << "	HoleMode = " << int ( fHoleMode ) << std::endl;
}

