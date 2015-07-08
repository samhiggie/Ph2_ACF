#include "PulseShape.h"


void PulseShape::Initialize()
{
	// gStyle->SetOptStat( 000000 );
	// gStyle->SetTitleOffset( 1.3, "Y" );
	for ( auto& cShelve : fShelveVector )
	{
		uint32_t cShelveId = cShelve->getShelveId();

		for ( auto& cBoard : cShelve->fBoardVector )
		{
			uint32_t cBoardId = cBoard->getBeId();

			for ( auto& cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();
				// fNCbc = cFe->getNCbc();

				for ( auto& cCbc : cFe->fCbcVector )
				{
					uint32_t cCbcId = cCbc->getCbcId();

					// Create the Canvas to draw
					TCanvas* ctmpCanvas = new TCanvas( Form( "c_online_canvas_fe%dcbc%d", cFeId, cCbcId ), Form( "FE%dCBC%d  Online Canvas", cFeId, cCbcId ) );
					ctmpCanvas->Divide( 2, 1 );
					fCanvasMap[cCbc] = ctmpCanvas;

					//Create graphs for each CBC
					TString cName =  Form( "g_cbc_pulseshape_Fe%dCbc%d", cFeId, cCbcId );
					TObject* cObj = gROOT->FindObject( cName );
					if ( cObj ) delete cObj;
					TGraph* cPulseGraph = new TGraph();
					cPulseGraph->GetXaxis()->SetTitle( "TestPulseDelay [ns]" );
					cPulseGraph->GetYaxis()->SetTitle( "TestPulseAmplitue [VCth]" );

					bookHistogram( cCbc, "cbc_pulseshape", cPulseGraph );
				}

			}
		}
	}

	parseSettings();
	// initializeHists();

	std::cout << "Histograms and Settings initialised." << std::endl;
}

std::map<Cbc*,std::pair<uint8_t, uint8_t>> PulseShape::ScanTestPulseDelay(uint8_t pVcth){

		CbcRegWriter cWriter( fCbcInterface, "VCth", pVcth );
		this->accept( cWriter );		

		for(uint32_t cTestPulseDelay = 0 ; cTestPulseDelay < 256; cTestPulseDelay++)
		{
			
			// set test pulse delay: not sure yet if beBoard register or CbcRegister
			BeBoardRegWriter cBeBoardWriter(fBeBoardInterface, "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE", cTestPulseDelay);
			this->accept( cBeBoardWriter);

			// initialize the historgram for the channel map
			for(auto& cChannel : fChannelMap) cChannel.second->initializeHist(pVcth, "VCth");
			// CbcRegWriter cWriter( fCbcInterface, "", cTestPulseDelay);
			// this->accept( cWriter);

			// then we take fNEvents
			uint32_t cN = 1;
			uint32_t cNthAcq = 0;
			int cNHits = 0;

			// Take Data for all Modules
			for ( auto& cShelve : fShelveVector )
			{
				for ( BeBoard* pBoard : cShelve->fBoardVector )
				{
					fBeBoardInterface->Start( pBoard );

					while ( cN <= fNevents )
					{
						fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
						const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );
						// everything from here will got into fillHistograms method
						cN+=fillDelayHist(pBoard, events, cTestPulseDelay);
						cNthAcq++;
					}
					fBeBoardInterface->Stop( pBoard, cNthAcq );
					std::cout << "Delay " << +cTestPulseDelay << " Hits " << cNHits  << " Events " << cN << std::endl;

				}
			}

			// done counting hits for all FE's, now update the Histograms
			updateHists("", false);
		}

		std::map<Cbc*,std::pair<uint8_t,uint8_t>> cFWHMpoints;

		uint8_t cMin;
		uint8_t cMax;
		
		for (auto& cChannel : fChannelMap) {
      
			cChannel.second->fScurve->Scale (1/double(fNevents));
			cMin = cChannel.second->fScurve->GetBinCenter(cChannel.second->fScurve->FindFirstBinAbove(0.5));
			cMax = cChannel.second->fScurve->GetBinCenter(cChannel.second->fScurve->FindLastBinAbove(0.5));
			std::pair<uint8_t,uint8_t> cPair(cMin,cMax); 
			cFWHMpoints[cChannel.first] = cPair;
 			

			// here we have measured the complete curve, now we need to extract the FWHM points


			// iterate over the channel map, get the histogram of each channel, normailze it to 1 (->Scale (1/double(fNevents)))

			// insert the points in a map <cbc, pair> and return
		}	

		return cFWHMpoints;
}

void PulseShape::printScanTestPulseDelay(uint8_t pStepSize){

		std::map<Cbc*, std::pair<uint8_t, uint8_t>> cCollectedPoints;
		
		for(uint8_t cVcth=0;cVcth<=0xFF; cVcth += pStepSize){		
			//add the channel id
			cCollectedPoints = ScanTestPulseDelay(cVcth);
			for (auto& cPoint : cCollectedPoints) {
				TGraph* cTmpGraph = static_cast<TGraph*>(getHist(cPoint.first, "cbc_pulseshape"));
				cTmpGraph->SetPoint(cTmpGraph->GetN(), cPoint.second.first, cVcth);
				cTmpGraph->SetPoint(cTmpGraph->GetN(), cPoint.second.second, cVcth);

				// (static_cast<TGraph*>(getHist(cPoint.first, "cbc_pulseshape")))->SetPoint((static_cast<TGraph*>(getHist(cPoint.first, "cbc_pulseshape")))->GetN(), cPoint.second.first, cVcth);
				// (static_cast<TGraph*>(getHist(cPoint.first, "cbc_pulseshape")))->SetPoint((static_cast<TGraph*>(getHist(cPoint.first, "cbc_pulseshape")))->GetN(), cPoint.second.second, cVcth);
			}
			updateHists( "cbc_pulseshape", false);
		}
    










}

//////////////////////////////////////		PRIVATE METHODS		/////////////////////////////////////////////
// void PulseShape::updateHists( std::string pHistName, bool pFinal )
// {
// 	for ( auto& cCanvas : fCanvasMap )
// 	{
// 		cCanvas.second->cd();

// 		// maybe need to declare temporary pointers outside the if condition?
// 		if ( pHistName == "module_latency" )
// 		{
// 			TH1F* cTmpHist = dynamic_cast<TH1F*>( getHist( static_cast<Ph2_HwDescription::Module*>(cCanvas.first), pHistName ) );
// 			cTmpHist->Draw( "same" );
// 		}
// 		else if ( pHistName == "module_stub_latency" )
// 		{
// 			TH1F* cTmpHist = dynamic_cast<TH1F*>( getHist( static_cast<Ph2_HwDescription::Module*>(cCanvas.first), pHistName ) );
// 			cTmpHist->Draw( "same" );
// 		}
// 		else if ( pHistName == "module_threshold_int" || pHistName == "module_threshold_ext" )
// 		{
// 			TH1F* cTmpHist = dynamic_cast<TH1F*>( getHist( static_cast<Ph2_HwDescription::Module*>(cCanvas.first), pHistName ) );
// 			cTmpHist->Draw( "P same" );

// 			if ( pFinal )
// 			{
				
// 				cTmpHist->Draw( "P same" );
				

	

// 			}
// 		}
		
// 		cCanvas.second->Update();
// 	}
// }

uint32_t PulseShape::fillDelayHist(BeBoard* pBoard, std::vector<Event*> pEventVector, uint32_t pTPDelay){
	// Loop over Events from this Acquisition
	for ( auto& cEvent : pEventVector )
	{
		for ( auto cFe : pBoard->fModuleVector )
		{
				for (auto cCbc : cFe->fCbcVector)
				{
					//  get histogram to fill
					
					auto cChannel = fChannelMap.find(cCbc);
					if(cChannel == std::end(fChannelMap)) std::cout << "Error, no channel mapped to this CBC ( " << +cCbc->getCbcId() << " )" << std::endl;
					else{
							if (cEvent->DataBit(cFe->getFeId(), cCbc->getCbcId(), cChannel->second->fChannelId)){
							// if the channel is hit, fill the histogram
							cChannel->second->fillHist(pTPDelay);
						}
					}
				}
		}
	}
	return pEventVector.size();
}



void PulseShape::parseSettings()
{
	// now read the settings from the map
	auto cSetting = fSettingsMap.find( "Nevents" );
	if ( cSetting != std::end( fSettingsMap ) ) fNevents = cSetting->second;
	else fNevents = 2000;
	cSetting = fSettingsMap.find( "HoleMode" );
	if ( cSetting != std::end( fSettingsMap ) )  fHoleMode = cSetting->second;
	else fHoleMode = 1;


	std::cout << "Parsed the following settings:" << std::endl;
	std::cout << "	Nevents = " << fNevents << std::endl;
	std::cout << "	HoleMode = " << int( fHoleMode ) << std::endl;
	// std::cout << "   InitialThreshold = " << int( fInitialThreshold ) << std::endl;

}

void PulseShape::setSystemTestPulse(uint8_t pTPAmplitude, uint8_t pChannelId)
{
	// translate the channel id to a test group
	// TestGroup cTestGroup = new TestGroup()

	// set the TestPulsePot register on the CBC to the correct amplituede
	CbcRegWriter cWriter(fCbcInterface, "TestPulsePot", pTPAmplitude);
	this->accept(cWriter);
	cWriter.setRegister("SelTestPulseDel&ChanGroup", 0xF1 );
	this->accept(cWriter);

	// BeBoardRegWriter cBoardWriter();
	// this->accept(cBoardWriter);

	// initialize the CBC vs Channel map
	// struct initMapVisitor : public HwDescriptionVisitor {
	// 	ChannelMap cChannelMap;
	// 	uint8_t cChannelId;
	// 	initMapVisitor(ChannelMap pChannelMap, uint8_t pChannelId){ 
	// 		cChannelMap=pChannelMap;
	// 		cChannelId=pChannelId;

	// 	}

	// 	void visit(Cbc* pCbc){
	// 		Channel cChannel = new Channel(pCbc->getBeId(), pCbc->getFeId(), pCbc->getCbcId(), cChannelId);
	// 		cChannelMap[pCbc] = cChannel;
	// 	}
	// };

	// initMapVisitor cInitMapVisitor(fChannelMap, pChannelId);
	// this->accept(cInitMapVisitor);

	for ( auto& cShelve : fShelveVector )
	{
		uint32_t cShelveId = cShelve->getShelveId();

		for ( auto& cBoard : cShelve->fBoardVector )
		{
			uint32_t cBoardId = cBoard->getBeId();

			for ( auto& cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();
				// fNCbc = cFe->getNCbc();

				for ( auto& cCbc : cFe->fCbcVector )
				{
					uint32_t cCbcId = cCbc->getCbcId();
					Channel* cChannel = new Channel(cBoardId, cFeId, cCbcId, pChannelId);
					fChannelMap[cCbc] = cChannel;
				}
			}
		}
	}
}

void PulseShape::updateHists( std::string pHistName, bool pFinal )
{
	for ( auto& cCanvas : fCanvasMap )
	{
		cCanvas.second->cd();
		if (pHistName == ""){
		// now iterate over the channels in the channel map and draw
		auto cChannel = fChannelMap.find(static_cast<Ph2_HwDescription::Cbc*>(cCanvas.first));
		if(cChannel == std::end(fChannelMap)) std::cout << "Error, no channel mapped to this CBC ( " << +cChannel->first->getCbcId() << " )" << std::endl;
		else
		{
			cCanvas.second->cd(1);
			cChannel->second->fScurve->Draw("same");
		}
	}
		// maybe need to declare temporary pointers outside the if condition?
		else if ( pHistName == "cbc_pulseshape" )
		{
			cCanvas.second->cd(2);
			TGraph* cTmpGraph = dynamic_cast<TGraph*>( getHist( static_cast<Ph2_HwDescription::Cbc*>(cCanvas.first), pHistName ) );
			cTmpGraph->Draw( "same" );
		}
	}
}