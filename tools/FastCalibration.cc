#include "FastCalibration.h"
#include <cmath>

// TODO: Canvas divisions

void FastCalibration::Initialise()
{
	//is to be called after system controller::ReadHW, ReadSettings
	// populates all the maps
	// create the canvases
	fVplusCanvas = new TCanvas( "SCurves", "SCurves", 650, 650 );
	fVcthVplusCanvas = new TCanvas( "Vplus vs. VCth", "Vplus vs. VCth", 650, 650 );
	fOffsetCanvas = new TCanvas( "SCurves Offset Tuning", "SCurves Offset Tuning", 650, 650 );
	fValidationCanvas = new TCanvas( "Occupancy", "Occupancy", 650, 650 );
	fNoiseCanvas = new TCanvas( "Final SCurves, Strip Noise", "Final SCurves, Noise", 650, 650 );
	fPedestalCanvas = new TCanvas( "Pedestal & Noise", "Pedestal & Noise", 650, 650 );
	fFeSummaryCanvas = new TCanvas( "Noise for each FE", "Noise for each FE", 650, 650 );

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

					// now the TGraphErrors
					TString cGraphname = Form( "VplusVcthGraph_Fe%d_Cbc%d", cFeId, cCbcId );
					TGraphErrors* ctmpGraph = dynamic_cast<TGraphErrors*>( gROOT->FindObject( cGraphname ) );
					if ( ctmpGraph ) delete ctmpGraph;
					ctmpGraph = new TGraphErrors();
					ctmpGraph->SetName( cGraphname );
					ctmpGraph->GetXaxis()->SetTitle( "SCurve Midpoint [VCth]" );
					ctmpGraph->GetXaxis()->SetRangeUser( 0, 255 );
					ctmpGraph->GetYaxis()->SetTitle( "Vplus" );
					ctmpGraph->GetYaxis()->SetRangeUser( 0, 255 );
					fGraphMap[cCbc] = ctmpGraph;

					// the fits are initialized when I fit!

					// validiation histograms
					TString cHistname = Form( "Validation_Fe%d_Cbc%d", cFeId, cCbcId );
					TH1F* cHist = dynamic_cast<TH1F*>( gROOT->FindObject( cHistname ) );
					if ( cHist ) delete cHist;
					cHist = new TH1F( cHistname, cHistname, 100, 0, 1 );
					fHistMap[cCbc] = cHist;

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
				TString cNoisehistname =  Form( "Fe%d_Noise", cFeId );
				TH1F* cNoise = new TH1F( cNoisehistname, cNoisehistname, 200, 0, 20 );
				bookHistogram( cFe, "Module_noisehist", cNoise );

				cNoisehistname = Form( "Fe%d_StripNoise", cFeId );
				TProfile* cStripnoise = new TProfile( cNoisehistname, cNoisehistname, ( NCHANNELS * cCbcCount ) + 1, -.5, cCbcCount * NCHANNELS + .5 );
				bookHistogram( cFe, "Module_Stripnoise", cStripnoise );
			}
			fNCbc = cCbcCount;
			fNFe = cFeCount;
		}
	}
	uint32_t cPads = ( cCbcIdMax > cCbcCount ) ? cCbcIdMax : cCbcCount;

	fVplusCanvas->DivideSquare( cPads );
	fVcthVplusCanvas->DivideSquare( cPads );
	fOffsetCanvas->DivideSquare( cPads );
	fValidationCanvas->DivideSquare( cPads );
	fNoiseCanvas->DivideSquare( 2 * cPads );
	fPedestalCanvas->DivideSquare( 2 * cPads );
	fFeSummaryCanvas->DivideSquare( 2 * cPads );

	// now read the settings from the map
	// fHoleMode = fSettingsMap.find( "HoleMode" )->second;
	// fEventsPerPoint = fSettingsMap.find( "Nevents" )->second;
	// fTargetVcth = fSettingsMap.find( "TargetVcth" )->second;
	auto cSetting = fSettingsMap.find( "HoleMode" );
	fHoleMode = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 1;
	cSetting = fSettingsMap.find( "TargetVcth" );
	fTargetVcth = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 120;
	cSetting = fSettingsMap.find( "Nevents" );
	fEventsPerPoint = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 10;
	cSetting = fSettingsMap.find( "FitSCurves" );
	fFitted = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 0;
	cSetting = fSettingsMap.find( "TestPulseAmplitude" );
	fTestPulseAmplitude = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 0;
	if ( fTestPulseAmplitude == 0 )fTestPulse = 0;
	else fTestPulse = 1;
	std::cout << "Created Object Maps and parsed settings:" << std::endl;
	std::cout << "	Hole Mode = " << fHoleMode << std::endl;
	std::cout << "	Nevents = " << fEventsPerPoint << std::endl;
	std::cout << "	TargetVcth = " << int( fTargetVcth ) << std::endl;
	std::cout << "	FitSCurves = " << int( fFitted ) << std::endl;
	std::cout << "  TestPulseAmplitude = " << int( fTestPulseAmplitude ) << std::endl;

}

void FastCalibration::ScanVplus()
{
	// Method to perform a Vplus Scan

	// first set the offset of all Channels to 0x0A
	//setOffset( 0x50,-1 );//change to this function
	std::cout << BOLDBLUE << "Scanning Vplus ..." << RESET << std::endl;
	for ( auto& cTGrpM : fTestGroupChannelMap )
	{
		// enable test group here


		if ( cTGrpM.first == -1 && fdoTGrpCalib )
			continue;
		if ( cTGrpM.first > -1 && !fdoTGrpCalib )
			break;
		// now loop over Vplus values
		std::cout << GREEN << "Enabling Test Group...." << cTGrpM.first << RESET << std::endl;
		setOffset( 0x50, cTGrpM.first );
		for ( auto& cVplus : fVplusVec )
		{
			// then set the correct Vplus
			CbcRegWriter cWriter( fCbcInterface, "Vplus", cVplus );
			accept( cWriter );

			std::cout << BOLDRED << "Vplus = " << int( cVplus ) << RESET << std::endl;

			// now initialize the Scurves
			initializeSCurves( "Vplus", cVplus, cTGrpM.first );

			// measure the SCurves, the false is indicating that I am sweeping Vcth
			measureSCurves( false, cTGrpM.first );

			// now process the measured SCuvers, true indicates that I am drawing, the TGraphErrors with Vcth vs Vplus are also filled
			processSCurves( "Vplus", cVplus, true, cTGrpM.first );
		}
		// After finishing with one test group, disable it again
		std::cout << BLUE << "Disabling Test Group...." << cTGrpM.first << RESET << std::endl;
		uint8_t cOffset = ( fHoleMode ) ? 0xFF : 0x00;
		setOffset( cOffset, cTGrpM.first );
	}
	std::cout << BOLDBLUE << "Finished scanning Vplus ..." << std::endl << "Now Fitting ... " << RESET << std::endl;

	//  fit the Vcth vs Vplus graphs
	findVplus( true );

}

void FastCalibration::measureNoise()
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


void FastCalibration::ScanOffset()
{

	// Method to tune the individual channel Offsets

	std::cout << BOLDBLUE << "Scanning Offsets..." << RESET << std::endl;
	for ( auto& cTGrpM : fTestGroupChannelMap )
	{
		if ( cTGrpM.first == -1 && fdoTGrpCalib )
			continue;
		if ( cTGrpM.first > -1 && !fdoTGrpCalib )
			break;
		std::cout << "Enabling Test Group...." << cTGrpM.first << std::endl;
		if ( fdoBitWisetuning )
		{
			uint8_t cOffset = ( fHoleMode ) ? 0xFF : 0x00;
			setOffset( cOffset, cTGrpM.first );
			// for bit 7 to 0
			for ( int cBit = 7; cBit >= 0; cBit-- )
			{
				std::cout << GREEN << "Toggling Bit " << cBit << RESET << std::endl;
				toggleOffsetBit( cBit, cTGrpM.first );
				initializeSCurves( "OffsetBit", cBit, cTGrpM.first );
				measureSCurves( false, cTGrpM.first );
				processSCurvesOffset( "OffsetBit", cBit, true, cTGrpM.first ); // this extracts the pedestal per channel, compares and flips back if necessary
			}
		}
		else
		{
			CbcRegReader cReader( fCbcInterface, "Vplus" );
			accept( cReader );

			CbcRegReader aReader( fCbcInterface, "VCth" );
			accept( aReader );
			initializeSCurves( "Offset", 0x00, cTGrpM.first );

			// // the true indicates that this time I am sweeping Offsets instead of Vcth
			measureSCurves( true, cTGrpM.first );

			processSCurves( "Offset", 0x00, true, cTGrpM.first );
			// set offset accordingly (check for mode, then either 0 or 255)

		}
		std::cout << "Disabling Test Group...." << cTGrpM.first << std::endl;
	}
	// reset VCth to the fTargetVcth
	if ( fdoBitWisetuning )
	{
		CbcRegWriter cWriter( fCbcInterface, "VCth", fTargetVcth );
		accept( cWriter );
	}
	std::cout << BOLDBLUE << "Finished scanning Offsets ..." << RESET << std::endl;

}

void FastCalibration::Validate()
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
				fCbcInterface->ReadAllCbc( cFe );

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

	uint32_t cTotalEvents = 2000;

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS


/*Currently this function sets offset for all 1-254 channels. But now to add testgroups, it has to set for 32
channels in the group only. So it has to take the group id as well.
Change to void FastCalibration::setOffset( uint8_t pOffset , int TGrpId )
Then by accessing the strip numbers for that particluar group it will set the offsets
*/////
void FastCalibration::setOffset( uint8_t pOffset, int  pTGrpId )
{

	// sets the offset to pOffset on each channel
	struct OffsetWriter : public HwDescriptionVisitor
	{
		CbcInterface* fInterface;
		RegisterVector fRegVec;
		uint8_t fOffset;
		std::vector<uint8_t> fTestGrpChannelIdVec;
		//will have to pass the channel vector to OffsetWriter
		OffsetWriter( CbcInterface* pInterface, uint8_t pOffset, std::vector<uint8_t> pTestGroupChnlVec ): fInterface( pInterface ),  fOffset( pOffset ) , fTestGrpChannelIdVec( pTestGroupChnlVec ) {
			//Here the loop will be over channels in the test group
			for ( auto& cChannel : fTestGrpChannelIdVec ) {
				TString cRegName = Form( "Channel%03d", cChannel + 1 );
				fRegVec.push_back( { cRegName.Data(), fOffset } );
			}
		}
		void visit( Cbc& pCbc ) {
			fInterface->WriteCbcMultReg( &pCbc, fRegVec );
		}
	};

	if ( fTestGroupChannelMap.find( pTGrpId ) != fTestGroupChannelMap.end() )
	{
		OffsetWriter cWriter( fCbcInterface, pOffset, fTestGroupChannelMap[pTGrpId] );
		accept( cWriter );
	}
}

void FastCalibration::enableTestGroupforNoise( int  pTGrpId )
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
					auto cOrigOffsetMap = fOffsetMap.find(cCbc);
					if(cOrigOffsetMap == std::end(fOffsetMap))std::cout << "Error, could not find the original offset map for CBC " << cCbcId << std::endl;
					// cOrigOffsetMap.second is the map of channel number vs original offset

					RegisterVector cRegVec;
					// iterate the groups (first is ID, second is vec<uint8_t>)
					for ( auto& cGrp : fTestGroupChannelMap )
					{
						// if grpid = -1, do nothing (all channels)
						if(cGrp.first == -1) continue;
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
						else if (cGrp.first == pTGrpId)
						{
							// iterate over the channels in the test group and find the corresponding offset in the original offset map
							for ( auto& cChan : cGrp.second )
							{
								auto cChanOffset = cOrigOffsetMap->second.find(cChan);
								if(cChanOffset == std::end(cOrigOffsetMap->second)) std::cout << "Error, could not find original offset for channel " << +cChan << "on CBC " << cCbcId << std::endl;
								TString cRegName = Form( "Channel%03d", cChan + 1 );
								cRegVec.push_back( { cRegName.Data(), cChanOffset->second } );
								// std::cout << GREEN << "DEBUG CBC " << cCbcId << " Channel " << +cChan << " group " << cGrp.first << " offset " << +cChanOffset->second << RESET << std::endl;
							}
						}
					}

					// now I should have 0 or FF as offset for all channels except the one in my test group
					// this now needs to be written to the CBCs
					fCbcInterface->WriteCbcMultReg(cCbc, cRegVec); 
				}
			}
		}
	}
	std::cout << "Disabling all TGroups except " << pTGrpId << " ! " << std::endl;
}

void FastCalibration::toggleOffsetBit( uint8_t pBit, int  pTGrpId )
{
	// this method reads the offset for each channel, toggles bit I and writes it back out
	// sets the offset to pOffset on each channel
	//pass the vector of channels to the OffsetToggler
	struct OffsetToggler : public HwDescriptionVisitor
	{
		CbcInterface* fInterface;
		RegisterVector fRegVec;
		std::vector<std::string> fReadVec;
		// uint8_t fOffset;
		uint8_t fBit;
		std::vector<uint8_t> fTestGrpChannelIdVec;
		OffsetToggler( CbcInterface* pInterface, uint8_t pBit, std::vector<uint8_t> pTestGroupChnlVec ): fInterface( pInterface ),  fBit( pBit ), fTestGrpChannelIdVec( pTestGroupChnlVec ) {
			// for ( uint8_t cChannel = 1; cChannel < 255; cChannel++ ) {
			//  TString cRegName = Form( "Channel%03d", cChannel );
			//  fReadVec.push_back( cRegName.Data() );
			// }
		}
		void visit( Cbc& pCbc ) {
			//  read all Channel Offset Registers
			// fInterface->ReadCbcMultReg( &pCbc, fReadVec );
			// now loop over the channels, get the register value, flip the target bit and encode it in the write vec
			//Loop over the channels in this particular vector of the this testgroup

			for ( auto& cChannel : fTestGrpChannelIdVec ) {
				TString cRegName = Form( "Channel%03d", cChannel + 1 );
				uint8_t cOffset = pCbc.getReg( cRegName.Data() );
				//std::cout << "DEBUG " << cRegName.Data() << " was " << std::hex << "0x" << int( cOffset );
				cOffset  ^= ( 1 << fBit );
				//std::cout << " is 0x" << int( cOffset ) << std::dec << std::endl;
				fRegVec.push_back( { cRegName.Data(), cOffset } );
			}
			fInterface->WriteCbcMultReg( &pCbc, fRegVec );
		}
	};
	if ( fTestGroupChannelMap.find( pTGrpId ) != fTestGroupChannelMap.end() )
	{
		OffsetToggler cToggler( fCbcInterface, pBit, fTestGroupChannelMap[pTGrpId] );
		accept( cToggler );
	}
}


void FastCalibration::measureSCurves( bool pOffset, int  pTGrpId )
{
	// Adaptive Loop to measure SCurves

	if ( pOffset ) std::cout << BOLDGREEN << "Measuring SCurves sweeping Channel Offsets ... " << RESET << std::endl;
	else std::cout << BOLDGREEN << "Measuring SCurves sweeping VCth ... " << RESET <<  std::endl;

	// Necessary variables
	bool cNonZero = false;
	bool cAllOne = false;
	uint32_t cAllOneCounter = 0;
	uint8_t cValue, cStartValue, cDoubleValue;
	int cStep;

	// the expression below mimics XOR
	if ( !pOffset != !fHoleMode )
	{
		cStartValue = cValue = 0xFF;
		cStep = -10;
	}
	else
	{
		cStartValue = cValue = 0x00;
		cStep = 10;
	}

	// Adaptive VCth loop
	while ( 0x00 <= cValue && cValue <= 0xFF )
	{
		// DEBUG
		if ( cAllOne ) break;
		if ( cValue == cDoubleValue )
		{
			cValue +=  cStep;
			continue;
		}

		// This decides if the SCurve is done sweeping Vcth or Offsets
		if ( !pOffset )
		{
			CbcRegWriter cWriter( fCbcInterface, "VCth", cValue );
			accept( cWriter );
		}
		else
			setOffset( cValue, pTGrpId ); //need to pass on the testgroup

		uint32_t cN = 1;
		uint32_t cNthAcq = 0;
		uint32_t cHitCounter = 0;

		for ( auto cShelve : fShelveVector )
		{
			// DEBUG
			if ( cAllOne ) break;

			for ( BeBoard* pBoard : cShelve->fBoardVector )
			{
				Counter cCounter;
				pBoard->accept( cCounter );

				fBeBoardInterface->Start( pBoard );

				while ( cN <= fEventsPerPoint )
				{
					// Run( pBoard, cNthAcq );
					fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
					const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );

					// Loop over Events from this Acquisition
					for ( auto& ev : events )
					{
						cHitCounter += fillSCurves( pBoard, ev, cValue, pTGrpId ); //pass test group here
						cN++;
					}
					cNthAcq++;
				}
				fBeBoardInterface->Stop( pBoard, cNthAcq );

				// if ( pOffset ) std::cout << "Offset " << int( cValue ) << " Hits: " << cHitCounter << std::endl;
				std::cout << "DEBUG Vcth " << int( cValue ) << " Hits " << cHitCounter << " and should be " <<  0.95 * fEventsPerPoint  * cCounter.getNCbc() * fTestGroupChannelMap[pTGrpId].size() << std::endl;

				// check if the hitcounter is all ones
				if ( cNonZero == false && cHitCounter != 0 )
				{
					cDoubleValue = cValue;
					cNonZero = true;
					if ( ( cStep > 0 && cValue > 0x14 ) || ( cStep < 0 && cValue < 0xEB ) ) cValue -= 2 * cStep;
					else if(cValue == 255) cValue = 255; 
					else if(cValue == 0) cValue =0;
					else cValue -= cStep;
					cStep /= 10;
					std::cout << GREEN << "Found > 0 Hits!, Falling back to " << +cValue  <<  RESET << std::endl;
					continue;
				}
				// the above counter counted the CBC objects connected to pBoard
				if ( cHitCounter > 0.95 * fEventsPerPoint  * fNCbc * fTestGroupChannelMap[pTGrpId].size() ) cAllOneCounter++;
				if ( cAllOneCounter >= 10 )
				{
					cAllOne = true;
					std::cout << RED << "Found maximum occupancy 10 times, SCurves finished! " << RESET << std::endl;
				}
				if ( cAllOne ) break;
				cValue += cStep;
			}
		}
	} // end of VCth loop
	if ( pOffset ) setOffset( cStartValue, pTGrpId );
}

void FastCalibration::initializeSCurves( TString pParameter, uint8_t pValue, int  pTGrpId )
{
	// Just call the initializeHist method of every channel and tell it what we are varying
	for ( auto& cCbc : fCbcChannelMap )
	{
		std::vector<uint8_t> cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];
		for ( auto& cChanId : cTestGrpChannelVec )
			( cCbc.second.at( cChanId ) ).initializeHist( pValue, pParameter );
	}
	std::cout << "SCurve Histograms for " << pParameter << " =  " << int( pValue ) << " initialized!" << std::endl;
}

uint32_t FastCalibration::fillSCurves( BeBoard* pBoard,  const Event* pEvent, uint8_t pValue, int  pTGrpId, bool pDraw )
{
	// loop over all FEs on board, check if channels are hit and if so , fill pValue in the histogram of Channel
	uint32_t cHitCounter = 0;
	for ( auto cFe : pBoard->fModuleVector )
	{

		for ( auto cCbc : cFe->fCbcVector )
		{

			CbcChannelMap::iterator cChanVec = fCbcChannelMap.find( cCbc );
			if ( cChanVec != fCbcChannelMap.end() )
			{
				const std::vector<uint8_t>& cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];
				for ( auto& cChanId : cTestGrpChannelVec )
				{
					if ( pEvent->DataBit( cFe->getFeId(), cCbc->getCbcId(), cChanVec->second.at( cChanId ).fChannelId ) )
					{
						cChanVec->second.at( cChanId ).fillHist( pValue );
						cHitCounter++;
					}

				}
			}
			else std::cout << RED << "Error: could not find the channels for CBC " << int( cCbc->getCbcId() ) << RESET << std::endl;
		}
	}
	return cHitCounter;
}

void FastCalibration::processSCurves( TString pParameter, uint8_t pValue, bool pDraw, int  pTGrpId )
{
	bool cOffset = ( pParameter == "Vplus" ) ? false : true;
	// First fitHits for every Channel, then extract the midpoint and noise and fill it in fVplusVcthGraphMap
	for ( auto& cCbc : fCbcChannelMap )
	{

		// this happens if I am scanning VCth
		// Find the Graph
		GraphMap::iterator cGraph = fGraphMap.find( cCbc.first );
		if ( !cOffset && cGraph == fGraphMap.end() ) std::cout << "Could not find the correct graph for Fe " << int( cCbc.first->getFeId() ) << " Cbc " << int( cCbc.first->getCbcId() ) << std::endl;

		// Loop the Channels
		bool cFirst = true;
		TString cOption;

		// Reg Vec for Offset Writing
		RegisterVector cRegVec;
		std::vector<uint8_t> cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];
		for ( auto& cChanId : cTestGrpChannelVec )
		{
			//for ( auto& cChan : cCbc.second )
			bool cFitMode;
			Channel cChan = cCbc.second.at( cChanId );
			// the expression below mimics XOR
			if ( !fHoleMode != !cOffset ) cFitMode = true;
			else cFitMode = false;

			// Fit or Differentiate
			if ( fFitted )
				cChan.fitHist( fEventsPerPoint, cFitMode, pValue, pParameter, fResultFile );
			else cChan.differentiateHist( fEventsPerPoint, cFitMode, pValue, pParameter, fResultFile );

			if ( !cOffset )
			{
				// Put in VplusVcthGraph
				cGraph->second->SetPoint( cGraph->second->GetN(), cChan.getPedestal(), pValue );
				cGraph->second->SetPointError( cGraph->second->GetN() - 1, cChan.getNoise(), 0 );
				if ( cChan.getPedestal() == 0 ) std::cout << RED << "Error, SCurve Fit for Fe " << int( cCbc.first->getFeId() ) << " Cbc " << int( cCbc.first->getCbcId() ) << " Channel " << int( cChan.fChannelId ) << " did not work correctly!" << RESET << std::endl;
			}
			else
			{
				// this happens when I sweep offsets!, channel Id +1 because chennel registers start with 1 on the CBC!
				uint8_t cOffsetVal;
				if ( cChan.getPedestal() > 0 && cChan.getPedestal() < 256 )
				{
					cOffsetVal = std::round( cChan.getPedestal() );
					std::cout << "Offset for Channel " << int( cChan.fChannelId ) << " on CBC " << int( cChan.fCbcId ) << "  : 0x" << std::hex <<  int( std::round( cChan.getPedestal() ) )  <<  std::dec << std::endl;
				}
				else
				{
					cOffsetVal = ( fHoleMode ) ? 0xFF : 0x00;
					std::cout << RED << "Error: Failed to extract Offset for Channel " << int( cChan.fChannelId ) << " on CBC " << int( cChan.fCbcId ) << "  -- disabling! (Fit: " <<  int( cChan.getPedestal() ) << ")" <<  RESET << std::endl;
				}
				cRegVec.push_back( { Form( "Channel%03d", cChan.fChannelId + 1 ), cOffsetVal } );
			}
			//Draw
			if ( pDraw )
			{
				if ( cFirst )
				{
					cOption = "P" ;
					cFirst = false;
				}
				else cOption = "P same";

				if ( !cOffset ) fVplusCanvas->cd( cCbc.first->getCbcId() + 1 );
				else fOffsetCanvas->cd( cCbc.first->getCbcId() + 1 );
				cChan.fScurve->Draw( cOption );
				if ( fFitted )
					cChan.fFit->Draw( "same" );
				else cChan.fDerivative->Draw( "same" );
			}
		}
		if ( pDraw && ! cOffset )
		{
			fVplusCanvas->Update();
#ifdef __HTTP__
			fHttpServer->ProcessRequests();
#endif
		}
		else
		{
			fOffsetCanvas->Update();
#ifdef __HTTP__
			fHttpServer->ProcessRequests();
#endif
		}
		// if ( cOffset ) mypause();

		// writing offset midpoints to CBC
		if ( cOffset )
		{
			fCbcInterface->WriteCbcMultReg( cCbc.first, cRegVec );
			std::cout << RED << "Offset Values written to CBC " << int( cCbc.first->getCbcId() ) << RESET << std::endl;
		}
	}

}

void FastCalibration::processSCurvesOffset( TString pParameter, uint8_t pTargetBit, bool pDraw, int pTGrpId )
{
	// process the Scurves: get the pedestal, check if larger or smaller than target Vcth and then toggle back
	for ( auto& cCbc : fCbcChannelMap )
	{

		// Loop the Channels
		bool cFirst = true;
		TString cOption;

		// Reg Vec for Offset Writing
		RegisterVector cRegVec;

		std::vector<uint8_t> cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];
		for ( auto& cChanId : cTestGrpChannelVec )
		{
			//for ( auto& cChan : cCbc.second ) {
			// Fit
			Channel cChan = cCbc.second.at( cChanId );
			if ( fFitted )
				cChan.fitHist( fEventsPerPoint, !fHoleMode, pTargetBit, pParameter, fResultFile );
			else cChan.differentiateHist( fEventsPerPoint, !fHoleMode, pTargetBit, pParameter, fResultFile );

			// check if the pedestal is larger than the targetVcth
			// if so, flip bit back down
			uint8_t cCurrentOffset = cCbc.first->getReg( Form( "Channel%03d", cChan.fChannelId + 1 ) );
			if ( ( fHoleMode && int( cChan.getPedestal() + 0.5 ) < fTargetVcth ) ||
					( !fHoleMode && int( cChan.getPedestal() + 0.5 ) > fTargetVcth ) ) cCurrentOffset ^= ( 1 << pTargetBit );
			std::cout <<  "Pedestal is " << int( cChan.getPedestal() + 0.5 ) << " and target Vcth is " << int( fTargetVcth ) << " Offset Value " << +cCurrentOffset << std::endl;
			cChan.setOffset( cCurrentOffset );
			cRegVec.push_back( { Form( "Channel%03d", cChan.fChannelId + 1 ), cCurrentOffset } );


			//Draw
			if ( pDraw )
			{
				if ( cFirst )
				{
					cOption = "P" ;
					cFirst = false;
				}
				else cOption = "P same";

				fOffsetCanvas->cd( cCbc.first->getCbcId() + 1 );
				cChan.fScurve->DrawCopy( cOption );
				if ( fFitted )
					cChan.fFit->DrawCopy( "same" );
				else cChan.fDerivative->DrawCopy( "same" );
			}
		}
		if ( pDraw )
		{
			fOffsetCanvas->Update();
#ifdef __HTTP__
			fHttpServer->ProcessRequests();
#endif
		}
		fCbcInterface->WriteCbcMultReg( cCbc.first, cRegVec );
	}
}

void FastCalibration::processSCurvesNoise( TString pParameter, uint8_t pValue, bool pDraw, int  pTGrpId )
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

void FastCalibration::findVplus( bool pDraw )
{
	// fit the VplusVcthGraph and evaluate at fTargetVcth, then use the key of the map I am iterating (Cbc*) with fCbcInterface to write the object
	for ( auto& cGraph : fGraphMap )
	{


		cGraph.second->Fit( "pol1" , "Q+" );
		TF1* cFit = cGraph.second->GetFunction( "pol1" );
		fFitMap[cGraph.first] = cFit;
		cFit->SetName( Form( "VplusVcthFit_Fe%d_Cbc%d", cGraph.first->getFeId(), cGraph.first->getCbcId() ) );

		if ( pDraw )
		{

			fVcthVplusCanvas->cd( cGraph.first->getCbcId() + 1 );
			cGraph.second->Draw( "AP" );
			cFit->Draw( "same" );
			// cTmpFit->Draw( "same" );
			fVcthVplusCanvas->Update();
#ifdef __HTTP__
			fHttpServer->ProcessRequests();
#endif
		}

		// now evaluate the fit at fTargetVcth and write to Cbcs
		uint8_t cVplusResult = ( uint8_t )( cFit->GetParameter( 1 ) * fTargetVcth + cFit->GetParameter( 0 ) );
		std::cout << BOLDBLUE << "TargetVCth = " << int( fTargetVcth ) << " -> Vplus = " << int( cVplusResult ) << RESET << std::endl;

		RegisterVector cRegVec;
		cRegVec.push_back( { "VCth", fTargetVcth } );
		cRegVec.push_back( { "Vplus", cVplusResult } );
		fCbcInterface->WriteCbcMultReg( cGraph.first, cRegVec );
	}

}

void FastCalibration::setSystemTestPulse( uint8_t pTPAmplitude, uint8_t pTestGroup )
{
	std::vector<std::pair<std::string, uint8_t>> cRegVec;
	uint8_t cRegValue =  to_reg( 0, pTestGroup );

	cRegVec.push_back( std::make_pair( "SelTestPulseDel&ChanGroup",  cRegValue ) );

	//set the value of test pulsepot registrer and MiscTestPulseCtrl&AnalogMux register
	if ( fHoleMode )
		cRegVec.push_back( std::make_pair( "MiscTestPulseCtrl&AnalogMux", 0xD1 ) );
	else
		cRegVec.push_back( std::make_pair( "MiscTestPulseCtrl&AnalogMux", 0x61 ) );

	cRegVec.push_back( std::make_pair( "TestPulsePot", pTPAmplitude ) );
	// cRegVec.push_back( std::make_pair( "Vplus",  fVplus ) );
	CbcMultiRegWriter cWriter( fCbcInterface, cRegVec );
	this->accept( cWriter );
	CbcRegReader cReader (fCbcInterface, "MiscTestPulseCtrl&AnalogMux");
	this->accept(cReader);
	cReader.setRegister("TestPulsePot");
	this->accept(cReader);

}

void FastCalibration::writeGraphs()
{
	// just use auto iterators to write everything to disk
	// this is the old method before Tool class was cool
	fResultFile->cd();
	for ( const auto& cGraph : fGraphMap )
		cGraph.second->Write( cGraph.second->GetName(), TObject::kOverwrite );
	for ( const auto& cFit : fFitMap )
		cFit.second->Write( cFit.second->GetName(), TObject::kOverwrite );
	// for ( const auto& cCanvas : fCanvasMap )
	// 	cCanvas.second->Write( cCanvas.second->GetName(), TObject::kOverwrite );
	for ( const auto& cHist : fHistMap )
		cHist.second->SetDirectory( fResultFile );
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

	fResultFile->cd();

	// This is re-implementing the new method for the FE's since they use the cool stuff from tool
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

	// Save canvasses too
	fVplusCanvas->Write( fVplusCanvas->GetName(), TObject::kOverwrite );
	fVcthVplusCanvas->Write( fVcthVplusCanvas->GetName(), TObject::kOverwrite );
	fOffsetCanvas->Write( fOffsetCanvas->GetName(), TObject::kOverwrite );
	fValidationCanvas->Write( fValidationCanvas->GetName(), TObject::kOverwrite );
	fNoiseCanvas->Write( fNoiseCanvas->GetName(), TObject::kOverwrite );
	fPedestalCanvas->Write( fPedestalCanvas->GetName(), TObject::kOverwrite );
	fFeSummaryCanvas->Write( fFeSummaryCanvas->GetName(), TObject::kOverwrite );

}

void FastCalibration::dumpConfigFiles()
{
	// visitor to call dumpRegFile on each Cbc
	struct RegMapDumper : public HwDescriptionVisitor
	{
		std::string fDirectoryName;
		RegMapDumper( std::string pDirectoryName ): fDirectoryName( pDirectoryName ) {};
		void visit( Cbc& pCbc ) {
			if ( !fDirectoryName.empty() ) {
				TString cFilename = fDirectoryName + Form( "/FE%dCBC%d.txt", pCbc.getFeId(), pCbc.getCbcId() );
				// cFilename += Form( "/FE%dCBC%d.txt", pCbc.getFeId(), pCbc.getCbcId() );
				pCbc.saveRegMap( cFilename.Data() );
			}
			else std::cout << "Error: no results Directory initialized! "  << std::endl;
		}
	};

	RegMapDumper cDumper( fDirectoryName );
	accept( cDumper );

	std::cout << BOLDBLUE << "Configfiles for all Cbcs written to " << fDirectoryName << RESET << std::endl;
}

void FastCalibration::saveInitialOffsets()
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

					for(uint8_t cChan = 0; cChan < NCHANNELS; cChan++)
					{
						TString cRegName = Form( "Channel%03d", cChan + 1 );
						uint8_t cOffset = cCbc->getReg(cRegName.Data());	
						cCbcOffsetMap[cChan] = cOffset;
						// std::cout << "DEBUG Original Offset for CBC " << cCbcId << " channel " << +cChan << " " << +cOffset << std::endl;				
					}
					fOffsetMap[cCbc] = cCbcOffsetMap;
				}
			}
		}
	}
}
