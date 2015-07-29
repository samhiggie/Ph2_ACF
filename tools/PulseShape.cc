#include "PulseShape.h"


void PulseShape::Initialize()
{
	fNCbc = 0;

	std::cerr << "void PulseShape::Initialize()"  << std::endl;
	for ( auto& cShelve : fShelveVector )
	{
		uint32_t cShelveId = cShelve->getShelveId();
		std::cerr << "cShelveId = " << cShelveId << std::endl;

		for ( auto& cBoard : cShelve->fBoardVector )
		{
			uint32_t cBoardId = cBoard->getBeId();
			std::cerr << "cBoardId = " << cBoardId << std::endl;
			// we could read the Delay_after_TestPulse Register in a variable
			uint32_t cDelayAfterPulse = fBeBoardInterface->ReadBoardReg( cBoard, DELAY_AF_TEST_PULSE );
			std::cout << "actual Delay: " << +cDelayAfterPulse << std::endl;
			for ( auto& cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();
				std::cerr << "cFeId = " << cFeId << std::endl;

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
					cPulseGraph->SetName( cName );
					cPulseGraph->SetMarkerStyle( 3 );
					cPulseGraph->GetXaxis()->SetTitle( "TestPulseDelay [ns]" );
					cPulseGraph->GetYaxis()->SetTitle( "TestPulseAmplitue [VCth]" );

					bookHistogram( cCbc, "cbc_pulseshape", cPulseGraph );

					cName = Form( "f_cbc_pulse_Fe%dCbc%d", cFeId, cCbcId );
					cObj = gROOT->FindObject( cName );
					if ( cObj ) delete cObj;
					TF1* cPulseFit = new TF1( cName, pulseshape, ( cDelayAfterPulse - 1 ) * 25, ( cDelayAfterPulse + 6 ) * 25, 4 );

					bookHistogram( cCbc, "cbc_pulsefit", cPulseFit );
				}

			}
		}
	}

	parseSettings();
	std::cout << "Histograms and Settings initialised." << std::endl;
}




void PulseShape::ScanTestPulseDelay( uint8_t pStepSize )
{
	setSystemTestPulse( fTPAmplitude, fChannel );
	enableChannel( fChannel );

	// initialize the historgram for the channel map
	int cCoarseDefault = 201;
	int cLow = ( cCoarseDefault - 1 ) * 25;
	int cHigh = ( cCoarseDefault + 8 ) * 25;
	std::map<Cbc*, uint8_t> cCollectedPoints;
	for ( uint32_t cTestPulseDelay = cLow ; cTestPulseDelay < cHigh; cTestPulseDelay += fStepSize )
	{
		setDelayAndTesGroup( cTestPulseDelay );
		cCollectedPoints =  ScanVcth( cTestPulseDelay );
		for ( auto& cPoint : cCollectedPoints )
		{
			TGraph* cTmpGraph = static_cast<TGraph*>( getHist( cPoint.first, "cbc_pulseshape" ) );
			cTmpGraph->SetPoint( cTmpGraph->GetN(), cTestPulseDelay, cPoint.second );

		}
		updateHists( "cbc_pulseshape", false );

	}
	this->fitGraph( cLow );
}


std::map<Cbc*, uint8_t> PulseShape::ScanVcth( uint32_t pDelay )
{

	for ( auto& cChannel : fChannelMap )
		cChannel.second->initializeHist( pDelay, "Delay" );

	uint8_t cVcth = ( fHoleMode ) ?  0xFF :  0x00;
	int cStep = ( fHoleMode ) ? -10 : +10;
	uint32_t cAllOneCounter = 0;
	// uint8_t cDoubleVcth;
	bool cAllOne = false;
	bool cNonZero = false;
	bool cSaturate = false;
	// Adaptive VCth loop
	while ( 0x00 <= cVcth && cVcth <= 0xFF )
	{
		if ( cAllOne ) break;
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
		if ( !cNonZero && cNHits != 0 )
		{
			cNonZero = true;
			cVcth -= 5 * cStep;
			cStep /= 10;
			continue;
		}
		if ( cNHits > 0.95 * fNCbc * fNevents )
			cAllOneCounter++;

		if ( cAllOneCounter > 6 ) cAllOne = true;
		if ( cAllOne ) break;
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

	std::map<Cbc*, uint8_t > cHpoint;

	uint8_t cVal;
	// uint8_t cMax;

	for ( auto& cChannel : fChannelMap )
	{
		cChannel.second->fitHist( fNevents, fHoleMode, pDelay, "Delay", fResultFile );
		cVal = cChannel.second->getPedestal();
		if ( !cSaturate ) cHpoint[cChannel.first] = cVal;
		else cHpoint[cChannel.first] = 255;
		std::cout << "Cbc Id " << +cChannel.first->getCbcId() << " Delay " << +pDelay << " VCth " << +cVal << std::endl;

	}
	updateHists( "", true );
	return cHpoint;
}


//////////////////////////////////////		PRIVATE METHODS		/////////////////////////////////////////////


void PulseShape::fitGraph( int pLow )
{
	// iterate over fCbcHistMap
	// for each Cbc
	// 	TGraph->Fit()
	for ( auto& cCbc : fCbcHistMap )
	{
		TGraph* cGraph = static_cast<TGraph*>( getHist( cCbc.first, "cbc_pulseshape" ) );
		TF1* cFit = static_cast<TF1*>( getHist( cCbc.first, "cbc_pulsefit" ) );
		//"scale_par"
		cFit->SetParLimits( 0, 160, 2000 );
		//"offset"
		cFit->SetParLimits( 1, pLow, pLow + 300 );
		//"time_constant"
		cFit->SetParLimits( 2, 5, 12.5 );
		//"y_offset"
		cFit->SetParLimits( 3, 0, 40 );
		cGraph->Fit( cFit );
	}
}

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

	std::string cReg = Form( "Channel%03d", pChannelId + 1 );;
	CbcRegWriter cWriter( fCbcInterface, cReg, fOffset );
	this->accept( cWriter );
}

void PulseShape::setDelayAndTesGroup( uint32_t pDelay )
{
	uint8_t cCoarseDelay = floor( pDelay  / 25 );
	uint8_t cFineDelay = ( cCoarseDelay * 25 ) + 24 - pDelay;

	std::cout << "cFineDelay: " << +cFineDelay << std::endl;
	std::cout << "cCoarseDelay: " << +cCoarseDelay << std::endl;
	std::cout << "Current Time: " << +pDelay << std::endl;
	BeBoardRegWriter cBeBoardWriter( fBeBoardInterface, DELAY_AF_TEST_PULSE, cCoarseDelay );
	this->accept( cBeBoardWriter );
	CbcRegWriter cWriter( fCbcInterface, "SelTestPulseDel&ChanGroup", to_reg( cFineDelay, fTestGroup ) );
	this->accept( cWriter );

}

uint32_t PulseShape::fillVcthHist( BeBoard* pBoard, std::vector<Event*> pEventVector, uint32_t pVcth )
{
	uint32_t cHits = 0;
	// Loop over Events from this Acquisition

	for ( auto cFe : pBoard->fModuleVector )
	{
		for ( auto cCbc : cFe->fCbcVector )
		{
			//  get histogram to fill

			auto cChannel = fChannelMap.find( cCbc );
			if ( cChannel == std::end( fChannelMap ) ) std::cout << "Error, no channel mapped to this CBC ( " << +cCbc->getCbcId() << " )" << std::endl;
			else
			{
				uint32_t cCbcHitCounter = 0;
				for ( auto& cEvent : pEventVector )
				{
					if ( cEvent->DataBit( cFe->getFeId(), cCbc->getCbcId(), cChannel->second->fChannelId ) )
					{
						cCbcHitCounter++;

						cHits++;

					}
				}
				TH1F* cTmpHist = cChannel->second->fScurve;
				if ( cTmpHist->GetBinContent( cTmpHist->FindBin( pVcth ) ) == 0 ) cChannel->second->fScurve->SetBinContent( pVcth, cCbcHitCounter );
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
	cSetting = fSettingsMap.find( "Vplus" );
	if ( cSetting != std::end( fSettingsMap ) )  fVplus = cSetting->second;
	else fVplus = 0x6F;
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
	std::cout << "	Vplus = " << int( fVplus ) << std::endl;
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
	cRegVec.push_back( std::make_pair( "Vplus",  fVplus ) );

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


				for ( auto& cCbc : cFe->fCbcVector )
				{
					uint32_t cCbcId = cCbc->getCbcId();
					Channel* cChannel = new Channel( cBoardId, cFeId, cCbcId, pChannelId );
					fChannelMap[cCbc] = cChannel;
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
		if ( pHistName == "" && pFinal )
		{
			// now iterate over the channels in the channel map and draw
			auto cChannel = fChannelMap.find( static_cast<Ph2_HwDescription::Cbc*>( cCanvas.first ) );
			if ( cChannel == std::end( fChannelMap ) ) std::cout << "Error, no channel mapped to this CBC ( " << +cCanvas.first << " )" << std::endl;
			else
			{
				cCanvas.second->cd( 1 );
				cChannel->second->fScurve->Draw( "P0" );
				cChannel->second->fFit->Draw( "same" );
			}
		}
		else if ( pHistName == "cbc_pulseshape" )
		{

			cCanvas.second->cd( 2 );
			TGraph* cTmpGraph = dynamic_cast<TGraph*>( getHist( static_cast<Ph2_HwDescription::Cbc*>( cCanvas.first ), pHistName ) );
			cTmpGraph->Draw( "AP" );
		}
		else if ( pHistName == "cbc_pulseshape" && pFinal )
		{

			cCanvas.second->cd( 2 );
			TGraph* cTmpGraph = dynamic_cast<TGraph*>( getHist( static_cast<Ph2_HwDescription::Cbc*>( cCanvas.first ), pHistName ) );
			TF1* cTmpFit = dynamic_cast<TF1*>( getHist( static_cast<Ph2_HwDescription::Cbc*>( cCanvas.first ), "cbc_pulsefit" ) );
			cTmpGraph->Draw( "AP" );
			cTmpFit->Draw( "same" );
		}

		cCanvas.second->Update();
	}
}

double pulseshape( double* x, double* par )
{
	double xx = x[0];
	double temp = pow( ( xx - par[1] ) / par[2] , 3 );
	double val = ( ( par[0] * temp * exp( -( ( xx - par[1] ) / par[2] ) ) ) ) + par[3];
	if ( xx < par[1] )
		val = par[3];
	return val;
}


