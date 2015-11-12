#include "PedeNoise.h"


void PedeNoise::Initialise()
{
	//is to be called after system controller::ReadHW, ReadSettings
	// populates all the maps
	// create the canvases

	if ( ! fPedeNoise ) fValidationCanvas = new TCanvas( "Occupancy", "Occupancy", 650, 650 );
	else
	{
		fNoiseCanvas = new TCanvas( "Final SCurves, Strip Noise", "Final SCurves, Noise", 650, 650 );
		fPedestalCanvas = new TCanvas( "Pedestal & Noise", "Pedestal & Noise", 650, 650 );
		fFeSummaryCanvas = new TCanvas( "Noise for each FE", "Noise for each FE", 650, 650 );
	}
	// count FEs & CBCs
	uint32_t cCbcCount = 0;
	uint32_t cCbcIdMax = 0;
	uint32_t cFeCount = 0;

	for ( auto cShelve : fShelveVector )
	{
		uint32_t cShelveId = cShelve->getShelveId();

		for ( auto cBoard : cShelve->fBoardVector )
		{
			uint32_t cBoardId = cBoard->getBeId();

			for ( auto cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();
				cFeCount++;

				for ( auto cCbc : cFe->fCbcVector )
				{
					uint32_t cCbcId = cCbc->getCbcId();
					cCbcCount++;
					if ( cCbcId > cCbcIdMax ) cCbcIdMax = cCbcId;

					// populate the channel vector
					std::vector<Channel> cChanVec;

					for ( uint8_t cChan = 0; cChan < 254; cChan++ )
						cChanVec.push_back( Channel( cBoardId, cFeId, cCbcId, cChan ) );

					fCbcChannelMap[cCbc] = cChanVec;

					// the fits are initialized when I fit!

					TString cHistname;
					TH1F* cHist;
					// validiation histograms
					if ( !fPedeNoise )
					{
						cHistname = Form( "Validation_Fe%d_Cbc%d", cFeId, cCbcId );
						cHist = dynamic_cast<TH1F*>( gROOT->FindObject( cHistname ) );
						if ( cHist ) delete cHist;
						cHist = new TH1F( cHistname, cHistname, 100, 0, 1 );
						fHistMap[cCbc] = cHist;
					}


					// for noise maps etc.
					else
					{

						cHistname = Form( "Fe%dCBC%d_Noise", cFe->getFeId(), cCbc->getCbcId() );
						cHist = dynamic_cast<TH1F*>( gROOT->FindObject( cHistname ) );
						if ( cHist ) delete cHist;
						cHist = new TH1F( cHistname, cHistname, 200, 0, 20 );
						fNoiseMap[cCbc] =  cHist;

						cHistname = Form( "Fe%dCBC%d_StripNoise", cFe->getFeId(), cCbc->getCbcId() );
						cHist = dynamic_cast<TH1F*>( gROOT->FindObject( cHistname ) );
						if ( cHist ) delete cHist;
						cHist = new TH1F( cHistname, cHistname, 254, -0.5, 253.5 );
						fNoiseStripMap[cCbc] =  cHist;

						cHistname = Form( "Fe%dCBC%d_Pedestal", cFe->getFeId(), cCbc->getCbcId() );
						cHist = dynamic_cast<TH1F*>( gROOT->FindObject( cHistname ) );
						if ( cHist ) delete cHist;
						cHist = new TH1F( cHistname, cHistname, 510, -0.5, 254.5 );
						fPedestalMap[cCbc] =  cHist;

						cHistname = Form( "Fe%dCBC%d_Noise_even", cFe->getFeId(), cCbc->getCbcId() );
						cHist = dynamic_cast<TH1F*>( gROOT->FindObject( cHistname ) );
						if ( cHist ) delete cHist;
						cHist = new TH1F( cHistname, cHistname, 128, -0.5, 127.5 );
						fSensorNoiseMapEven[cCbc] =  cHist;

						cHistname = Form( "Fe%dCBC%d_Noise_odd", cFe->getFeId(), cCbc->getCbcId() );
						cHist = dynamic_cast<TH1F*>( gROOT->FindObject( cHistname ) );
						if ( cHist ) delete cHist;
						cHist = new TH1F( cHistname, cHistname, 128, -0.5, 127.5 );
						cHist->SetLineColor( 2 );
						fSensorNoiseMapOdd[cCbc] =  cHist;
					}
				}

				if ( fPedeNoise )
				{
					TString cNoisehistname =  Form( "Fe%d_Noise", cFeId );
					TH1F* cNoise = new TH1F( cNoisehistname, cNoisehistname, 200, 0, 20 );
					bookHistogram( cFe, "Module_noisehist", cNoise );

					cNoisehistname = Form( "Fe%d_StripNoise", cFeId );
					TProfile* cStripnoise = new TProfile( cNoisehistname, cNoisehistname, ( NCHANNELS * cCbcCount ) + 1, -.5, cCbcCount * NCHANNELS + .5 );
					bookHistogram( cFe, "Module_Stripnoise", cStripnoise );
				}
			}
			fNCbc = cCbcCount;
			fNFe = cFeCount;
		}
	}
	uint32_t cPads = ( cCbcIdMax > cCbcCount ) ? cCbcIdMax : cCbcCount;


	if ( !fPedeNoise ) fValidationCanvas->DivideSquare( cPads );
	else
	{
		fNoiseCanvas->DivideSquare( 2 * cPads );
		fPedestalCanvas->DivideSquare( 2 * cPads );
		fFeSummaryCanvas->DivideSquare( 2 * cPads );
	}

	// now read the settings from the map
	auto cSetting = fSettingsMap.find( "HoleMode" );
	fHoleMode = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 1;
	cSetting = fSettingsMap.find( "Nevents" );
	fEventsPerPoint = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 10;
	cSetting = fSettingsMap.find( "FitSCurves" );
	fFitted = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 0;
	cSetting = fSettingsMap.find( "TestPulseAmplitude" );
	fTestPulseAmplitude = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 0;

	// Decide if test pulse or not
	if ( ( fTestPulseAmplitude == 0x00 ) || ( fTestPulseAmplitude == 0xFF ) ) fTestPulse = 0;
	else fTestPulse = 1;
	std::cout << "Created Object Maps and parsed settings:" << std::endl;
	std::cout << "	Hole Mode = " << fHoleMode << std::endl;
	std::cout << "	Nevents = " << fEventsPerPoint << std::endl;
	std::cout << "	FitSCurves = " << int( fFitted ) << std::endl;
	std::cout << "	TestPulseAmplitude = " << int( fTestPulseAmplitude ) << std::endl;
}


void PedeNoise::measureNoise()
{
	saveInitialOffsets();
	// method to measure one final set of SCurves with the final calibration applied to extract the noise
	// now measure some SCurves
	for ( auto& cTGrpM : fTestGroupChannelMap )
	{
		if ( cTGrpM.first == -1 && fdoTGrpCalib )
			continue;
		if ( cTGrpM.first > -1 && !fdoTGrpCalib )
			break;
		// if we want to run with test pulses, we'll have to enable commissioning mode and enable the TP for each test group
		if ( fTestPulse )
		{
			BeBoardRegWriter cBeBoardWriter( fBeBoardInterface, RQ, 1 );
			this->accept( cBeBoardWriter );
			cBeBoardWriter.setRegister( ENABLE_TP, 1 );
			this->accept( cBeBoardWriter );
			std::cout << "Enabling Test Pulse for Test Group " << cTGrpM.first << " with amplitude " << +fTestPulseAmplitude << std::endl;
			setSystemTestPulse( fTestPulseAmplitude, cTGrpM.first );
			// setSystemTestPulse( fTestPulseAmplitude, 0 );

		}

		std::cout << "Measuring Test Group...." << cTGrpM.first << std::endl;
		// this leaves the offset values at the tuned values for cTGrp and disables all other groups
		enableTestGroupforNoise( cTGrpM.first );

		// now initialize the Scurves
		initializeSCurves( "Final", fTestPulseAmplitude, cTGrpM.first );

		// measure the SCurves, the false is indicating that I am sweeping Vcth
		measureSCurves( false, cTGrpM.first );

		// now process the measured SCuvers, true indicates that I am drawing, the TGraphErrors with Vcth vs Vplus are also filled
		processSCurvesNoise( "Final", fTestPulseAmplitude, true, cTGrpM.first );
	}
	std::cout << BOLDBLUE << "Finished measuring the noise ..." << std::endl << RESET << std::endl;

	// now plot the histogram with the noise

	// instead of looping over the Histograms and finding everything according to the CBC from the map, just loop the CBCs
	for ( auto cShelve : fShelveVector )
	{
		uint32_t cShelveId = cShelve->getShelveId();

		for ( auto cBoard : cShelve->fBoardVector )
		{
			uint32_t cBoardId = cBoard->getBeId();

			for ( auto cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();

				// here get the per FE histograms
				TH1F* cTmpHist = static_cast<TH1F*>( getHist( cFe, "Module_noisehist" ) );
				TProfile* cTmpProfile = static_cast<TProfile*>( getHist( cFe, "Module_Stripnoise" ) );

				for ( auto cCbc : cFe->fCbcVector )
				{
					uint32_t cCbcId = static_cast<int>( cCbc->getCbcId() );
					// here get the per-CBC histograms
					auto cHist = fNoiseMap.find( cCbc );
					if ( cHist == std::end( fNoiseMap ) ) std::cout << "Error: could not find the Noise Histogram for CBC " << int( cCbc->getCbcId() ) << std::endl;
					auto cPedestalHist = fPedestalMap.find( cCbc );
					if ( cPedestalHist == std::end( fNoiseMap ) ) std::cout << "Error: could not find the Pedestal Histogram for CBC " << int( cCbc->getCbcId() ) << std::endl;
					auto cStripHist = fNoiseStripMap.find( cCbc );
					if ( cStripHist == std::end( fNoiseStripMap ) ) std::cout << "Error: could not find the Strip Noise Profile for CBC " << int( cCbc->getCbcId() ) << std::endl;

					auto cStripHistEven = fSensorNoiseMapEven.find( cCbc );
					if ( cStripHistEven == std::end( fSensorNoiseMapEven ) ) std::cout << "Error: could not find the even Strip Noise Profile for CBC " << int( cCbc->getCbcId() ) << std::endl;

					auto cStripHistOdd = fSensorNoiseMapOdd.find( cCbc );
					if ( cStripHistOdd == std::end( fSensorNoiseMapOdd ) ) std::cout << "Error: could not find the odd Strip Noise Profile for CBC " << int( cCbc->getCbcId() ) << std::endl;

					std::cout << BOLDRED << "Average noise on FE " << +cHist->first->getFeId() << " CBC " << +cHist->first->getCbcId() << " : " << cHist->second->GetMean() << " ; RMS : " << cHist->second->GetRMS() << " ; Pedestal : " << cPedestalHist->second->GetMean() << " VCth units." << RESET << std::endl;

					fNoiseCanvas->cd( fNCbc + cCbc->getCbcId() + 1 );
					// cStripHist->second->DrawCopy();
					cStripHistEven->second->DrawCopy();
					cStripHistOdd->second->DrawCopy( "same" );

					fPedestalCanvas->cd( cCbc->getCbcId() + 1 );
					cHist->second->DrawCopy();

					fPedestalCanvas->cd( fNCbc + cCbc->getCbcId() + 1 );
					cPedestalHist->second->DrawCopy();
					fNoiseCanvas->Update();
					fPedestalCanvas->Update();
#ifdef __HTTP__
					fHttpServer->ProcessRequests();
#endif
					// here add the CBC histos to the module histos
					cTmpHist->Add( cHist->second );
					for ( int cBin = 0; cBin < NCHANNELS; cBin++ )
					{
						// std::cout << cBin << " Strip " << +cCbcId * 254 + cBin << " Noise " << cStripHist->second->GetBinContent( cBin ) << std::endl;
						if ( cStripHist->second->GetBinContent( cBin ) > 0 && cStripHist->second->GetBinContent( cBin ) < 255 ) cTmpProfile->Fill( cCbcId * 254 + cBin, cStripHist->second->GetBinContent( cBin ) );
						// else cTmpProfile->Fill( cCbcId * 254 + cBin, 255 );
					}
				}
				fFeSummaryCanvas->cd( cFeId + 1 );
				cTmpHist->DrawCopy();
				fFeSummaryCanvas->cd( fNFe + cFeId + 1 );
				cTmpProfile->DrawCopy();
				fFeSummaryCanvas->Update();
#ifdef __HTTP__
				fHttpServer->ProcessRequests();
#endif
			}
		}
	}
}


void PedeNoise::Validate()
{
	// Validate the calibration result by applying the calibration, taking data & histograming the variation of occupancies

	std::cout << "Mesuring Efficiency per Strip ... " << std::endl;

	// Initialize Temporary Profile for single strip efficiency / occupancy
	std::map<Cbc*, TProfile*> cProfileMap;
	for ( auto& cShelve : fShelveVector )
	{
		for ( BeBoard* pBoard : cShelve->fBoardVector )
		{
			for ( auto cFe : pBoard->fModuleVector )
			{
				// just refresh all register values from the actual HW before doing the test!
				// fCbcInterface->ReadAllCbc( cFe );

				for ( auto cCbc : cFe->fCbcVector )
				{
					TString cProfileName = Form( "Fe%dCBC%d_Occupancy", cFe->getFeId(), cCbc->getCbcId() );
					TProfile* cTempProf = dynamic_cast<TProfile*>( gROOT->FindObject( cProfileName ) );
					if ( cTempProf ) delete cTempProf;
					cTempProf = new TProfile( cProfileName, cProfileName, 255, -0.5, 254.5 );
					cProfileMap[cCbc] =  cTempProf;
				}

			}
		}
	}

	// Now Take Data

	uint32_t cTotalEvents = 5000;

	CbcRegReader cReader( fCbcInterface, "VCth" );
	accept( cReader );

	std::cout << "Taking data ... " << std::endl;

	for ( auto& cShelve : fShelveVector )
	{
		for ( BeBoard* pBoard : cShelve->fBoardVector )
		{
			uint32_t cN = 1;
			uint32_t cNthAcq = 0;

			fBeBoardInterface->Start( pBoard );
			while ( cN <=  cTotalEvents )
			{
				// Run( pBoard, cNthAcq );
				fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
				const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );
				// Loop over Events from this Acquisition
				for ( auto& ev : events )
				{
					uint32_t cHitCounter = 0;
					for ( auto& cFe : pBoard->fModuleVector )
					{
						for ( auto& cCbc : cFe->fCbcVector )
						{
							auto cHitProfile = cProfileMap.find( cCbc );
							if ( cHitProfile == std::end( cProfileMap ) ) std::cout << "Error: could not find the profile for CBC " << int( cCbc->getCbcId() ) << std::endl;
							else
							{
								const std::vector<bool>& list = ev->DataBitVector( cFe->getFeId(), cCbc->getCbcId() );
								int cChannel = 0;
								for ( const auto& b : list )
									cHitProfile->second->Fill( cChannel++, ( ( b ) ? 1 : 0 ) );
							}
						}
					}
					cN++;
				}
				cNthAcq++;
			}
			fBeBoardInterface->Stop( pBoard, cNthAcq );
		}
	}

	// done with datataking, now iterate over ProfileMap, find corresponding validation hist in member map and fill it with the deviation of the bin content from the mean bin content, then draw it to canvas
	for ( auto& cHist : fHistMap )
	{
		auto cProfile = cProfileMap.find( cHist.first );
		if ( cProfile == std::end( cProfileMap ) ) std::cout << "Error: could not find the profile for CBC " << int( cHist.first->getCbcId() ) << std::endl;
		else
		{
			for ( uint32_t iBin = 0; iBin < cProfile->second->GetNbinsX(); iBin++ )
				cHist.second->Fill( cProfile->second->GetBinContent( iBin ) );
		}

		fValidationCanvas->cd( cHist.first->getCbcId() + 1 );
		cHist.second->DrawCopy();
		fValidationCanvas->Update();
#ifdef __HTTP__
		fHttpServer->ProcessRequests();
#endif
		//cProfile->second->SetDirectory( fResultFile );

	}
}

void PedeNoise::enableTestGroupforNoise( int  pTGrpId )
{
	uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;

	for ( auto cShelve : fShelveVector )
	{
		uint32_t cShelveId = cShelve->getShelveId();

		for ( auto cBoard : cShelve->fBoardVector )
		{
			uint32_t cBoardId = cBoard->getBeId();

			for ( auto cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();

				for ( auto cCbc : cFe->fCbcVector )
				{
					uint32_t cCbcId = cCbc->getCbcId();

					// first, find the offset Map for this CBC
					auto cOrigOffsetMap = fOffsetMap.find( cCbc );
					if ( cOrigOffsetMap == std::end( fOffsetMap ) )std::cout << "Error, could not find the original offset map for CBC " << cCbcId << std::endl;
					// cOrigOffsetMap.second is the map of channel number vs original offset

					RegisterVector cRegVec;
					// iterate the groups (first is ID, second is vec<uint8_t>)
					for ( auto& cGrp : fTestGroupChannelMap )
					{
						// if grpid = -1, do nothing (all channels)
						if ( cGrp.first == -1 ) continue;
						// if the group is not my current grout
						if ( cGrp.first != pTGrpId )
						{
							// iterate the channels and push back 0 or FF
							for ( auto& cChan : cGrp.second )
							{
								TString cRegName = Form( "Channel%03d", cChan + 1 );
								cRegVec.push_back( { cRegName.Data(), cOffset } );
								// std::cout << "DEBUG CBC " << cCbcId << " Channel " << +cChan << " group " << cGrp.first << " offset " << +cOffset << std::endl;
							}
						}
						// if it is the current group, get the original offset values
						else if ( cGrp.first == pTGrpId )
						{
							// iterate over the channels in the test group and find the corresponding offset in the original offset map
							for ( auto& cChan : cGrp.second )
							{
								auto cChanOffset = cOrigOffsetMap->second.find( cChan );
								if ( cChanOffset == std::end( cOrigOffsetMap->second ) ) std::cout << "Error, could not find original offset for channel " << +cChan << "on CBC " << cCbcId << std::endl;
								TString cRegName = Form( "Channel%03d", cChan + 1 );
								cRegVec.push_back( { cRegName.Data(), cChanOffset->second } );
								// std::cout << GREEN << "DEBUG CBC " << cCbcId << " Channel " << +cChan << " group " << cGrp.first << " offset " << +cChanOffset->second << RESET << std::endl;
							}
						}
					}

					// now I should have 0 or FF as offset for all channels except the one in my test group
					// this now needs to be written to the CBCs
					fCbcInterface->WriteCbcMultReg( cCbc, cRegVec );
				}
			}
		}
	}
	std::cout << "Disabling all TGroups except " << pTGrpId << " ! " << std::endl;
}


void PedeNoise::processSCurvesNoise( TString pParameter, uint8_t pValue, bool pDraw, int  pTGrpId )
{

	// First fitHits for every Channel, then extract the midpoint and noise and fill it in fVplusVcthGraphMap
	for ( auto& cCbc : fCbcChannelMap )
	{

		// use another histogram for the noise
		HistMap::iterator cHist = fNoiseMap.find( cCbc.first );
		if ( cHist == fNoiseMap.end() ) std::cout << "Could not find the correct Noise Histogram for Fe " << int( cCbc.first->getFeId() ) << " Cbc " << int( cCbc.first->getCbcId() ) << std::endl;

		HistMap::iterator cPedestalHist = fPedestalMap.find( cCbc.first );
		if ( cPedestalHist == fPedestalMap.end() ) std::cout << "Could not find the correct Pedestal Histogram for Fe " << int( cCbc.first->getFeId() ) << " Cbc " << int( cCbc.first->getCbcId() ) << std::endl;

		HistMap::iterator cStripHist = fNoiseStripMap.find( cCbc.first );
		if ( cStripHist == fNoiseStripMap.end() ) std::cout << "Could not find the correct Noise Strip Histogram for Fe " << int( cCbc.first->getFeId() ) << " Cbc " << int( cCbc.first->getCbcId() ) << std::endl;

		HistMap::iterator cSensorHistEven = fSensorNoiseMapEven.find( cCbc.first );
		if ( cSensorHistEven == fSensorNoiseMapEven.end() ) std::cout << "Could not find the correct Noise Histogram Even for Fe " << int( cCbc.first->getFeId() ) << " Cbc " << int( cCbc.first->getCbcId() ) << std::endl;

		HistMap::iterator cSensorHistOdd = fSensorNoiseMapOdd.find( cCbc.first );
		if ( cSensorHistOdd == fNoiseStripMap.end() ) std::cout << "Could not find the correct Sensor Noise Histogram Odd for Fe " << int( cCbc.first->getFeId() ) << " Cbc " << int( cCbc.first->getCbcId() ) << std::endl;


		// Loop the Channels
		bool cFirst = true;
		TString cOption;

		std::vector<uint8_t> cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];
		for ( auto& cChanId : cTestGrpChannelVec )
		{
			//for ( auto& cChan : cCbc.second )
			Channel cChan = cCbc.second.at( cChanId );

			// Fit or Differentiate
			if ( fFitted ) cChan.fitHist( fEventsPerPoint, fHoleMode, pValue, pParameter, fResultFile );
			else cChan.differentiateHist( fEventsPerPoint, fHoleMode, pValue, pParameter, fResultFile );


			// instead of the code below, use a histogram to histogram the noise
			if ( cChan.getNoise() == 0 || cChan.getNoise() > 255 ) std::cout << RED << "Error, SCurve Fit for Fe " << int( cCbc.first->getFeId() ) << " Cbc " << int( cCbc.first->getCbcId() ) << " Channel " << int( cChan.fChannelId ) << " did not work correctly! Noise " << cChan.getNoise() << RESET << std::endl;

			cHist->second->Fill( cChan.getNoise() );
			cPedestalHist->second->Fill( cChan.getPedestal() );

			// Even and odd channel noise
			if ( ( int( cChan.fChannelId ) % 2 ) == 0 )
				cSensorHistEven->second->Fill( int( cChan.fChannelId / 2 ), cChan.getNoise() );
			else
				cSensorHistOdd->second->Fill( int( cChan.fChannelId / 2.0 ), cChan.getNoise() );

			// some output
			std::cout << "FE " << +cCbc.first->getFeId() << " CBC " << +cCbc.first->getCbcId() << " Chanel " << +cChan.fChannelId << " Pedestal " << cChan.getPedestal() << " Noise " << cChan.getNoise() << std::endl;

			cStripHist->second->Fill( cChan.fChannelId, cChan.getNoise() );

			//Draw
			if ( pDraw )
			{
				if ( cFirst )
				{
					cOption = "P" ;
					cFirst = false;
				}
				else cOption = "P same";

				fNoiseCanvas->cd( cCbc.first->getCbcId() + 1 );
				cChan.fScurve->DrawCopy( cOption );
				if ( fFitted )
					cChan.fFit->DrawCopy( "same" );
				else cChan.fDerivative->DrawCopy( "same" );
			}
		}
		if ( pDraw )
		{
			fNoiseCanvas->Update();
#ifdef __HTTP__
			fHttpServer->ProcessRequests();
#endif
		}
	}

}

void PedeNoise::saveInitialOffsets()
{
	std::cout << "Initializing map with original Offsets for later ... " << std::endl;
	// save the initial offsets for Noise measurement in a map
	for ( auto cShelve : fShelveVector )
	{
		uint32_t cShelveId = cShelve->getShelveId();

		for ( auto cBoard : cShelve->fBoardVector )
		{
			uint32_t cBoardId = cBoard->getBeId();

			for ( auto cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();

				for ( auto cCbc : cFe->fCbcVector )
				{
					uint32_t cCbcId = cCbc->getCbcId();

					// map to instert in fOffsetMap
					// <cChan, Offset>
					std::map<uint8_t, uint8_t> cCbcOffsetMap;

					for ( uint8_t cChan = 0; cChan < NCHANNELS; cChan++ )
					{
						TString cRegName = Form( "Channel%03d", cChan + 1 );
						uint8_t cOffset = cCbc->getReg( cRegName.Data() );
						cCbcOffsetMap[cChan] = cOffset;
						// std::cout << "DEBUG Original Offset for CBC " << cCbcId << " channel " << +cChan << " " << +cOffset << std::endl;
					}
					fOffsetMap[cCbc] = cCbcOffsetMap;
				}
			}
		}
	}
}

void PedeNoise::writeGraphs()
{
	// just use auto iterators to write everything to disk
	// this is the old method before Tool class was cool
	fResultFile->cd();
	if ( !fPedeNoise )
	{
		for ( const auto& cHist : fHistMap )
			cHist.second->SetDirectory( fResultFile );
	}
	else
	{
		for ( const auto& cHist : fNoiseMap )
			cHist.second->SetDirectory( fResultFile );
		for ( const auto& cHist : fPedestalMap )
			cHist.second->SetDirectory( fResultFile );
		for ( const auto& cNoiseStripHist : fNoiseStripMap )
			cNoiseStripHist.second->SetDirectory( fResultFile );

		for ( const auto& cNoiseEven : fSensorNoiseMapEven )
			cNoiseEven.second->SetDirectory( fResultFile );
		for ( const auto& cNoiseOdd : fSensorNoiseMapOdd )
			cNoiseOdd.second->SetDirectory( fResultFile );
	}
	fResultFile->cd();

	// This is re-implementing the new method for the FE's since they use the cool stuff from tool
	if ( fPedeNoise )
	{
		for ( const auto& cFe : fModuleHistMap )
		{
			TString cDirName = Form( "FE%d", cFe.first->getFeId() );
			TObject* cObj = gROOT->FindObject( cDirName );
			if ( cObj ) delete cObj;
			fResultFile->mkdir( cDirName );
			fResultFile->cd( cDirName );

			for ( const auto& cHist : cFe.second )
				cHist.second->Write( cHist.second->GetName(), TObject::kOverwrite );
			fResultFile->cd();
		}
	}

	// Save canvasses too
	if ( !fPedeNoise ) fValidationCanvas->Write( fValidationCanvas->GetName(), TObject::kOverwrite );
	else
	{
		fNoiseCanvas->Write( fNoiseCanvas->GetName(), TObject::kOverwrite );
		fPedestalCanvas->Write( fPedestalCanvas->GetName(), TObject::kOverwrite );
		fFeSummaryCanvas->Write( fFeSummaryCanvas->GetName(), TObject::kOverwrite );
	}
}
