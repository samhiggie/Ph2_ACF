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
					uint32_t cCbcId = cCbc = getCbcId();

					// Create the Canvas to draw
					TCanvas* ctmpCanvas = new TCanvas( Form( "c_online_canvas_fe%dcbc%d", cFeId, cCbcId ), Form( "FE%dCBC%d  Online Canvas", cFeId, cCbcId ) );
					ctmpCanvas->Divide( 2, 1 );
					fCanvasMap[cCbc] = ctmpCanvas;

					//Create graphs for each CBC
					TString cName =  Form( "g_cbc_pulseshape_Fe%dCbc%d", cFeId, cCbcId );
					TObject* cObj = gROOT->FindObject( cName );
					if ( cObj ) delete cObj;
					TMultiGraph* cPulseGraph = new TMultiGraph();
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











//////////////////////////////////////		PRIVATE METHODS		/////////////////////////////////////////////

// TObject* Commissioning::getHist( Cbc* pCbc, std::string pName )
// {
// 	auto cCbcHistMap = fCbcHistoMap.find( pCbc );
// 	if ( cCbcHistMap == std::end( fCbcHistoMap ) ) std::cerr << RED << "Error: could not find the Histograms for CBC " << int( pCbc->getCbcId() ) <<  " (FE " << int( pCbc->getFeId() ) << ")" << RESET << std::endl;
// 	else
// 	{
// 		auto cHisto = cCbcHistMap->second.find( pName );
// 		if ( cHisto == std::end( cCbcHistMap->second ) ) std::cerr << RED << "Error: could not find the Histogram with the name " << pName << RESET << std::endl;
// 		else
// 			return cHisto->second;
// 	}
// }

// TObject* Commissioning::getHist( Module* pModule, std::string pName )
// {
// 	auto cModuleHistMap = fModuleHistMap.find( pModule );
// 	if ( cModuleHistMap == std::end( fModuleHistMap ) ) std::cerr << RED << "Error: could not find the Histograms for Module " << int( pModule->getFeId() ) << RESET << std::endl;
// 	else
// 	{
// 		auto cHisto = cModuleHistMap->second.find( pName );
// 		if ( cHisto == std::end( cModuleHistMap->second ) ) std::cerr << RED << "Error: could not find the Histogram with the name " << pName << RESET << std::endl;
// 		else return cHisto->second;
// 	}
// }


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

// void Commissioning::initializeHists()
// {
// 	// method to loop over all Modules / Cbcs and creating histograms for each

// 	for ( auto& cShelve : fShelveVector )
// 	{
// 		uint32_t cShelveId = cShelve->getShelveId();

// 		for ( auto& cBoard : cShelve->fBoardVector )
// 		{
// 			uint32_t cBoardId = cBoard->getBeId();

// 			for ( auto& cFe : cBoard->fModuleVector )
// 			{
// 				uint32_t cFeId = cFe->getFeId();

// 				for ( auto& cCbcID = cFe->fCbcVector )
// 				{
// 					uint32_t cCbcId = cCbc->getCbcId();

// 					// Here create the CBC-wise histos

// 					std::map<std::string, TObject*> cCbcMap;

// 					// 1D Hist forlatency scan
// 					TString cName =  Form( "g_cbc_pulseshape_Fe%dCbc%d", cFeId, cCbcId );
// 					TObject* cObj = gROOT->FindObject( cName );
// 					if ( cObj ) delete cObj;
// 					TMultiGraph* cPulseGraph = new TMultiGraph();
// 					cPulseGraph->GetXaxis()->SetTitle( "TestPulseDelay [ns]" );
// 					cPulseGraph->GetYaxis()->SetTitle( "TestPulseAmplitue [VCth]" );
// 					cCbcMap["cbc_pulseshape"] = cPulseGraph;

// 					// cName =  Form( "h_module_stub_latency_Fe%d", cFeId );
// 					// cObj = gROOT->FindObject( cName );
// 					// if ( cObj ) delete cObj;
// 					// TH1F* cStubHist = new TH1F( cName, Form( "Stub Lateny FE%d; Stub Lateny; # of Stubs", cFeId ), 256, -0.5, 255.5 );
// 					// cStubHist->SetMarkerStyle( 2 );
// 					// cCbcMap["module_stub_latency"] = cStubHist;


// 					// now add to fModuleHistoMap
// 					fModuleHistMap[cCbc] = cModuleMap;
// 				}
// 			}
// 		}
// 	}
// }