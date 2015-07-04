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

	std::pair<uint8_t, uint8_t> PulseShape::ScanTestPulseDelay(uint8_t pVcth, uint8_t pChannelId){

		for(uint8_t cTestPulseDelay = 0 ; cTestPulseDelay < 256; cTestPulseDelay++)
		{
			
			// set test pulse delay: not sure yet if beBoard register or CbcRegister
			CbcRegWriter cWriter( fCbcInterface, "VCth", pVcth );
			this->accept( cWriter );
			// CbcRegWriter cWriter( fCbcInterface, "", cTestPulseDelay);
			// this->accept( cWriter);

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
						fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
						const std::vector<Event*>& events = fBeBoardInterface->GetEvents( pBoard );

						// Loop over Events from this Acquisition
						for ( auto& cEvent : events )
						{
							for ( auto cFe : pBoard->fModuleVector )
								cNHits += countHits( cFe, cEvent, "Delay", cTestPulseDelay );
							cN++;
						}
						cNthAcq++;
					}
					fBeBoardInterface->Stop( pBoard, cNthAcq );
					std::cout << "Delay " << +cTestPulseDelay << " Hits " << cNHits  << " Events " << cN << std::endl;

				}
			}

			// done counting hits for all FE's, now update the Histograms
			updateHists( "Delay", false );
			// for each event, we check if Channel pChannelId has a hit (this can go in a separate method that also fills the histogram)


		}

	}

//////////////////////////////////////		PRIVATE METHODS		/////////////////////////////////////////////



void PulseShape::parseSettings()
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

void PulseShape::setSystemTestPulse(uint8_t pTPAmplitude, uint8_t pChannelId)
{
	// translate the channel id to a test group
	TestGroup cTestGroup = new TestGroup()

	// set the TestPulsePot register on the CBC to the correct amplituede
	// CbcRegWriter cWriter(fCbcInterface, "TestPulsePot", pTPAmpliude);
	// this->accept(cWriter);

	// BeBoardRegWriter cBoardWriter();
	// this->accept(cBoardWriter);

	// initialize the CBC vs Channel map
	struct initMapVisitor : public HWDescriptionVisitor {
		ChannelMap cChannelMap;
		uint8_t cChannelId;
		initMapVisitor(ChannelMap pChannelMap, uint8_t pChannelId): cChannelMap(pChannelMap), cChannelId(pChannelId);

		void visit(Cbc* pCbc){
			Channel cChannel(pCbc->getBeId(), pCbc->getFeId(), pCbc->getCbcId(), cChannelId);
			cChannelMap[pCbc] = cChannel;
		}
	};

	initMapVisitor cInitMapVisitor(fChannelMap, pChannelId);
	this->accept(cInitMapVisitor);
}
