#include "PulseShape.h"


void PulseShape::Initialize()
{
	fNCbc = 0;
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
					fNCbc++;
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
					cPulseGraph->SetMarkerStyle( 3 );
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




void PulseShape::ScanTestPulseDelay( uint8_t pStepSize )
{
	setSystemTestPulse( fTPAmplitude, fChannel ); // we look at channel 9
	enableChannel( fChannel );

	// initialize the historgram for the channel map
	int cCoarseDefault = 201;
	int cLow = ( cCoarseDefault - 1 ) * 25;
	int cHigh = ( cCoarseDefault + 3 ) * 25;
	std::map<Cbc*, uint8_t> cCollectedPoints;
	for ( uint32_t cTestPulseDelay = cLow ; cTestPulseDelay < cHigh; cTestPulseDelay += fStepSize )
	{
		setDelayAndTesGroup( cTestPulseDelay );
		cCollectedPoints =  ScanVcth( cTestPulseDelay );
		for ( auto& cPoint : cCollectedPoints )
		{
			TGraph* cTmpGraph = static_cast<TGraph*>( getHist( cPoint.first, "cbc_pulseshape" ) );
			cTmpGraph->SetPoint( cTmpGraph->GetN(), cTestPulseDelay, cPoint.second );
			// cTmpGraph->SetPoint( cTmpGraph->GetN(), cPoint.second.second, cVcth );
		}
		updateHists( "cbc_pulseshape", false );

	}
}


std::map<Cbc*, uint8_t> PulseShape::ScanVcth( uint32_t pDelay )
{

	for ( auto& cChannel : fChannelMap )
		cChannel.second->initializeHist( pDelay, "Delay" );

	uint8_t cVcth = ( fHoleMode ) ?  0x00 :  0xFF;
	int cStep = ( fHoleMode ) ? 10 : -10;
	uint32_t cAllZeroCounter = 0;
	// uint8_t cDoubleVcth;
	bool cAllZero = false;
	bool cNonZero = false;
	bool cSaturate = false;
	// Adaptive VCth loop
	while ( 0x00 <= cVcth && cVcth <= 0xFF )
	{
		CbcRegWriter cWriter( fCbcInterface, "VCth", cVcth );
		this->accept( cWriter );

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
					cN += fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
					const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );
					cNHits += fillVcthHist( pBoard, events, cVcth );
					cNthAcq++;
				}
				fBeBoardInterface->Stop( pBoard, cNthAcq );
				std::cout << "Vcth " << +cVcth << " Hits " << cNHits  << " Events " << cN - 1 << std::endl;
				if ( cNHits != 0 ) cNonZero = true;
			}
		}
		if ( cNonZero && cNHits == 0 && cAllZero == false )
		{
			cAllZero = true;
			cVcth -= 5 * cStep;
			cStep /= 10;
			continue;
		}
		if ( cNHits == 0 &&  cAllZero )
			cAllZeroCounter++;

		if ( cAllZeroCounter > 3 )
			break;
		if ( fHoleMode && cVcth >= 0xFE && cNHits != 0 )
		{
			cSaturate = true;
			break;
		}
		if ( !fHoleMode && cVcth <= 0x01 && cNHits != 0 )
		{
			cSaturate = true;
			break;
		}
		cVcth += cStep;
		updateHists( "", false );

	}

	// done counting hits for all FE's, now update the Histograms



	std::map<Cbc*, uint8_t > cHpoint;

	uint8_t cVal;
	// uint8_t cMax;

	for ( auto& cChannel : fChannelMap )
	{

		cChannel.second->fScurve->Scale( 1 / double( fNevents ) );
		// cMin = cChannel.second->fScurve->GetBinCenter( cChannel.second->fScurve->FindFirstBinAbove( 0.5 ) );
		cVal = cChannel.second->fScurve->GetBinCenter( cChannel.second->fScurve->FindLastBinAbove( 0.5 ) );
		// std::pair<uint8_t, uint8_t> cPair( cMin, cMax );
		if ( !cSaturate ) cHpoint[cChannel.first] = cVal;
		else cHpoint[cChannel.first] = 0;
		std::cout << "Cbc Id " << +cChannel.first->getCbcId() << " Delay " << +pDelay << " VCth " << +cVal << std::endl;

		// here we have measured the complete curve, now we need to extract the FWHM points
		// iterate over the channel map, get the histogram of each channel, normailze it to 1 (->Scale (1/double(fNevents)))
		// insert the points in a map <cbc, pair> and return
	}
	updateHists( "", false );
	return cHpoint;
}
//////////////////////////////////////		PRIVATE METHODS		/////////////////////////////////////////////



int PulseShape::findTestGroup( uint32_t pChannelId )
{

	int cGrp = -1;
	for ( int cChIndex = 0; cChIndex < 16; cChIndex++ )
	{
		uint32_t cResult = pChannelId / 2 - cChIndex * 8;
		if ( cResult < 8 )
			cGrp = cResult;
	}
	return cGrp;

}




void PulseShape::enableChannel( uint8_t pChannelId )
{
	// uint8_t cOffset;
	// std::vector<std::pair<std::string, uint8_t>> cRegVec;

	// if ( fHoleMode ) cOffset = 0x00;
	// else cOffset = 0xFF;

	// for ( uint8_t cChannelId = 1; cChannelId <= 254; cChannelId++ )
	// {
	// 	std::string cReg = Form( "Channel%03d", cChannelId + 1 );;
	// 	// std::cout << cReg << std::endl;
	// 	if ( cChannelId != pChannelId + 1 )
	// 		cRegVec.push_back( std::make_pair( cReg, cOffset ) );

	// }
	// CbcMultiRegWriter cWriter( fCbcInterface, cRegVec );

	// uint8_t cOffset = 0x05;
	std::string cReg = Form( "Channel%03d", pChannelId + 1 );;
	CbcRegWriter cWriter( fCbcInterface, cReg, fOffset );
	this->accept( cWriter );

}



void PulseShape::setDelayAndTesGroup( uint32_t pDelay )
{
	uint8_t cCoarseDelay = ( pDelay - 1 ) / 25;
	uint8_t cFineDelay = pDelay - ( cCoarseDelay * 25 );

	// std::cout << "cFineDelay: " << +cFineDelay << std::endl;
	// std::cout << "cCoarseDelay: " << +cCoarseDelay << std::endl;
	// std::cout << "Current Time: " << +pDelay << std::endl;
	BeBoardRegWriter cBeBoardWriter( fBeBoardInterface, DELAY_AF_TEST_PULSE, cCoarseDelay );
	this->accept( cBeBoardWriter );
	CbcRegWriter cWriter( fCbcInterface, "SelTestPulseDel&ChanGroup", to_reg( cFineDelay, fTestGroup ) );
	this->accept( cWriter );

}



uint32_t PulseShape::fillVcthHist( BeBoard* pBoard, std::vector<Event*> pEventVector, uint32_t pVcth )
{
	uint32_t cHits = 0;
	// Loop over Events from this Acquisition
	for ( auto& cEvent : pEventVector )
	{
		std::cout << *cEvent << std::endl;
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
						TH1F* cTmpHist = cChannel->second->fScurve;
						// if ( cTmpHist->GetBinContent( cTmpHist->FindBin( pVcth ) ) < fNevents )
						cChannel->second->fScurve->Fill( pVcth );
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
	cSetting = fSettingsMap.find( "ChannelOffset" );
	if ( cSetting != std::end( fSettingsMap ) ) fOffset = cSetting->second;
	else fOffset = 0x05;
	cSetting = fSettingsMap.find( "Channel" );
	if ( cSetting != std::end( fSettingsMap ) ) fChannel = cSetting->second;
	else fChannel = 9;
	cSetting = fSettingsMap.find( "StepSize" );
	if ( cSetting != std::end( fSettingsMap ) ) fStepSize = cSetting->second;
	else fStepSize = 5;


	std::cout << "Parsed the following settings:" << std::endl;
	std::cout << "	Nevents = " << fNevents << std::endl;
	std::cout << "	HoleMode = " << int( fHoleMode ) << std::endl;
	std::cout << "	TPAmplitude = " << int( fTPAmplitude ) << std::endl;
	std::cout << "	Channel = " << int( fChannel ) << std::endl;
	std::cout << "	ChOffset = " << int( fOffset ) << std::endl;
	std::cout << "	StepSize = " << int( fStepSize ) << std::endl;

}

void PulseShape::setSystemTestPulse( uint8_t pTPAmplitude, uint8_t pChannelId )
{
	// translate the channel id to a test group
	std::vector<std::pair<std::string, uint8_t>> cRegVec;

	//calculate the right test group
	this->fTestGroup = findTestGroup( pChannelId );

	uint8_t cRegValue =  to_reg( 0, fTestGroup );
	cRegVec.push_back( std::make_pair( "SelTestPulseDel&ChanGroup",  cRegValue ) );

	//set the value of test pulsepot registrer and MiscTestPulseCtrl&AnalogMux register
	if ( fHoleMode )
		cRegVec.push_back( std::make_pair( "MiscTestPulseCtrl&AnalogMux", 0xD1 ) );
	else
		cRegVec.push_back( std::make_pair( "MiscTestPulseCtrl&AnalogMux ", 0x61 ) );

	cRegVec.push_back( std::make_pair( "TestPulsePot", pTPAmplitude ) );

	CbcMultiRegWriter cWriter( fCbcInterface, cRegVec );
	this->accept( cWriter );

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

					// std::cout << "Settung/updating map fChannelMap[" << Form( "0x%x", cCbc ) << "] = " << Form( "0x%x", cChannel ) << std::endl;
				}
				// enableChannel( pChannelId );
				std::cout << "Channel: " << +pChannelId << std::endl;
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
		else if ( pHistName == "cbc_pulseshape" )
		{

			cCanvas.second->cd( 2 );
			TGraph* cTmpGraph = dynamic_cast<TGraph*>( getHist( static_cast<Ph2_HwDescription::Cbc*>( cCanvas.first ), pHistName ) );
			cTmpGraph->Draw( "AP" );
		}

		cCanvas.second->Update();
	}
}


// std::map<Cbc*, std::pair<uint8_t, uint8_t>> PulseShape::ScanTestPulseDelay( uint8_t pVcth )
// {

// 	CbcRegWriter cWriter( fCbcInterface, "VCth", pVcth );
// 	this->accept( cWriter );

// 	// initialize the historgram for the channel map
// 	int cLow = 4800;
// 	int cHigh = 5300;
// 	for ( auto& cChannel : fChannelMap )
// 		cChannel.second->initializeHistTiming( pVcth, "VCth", cHigh - cLow, cLow, cHigh );

// 	for ( uint32_t cTestPulseDelay = cLow ; cTestPulseDelay < cHigh; cTestPulseDelay += 10 )
// 	{
// 		setDelayAndTesGroup( cTestPulseDelay );

// 		// set test pulse delay: not sure yet if beBoard register or CbcRegister
// 		// BeBoardRegWriter cBeBoardWriter(fBeBoardInterface, "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE", cTestPulseDelay);
// 		// this->accept( cBeBoardWriter);
// 		// setDelayAndTesGroup( cTestPulseDelay, fTestGroup );

// 		// then we take fNEvents
// 		uint32_t cN = 1;
// 		uint32_t cNthAcq = 0;
// 		int cNHits = 0;

// 		// Take Data for all Modules
// 		for ( auto& cShelve : fShelveVector )
// 		{
// 			for ( BeBoard* pBoard : cShelve->fBoardVector )
// 			{
// 				fBeBoardInterface->Start( pBoard );
// 				while ( cN <= fNevents )
// 				{
// 					cN += fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
// 					const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );
// 					// everything from here will got into fillHistograms method
// 					cNHits += fillDelayHist( pBoard, events, cTestPulseDelay );
// 					cNthAcq++;
// 				}
// 				fBeBoardInterface->Stop( pBoard, cNthAcq );
// 				std::cout << "Delay " << +cTestPulseDelay << " Hits " << cNHits  << " Events " << cN << std::endl;

// 			}
// 		}

// 		// done counting hits for all FE's, now update the Histograms
// 		updateHists( "", false );
// 	}

// 	std::map<Cbc*, std::pair<uint8_t, uint8_t>> cFWHMpoints;

// 	uint8_t cMin;
// 	uint8_t cMax;

// 	for ( auto& cChannel : fChannelMap )
// 	{

// 		cChannel.second->fScurve->Scale( 1 / double( fNevents ) );
// 		cMin = cChannel.second->fScurve->GetBinCenter( cChannel.second->fScurve->FindFirstBinAbove( 0.5 ) );
// 		cMax = cChannel.second->fScurve->GetBinCenter( cChannel.second->fScurve->FindLastBinAbove( 0.5 ) );
// 		std::pair<uint8_t, uint8_t> cPair( cMin, cMax );
// 		cFWHMpoints[cChannel.first] = cPair;

// 		// here we have measured the complete curve, now we need to extract the FWHM points
// 		// iterate over the channel map, get the histogram of each channel, normailze it to 1 (->Scale (1/double(fNevents)))
// 		// insert the points in a map <cbc, pair> and return
// 	}
// 	return cFWHMpoints;
// }





// void PulseShape::printScanTestPulseDelay( uint8_t pStepSize )
// {

// 	std::map<Cbc*, std::pair<uint8_t, uint8_t>> cCollectedPoints;
// 	setSystemTestPulse( fTPAmplitude, 11 ); // we look at channel 9
// 	uint8_t cVcth = ( fHoleMode ) ?  0x96 :  0x00;
// 	int cStep = ( fHoleMode ) ? -10 : 10;
// 	// uint8_t cVcth = 0x87;
// 	// Adaptive VCth loop
// 	while ( 0x78 <= cVcth && cVcth <= 0xFF )
// 	{
// 		std::cout << "Threshold " << +cVcth << " : " << std::endl;
// 		//add the channel id
// 		cCollectedPoints = ScanTestPulseDelay( cVcth );
// 		for ( auto& cPoint : cCollectedPoints )
// 		{
// 			TGraph* cTmpGraph = static_cast<TGraph*>( getHist( cPoint.first, "cbc_pulseshape" ) );
// 			cTmpGraph->SetPoint( cTmpGraph->GetN(), cPoint.second.first, cVcth );
// 			cTmpGraph->SetPoint( cTmpGraph->GetN(), cPoint.second.second, cVcth );
// 		}
// 		updateHists( "cbc_pulseshape", false );
// 		cVcth += cStep;
// 	}
// }