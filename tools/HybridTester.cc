#include "HybridTester.h"
using namespace std;

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

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

	fHistTop = new TH1F( cFrontName, "Front Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 253 ) + 1, -0.5, ( fNCbc / 2 * 253 ) + .5 );
	fHistTop->SetFillColor( 4 );
	fHistTop->SetFillStyle( 3001 );

	TString cBackName( "fHistBottom" );
	fHistBottom = ( TH1F* )( gROOT->FindObject( cBackName ) );
	if ( fHistBottom ) delete fHistBottom;

	fHistBottom = new TH1F( cBackName, "Back Pad Channels; Pad Number; Occupancy [%]", ( fNCbc / 2 * 253 ) + 1 , -0.5, ( fNCbc / 2 * 253 ) + .5 );
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

void HybridTester::Initialize( bool pThresholdScan )
{
	fThresholdScan = pThresholdScan;
	gStyle->SetOptStat( 000000 );
	gStyle->SetTitleOffset( 1.3, "Y" );
	//  special Visito class to count objects
	Counter cCbcCounter;
	accept( cCbcCounter );
	fNCbc = cCbcCounter.getNCbc();

	// need to read the calibrated offset Values from all Cbc's and fill the Calibrated OffsetMap
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
					std::map< uint8_t, uint8_t > cTmpCalibratedOffsetMap;

					for(uint8_t cChannel = 0; cChannel < 254; cChannel++)
					{
						TString cName = Form( "Channel%03d", cChannel + 1 );
						cTmpCalibratedOffsetMap[cChannel] = cCbc->getReg(cName.Data());
					}
					
					fOffsetMap[cCbc] = cTmpCalibratedOffsetMap;
				}
			}
		}
	}

	fDataCanvas = new TCanvas( "fDataCanvas", "SingleStripEfficiency", 1200, 800 );
	fDataCanvas->Divide( 2 );

	if ( fThresholdScan )
	{
		fSCurveCanvas = new TCanvas( "fSCurveCanvas", "Noise Occupancy as function of VCth" );
		fSCurveCanvas->Divide( fNCbc );
	}
	InitializeHists();




}

void HybridTester::InitializeGUI( bool pThresholdScan, const std::vector<TCanvas*>& pCanvasVector )
{
	fThresholdScan = pThresholdScan;
	gStyle->SetOptStat( 000000 );
	gStyle->SetTitleOffset( 1.3, "Y" );
	//  special Visito class to count objects
	Counter cCbcCounter;
	accept( cCbcCounter );
	fNCbc = cCbcCounter.getNCbc();

	fDataCanvas = pCanvasVector.at( 1 ); //since I ounly need one here
	fDataCanvas->SetName( "fDataCanvas" );
	fDataCanvas->SetTitle( "SingleStripEfficiency" );
	fDataCanvas->Divide( 2 );

	if ( fThresholdScan )
	{
		fSCurveCanvas = pCanvasVector.at( 2 ); // only if the user decides to do a thresholdscan
		fSCurveCanvas->SetName( "fSCurveCanvas" );
		fSCurveCanvas->SetTitle( "NoiseOccupancy" );
		fSCurveCanvas->Divide( fNCbc );
	}

	InitializeHists();
}



void HybridTester::ScanThreshold()
{
	std::cout << "Scanning noise Occupancy to find threshold for test with external source ... " << std::endl;

	auto cSetting = fSettingsMap.find( "HoleMode" );
	bool cHoleMode = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : true;

	// Necessary variables
	uint32_t cEventsperVcth = 10;
	bool cNonZero = false;
	bool cAllOne = false;
	bool cSlopeZero = false;
	uint32_t cAllOneCounter = 0;
	uint32_t cSlopeZeroCounter = 0;
	uint32_t cOldHitCounter = 0;
	uint8_t  cDoubleVcth;
	uint8_t cVcth = ( cHoleMode ) ?  0xFF :  0x00;
	int cStep = ( cHoleMode ) ? -10 : 10;



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

		uint32_t cN = 0;
		uint32_t cNthAcq = 0;
		uint32_t cHitCounter = 0;

		// maybe restrict to pBoard? instead of looping?
		for ( auto& cShelve : fShelveVector )
		{
			if ( cAllOne ) break;
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

						// loop over Modules & Cbcs and count hits separately
						cHitCounter += fillSCurves( pBoard,  cEvent, cVcth );
						cN++;

						if ( cN < cEventsperVcth )
							cEvent = fBeBoardInterface->GetNextEvent( pBoard );
						else break;
					}
					cNthAcq++;
				}
				std::cout << "DEBUG: Vcth: " << +cVcth << " Hits: " << cHitCounter << std::endl;
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

	// Wait for user to acknowledge and turn on external Source!
	std::cout << "Identified the threshold for 0 noise occupancy - Start external Signal source!" << std::endl;
	mypause();
}

void HybridTester::processSCurves( uint32_t pEventsperVcth )
{
	auto cSetting = fSettingsMap.find( "Threshold_NSigmas" );
	int cSigmas = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 4;
	bool cHoleMode = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : true;

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
		if ( !cHoleMode )
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

			uint8_t cThreshold = ceil( pedestal + cSigmas * fabs( noise ) );

			std::cout << "Identified a noise Occupancy of 50% at VCth " << static_cast<int>( pedestal ) << " -- increasing by " << cSigmas <<  " sigmas (=" << fabs( noise ) << ") to " << +cThreshold << " for Cbc " << int( cScurve.first->getCbcId() ) << std::endl;

			TLine* cLine = new TLine( cThreshold, 0, cThreshold, 1 );
			cLine->SetLineWidth( 3 );
			cLine->SetLineColor( 2 );
			cLine->Draw( "same" );

			fCbcInterface->WriteCbcReg( cScurve.first, "VCth", cThreshold );
		}

	}
	fSCurveCanvas->Update();

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

		void dumpResult(std::string fDirectoryName) {
			ofstream report( fDirectoryName + "/registers_test.txt"); // Creates a file in the current directory
			report << "Testing Cbc Registers one-by-one with complimentary bit-patterns (0xAA, 0x55)"<<endl;
			for ( const auto& cCbc : fBadRegisters ) {
				report << "Malfunctioning Registers on Cbc " << cCbc.first << " : " << std::endl;
				for ( const auto& cReg : cCbc.second ) report << cReg << std::endl;
				
			}
			report.close();
			cout << "Channels diagnosis report written to: " + fDirectoryName + "/registers_test.txt" << endl;
		}
	};

	// This should probably be done in the top level application but there I do not have access to the settings map

	std::cout << endl << "Running registers testing tool ... " << std::endl;
	RegTester cRegTester( fCbcInterface );
	accept( cRegTester );
	cRegTester.dumpResult(fDirectoryName);
	std::cout << "Done testing registers, re-configuring to calibrated state!" << std::endl;
	ConfigureHw();
}



void HybridTester::TestChannels()
{
	cout << endl << "Running channels testing tool ... " << endl;
	double cChannelDiagnosisThreshold = CH_DIAGNOSIS_DECISION_TH;
	std::vector<int> cBadChannelsTop;
    	std::vector<int> cBadChannelsBottom;
	int cHistogramBinId;
	int cTopHistSize = fNCbc*127+1;
	int cBottomHistSize = cTopHistSize;

	for(cHistogramBinId = 1; cHistogramBinId < cTopHistSize; cHistogramBinId++) 
	{	
		if (fTopHistogramMerged[cHistogramBinId] * 100 / fTotalEvents < cChannelDiagnosisThreshold) cBadChannelsTop.push_back(cHistogramBinId-1);
	}

	for(cHistogramBinId = 1; cHistogramBinId < cBottomHistSize; cHistogramBinId++) 
	{
		if (fBottomHistogramMerged[cHistogramBinId] * 100 / fTotalEvents < cChannelDiagnosisThreshold) cBadChannelsBottom.push_back(cHistogramBinId-1);
	}

	ofstream report( fDirectoryName + "/channels_test.txt"); // Create a file in the current directory
	report << "Testing run with decision threshold: " + patch::to_string(cChannelDiagnosisThreshold) + "%"<<endl;
	report << "Channels numbering convention from 0 to " + patch::to_string(cTopHistSize-2) + " for top and to " + patch::to_string(cBottomHistSize-2) + " for bottom side" <<endl;
	report << "Number of malfunctioning channels:  " + patch::to_string(cBadChannelsTop.size() + cBadChannelsBottom.size())<<endl;
    	report << "Malfunctioning channels from TOP side:  " + int_vector_to_string(cBadChannelsTop)<<endl;
    	report << "Malfunctioning channels from BOTTOM side:  " + int_vector_to_string(cBadChannelsBottom)<<endl;
	report.close();
	cout << "Channels testing report written to: " << endl << fDirectoryName + "/channels_test.txt" << endl;
}

void HybridTester::TestChannels(double pTopHistogram[], int pTopHistogramSize, double pBottomHistogram[], int pBottomHistogramSize, double pDecisionThreshold)
{
	cout << endl << "Running channels testing tool 2... " << endl;
	std::vector<int> cBadChannelsTop;
    	std::vector<int> cBadChannelsBottom;
	int cHistogramBinId;

	for(cHistogramBinId = 1; cHistogramBinId < pTopHistogramSize; cHistogramBinId++) 
	{	
		if (pTopHistogram[cHistogramBinId] * 100 / fTotalEvents < pDecisionThreshold) cBadChannelsTop.push_back(cHistogramBinId-1);
	}

	for(cHistogramBinId = 1; cHistogramBinId < pBottomHistogramSize; cHistogramBinId++) 
	{
		if (pBottomHistogram[cHistogramBinId] * 100 / fTotalEvents < pDecisionThreshold) cBadChannelsBottom.push_back(cHistogramBinId-1);
	}

	ofstream report( fDirectoryName + "/channels_test2.txt"); // Create a file in the current directory
	report << "Testing run with decision threshold: " + patch::to_string(pDecisionThreshold) + "%"<<endl;
	report << "Channels numbering convention from 0 to " + patch::to_string(pTopHistogramSize-2) + " for top and to " + patch::to_string(pBottomHistogramSize-2) + " for bottom side" <<endl;
	report << "Number of malfunctioning channels:  " + patch::to_string(cBadChannelsTop.size() + cBadChannelsBottom.size())<<endl;
    	report << "Malfunctioning channels from TOP side:  " + int_vector_to_string(cBadChannelsTop)<<endl;
    	report << "Malfunctioning channels from BOTTOM side:  " + int_vector_to_string(cBadChannelsBottom)<<endl;
	report.close();
	cout << "Channels testing report written to: " << endl << fDirectoryName + "/channels_test2.txt" << endl;
}

void HybridTester::Measure()
{
	std::cout << "Mesuring Efficiency per Strip ... " << std::endl;
	auto cSetting = fSettingsMap.find( "Nevents" );
	uint32_t cTotalEvents = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 200;
	fTotalEvents = cTotalEvents;
	std::cout << "Taking data with " << cTotalEvents << " Events!" << std::endl;
	CbcRegReader cReader( fCbcInterface, "VCth" );
	accept( cReader );
		
	//here I need to set up usb connection parameters for antenna board
	const static int cUsbEndpointBulkIn = 0x82;  // usb endpoint 0x82 address for USB IN bulk transfers
	const static int cUsbEndpointBulkOut = 0x01;  // usb endpoint 0x01 address for USB OUT bulk transfers
	const static int cUsbTimeout = 5000;  // usb operation timeout in ms
	
	usb_dev_handle *antenna_usb_handle; // pointer to the device handle from libusb0.1, see usb.h file for more details
 	int result; // variable for gathering results from USB related transfers, useful only for debugging 
	
 	if ((antenna_usb_handle = setup_libusb_access()) == NULL)
	{ 
		cout<<"Abandon the ship! Failed to connect with antenna setup, check if it is plugged in the USB port."<<endl;
		exit(-1); // ok this is maybe a slight overreaction, but there is no point to continue testing if antenna test board is not connected
	}
	usb_claim_interface(antenna_usb_handle, 0); // claim the interface for antenna connection, so kernel cannot do it when we need to use the device
	usb_reset(antenna_usb_handle); // this thing is extremely useful, chip needs to wake up from whatever state it was in after its interface was reclaimed
    	char buf_in[4] = {0}; // buf_in[] is used just for reading back data from chip, via usb, I used it only for debugging purpose to check if correct CS line has been set for SPI transfers
    	char analog_switch_cs = 0; // analog switch CS line number (for 2cbc2 hybrid there are only two switches that can be connected to CS0 or CS1 line
     	uint8_t channel_position = 1; // analog switch channel number from 1 to 8
	char control_msg_set_spi_word[2] = {0, 0x19};
	char buf_out[2] = {0, 2};
	char bulk_buffer_out[9] = {0, 0, 1, 0, 1, 0, 0, 0, 0};
	double zero_fill[255] = {0}; // this is an array of zeros to clear histograms, since I could not find a method for clearing histograms, I just fill them with zeros
	double cTopHistogramMerged[fNCbc*127+1];
	double cBottomHistogramMerged[fNCbc*127+1];	
	for (int i=0; i<fNCbc*127+1; i++)
	{
		cTopHistogramMerged[i] = 0;
		cBottomHistogramMerged[i] = 0;
	}

	//setting offset to 0x00 for all the groups except the group of ID = 0 
	//for (uint8_t cGroup = 1; cGroup<8; cGroup++){
		
		//setOffset(0x00, cGroup);
	 	/*need also to disable the channels because setting offset to 0x00 is not sufficient, still some hits are measured 
		on these channels and they add up to the histogram with every iteration*/
		/*TO DO HERE*/		
		
	//	}
	/*
	fHistTop->Scale( 100 / double_t( cTotalEvents ) );
	fHistTop->GetYaxis()->SetRangeUser( 0, cTotalEvents );
	fHistBottom->Scale( 100 / double_t( cTotalEvents ) );
	fHistBottom->GetYaxis()->SetRangeUser( 0, cTotalEvents );
	*/
	fHistTop->GetYaxis()->SetRangeUser( 0, cTotalEvents );
	fHistBottom->GetYaxis()->SetRangeUser( 0, cTotalEvents );
	//for (analog_switch_cs = 0; analog_switch_cs < 1; analog_switch_cs++){ 
	for (analog_switch_cs = 0; analog_switch_cs < fNCbc; analog_switch_cs++){ 
		buf_out[0] = analog_switch_cs;
		control_msg_set_spi_word[0] = analog_switch_cs;
		/*First activate chip select line of corresponding analog switch for SPI communication.*/
    		result = usb_control_msg(antenna_usb_handle, 0x40, 0x25, 0, 0, (char *) buf_out, sizeof(buf_out), cUsbTimeout);
		/*Check if correct number of chip select channel was stored in the cp2130 chip.*/
    		result = usb_control_msg(antenna_usb_handle, 0xC0, 0x24, 0, 0, (char *) buf_in, sizeof(buf_in), cUsbTimeout);
		/*Set SPI transfer parameters.*/
    		result = usb_control_msg(antenna_usb_handle, 0x40, 0x31, 0, 0, (char *) control_msg_set_spi_word, sizeof(control_msg_set_spi_word), cUsbTimeout);
		/*Finally we can write through cp2130 to analog switch. We are writing an array of chars where the last byte is giving the position of channels to be turned of. 
		It is identified by '1' position in binary representation of that byte value.*/
		result = usb_bulk_write(antenna_usb_handle, cUsbEndpointBulkOut, (char *) bulk_buffer_out, sizeof(bulk_buffer_out), cUsbTimeout); // turning off all channels of analog switch 
		sleep(0.1);
		
		//Sliding window for taking measurements, the channels groups are iterated to avoid measuring through all the channels at once which was a possible source of X-talk
		//for (channel_position = 1; channel_position < 2; channel_position++){
		for (channel_position = 1; channel_position < 10; channel_position++){	
		
			if (channel_position == 9)
			{ 
				bulk_buffer_out[8] = 0; // this is just to turn off all the channels of analog switch (if powered it holds last written configuration) at the end of the loop
			}
			else
			{ 
				bulk_buffer_out[8] = (char)((1<<(channel_position-1))&0xFF);
			}
			//print_buffer((char *) bulk_buffer_out, sizeof(bulk_buffer_out));
			result = usb_bulk_write(antenna_usb_handle, cUsbEndpointBulkOut, (char *) bulk_buffer_out, sizeof(bulk_buffer_out), cUsbTimeout);
			sleep(0.1);
	
			//if (cSlider){
				//settting offset to 0x00 for previously active group, do not do it for the first iteration when cSlider = 0
				//setOffset(0x00, cSlider-1);
				/*need also to disable the channel because setting offset to 0x00 is not sufficient, still some hits are measured 
				on these channels and they add up to the histogram with every iteration*/
				/*TO DO HERE*/
			
				/*Enabling the current group for measurements*/
				/*TO DO HERE*/
		
				//Setting calibrated offset to the currently enabled group
				//setOffset(cSlider);
			//}
		
	
			for ( auto& cShelve : fShelveVector ) //(to be deleted) loop fires 1 time for 2CBC2 hybrid
			{
				for ( BeBoard* pBoard : cShelve->fBoardVector ) //(to be deleted) loop fires 1 time for 2CBC2 hybrid
				{
					uint32_t cN = 0;
					uint32_t cNthAcq = 0;
				
					/*
					//loops over modules and cbcs made to perform channels masking - doesnt work, need to clarify why!
					//masking in post-calibration files FE0CBC0.txt/FE0CBC1.txt directly also do not affect occupancy measurements at all! 
					for (Module* pModule : pBoard->fModuleVector){
						for (Cbc* pCbc : pModule->fCbcVector){										
							fCbcInterface->WriteCbcReg(pCbc, "MaskChannelFrom008downto001", 0x00);
							//fCbcInterface->WriteCbcReg(pCbc, "Channel009", 0x00);
							std::cout<<"I am masking"<<endl;
						}
					}*/
				 
					while ( cN <  cTotalEvents )
					{
						Run( pBoard, cNthAcq );

						const Event* cEvent = fBeBoardInterface->GetNextEvent( pBoard );
						// Loop over Events from this Acquisition
						while ( cEvent )
						{
							if ( cN == cTotalEvents )
								break;

							HistogramFiller cFiller( fHistBottom, fHistTop, cEvent );
							pBoard->accept( cFiller );

							//if ( (cN+1) % 100 == 0 )
							if (cN == cTotalEvents-1)							
								UpdateHists();

							cN++;

							if ( cN < cTotalEvents )
								cEvent = fBeBoardInterface->GetNextEvent( pBoard );
							else break;
						}
					
						cNthAcq++;
					}
					/*Here the reconstruction of histograms happens*/
					if(analog_switch_cs == 0) // it means that I am illuminating top pads (of course if top antenna switch chip select line is 0
					{
						for(uint8_t channel_id = 1; channel_id < fNCbc*127 + 1; channel_id++)
						{
							if (fTopHistogramMerged[channel_id] < fHistTop->GetBinContent(channel_id)) fTopHistogramMerged[channel_id] = fHistTop->GetBinContent(channel_id);
							if (cTopHistogramMerged[channel_id] < fHistTop->GetBinContent(channel_id)) cTopHistogramMerged[channel_id] = fHistTop->GetBinContent(channel_id);
						}
				
					}
					else if(analog_switch_cs == 1) // it means that I am illuminating top pads (of course if top antenna switch chip select line is 0
					{
						for(uint8_t channel_id = 1; channel_id < fNCbc*127 + 1; channel_id++)
						{
							if (fBottomHistogramMerged[channel_id] < fHistBottom->GetBinContent(channel_id)) fBottomHistogramMerged[channel_id] = fHistBottom->GetBinContent(channel_id);
							if (cBottomHistogramMerged[channel_id] < fHistBottom->GetBinContent(channel_id)) cBottomHistogramMerged[channel_id] = fHistBottom->GetBinContent(channel_id);
						}			
					}
					/*Here clearing histograms after each event*/
					fHistBottom->SetContent((double *) zero_fill);
					fHistTop->SetContent((double *) zero_fill);
				
				}
			}
		}

	}
	
	fHistBottom->SetContent((double *) fBottomHistogramMerged);
	fHistTop->SetContent((double *) fTopHistogramMerged);	
	
	fHistTop->Scale( 100 / double_t( cTotalEvents ) );
	fHistTop->GetYaxis()->SetRangeUser( 0, 100 );
	fHistBottom->Scale( 100 / double_t( cTotalEvents ) );
	fHistBottom->GetYaxis()->SetRangeUser( 0, 100 );
	
	UpdateHists();
	
	/*We release interface so any other software or kernel driver can claim it */
	usb_release_interface(antenna_usb_handle, 0);
	/*we close the usb connection with the cp2130 chip*/
 	usb_close(antenna_usb_handle);
	/*waiting for keyboard input*/
 	//std::cin.ignore();
	TestChannels((double *) cTopHistogramMerged, sizeof(cTopHistogramMerged)/sizeof(cTopHistogramMerged[0]), (double *) cBottomHistogramMerged, sizeof(cBottomHistogramMerged)/sizeof(cBottomHistogramMerged[0]), 80.0);
	
}

void HybridTester::setOffset( uint8_t pOffset, int  pTGrpId )
{
	// add a check: if pOriginal values is true, not pOffset is written to the Registers but the original offset values from the fOffsetMap (calibrated values)
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
				fRegVec.push_back( std::make_pair( cRegName.Data(), fOffset ) );
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

void HybridTester::setOffset( int  pTGrpId )
{
	// sets the offset to the calibrated value on each channel of the group
	struct OffsetWriter : public HwDescriptionVisitor
	{
		CbcInterface* fInterface;
		RegisterVector fRegVec;
		CalibratedOffsetMap fOffsetMap;
		std::vector<uint8_t> fTestGrpChannelIdVec;

		//will have to pass the channel vector to OffsetWriter
		OffsetWriter( CbcInterface* pInterface, std::vector<uint8_t> pTestGroupChnlVec, CalibratedOffsetMap pOffsetMap ): fInterface( pInterface ), fTestGrpChannelIdVec( pTestGroupChnlVec ), fOffsetMap(pOffsetMap) {}

		void visit( Cbc& pCbc ) {
			// find the original offset value map for this CBC
			auto cCbcOffsets = fOffsetMap.find(&pCbc);
			if(cCbcOffsets == std::end(fOffsetMap)) std::cout << "ERROR: could not find original offset values for CBC "<< +pCbc.getCbcId() << std::endl;
			else
			{
				//Here the loop will be over channels in the test group
				for ( auto& cChannel : fTestGrpChannelIdVec ) {
					TString cRegName = Form( "Channel%03d", cChannel + 1 );
					uint8_t cCalibratedOffset;
					auto cOffsetValue = cCbcOffsets->second.find(cChannel);
					if(cOffsetValue == std::end(cCbcOffsets->second))
					{
						cCalibratedOffset = 0x00;
						std::cout << "ERROR: Could not find original Offset." << std::endl; 
					}
					else
					{
						cCalibratedOffset = cOffsetValue->second;
					}
					fRegVec.push_back( std::make_pair( cRegName.Data(), cCalibratedOffset ) );
				}
			fInterface->WriteCbcMultReg( &pCbc, fRegVec );
			
			}
		}
	};

	if ( fTestGroupChannelMap.find( pTGrpId ) != fTestGroupChannelMap.end() )
	{
		OffsetWriter cWriter( fCbcInterface, fTestGroupChannelMap[pTGrpId], fOffsetMap );
		accept( cWriter );
	}
}


void HybridTester::SaveResults()
{
	int hybrid_id = -1;
	ifstream infile;
	string line_buffer;
	string content_buffer;
    	string date_string = currentDateTime();
	string hybrid_id_string = patch::to_string(hybrid_id);
    	string filename = "Results/HybridTestingDatabase/Hybrid" + hybrid_id_string + "_on_" + date_string + ".txt";
	ofstream myfile;
    	myfile.open (filename.c_str());
    	myfile << "Hybrid ID: "<<hybrid_id_string<<endl;
    	myfile << "Created on: "<<date_string<<endl<<endl;
    	myfile << " Hybrid Testing Report"<<endl;
    	myfile << "-----------------------"<<endl<<endl;
    	myfile << " Write/Read Registers Test"<<endl;
    	myfile << "---------------------------"<<endl;

	infile.open (fDirectoryName + "/registers_test.txt");
        while(getline(infile,line_buffer)) content_buffer += line_buffer + "\r\n"; // To get you all the lines.
	if (content_buffer == "") myfile << "Test not performed!"<<endl;

	infile.close();
    	myfile << content_buffer<<endl;
	content_buffer = "";
	myfile << " Channels Functioning Test"<<endl;
    	myfile << "---------------------------"<<endl;
	infile.open (fDirectoryName + "/channels_test2.txt");
        while(getline(infile,line_buffer)) content_buffer += line_buffer + "\r\n"; // To get you all the lines.
	if (content_buffer == "") myfile << "Test not performed!"<<endl;
	infile.close();
	myfile << content_buffer<<endl;
	myfile.close();

	fHistTop->Write( fHistTop->GetName(), TObject::kOverwrite );
	fHistBottom->Write( fHistBottom->GetName(), TObject::kOverwrite );
	fDataCanvas->Write( fDataCanvas->GetName(), TObject::kOverwrite );

	fResultFile->Write();
	fResultFile->Close();


	std::cout<<endl << "Resultfile written correctly!" << std::endl;

	std::string cPdfName = fDirectoryName + "/HybridTestResults.pdf";
	fDataCanvas->SaveAs( cPdfName.c_str() );
	if ( fThresholdScan )
	{
		cPdfName = fDirectoryName + "/ThresholdScanResults.pdf";
		fSCurveCanvas->SaveAs( cPdfName.c_str() );
	}
	cout <<endl<< "Summary testing report written to: " << endl << filename <<endl;

}
/*string DecimalToBinaryString(int a)
{
    std::string binary = "";
    int mask = 1;
    for(int i = 0; i < 31; i++)
    {
        if((mask&a) >= 1)
            binary = "1"+binary;
        else
            binary = "0"+binary;
        mask<<=1;
    }
    return binary;
}

string CharToBinaryString(int a)
{
    std::string binary = "";
    int mask = 1;
    for(int i = 0; i < 7; i++)
    {
        if((mask&a) >= 1)
            binary = "1"+binary;
        else
            binary = "0"+binary;
        mask<<=1;
    }
    return binary;
}

void print_buffer(char* buf, int8_t buf_size)
{
    for(uint8_t i=0; i<buf_size; i++)
    {
        std::cout<<buf[i]+0<<" ";
    }
    std::cout<<std::endl;
}

std::string int_vector_to_string(std::vector<int> int_vector)
{
    std::string output_string = "";
    for (std::vector<int>::iterator it = int_vector.begin(); it != int_vector.end(); ++it)
    {
        output_string += patch::to_string(*it) + "; ";
    }
    return output_string;
}

std::string double_vector_to_string(std::vector<double> int_vector)
{
    std::string output_string = "";
    for (std::vector<double>::iterator it = int_vector.begin(); it != int_vector.end(); ++it)
    {
        output_string += patch::to_string(*it) + "; ";
    }
    return output_string;
}
*/

