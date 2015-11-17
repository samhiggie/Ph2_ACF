#include "Calibration.h"

void Calibration::Initialise( bool pAllChan )
{
	// Initialize the TestGroups
	MakeTestGroups( pAllChan );

	// now read the settings from the map
	auto cSetting = fSettingsMap.find( "HoleMode" );
	fHoleMode = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 1;
	cSetting = fSettingsMap.find( "TargetVcth" );
	fTargetVcth = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 0x78;
	cSetting = fSettingsMap.find( "TargetOffset" );
	fTargetOffset = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 0x50;
	cSetting = fSettingsMap.find( "Nevents" );
	fEventsPerPoint = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 10;
	cSetting = fSettingsMap.find( "TestPulseAmplitude" );
	fTestPulseAmplitude = ( cSetting != std::end( fSettingsMap ) ) ? cSetting->second : 0;

	if ( fTestPulseAmplitude == 0 )fTestPulse = 0;
	else fTestPulse = 1;


	// Canvases
	fVplusCanvas = new TCanvas( "VPlus", "VPlus", 650, 650 );
	fOffsetCanvas = new TCanvas( "Offset", "Offset", 650, 650 );
	fOccupancyCanvas = new TCanvas( "Occupancy", "Occupancy", 650, 650 );
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

					fVplusMap[cCbc] = 0;

					TString cName = Form( "h_VplusValues_Fe%dCbc%d", cFeId, cCbcId );
					TObject* cObj = gROOT->FindObject( cName );
					if ( cObj ) delete cObj;
					TProfile* cHist = new TProfile( cName, Form( "Vplus Values for Test Groups FE%d CBC%d ; CBCId; Vplus", cFeId, cCbcId ), 9, -.5, 8.5 );
					cHist->SetLineColor( 9 );
					cHist->SetLineWidth( 2 );
					bookHistogram( cCbc, "Vplus", cHist );

					cName = Form( "h_Offsets_Fe%dCbc%d", cFeId, cCbcId );
					cObj = gROOT->FindObject( cName );
					if ( cObj ) delete cObj;
					TH1F* cOffsetHist = new TH1F( cName, Form( "Offsets FE%d CBC%d ; Channel; Offset", cFeId, cCbcId ), 254, -.5, 253.5 );
					uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;
					for ( int iBin = 0; iBin < NCHANNELS; iBin++ )
						cOffsetHist->SetBinContent( iBin, cOffset );
					bookHistogram( cCbc, "Offsets", cOffsetHist );

					cName = Form( "h_Occupancy_Fe%dCbc%d", cFeId, cCbcId );
					cObj = gROOT->FindObject( cName );
					if ( cObj ) delete cObj;
					TH1F* cOccHist = new TH1F( cName, Form( "Occupancy FE%d CBC%d ; Channel; Occupancy", cFeId, cCbcId ), 254, -.5, 254.5 );
					bookHistogram( cCbc, "Occupancy", cOccHist );
				}
			}
			fNCbc = cCbcCount;
			fNFe = cFeCount;
		}
	}

	uint32_t cPads = ( cCbcIdMax > cCbcCount ) ? cCbcIdMax : cCbcCount;

	fVplusCanvas->DivideSquare( cPads );
	fOffsetCanvas->DivideSquare( cPads );
	fOccupancyCanvas->DivideSquare( cPads );


	std::cout << "Created Object Maps and parsed settings:" << std::endl;
	std::cout << "	Hole Mode = " << fHoleMode << std::endl;
	std::cout << "	Nevents = " << fEventsPerPoint << std::endl;
	std::cout << "	TargetVcth = " << int( fTargetVcth ) << std::endl;
	std::cout << "	TargetOffset = " << int( fTargetOffset ) << std::endl;
	std::cout << "	TestPulseAmplitude = " << int( fTestPulseAmplitude ) << std::endl;
}

void Calibration::MakeTestGroups( bool pAllChan )
{
	if ( !pAllChan )
	{
		for ( int cGId = 0; cGId < 8; cGId++ )
		{
			std::vector<uint8_t> tempchannelVec;

			for ( int idx = 0; idx < 16; idx++ )
			{
				int ctemp1 = idx * 16 + cGId * 2;
				int ctemp2 = ctemp1 + 1;
				if ( ctemp1 < 254 ) tempchannelVec.push_back( ctemp1 );
				if ( ctemp2 < 254 )  tempchannelVec.push_back( ctemp2 );

			}
			fTestGroupChannelMap[cGId] = tempchannelVec;

		}
	}
	else
	{
		int cGId = -1;
		std::vector<uint8_t> tempchannelVec;

		for ( int idx = 0; idx < 254; idx++ )
			tempchannelVec.push_back( idx );
		fTestGroupChannelMap[cGId] = tempchannelVec;

	}
}


void Calibration::FindVplus()
{
	// first, set VCth to the target value for each CBC
	CbcRegWriter cWriter( fCbcInterface, "VCth", fTargetVcth );
	accept( cWriter );

	// now all offsets are either off (0x00 in holes mode, 0xFF in electrons mode)
	// next a group needs to be enabled - therefore now the group loop
	std::cout << BOLDBLUE << "Extracting Vplus ..." << RESET << std::endl;
	for ( auto& cTGroup : fTestGroupChannelMap )
	{
		// start with a fresh <Cbc, Vplus> map
		clearVPlusMap();

		// looping over the test groups, enable it
		std::cout << GREEN << "Enabling Test Group...." << cTGroup.first << RESET << std::endl;
		setOffset( fTargetOffset, cTGroup.first );  // takes the group ID

		bitwiseVplus( cTGroup.first );

		std::cout << GREEN << "Disabling Test Group...." << cTGroup.first << RESET << std::endl;
		uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;
		setOffset( cOffset, cTGroup.first );
		// done looping all the bits - I should now have the Vplus value that corresponds to 50% occupancy at the desired VCth and Offset for this test group mapped against CBC
		for ( auto& cCbc : fVplusMap )
		{
			TProfile* cTmpProfile = static_cast<TProfile*>( getHist( cCbc.first, "Vplus" ) );
			cTmpProfile->Fill( cTGroup.first, cCbc.second ); // fill Vplus value for each test group
			updateHists( "Vplus" );
		}
	}
	// done extracting reasonable Vplus values for all test groups, now find the mean
	// since I am lazy and do not want to iterate all boards, FEs etc, i Iterate fVplusMap
	for ( auto& cCbc : fVplusMap ) //this toggles bit i on Vplus for each
	{
		TProfile* cTmpProfile = static_cast<TProfile*>( getHist( cCbc.first, "Vplus" ) );
		cCbc.second = cTmpProfile->GetMean( 2 );

		fCbcInterface->WriteCbcReg( cCbc.first, "Vplus", cCbc.second );
		std::cout << BOLDGREEN <<  "Mean Vplus value found considering all test groups is " << BOLDRED << +cCbc.second << RESET << std::endl;
	}
}

void Calibration::bitwiseVplus( int pTGroup )
{
	// now go over the VPlus bits for each CBC, start with the MSB, flip it to one and measure the occupancy
	for ( int iBit = 7; iBit >= 0; iBit-- )
	{
		for ( auto& cCbc : fVplusMap ) //this toggles bit i on Vplus for each
		{
			toggleRegBit( cCbc.second, iBit );
			fCbcInterface->WriteCbcReg( cCbc.first, "Vplus", cCbc.second );
			// std::cout << "IBIT " << +iBit << " DEBUG Setting Vplus for CBC " << +cCbc.first->getCbcId() << " to " << +cCbc.second << " (= 0b" << std::bitset<8>( cCbc.second ) << ")" << std::endl;
		}
		// now each CBC has the MSB Vplus Bit written
		// now take data
		const std::vector<Event*> events = takeNEvents( fEventsPerPoint );

		// done taking data, now find the occupancy per CBC
		for ( auto& cCbc : fVplusMap ) //this toggles bit i on Vplus for each
		{
			// if the occupancy is larger than 0.5 I need to flip the bit back to 0, else leave it
			float cOccupancy = findOccupancy( cCbc.first, pTGroup, events );
			if ( cOccupancy > 0.50 )
			{
				toggleRegBit( cCbc.second, iBit ); //here could also use setRegBit to set to 0 explicitly
				fCbcInterface->WriteCbcReg( cCbc.first, "Vplus", cCbc.second );
			}

			if ( iBit == 0 ) std::cout << "DEBUG Found Occupancy of " << cOccupancy << "for CBC " << +cCbc.first->getCbcId() << " , test Group " << pTGroup << " to " << +cCbc.second << " (= 0b" << std::bitset<8>( cCbc.second ) << ")" << std::endl;
		}
	}
}

void Calibration::FindOffsets( bool pStandalone )
{
	// do a binary search for the correct offset value

	// just to be sure, configure the correct VCth and VPlus values
	CbcRegWriter cWriter( fCbcInterface, "VCth", fTargetVcth );
	accept( cWriter );

	// uint8_t cInitialOffset = ( fHoleMode ) ? 0x00 : 0xFF;
	// initializeOffsetMap( cInitialOffset );

	// ok, done, all the offsets are at the starting value, VCth & Vplus are written

	// now loop over test groups
	for ( auto& cTGroup : fTestGroupChannelMap )
	{
		std::cout << GREEN << "Enabling Test Group...." << cTGroup.first << RESET << std::endl;

		bitwiseOffset( cTGroup.first );


		// now all the bits are toggled or not, I still want to verify that the occupancy is ok
		int cMultiple = 3;
		std::cout << "Verifying Occupancy with final offsets by taking " << fEventsPerPoint* cMultiple << " Triggers!" << std::endl;
		const std::vector<Event*> events = takeNEvents( fEventsPerPoint * cMultiple );
		// now find the occupancy for each channel and update the TProfile
		// findOccupancyChannel( cTGroup.first, events );
		updateHists( "Occupancy" );

		uint8_t cOffset = ( fHoleMode ) ? 0x00 : 0xFF;
		setOffset( cOffset, cTGroup.first );
		std::cout << GREEN << "Disabling Test Group...." << cTGroup.first << RESET << std::endl;
	}
	setRegValues();
}


void Calibration::bitwiseOffset( int pTGroup )
{
	// loop over the bits
	for ( int iBit = 7; iBit >= 0; iBit-- )
	{
		std::cout << "Searching for the correct offsets by flipping bit " << iBit << std::endl;
		// now, for all the channels in the group and for each cbc, toggle the MSB of the offset from the map
		toggleOffset( pTGroup, iBit, true );

		updateHists( "Offsets" );

		// now the offset for the current group is changed
		// now take data
		const std::vector<Event*> events = takeNEvents( fEventsPerPoint );
		// now find the occupancy for each channel and update the TProfile
		findOccupancyChannel( pTGroup, events );

		updateHists( "Occupancy" );

		// now call toggleOffset again with pBegin = false; this method checks the occupancy and flips a bit back if necessary
		toggleOffset( pTGroup, iBit, false );
		updateHists( "Offsets" );
	}
}


const std::vector<Event*> Calibration::takeNEvents( uint32_t pNEvents )
{
	uint32_t cN = 0;
	uint32_t cNthAcq = 0;
	std::vector<Event*> events;

	for ( auto cShelve : fShelveVector )
	{
		for ( BeBoard* pBoard : cShelve->fBoardVector )
		{
			// Counter cCounter;
			// pBoard->accept( cCounter );

			fBeBoardInterface->Start( pBoard );

			while ( cN < pNEvents )
			{
				fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
				std::vector<Event*> tmpevents = fBeBoardInterface->GetEvents( pBoard );

				events.insert( std::end( events ), std::begin( tmpevents ), std::end( tmpevents ) );
				cN = events.size();
				cNthAcq++;
			}
			fBeBoardInterface->Stop( pBoard, cNthAcq );
		}
	}
	return events;
}

float Calibration::findOccupancy( Cbc* pCbc, int pTGroup, const std::vector<Event*> pEvents )
{
	float cOccupancy = 0;

	// iterate the events
	for ( auto& cEvent : pEvents )
	{
		// iterate the channels in current group
		for ( auto& cChanId : fTestGroupChannelMap[pTGroup] )
		{
			// I am counting all hits in a group for each event
			if ( cEvent->DataBit( pCbc->getFeId(), pCbc->getCbcId(), cChanId ) ) cOccupancy++;
		}
	}
	// return the hitcount divided by the the number of channels and events
	return cOccupancy / ( static_cast<float>( fTestGroupChannelMap[pTGroup].size() * pEvents.size() ) );
}

void Calibration::findOccupancyChannel( int pTGroup, const std::vector<Event*> pEvents )
{
	// iterate the events
	for ( auto& cEvent : pEvents )
	{
		for ( auto cShelve : fShelveVector )
		{
			for ( auto cBoard : cShelve->fBoardVector )
			{
				for ( auto cFe : cBoard->fModuleVector )
				{
					for ( auto cCbc : cFe->fCbcVector )
					{
						// find the TProfile for occupancy measurment of current channel
						TH1F* cOccHist = static_cast<TH1F*>( getHist( cCbc, "Occupancy" ) );
						// iterate the channels in current group
						for ( auto& cChanId : fTestGroupChannelMap[pTGroup] )
						{
							// I am filling the occupancy profile for each CBC for the current test group
							if ( cEvent->DataBit( cCbc->getFeId(), cCbc->getCbcId(), cChanId ) )
								cOccHist->Fill( cChanId );
						}
					}
				}
			}
		}
	}
}

void Calibration::clearVPlusMap()
{
	for ( auto cShelve : fShelveVector )
	{
		for ( auto cBoard : cShelve->fBoardVector )
		{
			for ( auto cFe : cBoard->fModuleVector )
			{
				for ( auto cCbc : cFe->fCbcVector )
					fVplusMap[cCbc] = 0;
			}
		}
	}
}

void Calibration::setOffset( uint8_t pOffset, int  pTGroupId )
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

	if ( fTestGroupChannelMap.find( pTGroupId ) != fTestGroupChannelMap.end() )
	{
		OffsetWriter cWriter( fCbcInterface, pOffset, fTestGroupChannelMap[pTGroupId] );
		accept( cWriter );
	}
	else std::cout << "Error, could not find the channels for this test group: " << pTGroupId << std::endl;
}

void Calibration::toggleOffset( uint8_t pGroup, uint8_t pBit, bool pBegin )
{
	for ( auto cShelve : fShelveVector )
	{
		for ( auto cBoard : cShelve->fBoardVector )
		{
			for ( auto cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();

				for ( auto cCbc : cFe->fCbcVector )
				{
					uint32_t cCbcId = cCbc->getCbcId();

					// first, find the offset Histogram for this CBC
					TH1F* cOffsetHist = static_cast<TH1F*>( getHist( cCbc, "Offsets" ) );
					// find the TProfile for occupancy measurment of current channel
					TH1F* cOccHist = static_cast<TH1F*>( getHist( cCbc, "Occupancy" ) );
					cOccHist->Scale( 1 / double( fEventsPerPoint ) );
					RegisterVector cRegVec;   // vector of pairs for the write operation

					// loop the channels of the current group and toggle bit i in the global map
					for ( auto& cChannel : fTestGroupChannelMap[pGroup] )
					{
						TString cRegName = Form( "Channel%03d", cChannel + 1 );
						if ( pBegin )
						{
							// get the offset
							uint8_t cOffset = cOffsetHist->GetBinContent( cChannel );
							// toggle Bit i
							toggleRegBit( cOffset, pBit );
							// modify the histogram
							cOffsetHist->SetBinContent( cChannel, cOffset );

							// push in a vector for CBC write transaction

							cRegVec.push_back( {cRegName.Data(), cOffset} );

							// std::cout << "DEBUG Toggling offset bit " << +pBit << " for channel " << +cChannel << " of group " << +pGroup << " to " << +cOffset << " (=0b" << std::bitset<8>( cOffset ) << ")" << std::endl;
						}
						else  //here it is interesting since now I will check if the occupancy is smaller or larger 50% and decide wether to toggle or not to toggle
						{
							// if the occupancy is larger than 50%, flip the bit back, if it is smaller, don't do anything
							// get the offset
							uint8_t cOffset = cOffsetHist->GetBinContent( cChannel );
							// get the occupancy
							// for ( int iBin = 0; iBin < cOccHist->GetNbinsX(); iBin++ )
							// 	std::cout << "Bin " << iBin << " occupancy " << cOccHist->GetBinContent( iBin ) << std::endl;
							int iBin = cOccHist->GetXaxis()->FindBin( cChannel );
							float cOccupancy = cOccHist->GetBinContent( iBin );
							// std::cout << +cChannel << " Offset " << +cOffset << " occupancy " << cOccupancy << std::endl;
							// only if the occupancy is too high I need to flip the bit back and write, if not, I can leave it
							if ( cOccupancy > 0.5 )
							{
								toggleRegBit( cOffset, pBit ); // toggle the bit back that was previously flipped
								cOffsetHist->SetBinContent( cChannel, cOffset );
								cRegVec.push_back( {cRegName.Data(), cOffset} );
							}
							// std::cout << "DEBUG Toggling offset bit " << +pBit << " for channel " << +cChannel << " of group " << +pGroup << " to " << +cOffset << " (=0b" << std::bitset<8>( cOffset ) << ") ; Occupancy was " << cOccupancy << std::endl;

							// since I extracted the info from the occupancy profile for this bit (this iteration), i need to clear the corresponding bins
							cOccHist->SetBinContent( iBin, 0 );
						}
					}
					fCbcInterface->WriteCbcMultReg( cCbc, cRegVec );
				}
			}
		}
	}
}

void Calibration::updateHists( std::string pHistname )
{
	// loop the CBCs
	for ( const auto& cCbc : fCbcHistMap )
	{
		// loop the map of string vs TObject
		auto cHist = cCbc.second.find( pHistname );
		if ( cHist != std::end( cCbc.second ) )
		{
			// now cHist.second is the Histogram
			if ( pHistname == "Vplus" )
			{
				fVplusCanvas->cd( cCbc.first->getCbcId() + 1 );
				TProfile* cTmpProfile = static_cast<TProfile*>( cHist->second );
				cTmpProfile->DrawCopy();
				fVplusCanvas->Update();
			}
			if ( pHistname == "Offsets" )
			{
				fOffsetCanvas->cd( cCbc.first->getCbcId() + 1 );
				TH1F* cTmpHist = static_cast<TH1F*>( cHist->second );
				cTmpHist->DrawCopy();
				fOffsetCanvas->Update();
			}
			if ( pHistname == "Occupancy" )
			{
				fOccupancyCanvas->cd( cCbc.first->getCbcId() + 1 );
				TProfile* cTmpProfile = static_cast<TProfile*>( cHist->second );
				cTmpProfile->DrawCopy();
				fOccupancyCanvas->Update();
			}
		}
		else std::cout << "Error, could not find Histogram with name " << pHistname << std::endl;
	}
#ifdef __HTTP__
	fHttpServer->ProcessRequests();
#endif
}

void Calibration::setRegValues()
{
	for ( auto cShelve : fShelveVector )
	{
		for ( auto cBoard : cShelve->fBoardVector )
		{
			for ( auto cFe : cBoard->fModuleVector )
			{
				uint32_t cFeId = cFe->getFeId();

				for ( auto cCbc : cFe->fCbcVector )
				{
					uint32_t cCbcId = cCbc->getCbcId();

					// first, find the offset Histogram for this CBC
					TH1F* cOffsetHist = static_cast<TH1F*>( getHist( cCbc, "Offsets" ) );

					for ( int iChan = 0; iChan <= NCHANNELS; iChan++ )
					{
						uint8_t cOffset = cOffsetHist->GetBinContent( iChan );
						cCbc->setReg( Form( "Channel%03d", iChan + 1 ), cOffset );
						std::cout << GREEN << "Offset for CBC " << cCbcId << " Channel " << iChan << " : 0x" << std::hex << +cOffset << std::dec << RESET << std::endl;
					}

				}
			}
		}
	}
}

void Calibration::writeGraphs()
{
	fResultFile->cd();

	// Save hist maps for CBCs
	//
	Tool::SaveResults();

	// save canvases too
	fVplusCanvas->Write( fVplusCanvas->GetName(), TObject::kOverwrite );
	fOffsetCanvas->Write( fOffsetCanvas->GetName(), TObject::kOverwrite );
	fOccupancyCanvas->Write( fOccupancyCanvas->GetName(), TObject::kOverwrite );

}

void Calibration::dumpConfigFiles()
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
