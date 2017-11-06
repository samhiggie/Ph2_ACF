#include "SignalScanFit.h"

void SignalScanFit::Initialize ()
{
    // Initialize all the plots
    for ( auto& cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto& cFe : cBoard->fModuleVector )
        {

            fType = cFe->getChipType();

            if ( fType == ChipType::CBC2 ) {
                fVCthMax = 245.5;
                fVCthMin = -0.5;
                fVCthRange = 255; 
            } else if ( fType == ChipType::CBC3 ) {
                fVCthMax = 1023.5;
                fVCthMin = -0.5;
                fVCthRange = 1024; 
            }

            uint32_t cFeId = cFe->getFeId();
            fNCbc = cFe->getNCbc();
            TCanvas* ctmpCanvas = new TCanvas ( Form ( "c_online_canvas_fe%d", cFeId ), Form ( "FE%d  Online Canvas", cFeId ) ); 
            //ctmpCanvas->Divide( 3, 2 );
            fCanvasMap[cFe] = ctmpCanvas;

            // 1D Hist for threshold scan
            TString cName =  Form ( "h_module_thresholdScan_Fe%d", cFeId );
            TObject* cObj = gROOT->FindObject ( cName );

            if ( cObj ) delete cObj;

            // 2D-plot with all the channels on the x-axix, Vcth on the y-axis and #clusters on the z.
            TH2F* cSignalHist = new TH2F ( cName, Form ( "Signal threshold vs channel ; Channel # ; Threshold; # of Hits", cFeId ), fNCbc * NCHANNELS, -0.5, fNCbc * NCHANNELS - 0.5, fVCthRange, fVCthMin, fVCthMax );
            bookHistogram ( cFe, "module_signal", cSignalHist );
             
            uint32_t cCbcCount = 0;
            uint32_t cCbcIdMax = 0;

	          for ( auto cCbc : cFe->fCbcVector )
            {
                uint32_t cCbcId = cCbc->getCbcId();
                cCbcCount++;

                if ( cCbcId > cCbcIdMax ) cCbcIdMax = cCbcId;

                TString cHistname;
	              TH1F* cHist;
	              TProfile * cProfile;
	              
	              cHistname = Form ( "Fe%dCBC%d_Clusters_even",cFeId, cCbcId );
	              cHist = new TH1F ( cHistname, cHistname, fVCthRange, fVCthMin, fVCthMax ); // binned in Vcth units
	              bookHistogram ( cCbc, "Cbc_Clusters_even", cHist );

	              cHistname = Form ( "Fe%dCBC%d_Clusters_odd",cFeId, cCbcId );
	              cHist = new TH1F ( cHistname, cHistname, fVCthRange, fVCthMin, fVCthMax );
	              bookHistogram ( cCbc, "Cbc_Clusters_odd", cHist );
	              
	              cHistname = Form ( "Fe%dCBC%d_ClusterSize_even",cFeId, cCbcId );
	              cHist = new TH1F ( cHistname, cHistname, 130, -0.5, 129.5 ); 
	              bookHistogram ( cCbc, "Cbc_ClusterSize_even", cHist );
	              
	              cHistname = Form ( "Fe%dCBC%d_ClusterSize_odd",cFeId, cCbcId );
	              cHist = new TH1F ( cHistname, cHistname, 130, -0.5, 129.5 ); 
	              bookHistogram ( cCbc, "Cbc_ClusterSize_odd", cHist );
	              
	              cHistname = Form ( "Fe%dCBC%d_Hits_even",cFeId, cCbcId );
	              cHist = new TH1F ( cHistname, cHistname, fVCthRange, fVCthMin, fVCthMax );
	              bookHistogram ( cCbc, "Cbc_Hits_even", cHist );
	              
	              cHistname = Form ( "Fe%dCBC%d_Hits_odd",cFeId, cCbcId );
	              cHist = new TH1F ( cHistname, cHistname, fVCthRange, fVCthMin, fVCthMax );
	              bookHistogram ( cCbc, "Cbc_Hits_odd", cHist );
	              
	              cHistname = Form ( "Fe%dCBC%d_ClusterOccupancy_even",cFeId, cCbcId );
	              cProfile = new TProfile ( cHistname, cHistname, fVCthRange, fVCthMin, fVCthMax );
	              bookHistogram ( cCbc, "Cbc_ClusterOccupancy_even", cProfile );

	              cHistname = Form ( "Fe%dCBC%d_ClusterOccupancy_odd",cFeId, cCbcId );
	              cProfile = new TProfile ( cHistname, cHistname, fVCthRange, fVCthMin, fVCthMax );
	              bookHistogram ( cCbc, "Cbc_ClusterOccupancy_odd", cProfile );
            }
        }
    }

    // To read the SignalScanFit-specific stuff
    parseSettings();
    LOG (INFO) << "Histograms & Settings initialised." ;
}

void SignalScanFit::ScanSignal (int pSignalScanLength)
{
    // The step scan is +1 for hole mode
    int cVcthDirection = ( fHoleMode == 1 ) ? +1 : -1;

    // Reading the current threshold value
    ThresholdVisitor cVisitor (fCbcInterface);
    this->accept (cVisitor);
    uint16_t cVCth = cVisitor.getThreshold();

    LOG (INFO) << "Programmed VCth value = " << +cVCth << ", the Initial VCth value for the scan = " << fInitialThreshold << " - falling back by " << fStepback ;

    // Setting the threshold to the start position of the scan, ffInitialThreshold which is TargetVCth in settings file
    //cVCth = uint16_t (cVCth - cVcthDirection * fStepback);
    cVCth = uint16_t ( fInitialThreshold ) ;    
    cVisitor.setOption ('w');
    cVisitor.setThreshold (cVCth);
    this->accept (cVisitor);

    for (int i = 0; i < pSignalScanLength; i += fSignalScanStep )
    {
        LOG (INFO) << "Threshold: " << +cVCth << " - Iteration " << i << " - Taking " << fNevents ;

        // Take Data for all Boards
        for ( BeBoard* pBoard : fBoardVector )
        {
	          uint32_t cTotalEvents = 0;
            fBeBoardInterface->Start (pBoard);

            while (cTotalEvents < fNevents)
            {
                try{
	                  ReadData ( pBoard );
                } catch (uhal::exception::exception& e){
                    LOG(ERROR)<< e.what();
                    updateHists ( "module_signal", false );
                    this->SaveResults();    
                    return;
                }

                const std::vector<Event*>& events = GetEvents ( pBoard );
                cTotalEvents += events.size();
	              int cEventHits = 0;
	              int cEventClusters = 0;

                // Loop over the Modules                    
	              for ( auto cFe : pBoard->fModuleVector )
	              {
	                  TH2F* cSignalHist = static_cast<TH2F*> (getHist ( cFe, "module_signal") );
	                
                    // Loop over the CBCs
	                  for ( auto cCbc : cFe->fCbcVector )
                    {
	                      TH1F* cHitsEvenHist      = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Hits_even" ) );
	                      TH1F* cHitsOddHist       = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Hits_odd" ) );
	                      TH1F* cClustersEvenHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Clusters_even" ) );
	                      TH1F* cClustersOddHist   = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Clusters_odd" ) );
                        TProfile* cClustersEvenProf = dynamic_cast<TProfile*> ( getHist ( cCbc, "Cbc_ClusterOccupancy_even" ) );
		                    TProfile* cClustersOddProf  = dynamic_cast<TProfile*> ( getHist ( cCbc, "Cbc_ClusterOccupancy_odd" ) );

                        uint32_t cHitsEven = 0, cHitsOdd = 0, cClustersEven = 0, cClustersOdd = 0;	                      

                        // Loop over Events from this Acquisition
	                      for ( auto& cEvent : events )
	                      {
		                        uint32_t lastEvenHit = -9;
		                        uint32_t lastOddHit = -9;

                            // Loop over the Hits in the Event and do clustering
                            for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
                            {
                                if ( cEvent->DataBit ( cCbc->getFeId(), cCbc->getCbcId(), cId ) ) //crash #2
                                {
                                    if ( ( int (cId) % 2 ) == 0 ) 
                                    {
			                                  cHitsEven++;
			                                  if (cId - lastEvenHit > 2)  cClustersEven++;
			                                  //else clusterSize++;
			                                  lastEvenHit = cId;
                                    }
			                              else 
                                    {
			                                  cHitsOdd++;
			                                  if (cId - lastOddHit > 2)  cClustersOdd++;
			                                  lastOddHit = cId;
                                    }
                                    cSignalHist->Fill (cCbc->getCbcId() * NCHANNELS + cId, cVCth );
                                    cEventHits++;
                                }
                            }
                            
                            // Fill the histograms
                            cHitsEvenHist->SetBinContent( cVCth, cHitsEven + cHitsEvenHist->GetBinContent(cVCth));
		                        cHitsOddHist ->SetBinContent( cVCth, cHitsOdd  + cHitsOddHist ->GetBinContent(cVCth));
		                        cClustersEvenHist->SetBinContent( cVCth, cClustersEven + cClustersEvenHist->GetBinContent(cVCth));
		                        cClustersOddHist ->SetBinContent( cVCth, cClustersOdd  + cClustersOddHist ->GetBinContent(cVCth));
                            
                            // Now fill the TProfile: how many clusters from this acquisition?
                            // Could easily make this a true occupancy by dividing by NChannels and NEventsPerAcquisition
                            cClustersEvenProf->Fill (cVCth, cClustersEven);	
                            cClustersOddProf ->Fill (cVCth, cClustersOdd);	                            

                            std::vector<Cluster> cClusters = cEvent->getClusters (cCbc->getFeId(), cCbc->getCbcId() ); 
                            cEventClusters += cClusters.size();
                        }
                    }
                } 

                LOG (INFO) <<  "Vcth: " << +cVCth << ". Recorded " << cTotalEvents << " Events, with " << cEventClusters << " clusters and " << cEventHits << " hits.";
                updateHists ( "module_signal", false ); // For online display
            }
		       fBeBoardInterface->Stop (pBoard);
        }

        // done counting hits for all FE's, now update the Histograms
        updateHists ( "module_signal", false );
        // now I need to increment the threshold by cVCth+fVcthDirecton*fSignalScanStep
        cVCth += cVcthDirection * fSignalScanStep;
        cVisitor.setOption ('w');
        cVisitor.setThreshold (cVCth);
        this->accept (cVisitor);
    }

    // Now do the fit if requested for all the boards, we're only interested in the clusters per CBC
    if ( fFitted ) {
        for ( BeBoard* pBoard : fBoardVector )
        {
            //processCurves ( pBoard, "Cbc_Clusters_even" );
            //processCurves ( pBoard, "Cbc_Clusters_odd" );    
            processCurves ( pBoard, "Cbc_ClusterOccupancy_even" );
            processCurves ( pBoard, "Cbc_ClusterOccupancy_odd" ); 
        }
    }
  
    // Last but not least, save the results. This also happens in the commissioning.cc but when we only use that some plots do not get saved properly!!! To be checked!
    SaveResults();
}

//////////////////////////////////////          PRIVATE METHODS             //////////////////////////////////////

void SignalScanFit::updateHists ( std::string pHistName, bool pFinal )
{
    for ( auto& cCanvas : fCanvasMap )
    {
        // maybe need to declare temporary pointers outside the if condition?
        if ( pHistName == "module_signal" )
        {
            cCanvas.second->cd();
            TH2F* cTmpHist = dynamic_cast<TH2F*> ( getHist ( static_cast<Ph2_HwDescription::Module*> ( cCanvas.first ), pHistName ) );
            cTmpHist->DrawCopy ( "colz" );
            cCanvas.second->Update();
        }
    }
}

void SignalScanFit::parseSettings ()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "Nevents" );

    if ( cSetting != std::end ( fSettingsMap ) ) fNevents = cSetting->second;
    else fNevents = 2000;

    cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )  fHoleMode = cSetting->second;
    else fHoleMode = 1;

    cSetting = fSettingsMap.find ( "PedestalStepBack" );

    if ( cSetting != std::end ( fSettingsMap ) )  fStepback = cSetting->second;
    else fStepback = 1;

    cSetting = fSettingsMap.find ( "SignalScanStep" );

    if ( cSetting != std::end ( fSettingsMap ) )  fSignalScanStep = cSetting->second;
    else fSignalScanStep = 1;

    cSetting = fSettingsMap.find ( "TargetVcth" );

    if ( cSetting != std::end ( fSettingsMap ) )  fInitialThreshold = cSetting->second;
    else fInitialThreshold = 1;

    cSetting = fSettingsMap.find ( "FitSCurves" ); // In this case we fit the signal which is an s curve with charge sharing (???)

    if ( cSetting != std::end ( fSettingsMap ) ) fFitted = cSetting->second;
    else fFitted = 1;

    // Write a log 
    LOG (INFO) << "Parsed the following settings:" ;
    LOG (INFO) << "	Nevents = " << fNevents ;
    LOG (INFO) << "	InitialThreshold = " << fInitialThreshold ;
    LOG (INFO) << "	HoleMode = " << int ( fHoleMode ) ;
    LOG (INFO) << "	Step back from Pedestal = " << fStepback ;
    LOG (INFO) << "	SignalScanStep = " << fSignalScanStep ;
    LOG (INFO) << "	Fit the scan = " << int ( fFitted ) ;
}

void SignalScanFit::processCurves ( BeBoard *pBoard, std::string pHistName )
{
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cCbc : cFe->fCbcVector )
        {
            // This one is not used yet
            TProfile* cProf = dynamic_cast<TProfile*> ( getHist ( cCbc, pHistName) );

	          TString clusters(pHistName);
	          clusters.ReplaceAll("Occupancy", "s");
            TH1F* cClustersHist = dynamic_cast<TH1F*> ( getHist ( cCbc, clusters.Data()) );
            //cHist->Scale (1 / double_t (fEventsPerPoint) );
            //in order to have proper binomial errors
            //cHist->Divide (cHist, fNormHist, 1, 1, "B");

	          TString hits(clusters);
	          hits.ReplaceAll("Clusters", "Hits");
	          TH1F* cHitsHist = dynamic_cast<TH1F*> ( getHist ( cCbc, hits.Data()) );

            // Make the clusterSize histos
	          TString size(clusters);
	          size.ReplaceAll("Clusters", "ClusterSize"); 
	          TH1F* cClusterSizeHist = dynamic_cast<TH1F*> ( getHist ( cCbc, size.Data()) );
	          for (int i = 1; i <= cClustersHist->GetNbinsX(); i++) 
            {
	              if (cClustersHist->GetBinContent(i)>0) 
		                cClusterSizeHist->SetBinContent(i, cHitsHist->GetBinContent(i) / cClustersHist->GetBinContent(i));
	              else 
		              cClusterSizeHist->SetBinContent(i, 0);
	          }

            // Make the differential histo
            // Do this with the histogram, not the profile
            this->differentiateHist (cCbc, clusters.Data());

            // Only do this if requested? Yes, see SignalScan and fFitted setting!
            if (fFitted) 
            this->fitHist (cCbc, pHistName);
        }
    }
}

void SignalScanFit::differentiateHist ( Cbc* pCbc, std::string pHistName )
{
    TH1F* cHist = dynamic_cast<TH1F*> ( getHist ( pCbc, pHistName) );
    TString cHistname(cHist->GetName());
    cHistname.ReplaceAll("Clusters", "ClustersDiff");
    TH1F* cDerivative = (TH1F*) cHist->Clone(cHistname);
    cDerivative->Sumw2();
    cDerivative->Reset();
    bookHistogram ( pCbc, pHistName + "_Diff", cDerivative );

    double_t cDiff;
    double_t cCurrent;
    double_t cPrev;
    bool cActive; // indicates existence of data points
    int cStep = 1;
    int cDiffCounter = 0;

    double cBin = 0;

    cPrev = cHist->GetBinContent ( cHist->FindBin ( 0 ) );
    cActive = false;

    for ( cBin = cHist->FindBin (fVCthMax); cBin >= cHist->FindBin (fVCthMin); cBin-- ) 
    //for ( cBin = cHist->FindBin (129); cBin >= cHist->FindBin (0); cBin-- ) // Hardcoded Max Vcth, careful...
    {

	      cCurrent = cHist->GetBinContent (cBin);
	      cDiff = cPrev - cCurrent;
	
	      if ( cPrev > 0.75 ) cActive = true; // sampling begins
	
	      int iBinDerivative = cDerivative->FindBin ( (cHist->GetBinCenter (cBin - 1) + cHist->GetBinCenter (cBin) ) / 2.0);
	
	      if ( cActive ) cDerivative->SetBinContent ( iBinDerivative, cDiff  );
	
	      if ( cActive && cDiff == 0 && cCurrent == 0 ) cDiffCounter++;
	
	      if ( cDiffCounter == 8 ) break;
	
	      cPrev = cCurrent;
    }
}

void SignalScanFit::fitHist ( Cbc* pCbc, std::string pHistName )
{

    // This works for CBC2, needs to be verified with CBC3!!! 
    TProfile* cHist = dynamic_cast<TProfile*> ( getHist ( pCbc, pHistName) );
    double cStart  = 0; 
    double cStop = 90; // Fit fails if it reaches the noise
    // Draw fit parameters
    TStyle * cStyle = new TStyle();
    cStyle->SetOptFit(1011);

    std::string cFitname = "CurveFit";
    TF1* cFit = dynamic_cast<TF1*> (gROOT->FindObject (cFitname.c_str() ) );
    if (cFit) delete cFit;
    
    double cPreviousBin = 0;
    uint32_t cBinAboveZero = 0;
    double cDifCurrent = -999;
    double cDifPrevious = -999;

    // Not Hole Mode
    if ( !fHoleMode )
    {
      
        for ( Int_t cBin = 1; cBin < cHist->GetNbinsX() - 1; cBin++ )
	      {
	          double cContent = cHist->GetBinContent ( cBin );
	          double cBinVcth = cHist->GetBinLowEdge ( cBin );

	          // Beginning of the fit range: find 5th filled bin, go back 20 bins
	          if ( cContent > 0 && cStart == 0)
	          {
	              cBinAboveZero++;     
		            cPreviousBin = cContent;              
		            if ( cBinAboveZero > 5 ) 
		            {
		                cStart = cBin - 20;
		            }
	          }
	          // End of the fit range: 10 bins after it has stopped growing
	          // For lower energies we never reach the plateau, so set a maximum of 110 Vcth
	          if ( cStart > 0 && cHist->GetBinCenter(cBin) < 95) 
	          {
	              if ( cDifCurrent != -999 )
		            {
		                cDifPrevious = cDifCurrent;
		            } 
		            cDifCurrent = cContent - cPreviousBin; 
		            if ( cDifPrevious > cDifCurrent )
		            {
		                cStop = cBin + 10;
		            }
	          } 
	      }
	
	      cFit = new TF1 ( "CurveFit", MyGammaSignal, cStart, cStop, 4 ); // MyGammaSignal is in Utils
    }
          

// Hole mode not implemented!
//        else
//        {
//            for ( Int_t cBin = cHist->GetNbinsX() - 1; cBin > 1; cBin-- )
//            {
//                double cContent = cHist->GetBinContent ( cBin );

//                if ( !cFirstNon0 )
//                {
//                    if ( cContent ) cFirstNon0 = cHist->GetBinCenter ( cBin );
//                }
//                else if ( cContent > 0.85 )
//                {
//                    cFirst1 = cHist->GetBinCenter ( cBin );
//                    break;
//                }
//            }

//            cFit = new TF1 (cFitname.c_str(), MyErf, cFirst1 - 10, cFirstNon0 + 10, 2 );
//        }

    // Get rough input parameters
    double cWidth = 10;
    double cNoise = 1; // This is not only the noise of the s-curve!
    double cPlateau = cHist->GetBinContent ( cStop-1 );
    double cVsignal = cHist->GetBinCenter ( cStart+20 );

    std::cout << "Fit parameters: width = " << cWidth << " , 'noise' = " << cNoise << " , plateau = " << " , VcthSignal = " << cVsignal << std::endl; 
    
    cFit->SetParameter ( 0, cPlateau );
    cFit->SetParameter ( 1, cWidth );
    cFit->SetParameter ( 2, cVsignal );
    cFit->SetParameter ( 3, cNoise );

    cFit->SetParName ( 0, "Plateau" );
    cFit->SetParName ( 1, "Width" );
    cFit->SetParName ( 2, "Vsignal" );
    cFit->SetParName ( 3, "Noise" );
        
    // Constraining the width to be positive
    cFit->SetParLimits ( 1, 0, 100 ); 
    
    // Fit
    //cHist->Fit ( cFit, "RQB+" );
    cHist->Fit ( cFit, "RB+" );

    // Would be nice to catch failed fits
    cHist->Write (cHist->GetName(), TObject::kOverwrite);
}
