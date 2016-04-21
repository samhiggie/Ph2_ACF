#include "Commissioning.h"

void Commissioning::Initialize (uint32_t pStartLatency, uint32_t pLatencyRange)
{
    // gStyle->SetOptStat( 000000 );
    // gStyle->SetTitleOffset( 1.3, "Y" );
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

            TH1F* cLatHist = new TH1F ( cName, Form ( "Latency FE%d; Latency; # of Hits", cFeId ), (pLatencyRange + 1) * fTDCBins, pStartLatency - .5, ( (pStartLatency + pLatencyRange) + .5) * fTDCBins );
            //Modify the axis ticks
            //pLatencyRange main divisions and 8 sub divisions per main division
            cLatHist->SetNdivisions (pLatencyRange + 100 * fTDCBins, "X");
            //and the labels
            for(uint32_t cBin = 0; cBin < cLatHist->GetNbinsX(); cBin++)
            {
                if(cBin % fTDCBins == 0) cLatHist->GetXaxis()->SetBinLabel(cBin, std::to_string(cBin/fTDCBins).c_str());
            }

            cLatHist->SetFillColor ( 4 );
            cLatHist->SetFillStyle ( 3001 );
            bookHistogram ( cFe, "module_latency", cLatHist );

            cName =  Form ( "h_module_stub_latency_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH1F* cStubHist = new TH1F ( cName, Form ( "Stub Lateny FE%d; Stub Lateny; # of Stubs", cFeId ), pLatencyRange + 1, pStartLatency - 0.5, pStartLatency + pLatencyRange + .5 );
            cStubHist->SetMarkerStyle ( 2 );
            bookHistogram ( cFe, "module_stub_latency", cStubHist );

            cName =  Form ( "h_module_threshold_ext_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH1F* cThresHist_ext = new TH1F ( cName, Form ( "Threshold FE%d w external trg; Vcth; # of Hits", cFeId ), 256, -0.5, 255.5 );
            cThresHist_ext->SetMarkerStyle ( 2 );
            bookHistogram ( cFe, "module_threshold_ext", cThresHist_ext );

            cName =  Form ( "h_module_threshold_int_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH1F* cThresHist_int = new TH1F ( cName, Form ( "Threshold FE%d w internal trg; Vcth; # of Hits", cFeId ), 256, -0.5, 255.5 );
            cThresHist_int->SetMarkerStyle ( 2 );
            cThresHist_int->SetMarkerColor ( 2 );
            bookHistogram ( cFe, "module_threshold_int", cThresHist_int );

            cName =  Form ( "f_module_threshold_Fit_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TF1* cThresFit = new TF1 ( cName, MyErf, 0, 255, 2 );
            bookHistogram ( cFe, "module_fit", cThresFit );

            cName =  Form ( "h_module_lat_threshold_Fe%d", cFeId );
            cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            TH2F* cThresLatHist = new TH2F ( cName, Form ( " Threshold/Latency FE%d; Latency; Threshold; # of Hits", cFeId ), 9, -4.5, 4.5, 256, -0.5, 255.5 );
            bookHistogram ( cFe, "module_lat_threshold", cThresLatHist );
        }
    }

    parseSettings();
    // initializeHists();

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
                cNHits += countHitsLat ( cFe, events, "module_latency", cLat, pStrasbourgFW );

                std::cout << "FE: " << +cFe->getFeId() << " Latency " << +cLat << " Hits " << cNHits  << " Events " << fNevents << std::endl;
            }
        }

        // done counting hits for all FE's, now update the Histograms
        updateHists ( "module_latency", false );
    }


    // analyze the Histograms
    std::map<Module*, uint8_t> cLatencyMap;

    std::cout << "Identified the Latency with the maximum number of Hits at: " << std::endl;

    for ( auto cFe : fModuleHistMap )
    {
        TH1F* cTmpHist = ( TH1F* ) getHist ( static_cast<Ph2_HwDescription::Module*> ( cFe.first ), "module_latency" );
        //the true latency now is the floor(iBin/8)
        uint8_t cLatency =  static_cast<uint8_t> ( floor ( (cTmpHist->GetMaximumBin() - 1 ) / 8) );
        cLatencyMap[cFe.first] = cLatency;
        cWriter.setRegister ( "TriggerLatency", cLatency );
        this->accept ( cWriter );

        std::cout << "	FE " << +cFe.first->getModuleId()  << ": " << +cLatency << " clock cycles!" << std::endl;
    }

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


void Commissioning::ScanThreshold ( bool pScanPedestal )
{
    //method to scan thresholds with actual particles - this will not stop but continue through the whole Vcth range
    std::cout << "Scanning the Threshold ... " ;

    if ( pScanPedestal ) std::cout << "including pedestals!";

    std::cout << std::endl;

    std::cout << "with external triggers ... " << std::endl;

    BeBoardRegWriter cWriter_ext ( fBeBoardInterface, "pc_commands.TRIGGER_SEL", 1 );
    this->accept ( cWriter_ext );
    measureScurve ( "module_threshold_ext", fNevents );

    if ( pScanPedestal )
    {

        std::cout << "and with internal triggers ... turn off particles and press Enter!" << std::endl;
        mypause();

        BeBoardRegWriter cWriter_int ( fBeBoardInterface, "pc_commands.TRIGGER_SEL", 0 );
        this->accept ( cWriter_int );
        measureScurve ( "module_threshold_int", fNevents );
    }

    std::cout << "Done scanning threshold!" << std::endl;

    // analyze
    for ( auto cFe : fModuleHistMap )
    {
        // find the apropriate canvas
        auto cCanvas = fCanvasMap.find ( cFe.first );

        if ( cCanvas == fCanvasMap.end() ) std::cout << "ERROR: Courld not find the right canvas!" << std::endl;
        else cCanvas->second->cd();

        // get the SCurve with internal & external trigger
        TH1F* cTmpHist_ext = dynamic_cast<TH1F*> ( getHist ( cFe.first, "module_threshold_ext" ) );

        if ( pScanPedestal ) TH1F* cTmpHist_int = dynamic_cast<TH1F*> ( getHist ( cFe.first, "module_threshold_int" ) );

        // subtract


        // cLatencyMap[cFe.first] = static_cast<uint8_t>( cTmpHist->GetMaxBin() );

    }
}


//////////////////////////////////////          PRIVATE METHODS             //////////////////////////////////////


int Commissioning::countHitsLat ( Module* pFe,  const std::vector<Event*> pEventVec, std::string pHistName, uint8_t pParameter, bool pStrasbourgGlib)
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
        //if this is a GLIB with Strasbourg FW, the TDC values are always between 5 and 12 which means that I have to subtract 4 from the TDC value to have it normalized between 1 and 8
        //if (pStrasbourgGlib) cTDCVal -= 4;

        //std::cout << "Latency " << +pParameter << " TDC Value (normalized) " << +cTDCVal << " NHits: " << cHitCounter << std::endl;
        cTmpHist->Fill ( (fTDCBins * pParameter) + cTDCVal, cHitCounter);
        cHitSum += cHitCounter;
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
        cCanvas.second->cd();

        // maybe need to declare temporary pointers outside the if condition?
        if ( pHistName == "module_latency" )
        {
            TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->Draw ( "same" );

            //TGaxis* axis2;
            //if (axis2 == nullptr)
            //{
            //axis2 = new TGaxis (cCanvas.second->GetUxmin(),
            //cCanvas.second->GetUymax(),
            //cCanvas.second->GetUxmax(),
            //cCanvas.second->GetUymax(), cTmpHist->GetMinimumBin(), cTmpHist->GetMaximumBin(), cTmpHist->GetNbinsX() / 8, "+"); //or better line below
            //// canv->GetUymax(),0.,10.,510,"-");

            //axis2->Draw ("same");
            //}
        }
        else if ( pHistName == "module_stub_latency" )
        {
            TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->Draw ( "same" );
        }
        else if ( pHistName == "module_threshold_int" || pHistName == "module_threshold_ext" )
        {
            TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->Draw ( "P same" );

            if ( pFinal )
            {
                // cTmpHist->Scale( double( 1 / ( NCHANNELS * fNCbc * fNevents ) ) );
                cTmpHist->Draw ( "P same" );
                // get the fit and draw that too
                // TF1* cFit = ( TF1* )getHist( cCanvas.first, "module_fit" );

                // // TODO: figure something smart out to restrict the range!

                // // Estimate parameters for the Fit
                // double cFirstNon0( 0 );
                // double cFirst1( 0 );

                // // Not Hole Mode
                // if ( !fHoleMode )
                // {
                //  for ( Int_t cBin = 1; cBin <= cTmpHist->GetNbinsX(); cBin++ )
                //  {
                //      double cContent = cTmpHist->GetBinContent( cBin );
                //      if ( !cFirstNon0 )
                //      {
                //          if ( cContent ) cFirstNon0 = cTmpHist->GetBinCenter( cBin );
                //      }
                //      else if ( cContent == 1 )
                //      {
                //          cFirst1 = cTmpHist->GetBinCenter( cBin );
                //          break;
                //      }
                //  }
                // }
                // // Hole mode
                // else
                // {
                //  for ( Int_t cBin = cTmpHist->GetNbinsX(); cBin >= 1; cBin-- )
                //  {
                //      double cContent = cTmpHist->GetBinContent( cBin );
                //      if ( !cFirstNon0 )
                //      {
                //          if ( cContent ) cFirstNon0 = cTmpHist->GetBinCenter( cBin );
                //      }
                //      else if ( cContent == 1 )
                //      {
                //          cFirst1 = cTmpHist->GetBinCenter( cBin );
                //          break;
                //      }
                //  }
                // }

                // // Get rough midpoint & width
                // double cMid = ( cFirst1 + cFirstNon0 ) * 0.5;
                // double cWidth = ( cFirst1 - cFirstNon0 ) * 0.5;

                // cFit->SetParameter( 0, cMid );
                // cFit->SetParameter( 1, cWidth );

                // cTmpHist->Fit( cFit, "RNQ+" );
                // cFit->Draw( "same" );

            }
        }

        // else if ( pHistName == "module_lat_threshold" )
        // {
        //  TH2F* cTmpHist = ( TH2F* )getHist( cCanvas.first, pHistName );
        //  cTmpHist->Draw( "box" );
        // }
        cCanvas.second->Update();
#ifdef __HTTP__
        fHttpServer->ProcessRequests();
#endif
    }
}

void Commissioning::measureScurve ( std::string pHistName, uint32_t pNEvents )
{
    // Necessary variables
    // uint32_t cEventsperVcth = 50;
    bool cNonZero = false;
    // bool cAllOne = false;
    bool cSlopeZero = false;
    // uint32_t cAllOneCounter = 0;
    uint32_t cSlopeZeroCounter = 0;
    uint32_t cOldHitCounter = 0;
    uint8_t  cDoubleVcth;
    int cVcth = ( fHoleMode ) ?  0xFF : 0x00;
    int cStep = ( fHoleMode ) ? -10 : 10;



    // Adaptive VCth loop
    while ( 0x00 <= cVcth && cVcth <= 0xFF )
    {
        // if ( cSlopeZero && (cVcth == 0x00 || cVcth = 0xFF) ) break;
        if ( cVcth == cDoubleVcth )
        {
            cVcth +=  cStep;
            continue;
        }

        // Set current Vcth value on all Cbc's
        CbcRegWriter cWriter ( fCbcInterface, "VCth", static_cast<uint8_t> ( cVcth ) );
        accept ( cWriter );

        uint32_t cN = 1;
        uint32_t cNthAcq = 0;
        uint32_t cHitCounter = 0;

        // maybe restrict to pBoard? instead of looping?
        // if ( cSlopeZero && (cVcth == 0x00 || cVcth = 0xFF) ) break;
        for ( BeBoard* pBoard : fBoardVector )
        {

            //fBeBoardInterface->Start ( pBoard );

            //while ( cN <=  pNEvents )
            //{
            fBeBoardInterface->ReadNEvents ( pBoard, pNEvents );

            const std::vector<Event*>& events = fBeBoardInterface->GetEvents ( pBoard );

            // Loop over Events from this Acquisition
            for ( auto& cEvent : events )
            {

                for ( auto cFe : pBoard->fModuleVector )
                    cHitCounter += countHits ( cFe, cEvent, pHistName, static_cast<uint8_t> ( cVcth ) );

                cN++;

            }

            cNthAcq++;
            //}

            //fBeBoardInterface->Stop ( pBoard );

            std::cout << "Threshold " << +cVcth << " Hits " << cHitCounter << " Events " << cN << std::endl;
            // now update the Histograms
            updateHists ( pHistName, false );

            // check if the hitcounter is all ones

            if ( cNonZero == false && cHitCounter > pNEvents / 10 )
            {
                cDoubleVcth = cVcth;
                cNonZero = true;
                cVcth -= 2 * cStep;
                cStep /= 10;
                continue;
            }

            if ( cNonZero && cHitCounter > NCHANNELS * fNCbc * pNEvents * 0.95 )
            {
                // check if all Cbcs have reached full occupancy
                // if ( cHitCounter > 0.95 * pNEvents * fNCbc * NCHANNELS ) cAllOneCounter++;
                // if the number of hits does not change any more,  increase stepsize by a factor of 10
                if ( fabs ( cHitCounter - cOldHitCounter ) < 10 && cHitCounter != 0 ) cSlopeZeroCounter++;
            }

            // if ( cSlopeZeroCounter >= 10 ) cAllOne = true;
            if ( cSlopeZeroCounter >= 10 ) cSlopeZero = true;

            // if ( cAllOne )
            // {
            //  std::cout << "All strips firing -- ending the scan at VCth " << +cVcth << std::endl;
            //  break;
            // }
            if ( cSlopeZero )
                cStep *= 10;

            cOldHitCounter = cHitCounter;
            cVcth += cStep;
        }
    }

    // finished scanning the comparator threshold range
    // need to see what range to fit and what threshold to extract automatically!!
    updateHists ( pHistName, true );
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
    // std::cout << "   InitialThreshold = " << int( fInitialThreshold ) << std::endl;

}


void Commissioning::dumpConfigFiles()
{
    // visitor to call dumpRegFile on each Cbc
    struct RegMapDumper : public HwDescriptionVisitor
    {
        std::string fDirectoryName;
        RegMapDumper ( std::string pDirectoryName ) : fDirectoryName ( pDirectoryName ) {};
        void visit ( Cbc& pCbc )
        {
            if ( !fDirectoryName.empty() )
            {
                TString cFilename = fDirectoryName + Form ( "/FE%dCBC%d.txt", pCbc.getFeId(), pCbc.getCbcId() );
                // cFilename += Form( "/FE%dCBC%d.txt", pCbc.getFeId(), pCbc.getCbcId() );
                pCbc.saveRegMap ( cFilename.Data() );
            }
            else std::cout << "Error: no results Directory initialized! "  << std::endl;
        }
    };

    RegMapDumper cDumper ( fDirectoryName );
    accept ( cDumper );

    std::cout << BOLDBLUE << "Configfiles for all Cbcs written to " << fDirectoryName << RESET << std::endl;
}
