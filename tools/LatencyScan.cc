#include "LatencyScan.h"

LatencyScan::LatencyScan() : Tool()
{}

LatencyScan::~LatencyScan() {}

void LatencyScan::Initialize (uint32_t pStartLatency, uint32_t pLatencyRange, bool pNoTdc)
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

            TH1F* cLatHist = nullptr;
            if (!pNoTdc)
            {
                cLatHist = new TH1F ( cName, Form ( "Latency FE%d; Latency; # of Hits", cFeId ), (pLatencyRange ) * fTDCBins, pStartLatency,  pStartLatency + (pLatencyRange )  * fTDCBins );
                //modify the axis labels
                uint32_t pLabel = pStartLatency;

                for (uint32_t cLatency = pStartLatency; cLatency < pStartLatency + pLatencyRange; ++cLatency)
                {
                    for (uint32_t cPhase = 0; cPhase < fTDCBins; ++cPhase)
                    {
                        int cBin = 1 + cLatHist->GetXaxis()->FindBin( convertLatencyPhase (pStartLatency, cLatency, cPhase) );
                        if( cBin == 0 )
                            LOG(INFO) << BOLDRED << "!!!! " << +cLatency << " [latency], " << pStartLatency << " [start latency] " << +cPhase << " [TDC phase] : converted latency " << convertLatencyPhase (pStartLatency, cLatency, cPhase) <<  RESET ; 
                        cLatHist->GetXaxis()->SetBinLabel (cBin, Form ("%d+%d", cLatency, cPhase) );
                    }
                }
            }
            else
                cLatHist = new TH1F ( cName, Form ( "Latency FE%d; Latency; # of Hits", cFeId ), (pLatencyRange ), pStartLatency,  pStartLatency + (pLatencyRange ) );

            cLatHist->GetXaxis()->SetTitle (Form ("Signal timing (reverse time) [TriggerLatency*%d+TDC]", fTDCBins) );
            cLatHist->SetFillColor ( 4 );
            cLatHist->SetFillStyle ( 3001 );
            bookHistogram ( cFe, "module_latency", cLatHist );

            cName =  Form ( "h_module_stub_latency_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH1F* cStubHist = new TH1F ( cName, Form ( "Stub Lateny FE%d; Stub Lateny; # of Stubs", cFeId ), pLatencyRange, pStartLatency, pStartLatency + pLatencyRange);
            cStubHist->SetMarkerStyle ( 2 );
            bookHistogram ( cFe, "module_stub_latency", cStubHist );

            cName =  Form ( "h_module_latency_tdc_2D_Fe%d", cFeId );
            TH2I* cLatHist2D =  new TH2I ( cName, Form ( "Latency FE%d; Latency; # of Hits", cFeId ), pLatencyRange  , pStartLatency - 0.5 ,  pStartLatency + (pLatencyRange ) - 0.5 , fTDCBins , 0 , fTDCBins );
            bookHistogram ( cFe, "module_latency_tdc_2D", cLatHist2D );
            
        }
    }

    parseSettings();
    LOG (INFO) << "Histograms and Settings initialised." ;
}

std::map<Module*, uint8_t> LatencyScan::ScanLatency ( uint8_t pStartLatency, uint8_t pLatencyRange, bool pNoTdc )
{
    // This is not super clean but should work
    // Take the default VCth which should correspond to the pedestal and add 8 depending on the mode to exclude noise
    // ThresholdVisitor in read mode
    //ThresholdVisitor cThresholdVisitor (fCbcInterface);
    //this->accept (cThresholdVisitor);
    //uint16_t cVcth = cThresholdVisitor.getThreshold();

    //int cVcthStep = ( fHoleMode == 1 ) ? +10 : -10;
    //LOG (INFO) << "VCth value from config file is: " << +cVcth << " ;  changing by " << cVcthStep << "  to " << + ( cVcth + cVcthStep ) << " supress noise hits for crude latency scan!" ;
    //cVcth += cVcthStep;

    ////  Set that VCth Value on all FEs
    //cThresholdVisitor.setOption ('w');
    //cThresholdVisitor.setThreshold (cVcth);
    //this->accept (cThresholdVisitor);

    // Now the actual scan
    LOG (INFO) << "Scanning Latency ... " ;
    uint32_t cIterationCount = 0;

    LatencyVisitor cVisitor (fCbcInterface, 0);
    //fBeBoardInterface->Start(pBoard);
    //fBeBoardInterface->Pause(pBoard);
    for ( uint16_t cLat = pStartLatency; cLat < pStartLatency + pLatencyRange; cLat++ )
    {
        //  Set a Latency Value on all FEs
        cVisitor.setLatency (  cLat );
        this->accept ( cVisitor );


        // Take Data for all Modules
        for ( BeBoard* pBoard : fBoardVector )
        {
            // I need this to normalize the TDC values I get from the Strasbourg FW
            ReadNEvents ( pBoard, fNevents );
            const std::vector<Event*>& events = GetEvents ( pBoard );

            // Loop over Events from this Acquisition
            countHitsLat ( pBoard, events, "module_latency", cLat, pStartLatency, pNoTdc );
        }

        // done counting hits for all FE's, now update the Histograms
        updateHists ( "module_latency", false );
        cIterationCount++;
    }


    // analyze the Histograms
    std::map<Module*, uint8_t> cLatencyMap;

    //LOG(INFO) << "Identified the Latency with the maximum number of Hits at: " ;

    //for ( auto cFe : fModuleHistMap )
    //{
    //TH1F* cTmpHist = ( TH1F* ) getHist ( static_cast<Ph2_HwDescription::Module*> ( cFe.first ), "module_latency" );
    ////the true latency now is the floor(iBin/8)
    //uint8_t cLatency =  static_cast<uint8_t> ( floor ( (cTmpHist->GetMaximumBin() - 1 ) / 8) );
    //cLatencyMap[cFe.first] = cLatency;
    //cWriter.setRegister ( "TriggerLatency", cLatency );
    //this->accept ( cWriter );

    //LOG(INFO) << "    FE " << +cFe.first->getModuleId()  << ": " << +cLatency << " clock cycles!" ;
    //}
    updateHists ( "module_latency", true );

    return cLatencyMap;
}

std::map<Module*, uint8_t> LatencyScan::ScanStubLatency ( uint8_t pStartLatency, uint8_t pLatencyRange )
{
    // This is not super clean but should work
    // Take the default VCth which should correspond to the pedestal and add 8 depending on the mode to exclude noise
    // ThresholdVisitor in read mode
    //ThresholdVisitor cThresholdVisitor (fCbcInterface);
    //this->accept (cThresholdVisitor);
    //uint16_t cVcth = cThresholdVisitor.getThreshold();

    //int cVcthStep = ( fHoleMode == 1 ) ? +10 : -10;
    //LOG (INFO) << "VCth value from config file is: " << +cVcth << " ;  changing by " << cVcthStep << "  to " << + ( cVcth + cVcthStep ) << " supress noise hits for crude latency scan!" ;
    //cVcth += cVcthStep;

    ////  Set that VCth Value on all FEs
    //cThresholdVisitor.setOption ('w');
    //cThresholdVisitor.setThreshold (cVcth);
    //this->accept (cThresholdVisitor);

    // set test pulse (if needed)
    //setFWTestPulse();
    //setSystemTestPulse ( fTestPulseAmplitude, 0, true, fHoleMode );

    // Now the actual scan
    LOG (INFO) << "Scanning Stub Latency ... " ;

    for ( uint8_t cLat = pStartLatency; cLat < pStartLatency + pLatencyRange; cLat++ )
    {
        uint32_t cN = 0;
        int cNStubs = 0;

        // Take Data for all Modules
        for ( BeBoard* pBoard : fBoardVector )
        {
            //here set the stub latency
            for (auto cReg : getStubLatencyName (pBoard->getBoardType() ) )
                fBeBoardInterface->WriteBoardReg (pBoard, cReg, cLat);

            ReadNEvents ( pBoard, fNevents );
            const std::vector<Event*>& events = GetEvents ( pBoard );

            // if(cN <3 ) LOG(INFO) << *cEvent ;

            // Loop over Events from this Acquisition
            for ( auto& cEvent : events )
            {
                for ( auto cFe : pBoard->fModuleVector )
                    cNStubs += countStubs ( cFe, cEvent, "module_stub_latency", cLat );

                cN++;
            }

            LOG (INFO) << "Stub Latency " << +cLat << " Stubs " << cNStubs  << " Events " << cN ;

        }

        // done counting hits for all FE's, now update the Histograms
        updateHists ( "module_stub_latency", false );
    }

    // analyze the Histograms
    std::map<Module*, uint8_t> cStubLatencyMap;

    LOG (INFO) << "Identified the Latency with the maximum number of Stubs at: " ;

    for ( auto cFe : fModuleHistMap )
    {
        TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( cFe.first, "module_stub_latency" ) );
        uint8_t cStubLatency =  static_cast<uint8_t> ( cTmpHist->GetMaximumBin() - 1 );
        cStubLatencyMap[cFe.first] = cStubLatency;

        //BeBoardRegWriter cLatWriter ( fBeBoardInterface, "", 0 );

        //if ( cFe.first->getFeId() == 0 ) cLatWriter.setRegister ( "cbc_stubdata_latency_adjust_fe1", cStubLatency );
        //else if ( cFe.first->getFeId() == 1 ) cLatWriter.setRegister ( "cbc_stubdata_latency_adjust_fe2", cStubLatency );

        //this->accept ( cLatWriter );

        LOG (INFO) << "Stub Latency FE " << +cFe.first->getModuleId()  << ": " << +cStubLatency << " clock cycles!" ;
    }

    return cStubLatencyMap;
}


//////////////////////////////////////          PRIVATE METHODS             //////////////////////////////////////


int LatencyScan::countHitsLat ( BeBoard* pBoard,  const std::vector<Event*> pEventVec, std::string pHistName, uint16_t pParameter, uint32_t pStartLatency, bool pNoTdc)
{
    BoardType cBoardType = pBoard->getBoardType();
    uint32_t cTotalHits = 0;

    for ( auto cFe : pBoard->fModuleVector )
    {
        uint32_t cHitSum = 0;
        //  get histogram to fill
        TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( cFe, pHistName ) );
        TString cName =  Form ( "h_module_latency_tdc_2D_Fe%d", cFe->getFeId() );
        TH2I* cTmpHist2D = dynamic_cast<TH2I*> ( getHist ( cFe, "module_latency_tdc_2D" ) );
        for (auto& cEvent : pEventVec)
        {
            //first, reset the hit counter - I need separate counters for each event
            int cHitCounter = 0;
            //get TDC value for this particular event
            uint8_t cTDCVal = cEvent->GetTDC();
            //if the TDC value for the GLIB is 4 it belongs to the next clock cycle bin 12
            //this should ensure that TDC value of 4 never happens
            uint8_t cFillVal = pParameter;

            if (cTDCVal == 4 && cBoardType == BoardType::GLIB)
            {
                cFillVal += 1;
                cTDCVal = 12;
            }

            //for Strasbourg FW normalize to sane 3 bit values
            if (cTDCVal != 0 && cBoardType == BoardType::GLIB) cTDCVal -= 5;
            else if (cTDCVal != 0 && cBoardType == BoardType::CTA) cTDCVal -= 3;
            
            if ( !pNoTdc && cBoardType == BoardType::D19C) 
                // this is the case for additional delay of 15 ns on external trigger 
                // expect to change for other values 
                // final fix to be implemented 
                if( cTDCVal == 6 || cTDCVal == 7 ) 
                {
                    cFillVal -= 1;
                }
                // Mykyta's equivalent fix (I'm thick so the one above is easier to understand....)
                //cTDCVal = ( cTDCVal < 6) ? (cTDCVal+2) : (cTDCVal-6);

            if (!pNoTdc && cTDCVal > 8 ) LOG (INFO) << "ERROR, TDC value not within expected range - normalized value is " << +cTDCVal << " - original Value was " << +cEvent->GetTDC() << "; not considering this Event!" <<  std::endl;

            else
            {
                for ( auto cCbc : cFe->fCbcVector )
                {
                    //now loop the channels for this particular event and increment a counter
                    cHitCounter += cEvent->GetNHits (cCbc->getFeId(), cCbc->getCbcId() );
                }

                //now I have the number of hits in this particular event for all CBCs and the TDC value
                uint32_t cBin = 0;

                if (pNoTdc) cBin = pParameter;
                else cBin = convertLatencyPhase (pStartLatency, cFillVal, cTDCVal);
                
                int cHistBin = 1 + cTmpHist->FindBin (cBin);
                float cBinContent = cTmpHist->GetBinContent (cHistBin);
                cBinContent += cHitCounter;
                cTmpHist->SetBinContent (cHistBin, cBinContent);

                //if (cHitCounter != 0 ) std::cout << "Found " << cHitCounter << " Hits in this event!" << std::endl;

                //GA: old, potentially buggy code
                //cTmpHist->Fill (cBin, cHitCounter);

                if( cHitCounter )
                    LOG (DEBUG) << BOLDBLUE << "TDC value of " << +cTDCVal << RESET ; 
                cTmpHist2D->Fill( pParameter , cTDCVal , cHitCounter );

                cHitSum += cHitCounter;
            }
        }

        LOG (INFO) << "FE: " << +cFe->getFeId() << "; Latency " << +pParameter << " clock cycles; Hits " << cHitSum  << "; Events " << fNevents ;
        cTotalHits += cHitSum;
    }

    return cTotalHits;
}

int LatencyScan::countStubs ( Module* pFe,  const Event* pEvent, std::string pHistName, uint8_t pParameter )
{
    // loop over Modules & Cbcs and count hits separately
    int cStubCounter = 0;

    //  get histogram to fill
    TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( pFe, pHistName ) );

    for ( auto cCbc : pFe->fCbcVector )
    {
        if ( pEvent->StubBit ( cCbc->getFeId(), cCbc->getCbcId() ) )
            cStubCounter += pEvent->StubVector ( cCbc->getFeId(), cCbc->getCbcId() ).size();
    }

    int cBin = cTmpHist->FindBin (pParameter);
    float cBinContent = cTmpHist->GetBinContent (cBin);
    cBinContent += cStubCounter;
    cTmpHist->SetBinContent (cBin, cBinContent);

    //if (cStubCounter != 0) std::cout << "Found " << cStubCounter << " Stubs in this event" << std::endl;

    //GA, old, potentially buggy code)
    //cTmpHist->Fill ( pParameter );

    return cStubCounter;
}

void LatencyScan::updateHists ( std::string pHistName, bool pFinal )
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

    }

    this->HttpServerProcess();
}


void LatencyScan::parseSettings()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );

    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 2000;

    cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )  fHoleMode = cSetting->second;
    else fHoleMode = 1;

    //cSetting = fSettingsMap.find ( "TestPulsePotentiometer" );
    //fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0x7F;

    LOG (INFO) << "Parsed the following settings:" ;
    LOG (INFO) << "	Nevents = " << fNevents ;
    LOG (INFO) << "	HoleMode = " << int ( fHoleMode ) ;
    //LOG (INFO) << "   TestPulseAmplitude = " << int ( fTestPulseAmplitude ) ;
}
void LatencyScan::writeObjects()
{
    this->SaveResults();
    // just use auto iterators to write everything to disk
    // this is the old method before Tool class was cool
    fResultFile->cd();

    // Save canvasses too
    //fNoiseCanvas->Write ( fNoiseCanvas->GetName(), TObject::kOverwrite );
    //fPedestalCanvas->Write ( fPedestalCanvas->GetName(), TObject::kOverwrite );
    //fFeSummaryCanvas->Write ( fFeSummaryCanvas->GetName(), TObject::kOverwrite );
    fResultFile->Flush();
}