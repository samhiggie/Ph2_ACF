#include "HybridTester.h"

// fill the Histograms, count the hits and increment Vcth

struct HistogramFiller  : public HwDescriptionVisitor
{
	TH1F* fBotHist;
	TH1F* fTopHist;
	const Event* fEvent;

	HistogramFiller( TH1F* pBotHist, TH1F* pTopHist, const Event* pEvent ): fBotHist( pBotHist ), fTopHist( pTopHist ), fEvent( pEvent ) {}

	void visit( Cbc& pCbc ) {
		std::vector<bool> cDataBitVector = fEvent->DataBitVector( pCbc.getFeId(), pCbc.getCbcId() );
		for ( uint32_t cId = 0; cId < NCHANNELS; cId++ ) {
			if ( cDataBitVector.at( cId ) ) {
				uint32_t globalChannel = ( pCbc.getCbcId() * 254 ) + cId;
				//              std::cout << "Channel " << globalChannel << " VCth " << int(pCbc.getReg( "VCth" )) << std::endl;
				// find out why histograms are not filling!
				if ( globalChannel % 2 == 0 )
					fBotHist->Fill( globalChannel / 2 );
				else
					fTopHist->Fill( ( globalChannel - 1 ) / 2 );

			}
		}
	}
};


void HybridTester::InitializeHists()
{
	TString cFrontName( "fHistTop" );
	fHistTop = ( TH1F* )( gROOT->FindObject( cFrontName ) );
	if ( fHistTop ) delete fHistTop;

	fHistTop = new TH1F( cFrontName, "Front Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ) + 1, -0.5, ( fNCbc / 2 * 254 ) + .5 );
	fHistTop->SetFillColor( 4 );
	fHistTop->SetFillStyle( 3001 );

	TString cBackName( "fHistBottom" );
	fHistBottom = ( TH1F* )( gROOT->FindObject( cBackName ) );
	if ( fHistBottom ) delete fHistBottom;

	fHistBottom = new TH1F( cBackName, "Back Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ) + 1 , -0.5, ( fNCbc / 2 * 254 ) + .5 );
	fHistBottom->SetFillColor( 4 );
	fHistBottom->SetFillStyle( 3001 );

	// Now the Histograms for SCurves
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

					TString cName = Form( "SCurve_Fe%d_Cbc%d", cFeId, cCbcId );
					TObject* cObject = static_cast<TObject*>( gROOT->FindObject( cName ) );
					if ( cObject ) delete cObject;
					TH1F* cTmpScurve = new TH1F( cName, Form( "Noise Occupancy Cbc%d; VCth; Counts", cCbcId ), 255, 0, 255 );
					cTmpScurve->SetMarkerStyle( 8 );
					fSCurveMap[cCbc] = cTmpScurve;

					cName = Form( "SCurveFit_Fe%d_Cbc%d", cFeId, cCbcId );
					cObject = static_cast<TObject*>( gROOT->FindObject( cName ) );
					if ( cObject ) delete cObject;
					TF1* cTmpFit = new TF1( cName, MyErf, 0, 255, 2 );
					fFitMap[cCbc] = cTmpFit;
				}
			}
		}
	}
}

void HybridTester::InitialiseSettings()
{
	auto cSetting = fSettingsMap.find( "Threshold_NSigmas" );
	fSigmas = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 4;
	cSetting = fSettingsMap.find( "Nevents" );
	fTotalEvents = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 999;
	cSetting = fSettingsMap.find( "HoleMode" );
	fHoleMode = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : true;


	// std::cout << "Read the following Settings: " << std::endl;
	// std::cout << "Hole Mode: " << fHoleMode << std::endl << "NEvents: " << fTotalEvents << std::endl << "NSigmas: " << fSigmas << std::endl;
}
void HybridTester::InitialiseGUI( int pVcth, int pNevents, bool pTestreg, bool pScanthreshold, bool pHolemode )
{
	fThresholdScan = pScanthreshold;
	fTotalEvents = pNevents;
	fHoleMode = pHolemode;
	fVcth = pVcth;

	CbcRegWriter cWriter( fCbcInterface, "VCth", fVcth );
	accept( cWriter ); //TODO pass safe

	gStyle->SetOptStat( 000000 );
	gStyle->SetTitleOffset( 1.3, "Y" );
	//  special Visito class to count objects
	Counter cCbcCounter;
	accept( cCbcCounter );
	fNCbc = cCbcCounter.getNCbc();

	fDataCanvas = new TCanvas( "fDataCanvas", "SingleStripEfficiency", 1200, 800 );
	fDataCanvas->Divide( 2 );

	if ( fThresholdScan )
	{
		fSCurveCanvas = new TCanvas( "fSCurveCanvas", "Noise Occupancy as function of VCth" );
		fSCurveCanvas->Divide( fNCbc );
	}
	InitializeHists();
}


void HybridTester::Initialize( bool pThresholdScan )
{
	fThresholdScan = pThresholdScan;
	gStyle->SetOptStat( 000000 );
	gStyle->SetTitleOffset( 1.3, "Y" );
	//  special Visito class to count objects
	Counter cCbcCounter;
	accept( cCbcCounter );
	fNCbc = cCbcCounter.getNCbc();

	fDataCanvas = new TCanvas( "fDataCanvas", "SingleStripEfficiency", 1200, 800 );
	fDataCanvas->Divide( 2 );

	if ( fThresholdScan )
	{
		fSCurveCanvas = new TCanvas( "fSCurveCanvas", "Noise Occupancy as function of VCth" );
		fSCurveCanvas->Divide( fNCbc );
	}
	InitializeHists();
	InitialiseSettings();
}


void HybridTester::ScanThreshold()
{
	std::cout << "Scanning noise Occupancy to find threshold for test with external source ... " << std::endl;

	// Necessary variables
	uint32_t cEventsperVcth = 10;
	bool cNonZero = false;
	bool cAllOne = false;
	bool cSlopeZero = false;
	uint32_t cAllOneCounter = 0;
	uint32_t cSlopeZeroCounter = 0;
	uint32_t cOldHitCounter = 0;
	uint8_t  cDoubleVcth;
	uint8_t cVcth = ( fHoleMode ) ?  0xFF :  0x00;
	int cStep = ( fHoleMode ) ? -10 : 10;

	// Adaptive VCth loop
	while ( 0x00 <= cVcth && cVcth <= 0xFF )
	{
		if ( cAllOne ) break;
		if ( cVcth == cDoubleVcth )
		{
			cVcth +=  cStep;
			continue;
		}
		// Set current Vcth value on all Cbc's
		CbcRegWriter cWriter( fCbcInterface, "VCth", cVcth );
		accept( cWriter );
		uint32_t cN = 1;
		uint32_t cNthAcq = 0;
		uint32_t cHitCounter = 0;

		// maybe restrict to pBoard? instead of looping?
		for ( auto& cShelve : fShelveVector )
		{
			if ( cAllOne ) break;
			for ( BeBoard* pBoard : cShelve->fBoardVector )
			{
				fBeBoardInterface->Start( pBoard );
				while ( cN <=  cEventsperVcth )
				{
					// Run( pBoard, cNthAcq );
					fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
					const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );

					// Loop over Events from this Acquisition
					for ( auto& cEvent : events )
					{
						// loop over Modules & Cbcs and count hits separately
						cHitCounter += fillSCurves( pBoard,  cEvent, cVcth );
						cN++;
					}
					cNthAcq++;
				}
				fBeBoardInterface->Stop( pBoard, cNthAcq );
				// std::cout << +cVcth << " " << cHitCounter << std::endl;
				// Draw the thing after each point
				updateSCurveCanvas( pBoard );

				// check if the hitcounter is all ones

				if ( cNonZero == false && cHitCounter != 0 )
				{
					cDoubleVcth = cVcth;
					cNonZero = true;
					cVcth -= 2 * cStep;
					cStep /= 10;
					continue;
				}
				if ( cNonZero && cHitCounter != 0 )
				{
					// check if all Cbcs have reached full occupancy
					if ( cHitCounter > 0.95 * cEventsperVcth * fNCbc * NCHANNELS ) cAllOneCounter++;
					// add a second check if the global SCurve slope is 0 for 10 consecutive Vcth values
					// if ( fabs( cHitCounter - cOldHitCounter ) < 10 && cHitCounter != 0 ) cSlopeZeroCounter++;
				}
				if ( cAllOneCounter >= 10 ) cAllOne = true;
				// if ( cSlopeZeroCounter >= 10 ) cSlopeZero = true;

				if ( cAllOne )
				{
					std::cout << "All strips firing -- ending the scan at VCth " << +cVcth << std::endl;
					break;
				}
				// else if ( cSlopeZero )
				// {
				//   std::cout << "Slope of SCurve 0 -- ending the scan at VCth " << +cVcth << std::endl;
				//  break;
				// }

				cOldHitCounter = cHitCounter;
				cVcth += cStep;
			}
		}
	}

	// Fit and save the SCurve & Fit - extract the right threshold
	// TODO
	processSCurves( cEventsperVcth );
}

void HybridTester::processSCurves( uint32_t pEventsperVcth )
{
	for ( auto cScurve : fSCurveMap )
	{
		fSCurveCanvas->cd( cScurve.first->getCbcId() + 1 );

		cScurve.second->Scale( 1 / double_t( pEventsperVcth * NCHANNELS ) );
		cScurve.second->Draw( "P" );
		// Write to file
		cScurve.second->Write( cScurve.second->GetName(), TObject::kOverwrite );

		// Estimate parameters for the Fit
		double cFirstNon0( 0 );
		double cFirst1( 0 );

		// Not Hole Mode
		if ( !fHoleMode )
		{
			for ( Int_t cBin = 1; cBin <= cScurve.second->GetNbinsX(); cBin++ )
			{
				double cContent = cScurve.second->GetBinContent( cBin );
				if ( !cFirstNon0 )
				{
					if ( cContent ) cFirstNon0 = cScurve.second->GetBinCenter( cBin );
				}
				else if ( cContent == 1 )
				{
					cFirst1 = cScurve.second->GetBinCenter( cBin );
					break;
				}
			}
		}
		// Hole mode
		else
		{
			for ( Int_t cBin = cScurve.second->GetNbinsX(); cBin >= 1; cBin-- )
			{
				double cContent = cScurve.second->GetBinContent( cBin );
				if ( !cFirstNon0 )
				{
					if ( cContent ) cFirstNon0 = cScurve.second->GetBinCenter( cBin );
				}
				else if ( cContent == 1 )
				{
					cFirst1 = cScurve.second->GetBinCenter( cBin );
					break;
				}
			}
		}

		// Get rough midpoint & width
		double cMid = ( cFirst1 + cFirstNon0 ) * 0.5;
		double cWidth = ( cFirst1 - cFirstNon0 ) * 0.5;

		// find the corresponding fit
		auto cFit = fFitMap.find( cScurve.first );
		if ( cFit == std::end( fFitMap ) ) std::cout << "Error: could not find Fit for Cbc " << int( cScurve.first->getCbcId() ) << std::endl;
		else
		{
			// Fit
			cFit->second->SetParameter( 0, cMid );
			cFit->second->SetParameter( 1, cWidth );

			cScurve.second->Fit( cFit->second, "RNQ+" );
			cFit->second->Draw( "same" );

			// Write to File
			cFit->second->Write( cFit->second->GetName(), TObject::kOverwrite );

			// TODO
			// Set new VCth - for the moment each Cbc gets his own Vcth - I shold add a mechanism to take one that works for all!
			double_t pedestal = cFit->second->GetParameter( 0 );
			double_t noise = cFit->second->GetParameter( 1 );

			uint8_t cThreshold = ceil( pedestal + fSigmas * fabs( noise ) );

			std::cout << "Identified a noise Occupancy of 50% at VCth " << static_cast<int>( pedestal ) << " -- increasing by " << fSigmas <<  " sigmas (=" << fabs( noise ) << ") to " << +cThreshold << " for Cbc " << int( cScurve.first->getCbcId() ) << std::endl;

			TLine* cLine = new TLine( cThreshold, 0, cThreshold, 1 );
			cLine->SetLineWidth( 3 );
			cLine->SetLineColor( 2 );
			cLine->Draw( "same" );

			fCbcInterface->WriteCbcReg( cScurve.first, "VCth", cThreshold );
		}

	}
	fSCurveCanvas->Update();
#ifdef __HTTP__
	fHttpServer->ProcessRequests();
#endif

	// Write and Save the Canvas as PDF
	fSCurveCanvas->Write( fSCurveCanvas->GetName(), TObject::kOverwrite );
	std::string cPdfName = fDirectoryName + "/NoiseOccupancy.pdf";
	fSCurveCanvas->SaveAs( cPdfName.c_str() );
}

uint32_t HybridTester::fillSCurves( BeBoard* pBoard,  const Event* pEvent, uint8_t pValue )
{
	uint32_t cHitCounter = 0;
	for ( auto cFe : pBoard->fModuleVector )
	{
		for ( auto cCbc : cFe->fCbcVector )
		{
			auto cScurve = fSCurveMap.find( cCbc );
			if ( cScurve == fSCurveMap.end() ) std::cout << "Error: could not find an Scurve object for Cbc " << int( cCbc->getCbcId() ) << std::endl;
			else
			{
				for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
				{
					if ( pEvent->DataBit( cCbc->getFeId(), cCbc->getCbcId(), cId ) )
					{
						cScurve->second->Fill( pValue );
						cHitCounter++;
					}
				}
			}
		}
	}
	return cHitCounter;
}

void HybridTester::updateSCurveCanvas( BeBoard* pBoard )
{

	// Here iterate over the fScurveMap and update
	// fSCurveCanvas->cd();
	for ( auto cFe : pBoard->fModuleVector )
	{
		for ( auto cCbc : cFe->fCbcVector )
		{
			uint32_t cCbcId = cCbc->getCbcId();
			auto cScurve = fSCurveMap.find( cCbc );
			if ( cScurve == fSCurveMap.end() ) std::cout << "Error: could not find an Scurve object for Cbc " << int( cCbc->getCbcId() ) << std::endl;
			else
			{
				fSCurveCanvas->cd( cCbcId + 1 );
				cScurve->second->Draw( "P" );
			}
		}
	}
	fSCurveCanvas->Update();
#ifdef __HTTP__
	fHttpServer->ProcessRequests();
#endif
}



void HybridTester::TestRegisters()
{
	// This method has to be followed by a configure call, otherwise the CBCs will be in an undefined state
	struct RegTester : public HwDescriptionVisitor
	{
		CbcInterface* fInterface;
		std::map<uint32_t, std::set<std::string>> fBadRegisters;
		RegTester( CbcInterface* pInterface ): fInterface( pInterface ) {
			std::set<std::string> tempset;
			fBadRegisters[0] = tempset;
			fBadRegisters[1] = tempset;
		}

		void visit( Cbc& pCbc ) {
			uint8_t cFirstBitPattern = 0xAA;
			uint8_t cSecondBitPattern = 0x55;

			CbcRegMap cMap = pCbc.getRegMap();
			for ( const auto& cReg : cMap ) {
				if ( !fInterface->WriteCbcReg( &pCbc, cReg.first, cFirstBitPattern, true ) ) fBadRegisters[pCbc.getCbcId()] .insert( cReg.first );
				if ( !fInterface->WriteCbcReg( &pCbc, cReg.first, cSecondBitPattern, true ) ) fBadRegisters[pCbc.getCbcId()] .insert( cReg.first );
			}
		}

		void dumpResult() {
			for ( const auto& cCbc : fBadRegisters ) {
				std::cout << "Bad Registers on Cbc " << cCbc.first << " : " << std::endl;
				for ( const auto& cReg : cCbc.second ) std::cout << cReg << std::endl;
			}
		}
	};

	// This should probably be done in the top level application but there I do not have access to the settings map

	std::cout << "Testing Cbc Registers one-by-one with complimentary bit-patterns (0xAA, 0x55) ..." << std::endl;
	RegTester cRegTester( fCbcInterface );
	accept( cRegTester );
	cRegTester.dumpResult();
	std::cout << "Done testing registers, re-configuring to calibrated state!" << std::endl;
	ConfigureHw();
}

void HybridTester::Measure()
{
	std::cout << "Mesuring Efficiency per Strip ... " << std::endl;
	std::cout << "Taking data with " << fTotalEvents << " Events!" << std::endl;

	CbcRegReader cReader( fCbcInterface, "VCth" );
	accept( cReader );

	for ( auto& cShelve : fShelveVector )
	{
		for ( BeBoard* pBoard : cShelve->fBoardVector )
		{
			uint32_t cN = 1;
			uint32_t cNthAcq = 0;

			fBeBoardInterface->Start( pBoard );

			while ( cN <=  fTotalEvents )
			{
				// Run( pBoard, cNthAcq );
				fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
				const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );

				// Loop over Events from this Acquisition
				for ( auto& cEvent : events )
				{
					HistogramFiller cFiller( fHistBottom, fHistTop, cEvent );
					pBoard->accept( cFiller );

					if ( cN % 100 == 0 )
						UpdateHists();

					cN++;
				}
				cNthAcq++;
			}
			fBeBoardInterface->Stop( pBoard, cNthAcq );
		}
	}
	fHistTop->Scale( 100 / double_t( fTotalEvents ) );
	fHistTop->GetYaxis()->SetRangeUser( 0, 100 );
	fHistBottom->Scale( 100 / double_t( fTotalEvents ) );
	fHistBottom->GetYaxis()->SetRangeUser( 0, 100 );
	UpdateHists();
}

void HybridTester::SaveResults()
{
	fHistTop->Write( fHistTop->GetName(), TObject::kOverwrite );
	fHistBottom->Write( fHistBottom->GetName(), TObject::kOverwrite );
	fDataCanvas->Write( fDataCanvas->GetName(), TObject::kOverwrite );

	fResultFile->Write();
	fResultFile->Close();


	std::cout << "Resultfile written correctly!" << std::endl;

	std::string cPdfName = fDirectoryName + "/HybridTestResults.pdf";
	fDataCanvas->SaveAs( cPdfName.c_str() );
	if ( fThresholdScan )
	{
		cPdfName = fDirectoryName + "/ThresholdScanResults.pdf";
		fSCurveCanvas->SaveAs( cPdfName.c_str() );
	}
}
