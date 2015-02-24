#include "Commissioning.h"

void Commissioning::Initialize()
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

				TCanvas* ctmpCanvas = new TCanvas( Form( "c_online_canvas_fe%d", cFeId ), Form( "FE%d  Online Canvas", cFeId ) );
				// ctmpCanvas->Divide( 2, 2 );
				fCanvasMap[cFe] = ctmpCanvas;
			}
		}
	}

	parseSettings();
	initializeHists();

	std::cout << "Histograms and Settings initialised." << std::endl;
}

std::map<Module*, uint8_t> Commissioning::ScanLatency( uint8_t pStartLatency, uint8_t pLatencyRange )
{
	// This is not super clean but should work
	// Take the default VCth which should correspond to the pedestal and add 8 depending on the mode to exclude noise
	CbcRegReader cReader( fCbcInterface, "VCth" );
	this->accept( cReader );
	uint8_t cVcth = cReader.fRegValue;

	// int cVcthStep = ( fHoleMode == 1 ) ? +8 : -8;
	// std::cout << "VCth value from config file is: " << +cVcth << " ;  changing by " << cVcthStep << "  to " << +( cVcth + cVcthStep ) << " supress noise hits for crude latency scan!" << std::endl;
	// cVcth += cVcthStep;

	// //  Set that VCth Value on all FEs
	// CbcRegWriter cVcthWriter( fCbcInterface, "VCth", cVcth );
	// this->accept( cVcthWriter );

	// Now the actual scan
	std::cout << "Scanning Latency ... " << std::endl;

	for ( uint8_t cLat = pStartLatency; cLat < pStartLatency + pLatencyRange; cLat++ )
	{
		//  Set a Latency Value on all FEs
		CbcRegWriter cLatWriter( fCbcInterface, "TriggerLatency", cLat );
		this->accept( cLatWriter );

		uint32_t cN = 0;
		uint32_t cNthAcq = 0;

		// Take Data for all Modules
		for ( auto& cShelve : fShelveVector )
		{
			for ( BeBoard* pBoard : cShelve->fBoardVector )
			{
				while ( cN < fNevents )
				{
					Run( pBoard, cNthAcq );

					const Event* cEvent = fBeBoardInterface->GetNextEvent( pBoard );

					int cNHits = 0;

					// Loop over Events from this Acquisition
					while ( cEvent )
					{
						if ( cN == fNevents )
							break;
						for ( auto cFe : pBoard->fModuleVector )
							cNHits += countHits( cFe, cEvent, "module_latency", cLat );
						cN++;

						if ( cN < fNevents )
							cEvent = fBeBoardInterface->GetNextEvent( pBoard );
						else break;
					}
					std::cout << "Latency " << +cLat << " Hits " << cNHits  << " Events " << cN << std::endl;

					cNthAcq++;
				}
			}
		}

		// done counting hits for all FE's, now update the Histograms
		updateHists( "module_latency", false );
	}

	// analyze the Histograms
	std::map<Module*, uint8_t> cLatencyMap;

	std::cout << "Identified the Latency with the maximum number of Hits at: " << std::endl;

	for ( auto cFe : fModuleHistMap )
	{
		TH1F* cTmpHist = ( TH1F* )getHist( cFe.first, "module_latency" );
		uint8_t cLatency =  static_cast<uint8_t>( cTmpHist->GetMaximumBin() );
		cLatencyMap[cFe.first] = cLatency;

		std::cout << "	FE " << +cFe.first->getModuleId()  << ": " << +cLatency << " clock cycles!" << std::endl;
	}

	return cLatencyMap;
}


void Commissioning::ScanThreshold()
{
	//method to scan thresholds with actual particles - this will not stop but continue through the whole Vcth range
	std::cout << "Scanning the Threshold ... " << std::endl;

	// Necessary variables
	uint32_t cEventsperVcth = 100;
	bool cNonZero = false;
	// bool cAllOne = false;
	bool cSlopeZero = false;
	// uint32_t cAllOneCounter = 0;
	uint32_t cSlopeZeroCounter = 0;
	uint32_t cOldHitCounter = 0;
	uint8_t  cDoubleVcth;
	int cVcth = ( fHoleMode ) ?  0xFF :  0x00;
	int cStep = ( fHoleMode ) ? -10 : 10;



	// Adaptive VCth loop
	while ( 0x00 <= cVcth && cVcth <= 0xFF )
	{
		// if ( cSlopeZero && (cVcth == 0x00 || cVcth = 0xFF) ) break;
		if ( cVcth == cDoubleVcth )
		{
			cVcth +=  cStep;
			continue;
		}
		// Set current Vcth value on all Cbc's
		CbcRegWriter cWriter( fCbcInterface, "VCth", static_cast<uint8_t>( cVcth ) );
		accept( cWriter );

		uint32_t cN = 0;
		uint32_t cNthAcq = 0;
		uint32_t cHitCounter = 0;

		// maybe restrict to pBoard? instead of looping?
		for ( auto& cShelve : fShelveVector )
		{
			// if ( cSlopeZero && (cVcth == 0x00 || cVcth = 0xFF) ) break;
			for ( BeBoard* pBoard : cShelve->fBoardVector )
			{
				while ( cN <  cEventsperVcth )
				{
					Run( pBoard, cNthAcq );

					const Event* cEvent = fBeBoardInterface->GetNextEvent( pBoard );

					// Loop over Events from this Acquisition
					while ( cEvent )
					{
						if ( cN == cEventsperVcth )
							break;

						for ( auto cFe : pBoard->fModuleVector )
							cHitCounter += countHits( cFe, cEvent, "module_threshold", static_cast<uint8_t>( cVcth ) );

						cN++;

						if ( cN < cEventsperVcth )
							cEvent = fBeBoardInterface->GetNextEvent( pBoard );
						else break;
					}
					cNthAcq++;
				}

				// now update the Histograms
				updateHists( "module_threshold", false );

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
					// if ( cHitCounter > 0.95 * cEventsperVcth * fNCbc * NCHANNELS ) cAllOneCounter++;
					// if the number of hits does not change any more,  increase stepsize by a factor of 10
					if ( fabs( cHitCounter - cOldHitCounter ) < 10 && cHitCounter != 0 ) cSlopeZeroCounter++;
				}
				// if ( cSlopeZeroCounter >= 10 ) cAllOne = true;
				if ( cSlopeZeroCounter >= 10 ) cSlopeZero = true;

				// if ( cAllOne )
				// {
				//  std::cout << "All strips firing -- ending the scan at VCth " << +cVcth << std::endl;
				//  break;
				// }
				if ( cSlopeZero )
					cStep *= 10;

				cOldHitCounter = cHitCounter;
				cVcth += cStep;
			}
		}
	}
	// finished scanning the comparator threshold range
	// need to see what range to fit and what threshold to extract automatically!!

	for ( auto cFe : fModuleHistMap )
	{
		TH1F* cTmpHist = ( TH1F* )getHist( cFe.first, "module_threshold" );

		// cLatencyMap[cFe.first] = static_cast<uint8_t>( cTmpHist->GetMaxBin() );

		std::cout << "Done scanning threshold!" << std::endl;
	}
}


void Commissioning::SaveResults()
{
	// Now per FE
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

	// Save Canvasses too
	// for ( const auto& cCanvas : fCanvasMap )
	// {
	//  cCanvas.second->Write( cCanvas.second->GetName(), TObject::kOverwrite );
	//  std::string cPdfName = fDirectoryName + "/" + cCanvas.second->GetName() + ".pdf";
	//  cCanvas.second->SaveAs( cPdfName.c_str() );
	// }

	fResultFile->Write();
	fResultFile->Close();

	std::cout << "Results saved!" << std::endl;
}


//////////////////////////////////////          PRIVATE METHODS             //////////////////////////////////////


int Commissioning::countHits( Module* pFe,  const Event* pEvent, std::string pHistName, uint8_t pParameter )
{
	// loop over Modules & Cbcs and count hits separately
	int cHitCounter = 0;

	//  get histogram to fill
	TH1F* cTmpHist = ( TH1F* )getHist( pFe, pHistName );

	for ( auto cCbc : pFe->fCbcVector )
	{
		for ( uint32_t cId = 0; cId < NCHANNELS; cId++ )
		{
			if ( pEvent->DataBit( cCbc->getFeId(), cCbc->getCbcId(), cId ) )
			{
				cTmpHist->Fill( pParameter );
				cHitCounter++;
			}
		}
	}
	return cHitCounter;
}

void Commissioning::updateHists( std::string pHistName, bool pFinal )
{
	for ( auto& cCanvas : fCanvasMap )
	{
		cCanvas.second->cd();

		// maybe need to declare temporary pointers outside the if condition?
		if ( pHistName == "module_latency" )
		{
			TH1F* cTmpHist = ( TH1F* )getHist( cCanvas.first, pHistName );
			cTmpHist->Draw( );
		}
		else if ( pHistName == "module_threshold" )
		{
			TH1F* cTmpHist = ( TH1F* )getHist( cCanvas.first, pHistName );
			cTmpHist->Draw( "P" );

			if ( pFinal )
			{
				// get the fit and draw that too
				TF1* cFit = ( TF1* )getHist( cCanvas.first, "module_fit" );

				// TODO: figure something smart out to restrict the range!

				// Estimate parameters for the Fit
				double cFirstNon0( 0 );
				double cFirst1( 0 );

				// Not Hole Mode
				if ( !fHoleMode )
				{
					for ( Int_t cBin = 1; cBin <= cTmpHist->GetNbinsX(); cBin++ )
					{
						double cContent = cTmpHist->GetBinContent( cBin );
						if ( !cFirstNon0 )
						{
							if ( cContent ) cFirstNon0 = cTmpHist->GetBinCenter( cBin );
						}
						else if ( cContent == 1 )
						{
							cFirst1 = cTmpHist->GetBinCenter( cBin );
							break;
						}
					}
				}
				// Hole mode
				else
				{
					for ( Int_t cBin = cTmpHist->GetNbinsX(); cBin >= 1; cBin-- )
					{
						double cContent = cTmpHist->GetBinContent( cBin );
						if ( !cFirstNon0 )
						{
							if ( cContent ) cFirstNon0 = cTmpHist->GetBinCenter( cBin );
						}
						else if ( cContent == 1 )
						{
							cFirst1 = cTmpHist->GetBinCenter( cBin );
							break;
						}
					}
				}

				// Get rough midpoint & width
				double cMid = ( cFirst1 + cFirstNon0 ) * 0.5;
				double cWidth = ( cFirst1 - cFirstNon0 ) * 0.5;

				cFit->SetParameter( 0, cMid );
				cFit->SetParameter( 1, cWidth );

				cTmpHist->Fit( cFit, "RNQ+" );
				cFit->Draw( "same" );

			}
		}
		else if ( pHistName == "module_lat_threshold" )
		{
			TH2F* cTmpHist = ( TH2F* )getHist( cCanvas.first, pHistName );
			cTmpHist->Draw( "box" );
		}
		cCanvas.second->Update();
	}
}

// TObject* Commissioning::getHist( Cbc* pCbc, std::string pName )
// {
//  auto cCbcHistMap = fCbcHistoMap.find( pCbc );
//  if ( cCbcHistMap == std::end( fCbcHistoMap ) ) std::cerr << RED << "Error: could not find the Histograms for CBC " << int( pCbc->getCbcId() ) <<  " (FE " << int( pCbc->getFeId() ) << ")" << RESET << std::endl;
//  else
//  {
//      auto cHisto = cCbcHistMap->second.find( pName );
//      if ( cHisto == std::end( cCbcHistMap->second ) ) std::cerr << RED << "Error: could not find the Histogram with the name " << pName << RESET << std::endl;
//      else
//          return cHisto->second;
//  }
// }

TObject* Commissioning::getHist( Module* pModule, std::string pName )
{
	auto cModuleHistMap = fModuleHistMap.find( pModule );
	if ( cModuleHistMap == std::end( fModuleHistMap ) ) std::cerr << RED << "Error: could not find the Histograms for Module " << int( pModule->getFeId() ) << RESET << std::endl;
	else
	{
		auto cHisto = cModuleHistMap->second.find( pName );
		if ( cHisto == std::end( cModuleHistMap->second ) ) std::cerr << RED << "Error: could not find the Histogram with the name " << pName << RESET << std::endl;
		else return cHisto->second;
	}
}

void Commissioning::parseSettings()
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

void Commissioning::initializeHists()
{
	// method to loop over all Modules / Cbcs and creating histograms for each

	for ( auto& cShelve : fShelveVector )
	{
		uint32_t cShelveId = cShelve->getShelveId();

		for ( auto& cBoard : cShelve->fBoardVector )
		{
			uint32_t cBoardId = cBoard->getBeId();

			for ( auto& cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();

				// Here create the Module-wise histos

				std::map<std::string, TObject*> cModuleMap;
				uint32_t cNCbc = cFe->getNCbc();

				// 1D Hist forlatency scan
				TString cName =  Form( "h_module_latency_Fe%d", cFeId );
				TObject* cObj = gROOT->FindObject( cName );
				if ( cObj ) delete cObj;
				TH1F* cLatHist = new TH1F( cName, Form( "Latency FE%d; Latency; # of Hits", cFeId ), 256, -.5, 255.5 );
				cModuleMap["module_latency"] = cLatHist;

				cName =  Form( "h_module_threshold_Fe%d", cFeId );
				cObj = gROOT->FindObject( cName );
				if ( cObj ) delete cObj;
				TH1F* cThresHist = new TH1F( cName, Form( "Threshold FE%d; Vcth; # of Hits", cFeId ), 256, -0.5, 255.5 );
				cModuleMap["module_threshold"] = cThresHist;

				cName =  Form( "f_module_threshold_Fit_Fe%d", cFeId );
				cObj = gROOT->FindObject( cName );
				if ( cObj ) delete cObj;
				TF1* cThresFit = new TF1( cName, MyErf, 0, 255, 2 );
				cModuleMap["module_fit"] = cThresFit;

				cName =  Form( "h_module_lat_threshold_Fe%d", cFeId );
				cObj = gROOT->FindObject( cName );
				if ( cObj ) delete cObj;
				TH2F* cThresLatHist = new TH2F( cName, Form( " Threshold/Latency FE%d; Latency; Threshold; # of Hits", cFeId ), 9, -4.5, 4.5, 256, -0.5, 255.5 );
				cModuleMap["module_lat_threshold"] = cThresLatHist;

				// now add to fModuleHistoMap
				fModuleHistMap[cFe] = cModuleMap;
			}
		}
	}
}