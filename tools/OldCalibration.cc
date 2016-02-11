#include "OldCalibration.h"
#include <cmath>

// TODO: Canvas divisions

void OldCalibration::Initialise()
{
	//is to be called after system controller::ReadHW, ReadSettings
	// populates all the maps
	// create the canvases
	fVplusCanvas = new TCanvas( "SCurves", "SCurves", 650, 650 );
	fVcthVplusCanvas = new TCanvas( "Vplus vs. VCth", "Vplus vs. VCth", 650, 650 );
	fOffsetCanvas = new TCanvas( "SCurves Offset Tuning", "SCurves Offset Tuning", 650, 650 );


	// count FEs & CBCs
	uint32_t cCbcCount = 0;
	uint32_t cCbcIdMax = 0;
	uint32_t cFeCount = 0;

	for ( auto cBoard : fBoardVector )
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
		  }
	      }
	    fNCbc = cCbcCount;
	    fNFe = cFeCount;
	  }
	uint32_t cPads = ( cCbcIdMax > cCbcCount ) ? cCbcIdMax : cCbcCount;
	
	fVplusCanvas->DivideSquare( cPads );
	fVcthVplusCanvas->DivideSquare( cPads );
	fOffsetCanvas->DivideSquare( cPads );
	
	// now read the settings from the map
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
	std::cout << "	TestPulseAmplitude = " << int( fTestPulseAmplitude ) << std::endl;

}

void OldCalibration::ScanVplus()
{
	// Method to perform a Vplus Scan

	// first set the offset of all Channels to 0x0A
	//setOffset( 0x50,-1 );//change to this function
	std::cout << BOLDBLUE << "Scanning Vplus ..." << RESET << std::endl;
	for ( auto& cTGrpM : fTestGroupChannelMap )
	{
		// enable test group here

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
			measureSCurves( cTGrpM.first );

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



void OldCalibration::ScanOffset()
{

	// Method to tune the individual channel Offsets

	std::cout << BOLDBLUE << "Scanning Offsets..." << RESET << std::endl;
	for ( auto& cTGrpM : fTestGroupChannelMap )
	{

		std::cout << "Enabling Test Group...." << cTGrpM.first << std::endl;

		CbcRegReader cReader( fCbcInterface, "Vplus" );
		accept( cReader );

		CbcRegReader aReader( fCbcInterface, "VCth" );
		accept( aReader );
		initializeSCurves( "Offset", 0x00, cTGrpM.first );

		// // the true indicates that this time I am sweeping Offsets instead of Vcth
		measureSCurvesOffset( cTGrpM.first );

		processSCurves( "Offset", 0x00, true, cTGrpM.first );
		// set offset accordingly (check for mode, then either 0 or 255)


		std::cout << "Disabling Test Group...." << cTGrpM.first << std::endl;
	}

	std::cout << BOLDBLUE << "Finished scanning Offsets ..." << RESET << std::endl;

}



// PRIVATE METHODS


void OldCalibration::processSCurves( TString pParameter, uint8_t pValue, bool pDraw, int  pTGrpId )
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



void OldCalibration::findVplus( bool pDraw )
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



void OldCalibration::writeGraphs()
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
	fResultFile->cd();

	// Save canvasses too
	fVplusCanvas->Write( fVplusCanvas->GetName(), TObject::kOverwrite );
	fVcthVplusCanvas->Write( fVcthVplusCanvas->GetName(), TObject::kOverwrite );
	fOffsetCanvas->Write( fOffsetCanvas->GetName(), TObject::kOverwrite );
}



