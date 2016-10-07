#include "HybridTester.h"
#include <ctime>

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

	fHistTop = new TH1F( cFrontName, "Front Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ), -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
	fHistTop->SetFillColor( 4 );
	fHistTop->SetFillStyle( 3001 );

	TString cBackName( "fHistBottom" );
	fHistBottom = ( TH1F* )( gROOT->FindObject( cBackName ) );
	if ( fHistBottom ) delete fHistBottom;

	fHistBottom = new TH1F( cBackName, "Back Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ), -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
	fHistBottom->SetFillColor( 4 );
	fHistBottom->SetFillStyle( 3001 );
	
	TString cFrontNameMerged( "fHistTopMerged" );
	fHistTopMerged = ( TH1F* )( gROOT->FindObject( cFrontNameMerged ) );
	if ( fHistTopMerged ) delete fHistTopMerged;

	fHistTopMerged = new TH1F( cFrontNameMerged, "Front Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ) , -0.5, ( fNCbc / 2 * 254 ) -0.5 );
	fHistTopMerged->SetFillColor( 4 );
	fHistTopMerged->SetFillStyle( 3001 );

	TString cBackNameMerged( "fHistBottomMerged" );
	fHistBottomMerged = ( TH1F* )( gROOT->FindObject( cBackNameMerged ) );
	if ( fHistBottomMerged ) delete fHistBottomMerged;

	fHistBottomMerged = new TH1F( cBackNameMerged, "Back Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 254 ) , -0.5, ( fNCbc / 2 * 254 ) - 0.5 );
	fHistBottomMerged->SetFillColor( 4 );
	fHistBottomMerged->SetFillStyle( 3001 );

	// Now the Histograms for SCurves
	for ( auto cBoard : fBoardVector )
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
			bookHistogram( cCbc, "Scurve", cTmpScurve );
			fSCurveMap[cCbc] = cTmpScurve;
		    
		    cName = Form( "SCurveFit_Fe%d_Cbc%d", cFeId, cCbcId );
		    cObject = static_cast<TObject*>( gROOT->FindObject( cName ) );
		    if ( cObject ) delete cObject;
		    TF1* cTmpFit = new TF1( cName, MyErf, 0, 255, 2 );
			bookHistogram( cCbc, "ScurveFit", cTmpFit );

		    fFitMap[cCbc] = cTmpFit;
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

void HybridTester::Initialize ( bool pThresholdScan )
{
	fThresholdScan = pThresholdScan;
	gStyle->SetOptStat( 000000 );
	//gStyle->SetTitleOffset( 1.3, "Y" );
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

uint32_t HybridTester::fillSCurves( BeBoard* pBoard,  const Event* pEvent, uint8_t pValue )
{
	uint32_t cHitCounter = 0;
	for ( auto cFe : pBoard->fModuleVector )
	{
		for ( auto cCbc : cFe->fCbcVector )
		{
			// SS
	      	/*TH1F* sCurveHist = static_cast<TH1F*>( getHist( cCbc, "Scurve" ) );
	      	uint32_t cbcEventCounter = 0;
	      	for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
			{
				if ( pEvent->DataBit( cCbc->getFeId(), cCbc->getCbcId(), cId ) )
				{
					sCurveHist->Fill( pValue );
					cHitCounter++;
					cbcEventCounter++;
				}
			}*/

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

void HybridTester::ScanThresholds()
{
	std::cout << "Mesuring Efficiency per Strip ... " << std::endl;
	std::cout << "Taking data with " << fTotalEvents << " Events!" << std::endl;

	int cVcthStep = 2;
	uint8_t cVcth = ( fHoleMode ) ?  0xFF :  0x00;
	int cStep = ( fHoleMode ) ? (-1*cVcthStep) : cVcthStep;

	int iVcth =  +(cVcth);
	//std::cout << RED << "Vcth = " <<  iVcth << RESET << std::endl;

	//simple VCth loop
	while ( 0 <= iVcth && iVcth <= 255 )
	{
		
		CbcRegWriter cWriter( fCbcInterface, "VCth", cVcth );
		accept( cWriter );

		fHistTop->GetYaxis()->SetRangeUser( 0, fTotalEvents );
		fHistBottom->GetYaxis()->SetRangeUser( 0, fTotalEvents );
		
		for ( BeBoard* pBoard : fBoardVector )
		  {
		    uint32_t cN = 1;
		    uint32_t cNthAcq = 0;
		    
		    fBeBoardInterface->Start( pBoard );
		    
		    while ( cN <=  fTotalEvents )
		      {
			// Run( pBoard, cNthAcq );
			fBeBoardInterface->ReadData( pBoard, false );
			const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );
			
			// Loop over Events from this Acquisition
			for ( auto& cEvent : events )
			  {
			    HistogramFiller cFiller( fHistBottom, fHistTop, cEvent );
			    pBoard->accept( cFiller );
			    
			    fillSCurves( pBoard,  cEvent , cVcth );
			    if ( cN % 100 == 0 )
				{
					updateSCurveCanvas( pBoard );
					UpdateHists();
			    }
			    cN++;
			  }
				cNthAcq++;
		      }
		    fBeBoardInterface->Stop( pBoard);
		}
		fHistTop->Scale( 100 / double_t( fTotalEvents ) );
		fHistTop->GetYaxis()->SetRangeUser( 0, 100 );
		fHistBottom->Scale( 100 / double_t( fTotalEvents ) );
		fHistBottom->GetYaxis()->SetRangeUser( 0, 100 );
		UpdateHists();
		
		

		//std::cout << RED << "Vcth = " <<  iVcth << RESET << " " ;
		//std::cout << GREEN << "Mean occupancy at the Top side: " << fHistTop->Integral()/(double)(fNCbc*127) << RESET << " ";
		//std::cout << BLUE << "Mean occupancy at the Bottom side: " << fHistBottom->Integral()/(double)(fNCbc*127) << RESET << std::endl;
		iVcth = +(cVcth+cStep);
		cVcth += cStep;

			
	}
	// Fit and save the SCurve & Fit - extract the right threshold
	// TODO
	processSCurves( fTotalEvents );

	//normalize noise scan 
	/*for ( BeBoard* pBoard : fBoardVector )
	{
		for ( auto cFe : pBoard->fModuleVector )
		{
			for ( auto cCbc : cFe->fCbcVector )
			{
				fSCurveCanvas->cd(cCbc->getCbcId()+1);
				TH1F* sCurveHist = static_cast<TH1F*>( getHist( cCbc, "Scurve" ) );
				sCurveHist->Scale(100./(NCHANNELS*fTotalEvents));
				sCurveHist->GetYaxis()->SetTitle("Occupancy (%)");
				sCurveHist->GetYaxis()->SetTitleOffset(1.2);
				sCurveHist->DrawCopy("P0");

				sCurveHist->Write( sCurveHist->GetName(), TObject::kOverwrite );
				fSCurveCanvas->cd(cCbc->getCbcId()+1)->Update();
			}
		}
	}*/
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
		if ( cAllOne ) break;
		for ( BeBoard* pBoard : fBoardVector )
		  {
		    fBeBoardInterface->Start( pBoard );
		    while ( cN <=  cEventsperVcth )
		      {
			// Run( pBoard, cNthAcq );
			fBeBoardInterface->ReadData( pBoard, false );
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
		    fBeBoardInterface->Stop( pBoard);
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

	// Fit and save the SCurve & Fit - extract the right threshold
	// TODO
	processSCurves( cEventsperVcth );
}

void HybridTester::processSCurves ( uint32_t pEventsperVcth )
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
			cScurve.second->SetLineStyle(2);
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
			cLine->SetLineColor( kCyan );
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

void HybridTester::updateSCurveCanvas( BeBoard* pBoard )
{

	/*for ( auto cFe : pBoard->fModuleVector )
	{
		for ( auto cCbc : cFe->fCbcVector )
		{
			fSCurveCanvas->cd(cCbc->getCbcId()+1);
			TH1F* sCurveHist = static_cast<TH1F*>( getHist( cCbc, "Scurve" ) );
			sCurveHist->DrawCopy("P0");
			fSCurveCanvas->cd(cCbc->getCbcId()+1)->Update();
		}
	}*/

	// Here iterate over the fScurveMap and update
	fSCurveCanvas->cd();
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
				cScurve->second->DrawCopy( "P" );
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
		RegTester( CbcInterface* pInterface, uint32_t pNCbc ): fInterface( pInterface ) {
			std::set<std::string> tempset;
			uint32_t cCbcIterator = 0;
			for (cCbcIterator = 0; cCbcIterator < pNCbc; cCbcIterator++)
			{
				fBadRegisters[cCbcIterator] = tempset;
			}			
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

		void dumpResult( std::string fDirectoryName ) {
			ofstream report( fDirectoryName + "/registers_test.txt" ); // Creates a file in the current directory
			report << "Testing Cbc Registers one-by-one with complimentary bit-patterns (0xAA, 0x55)" << std::endl;
			for ( const auto& cCbc : fBadRegisters ) {
				report << "Malfunctioning Registers on Cbc " << cCbc.first << " : " << std::endl;
				for ( const auto& cReg : cCbc.second ) report << cReg << std::endl;

			}
			report.close();
			std::cout << "Channels diagnosis report written to: " + fDirectoryName + "/registers_test.txt" << std::endl;
		}
	};

	// This should probably be done in the top level application but there I do not have access to the settings map
	time_t start_time = time(0);
	char* start = ctime(&start_time);
	std::cout << "start: "<< start << std::endl;
	std::cout << std::endl << "Running registers testing tool ... " << std::endl;
	RegTester cRegTester( fCbcInterface, fNCbc );
	accept( cRegTester );
	cRegTester.dumpResult( fDirectoryName );
	std::cout << "Done testing registers, re-configuring to calibrated state!" << std::endl;
	start_time = time(0);
	char* stop = ctime(&start_time);
	std::cout << "stop: " << stop << std::endl;
	ConfigureHw();
}

void HybridTester::DisplayGroupsContent(std::array<std::vector<std::array<int, 5>>, 8> pShortedGroupsArray)
{
	for ( int i = 0; i < 8; i++)
	{
		std::cout<<"TP group ID: "<<i<<std::endl;
		for (auto cShortsVector : pShortedGroupsArray[i])
		{
			for (auto i: cShortsVector) std::cout << i << ' ';
			std::cout<<std::endl;
		}
		std::cout<<std::endl;
	}
}

std::vector<std::array<int, 2>> HybridTester::MergeShorts(std::vector<std::array<int, 2>> pShortA, std::vector<std::array<int, 2>> pShortB)
{
	for (auto cChannel : pShortA)
	{
		if (!CheckChannelInShortPresence(cChannel, pShortB))
		{
			pShortB.push_back(cChannel);
		}
	}

	for (auto cMemberChannel: pShortB)
	{
		for (auto i: cMemberChannel) std::cout << i << ' ';
	}
	std::cout<<std::endl;
	
	return pShortB;
}

bool HybridTester::CheckShortsConnection(std::vector<std::array<int, 2>> pShortA, std::vector<std::array<int, 2>> pShortB)
{
	for (auto cChannel : pShortA)
	{
		if (CheckChannelInShortPresence(cChannel, pShortB)) return true;
	} 
	return false;
}

bool HybridTester::CheckChannelInShortPresence( std::array<int, 2> pShortedChannel, std::vector<std::array<int, 2>> pShort)
{
	for (auto cChannel : pShort)
	{
		if (cChannel[0] == pShortedChannel[0] && cChannel[1] == pShortedChannel[1]) return true;
	}
	return false;
}

void HybridTester::ReconstructShorts(std::array<std::vector<std::array<int, 5>>, 8> pShortedGroupsArray)
{
	std::cout<<std::endl<<"---------Creating shorted pairs-----------------"<<std::endl;
	std::vector<std::vector<std::array <int, 2>>> cShortsVector;
	std::vector<std::array<int, 2>> cShort;
	std::array<int, 2> temp_shorted_channel;

	std::array<int, 2> best_candidate;
	int best_candidate_index = 0;
	int index = 0;
	int sub_index = 0;
	int cross_side_punishment = 2;
	int smallest_distance = 10000;
	bool matching_channel_found;

	for ( auto cShortedChannelsGroup : pShortedGroupsArray)
	{
		for (auto cShortedChannelInfo : cShortedChannelsGroup)
		{
			temp_shorted_channel[0] = cShortedChannelInfo[0];
			temp_shorted_channel[1] = cShortedChannelInfo[1];

			cShort.push_back(temp_shorted_channel);
			cShort.push_back(temp_shorted_channel);
			
			index = 0;
			matching_channel_found = false;

			for ( auto cCandidate : pShortedGroupsArray[cShortedChannelInfo[2]] )
			{	
				if(cShortedChannelInfo[3] == cCandidate[2])
				{
					if ( smallest_distance > (cross_side_punishment * (cShort[0][0] - cCandidate[0]) * (cShort[0][0] - cCandidate[0]) + (cShort[0][1] - cCandidate[1]) *  (cShort[0][1] - cCandidate[1]) ) )
					{
						smallest_distance = (cross_side_punishment * (cShort[0][0] - cCandidate[0]) * (cShort[0][0] - cCandidate[0]) + (cShort[0][1] - cCandidate[1]) * (cShort[0][1] - cCandidate[1]) );
						best_candidate[0] = cCandidate[0];
						best_candidate[1] = cCandidate[1];
						best_candidate_index = index;
						matching_channel_found = true;
					}
				}
				index++;
			}
			if (matching_channel_found)
			{
				if (best_candidate[0] <= cShort[0][0] && best_candidate[1] < cShort[0][1] )
				{
					cShort[0][0] = best_candidate[0];
					cShort[0][1] = best_candidate[1];
				}

				else
				{
					cShort[1][0] = best_candidate[0];
					cShort[1][1] = best_candidate[1];
				}

				pShortedGroupsArray[cShortedChannelInfo[2]].erase(pShortedGroupsArray[cShortedChannelInfo[2]].begin() + best_candidate_index);

				for (auto cMemberChannel: cShort)
				{
					for (auto i: cMemberChannel) std::cout << i << ' ';
				} 
				std::cout<<"smallest distance: "<<smallest_distance<<std::endl;
				//DisplayGroupsContent(pShortedGroupsArray);

				cShortsVector.push_back(cShort);
			}
			else std::cout<<"ERROR: No matching channel found for detected short (watch the level of noise)!"<<std::endl;

			smallest_distance = 10000;
			cShort.clear();
		}
	}

	std::cout<<"---------Merging shorts-------------------------"<<std::endl;
	index = cShortsVector.size();
	std::cout<<"Number of shorted connections found: "<<index<<std::endl;

	while ( index > 1 ) 
	{
		index--;
		sub_index = index;
		while (sub_index > 0)
		{
			sub_index--;
			if (CheckShortsConnection( cShortsVector[index], cShortsVector[sub_index]))
			{
				cShortsVector[sub_index] = MergeShorts( cShortsVector[index], cShortsVector[sub_index]);
				cShortsVector.erase(cShortsVector.begin() + index);
	    		break;
			}
		}  
	}

	std::cout<<"---------Outcome--------------------------------"<<std::endl;
	for (auto someShort: cShortsVector)
	{
		for (auto cMemberChannel: someShort)
		{
			for (auto i: cMemberChannel) std::cout << i << ' ';
		}
		std::cout<<std::endl;
	}
}

void HybridTester::SetBeBoardForShortsFinding(BeBoard* pBoard)
{
	fBeBoardInterface->WriteBoardReg (pBoard, "COMMISSIONNING_MODE_RQ", 1 );
    fBeBoardInterface->WriteBoardReg (pBoard, "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID", 1 );
    fBeBoardInterface->WriteBoardReg (pBoard, "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE", 2 );
	
	std::cout << "COMMISSIONNING_MODE_RQ: " << fBeBoardInterface->ReadBoardReg( pBoard, "COMMISSIONNING_MODE_RQ" ) << std::endl;
	std::cout << "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID: " << fBeBoardInterface->ReadBoardReg( pBoard, "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID" ) << std::endl;
	std::cout << "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE: " << fBeBoardInterface->ReadBoardReg( pBoard, "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE" ) << std::endl;

    std::vector<std::pair<std::string, uint8_t>> cRegVec;    

    if ( fHoleMode )
        cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", 0xE1 ) );
    else
        cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", 0x61 ) );

    cRegVec.push_back ( std::make_pair ( "TestPulsePot", 0xF0 ) );

    cRegVec.push_back ( std::make_pair ( "VCth", 0x90 ) );

    cRegVec.push_back ( std::make_pair ( "TriggerLatency", 0x01 ) );

    CbcMultiRegWriter cMultiWriter ( fCbcInterface, cRegVec );
    this->accept ( cMultiWriter );
}

void HybridTester::FindShorts()
{
	
	uint8_t cGroupAddress[8] = {0, 4, 2, 6, 1, 5, 3, 7};
	uint8_t cTestPulseGroupId = 0;
	std::array<int, 5> cShortedChannelInfo;	
	std::array<std::vector<std::array<int, 5>>, 8> cShortedGroupsArray;
	CbcRegReader cReader( fCbcInterface, "VCth" );
	accept( cReader );
	fHistTop->GetYaxis()->SetRangeUser( 0, fTotalEvents );
	fHistBottom->GetYaxis()->SetRangeUser( 0, fTotalEvents );
	
	for ( BeBoard* pBoard : fBoardVector )
  	{
	    uint32_t cN = 1;
	    uint32_t cNthAcq = 0;   

	    SetBeBoardForShortsFinding(pBoard);
	     	    
	   	CbcRegWriter cWriter( fCbcInterface, "SelTestPulseDel&ChanGroup", cGroupAddress[cTestPulseGroupId]);
	   	//CbcRegReader cReader( fCbcInterface, "SelTestPulseDel&ChanGroup" );
		std::cout<<"\nShorted channels searching procedure\nSides: Top - 0\tBottom - 1 (Channel numbering starts from 0)\n"<<std::endl;
		std::cout<<"      Side\t| Channel_ID\t| Group_ID\t| Shorted_With_Group_ID"<<std::endl;
	    for (cTestPulseGroupId = 0; cTestPulseGroupId < 8; cTestPulseGroupId++)
	    {
	    	cN = 1;
	    	cNthAcq = 0;

	    	cWriter.setRegister( "SelTestPulseDel&ChanGroup", cGroupAddress[cTestPulseGroupId]);
	    	accept( cWriter );
			//accept( cReader );	

		    fBeBoardInterface->Start( pBoard );
		    while ( cN <=  fTotalEvents )
		    {
				//Run( pBoard, cNthAcq );
				fBeBoardInterface->ReadData( pBoard, false );
				//fBeBoardInterface->ReadNEvents ( pBoard, cNthAcq );
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
		    fBeBoardInterface->Stop( pBoard);

		    std::vector<std::array<int, 5>> cShortedChannelsGroup;
		    for ( uint16_t cChannelId = 1; cChannelId < fNCbc * 127 + 1; cChannelId++ )
			{	
				if ( fHistTop->GetBinContent( cChannelId ) > 0.5*fTotalEvents && ((cChannelId-1)%127)%8 != cTestPulseGroupId ) 
				{
					cShortedChannelInfo = {0, (cChannelId-1), ((cChannelId-1)%127)%8, cTestPulseGroupId, 0};				
					cShortedChannelsGroup.push_back(cShortedChannelInfo);
					std::cout<<"\t0\t|\t"<<cShortedChannelInfo[1]<<"\t|\t"<<cShortedChannelInfo[2]<<"\t|\t"<<cShortedChannelInfo[3]<<std::endl;
					fHistTopMerged->SetBinContent( cChannelId, fHistTop->GetBinContent( cChannelId ) );
				}

				if ( fHistBottom->GetBinContent( cChannelId ) > 0.5*fTotalEvents && ((cChannelId-1)%127)%8 != cTestPulseGroupId) 
				{
					cShortedChannelInfo = {1, (cChannelId-1), ((cChannelId-1)%127)%8, cTestPulseGroupId, 0};
					cShortedChannelsGroup.push_back(cShortedChannelInfo);
					std::cout<<"\t1\t|\t"<<cShortedChannelInfo[1]<<"\t|\t"<<cShortedChannelInfo[2]<<"\t|\t"<<cShortedChannelInfo[3]<<std::endl;
					fHistBottomMerged->SetBinContent( cChannelId, fHistBottom->GetBinContent( cChannelId ) );					
				}
				
			}
			cShortedGroupsArray[cTestPulseGroupId] = cShortedChannelsGroup;
			//if (cTestPulseGroupId == 2) return;
			fHistBottom->Reset();
			fHistTop->Reset();
			std::cout<<"------------------------------------------------------------------------"<<std::endl;


		}
  	}
	fHistTopMerged->Scale( 100 / double_t( fTotalEvents ) );
	fHistTopMerged->GetYaxis()->SetRangeUser( 0, 100 );
	fHistBottomMerged->Scale( 100 / double_t( fTotalEvents ) );
	fHistBottomMerged->GetYaxis()->SetRangeUser( 0, 100 );
	ReconstructShorts(cShortedGroupsArray);
	UpdateHistsMerged();
}

void HybridTester::Measure()
{
	std::cout << "Mesuring Efficiency per Strip ... " << std::endl;
	std::cout << "Taking data with " << fTotalEvents << " Events!" << std::endl;

	CbcRegReader cReader( fCbcInterface, "VCth" );
	accept( cReader );
	fHistTop->GetYaxis()->SetRangeUser( 0, fTotalEvents );
	fHistBottom->GetYaxis()->SetRangeUser( 0, fTotalEvents );

	
	
	for ( BeBoard* pBoard : fBoardVector )
	  {
	    uint32_t cN = 1;
	    uint32_t cNthAcq = 0;
	    
	    fBeBoardInterface->Start( pBoard );
	    while ( cN <=  fTotalEvents )
	      {
		//Run( pBoard, cNthAcq );
		fBeBoardInterface->ReadData( pBoard, false );
		//fBeBoardInterface->ReadNEvents ( pBoard, cNthAcq );
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
	    fBeBoardInterface->Stop( pBoard);
	  }
	fHistTop->Scale( 100 / double_t( fTotalEvents ) );
	fHistTop->GetYaxis()->SetRangeUser( 0, 100 );
	fHistBottom->Scale( 100 / double_t( fTotalEvents ) );
	fHistBottom->GetYaxis()->SetRangeUser( 0, 100 );
	UpdateHists();

	std::cout << "Mean occupancy at the Top side: " << fHistTop->Integral()/(double)(fNCbc*127) << std::endl;
	std::cout << "Mean occupancy at the Bottom side: " << fHistBottom->Integral()/(double)(fNCbc*127) << std::endl;
}

void HybridTester::AntennaScan()
{
#ifdef __ANTENNA__
	std::cout << "Mesuring Efficiency per Strip ... " << std::endl;
	std::cout << "Taking data with " << fTotalEvents << " Events!" << std::endl;

	CbcRegReader cReader( fCbcInterface, "VCth" );
	accept( cReader );

	Antenna cAntenna;
	cAntenna.initializeAntenna();
	for (int channel = 0; channel < fNCbc; channel++) cAntenna.ConfigureSpiSlave( channel );

	fHistTop->GetYaxis()->SetRangeUser( 0, fTotalEvents );
	fHistBottom->GetYaxis()->SetRangeUser( 0, fTotalEvents );

	for ( uint8_t analog_switch_cs = 0; analog_switch_cs < fNCbc; analog_switch_cs++ )
	{
		std::cout << "Chip Select ID " << +analog_switch_cs << std::endl;
		cAntenna.ConfigureSpiSlave( analog_switch_cs );

		for ( uint8_t channel_position = 1; channel_position < 10; channel_position++ )
		{
			cAntenna.TurnOnAnalogSwitchChannel( channel_position );
			
			if (channel_position == 9) break;

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

							if ( cN % 100 == 0 ) UpdateHists();

							cN++;
						}
						cNthAcq++;
					}
					fBeBoardInterface->Stop( pBoard, cNthAcq );
					
					/*Here the reconstruction of histograms happens*/
					for ( uint16_t channel_id = 1; channel_id < fNCbc * 127 + 1; channel_id++ )
						{						
							if ( fHistTopMerged->GetBinContent( channel_id ) < fHistTop->GetBinContent( channel_id ) ) fHistTopMerged->SetBinContent( channel_id, fHistTop->GetBinContent( channel_id ) );
							if ( fHistBottomMerged->GetBinContent( channel_id ) < fHistBottom->GetBinContent( channel_id ) ) fHistBottomMerged->SetBinContent( channel_id, fHistBottom->GetBinContent( channel_id ) );
						}
					
					/*Here clearing histograms after each event*/
					fHistBottom->Reset();
					fHistTop->Reset();
				}

			}

		}
	}

	fHistTopMerged->Scale( 100 / double_t( fTotalEvents ) );
	fHistTopMerged->GetYaxis()->SetRangeUser( 0, 100 );
	fHistBottomMerged->Scale( 100 / double_t( fTotalEvents ) );
	fHistBottomMerged->GetYaxis()->SetRangeUser( 0, 100 );
	
	UpdateHistsMerged();
	
	cAntenna.close();

	TestChannels( fDecisionThreshold );
#endif
}

void HybridTester::SaveTestingResults(std::string pHybridId)
{
	
	ifstream infile;
	std::string line_buffer;
	std::string content_buffer;
	std::string date_string = currentDateTime();
	std::string filename = "Results/HybridTestingDatabase/Hybrid_ID" + pHybridId + "_on" + date_string + ".txt";
	ofstream myfile;
	myfile.open( filename.c_str() );
	myfile << "Hybrid ID: " << pHybridId << std::endl;
	myfile << "Created on: " << date_string << std::endl << std::endl;
	myfile << " Hybrid Testing Report" << std::endl;
	myfile << "-----------------------" << std::endl << std::endl;
	myfile << " Write/Read Registers Test" << std::endl;
	myfile << "---------------------------" << std::endl;

	infile.open( fDirectoryName + "/registers_test.txt" );
	while ( getline( infile, line_buffer ) ) content_buffer += line_buffer + "\r\n"; // To get all the lines.
	if ( content_buffer == "" ) myfile << "Test not performed!" << std::endl;

	infile.close();
	myfile << content_buffer << std::endl;
	content_buffer = "";
	myfile << " Channels Functioning Test" << std::endl;
	myfile << "---------------------------" << std::endl;
	infile.open( fDirectoryName + "/channels_test2.txt" );
	while ( getline( infile, line_buffer ) ) content_buffer += line_buffer + "\r\n"; // To get all the lines.
	if ( content_buffer == "" ) myfile << "Test not performed!" << std::endl;
	infile.close();
	myfile << content_buffer << std::endl;
	myfile.close();
	std::cout << std::endl << "Summary testing report written to: " << std::endl << filename << std::endl;
}

void HybridTester::SaveResults()
{
	fHistTop->Write( fHistTop->GetName(), TObject::kOverwrite );
	fHistBottom->Write( fHistBottom->GetName(), TObject::kOverwrite );
	fDataCanvas->Write( fDataCanvas->GetName(), TObject::kOverwrite );

	fResultFile->Write();
	fResultFile->Close();


	std::cout << std::endl << "Resultfile written correctly!" << std::endl;

	std::string cPdfName = fDirectoryName + "/HybridTestResults.pdf";
	fDataCanvas->SaveAs( cPdfName.c_str() );
	if ( fThresholdScan )
	{
		cPdfName = fDirectoryName + "/ThresholdScanResults.pdf";
		fSCurveCanvas->SaveAs( cPdfName.c_str() );
	}
}
