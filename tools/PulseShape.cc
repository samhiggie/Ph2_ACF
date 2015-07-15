#include "PulseShape.h"


void PulseShape::Initialize()
{
	// gStyle->SetOptStat( 000000 );
	// gStyle->SetTitleOffset( 1.3, "Y" );
	std::cerr << "void PulseShape::Initialize()"  << std::endl;
	for ( auto& cShelve : fShelveVector )
	{
		uint32_t cShelveId = cShelve->getShelveId();
		std::cerr << "cShelveId = " << cShelveId << std::endl;

		for ( auto& cBoard : cShelve->fBoardVector )
		{
			uint32_t cBoardId = cBoard->getBeId();
			std::cerr << "cBoardId = " << cBoardId << std::endl;

			for ( auto& cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();
				std::cerr << "cFeId = " << cFeId << std::endl;
				// fNCbc = cFe->getNCbc();

				for ( auto& cCbc : cFe->fCbcVector )
				{
					uint32_t cCbcId = cCbc->getCbcId();
					std::cerr << "cCbcId = " << cCbcId << std::endl;

					// Create the Canvas to draw
					TCanvas* ctmpCanvas = new TCanvas( Form( "c_online_canvas_fe%dcbc%d", cFeId, cCbcId ), Form( "FE%dCBC%d  Online Canvas", cFeId, cCbcId ) );
					ctmpCanvas->Divide( 2, 1 );
					fCanvasMap[cCbc] = ctmpCanvas;
					std::cerr << "Initializing map fCanvasMap[" << Form( "0x%x", cCbc ) << "] = " << Form( "0x%x", ctmpCanvas ) << std::endl;

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

std::map<Cbc*, std::pair<uint8_t, uint8_t>> PulseShape::ScanTestPulseDelay( uint8_t pVcth )
{

	CbcRegWriter cWriter( fCbcInterface, "VCth", pVcth );
	this->accept( cWriter );

	// initialize the historgram for the channel map
	for ( auto& cChannel : fChannelMap ) cChannel.second->initializeHist( pVcth, "VCth" );

	for ( uint32_t cTestPulseDelay = 0 ; cTestPulseDelay < 350; cTestPulseDelay += 10 )
	{

		// set test pulse delay: not sure yet if beBoard register or CbcRegister
		// BeBoardRegWriter cBeBoardWriter(fBeBoardInterface, "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE", cTestPulseDelay);
		// this->accept( cBeBoardWriter);
		// setDelayAndTesGroup( cTestPulseDelay, fTestGroup );

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
				setDelayAndTesGroup( pBoard, cTestPulseDelay, fTestGroup );
				while ( cN <= fNevents )
				{
					cN += fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
					const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );
					// everything from here will got into fillHistograms method
					cNHits += fillDelayHist( pBoard, events, cTestPulseDelay );
					cNthAcq++;
				}
				fBeBoardInterface->Stop( pBoard, cNthAcq );
				std::cout << "Delay " << +cTestPulseDelay << " Hits " << cNHits  << " Events " << cN << std::endl;

			}
		}

		// done counting hits for all FE's, now update the Histograms
		updateHists( "", false );
	}

	std::map<Cbc*, std::pair<uint8_t, uint8_t>> cFWHMpoints;

	uint8_t cMin;
	uint8_t cMax;

	for ( auto& cChannel : fChannelMap )
	{

		cChannel.second->fScurve->Scale( 1 / double( fNevents ) );
		cMin = cChannel.second->fScurve->GetBinCenter( cChannel.second->fScurve->FindFirstBinAbove( 0.5 ) );
		cMax = cChannel.second->fScurve->GetBinCenter( cChannel.second->fScurve->FindLastBinAbove( 0.5 ) );
		std::pair<uint8_t, uint8_t> cPair( cMin, cMax );
		cFWHMpoints[cChannel.first] = cPair;

		// here we have measured the complete curve, now we need to extract the FWHM points
		// iterate over the channel map, get the histogram of each channel, normailze it to 1 (->Scale (1/double(fNevents)))
		// insert the points in a map <cbc, pair> and return
	}
	return cFWHMpoints;
}

void PulseShape::printScanTestPulseDelay( uint8_t pStepSize )
{

	std::map<Cbc*, std::pair<uint8_t, uint8_t>> cCollectedPoints;
	setSystemTestPulse( fTPAmplitude, 9 ); // we look at channel 9
	uint8_t cVcth = ( fHoleMode ) ?  0xFF :  0x00;
	int cStep = ( fHoleMode ) ? -10 : 10;

	// Adaptive VCth loop
	while ( 0x00 <= cVcth && cVcth <= 0xFF )
	{
		std::cout << "Threshold " << +cVcth << " : " << std::endl;
		//add the channel id
		cCollectedPoints = ScanTestPulseDelay( cVcth );
		for ( auto& cPoint : cCollectedPoints )
		{
			TGraph* cTmpGraph = static_cast<TGraph*>( getHist( cPoint.first, "cbc_pulseshape" ) );
			cTmpGraph->SetPoint( cTmpGraph->GetN(), cPoint.second.first, cVcth );
			cTmpGraph->SetPoint( cTmpGraph->GetN(), cPoint.second.second, cVcth );
		}
		updateHists( "cbc_pulseshape", false );
		cVcth += cStep;
	}
}




//////////////////////////////////////		PRIVATE METHODS		/////////////////////////////////////////////

//convert in uint 32 reprasanting 5 bit delay concat with 3 bit test group
void PulseShape::enableChannel( uint8_t pChannelId )
{
	uint8_t cOffset = 0xFF;
	std::vector<std::pair<std::string, uint8_t>> cRegVec;
	std::string cReg;


	if ( fHoleMode )
		cOffset = 0;

	for ( uint8_t cChannelId = 1; cChannelId <= 254; cChannelId++ )
	{
		cReg = Form( "Channel%03d", cChannelId );
		//std::cout << cReg << std::endl;
		if ( cChannelId != pChannelId + 1 )
			cRegVec.push_back( std::make_pair( cReg, cOffset ) );
	}
	CbcMultiRegWriter cWriter( fCbcInterface, cRegVec );
	this->accept( cWriter );
	CbcRegReader cReader( fCbcInterface, "Channel000" );
	this->accept( cReader );
	cReader.setRegister( "Channel009" );
	this->accept( cReader );


}



void PulseShape::setDelayAndTesGroup( BeBoard* pBoard, uint8_t pDelay, uint8_t pTestGroup )
{

	// uint8_t cFineDelay = pDelay % 25;
	// uint8_t cCoarseDelay = pDelay / 25; // Trigger Latency

	// std::string cBitSetFineDelay = std::bitset<5>( cFineDelay ).to_string();
	// std::string cBitSetTestGroup = std::bitset<3>( fTestGroup ).to_string();
	// std::string cResult = std::bitset<8> ( cBitSetFineDelay + cBitSetTestGroup ).to_string();
	// // std::cout << "Total Delay of " << +pDelay << " ns  Trigger Latency = " << +cCoarseDelay << "  Fine Delay = " << +cFineDelay << " ns" << std::endl;

	// std::vector<std::pair<std::string, uint8_t> > cRegVec;
	// cRegVec.push_back( std::make_pair( "SelTestPulseDel&ChanGroup", atoi( cResult.c_str() ) ) );
	// cRegVec.push_back( std::make_pair( "TriggerLatency", cCoarseDelay ) );
	// CbcMultiRegWriter cWriter( fCbcInterface, cRegVec );
	// this->accept( cWriter );

	uint8_t cFineDelay = pDelay % 25;
	uint8_t cCoarseDelay = ( pDelay + cFineDelay - 24 ) / 25;

	std::string cBitSetFineDelay = std::bitset<5>( cFineDelay ).to_string();
	std::string cBitSetTestGroup = std::bitset<3>( fTestGroup ).to_string();
	std::string cResult = std::bitset<8> ( cBitSetFineDelay + cBitSetTestGroup ).to_string();
	CbcRegWriter cWriter( fCbcInterface, "SelTestPulseDel&ChanGroup", atoi( cResult.c_str() ) );
	this->accept( cWriter );

	const char* cTriggerDelayString = "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE";
	uint8_t cBaseDelay = fBeBoardInterface->ReadBoardReg( pBoard, cTriggerDelayString );
	uint32_t cVal = ( uint32_t )( cBaseDelay + cCoarseDelay );
	std::cout << "cBaseDelay: " << +cBaseDelay << std::endl;
	std::cout << "cCoarseDelay: " << +cCoarseDelay << std::endl;
	std::cout << "cVal: " << cVal << std::endl;
	BeBoardRegWriter cBeBoardWriter( fBeBoardInterface, cTriggerDelayString, cVal );
	this->accept( cBeBoardWriter );



}



uint32_t PulseShape::fillDelayHist( BeBoard* pBoard, std::vector<Event*> pEventVector, uint32_t pTPDelay )
{
	uint32_t cHits = 0;
	// Loop over Events from this Acquisition
	for ( auto& cEvent : pEventVector )
	{
		for ( auto cFe : pBoard->fModuleVector )
		{
			for ( auto cCbc : cFe->fCbcVector )
			{
				//  get histogram to fill

				auto cChannel = fChannelMap.find( cCbc );
				if ( cChannel == std::end( fChannelMap ) ) std::cout << "Error, no channel mapped to this CBC ( " << +cCbc->getCbcId() << " )" << std::endl;
				else
				{
					if ( cEvent->DataBit( cFe->getFeId(), cCbc->getCbcId(), cChannel->second->fChannelId ) )
					{
						// if the channel is hit, fill the histogram
						// cChannel->second->fillHist( pTPDelay );
						cChannel->second->fScurve->Fill( pTPDelay );
						cHits++;
					}
				}
			}
		}
	}
	return cHits;
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
	cSetting = fSettingsMap.find( "TPAmplitude" );
	if ( cSetting != std::end( fSettingsMap ) ) fTPAmplitude = cSetting->second;
	else fTPAmplitude = 0x78;


	std::cout << "Parsed the following settings:" << std::endl;
	std::cout << "	Nevents = " << fNevents << std::endl;
	std::cout << "	HoleMode = " << int( fHoleMode ) << std::endl;
	std::cout << "	TPAmplitude = " << int( fTPAmplitude ) << std::endl;

}

void PulseShape::setSystemTestPulse( uint8_t pTPAmplitude, uint8_t pChannelId )
{
	// translate the channel id to a test group
	// TestGroup cTestGroup = new TestGroup()
	this->fTestGroup = pChannelId % 8;

	// set the TestPulsePot register on the CBC to the correct amplituede
	CbcRegWriter cWriter( fCbcInterface, "TestPulsePot", pTPAmplitude );
	this->accept( cWriter );

	std::cout << "Channel Id " << +pChannelId << " TestGroup " << +fTestGroup << std::endl;

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
					Channel* cChannel = new Channel( cBoardId, cFeId, cCbcId, pChannelId );
					fChannelMap[cCbc] = cChannel;

					std::cout << "Settung/updating map fChannelMap[" << Form( "0x%x", cCbc ) << "] = " << Form( "0x%x", cChannel ) << std::endl;
				}
				enableChannel( pChannelId );
			}
		}
	}

}

void PulseShape::updateHists( std::string pHistName, bool pFinal )
{
	for ( auto& cCanvas : fCanvasMap )
	{
		cCanvas.second->cd();
		if ( pHistName == "" )
		{
			// now iterate over the channels in the channel map and draw
			auto cChannel = fChannelMap.find( static_cast<Ph2_HwDescription::Cbc*>( cCanvas.first ) );
			if ( cChannel == std::end( fChannelMap ) ) std::cout << "Error, no channel mapped to this CBC ( " << +cCanvas.first << " )" << std::endl;
			else
			{
				cCanvas.second->cd( 1 );
				cChannel->second->fScurve->Draw( "P0" );
			}
		}
		// maybe need to declare temporary pointers outside the if condition?
		else if ( pHistName == "cbc_pulseshape" )
		{
			cCanvas.second->cd( 2 );
			TGraph* cTmpGraph = dynamic_cast<TGraph*>( getHist( static_cast<Ph2_HwDescription::Cbc*>( cCanvas.first ), pHistName ) );
			cTmpGraph->Draw( "same" );
		}

		cCanvas.second->Update();
	}
}
