/*!
 *
 * \file OldCalibration.cc
 * \brief OldCalibration class, calibration of the hardware
 * \author Georg Auzinger
 * \date 13/08/14
 *
 * Support : mail to : georg.auzinger@cern.ch
 *
 */

#include "OldCalibration.h"

OldCalibration::OldCalibration()
{
}

OldCalibration::~OldCalibration()
{

	fResultFile->Write();
	fResultFile->Close();
}

void OldCalibration::InitialiseTestGroup()
{
	// Iterating over the Shelves
	for ( Shelve* cShelve : fShelveVector )
	{
		for ( BeBoard* cBoard : cShelve->fBoardVector )
		{
			for ( Module* cFe : cBoard->fModuleVector )
			{
				for ( Cbc* cCbc : cFe->fCbcVector )
				{
					ConstructTestGroup( cShelve->getShelveId(), cBoard->getBeId(), cFe->getFeId(), cCbc->getCbcId() );

					TCanvas* cTmpCanvas = new TCanvas( Form( "BE%d_FE%d_CBC%d", cBoard->getBeId(), cFe->getFeId(), cCbc->getCbcId() ), Form( "BE%d_FE%d_CBC%d", cBoard->getBeId(), cFe->getFeId(), cCbc->getCbcId() ), 850, 850 );
					cTmpCanvas->Divide( 3, 3 );
					fCbcCanvasMap[cShelve->getBoard( cBoard->getBeId() )->getModule( cFe->getFeId() )->getCbc( cCbc->getCbcId() )] = cTmpCanvas;

				}
			}
		}
	}
}


void OldCalibration::ConstructTestGroup( uint8_t pShelveId, uint8_t pBeId, uint8_t pFeId, uint8_t pCbcId )
{
	for ( uint8_t lgroup = 0; lgroup < 8; lgroup++ )
	{
		TestGroup fTestgroup( pShelveId, pBeId, pFeId, pCbcId, lgroup );

		std::vector<Channel> cChannelVect;

		// In the CBC Registers, Channels start with 1
		for ( uint8_t lchannel = 0; lchannel < 16; lchannel++ )
		{

			if ( lchannel * 16 + lgroup * 2 < 254 )
			{
				Channel cChannel( pBeId, pFeId, pCbcId, lchannel * 16 + lgroup * 2 + 1 );
				// std::cout<<"     Channel: "<<int(lchannel*16+lgroup*2)<<"created!"<<std::endl;
				cChannelVect.push_back( cChannel );
			}

			if ( ( lchannel * 16 + lgroup * 2 ) + 1 < 254 )
			{
				Channel cChannel( pBeId, pFeId, pCbcId, ( lchannel * 16 + lgroup * 2 ) + 2 );
				// std::cout<<"     Channel: "<<int(lchannel*16+lgroup*2 +1)<<"created!"<<std::endl;
				cChannelVect.push_back( cChannel );
			}

		}

		TestGroupGraph cTestGroupGraph( pBeId, pFeId, pCbcId, lgroup );


		fTestGroupMap[fTestgroup] = cChannelVect;
		fTestGroupGraphMap[fTestgroup] = cTestGroupGraph;
		// std::cout<<" Group: "<<lgroup<<" created @ " << &fTestGroupMap[fTestgroup] <<std::endl;

	}
}


void OldCalibration::OffsetScan()
{

	// Read values from Settings
	uint32_t cEventsperVcth = fSettingsMap.find( "Nevents" )->second;
	bool cHoleMode = fSettingsMap.find( "HoleMode" )->second;
	uint8_t cTargetVcth = fSettingsMap.find( "TargetVcth" )->second;

	for ( Shelve* cShelve : fShelveVector )
	{
		// Iterating over the Boards
		for ( BeBoard* board : cShelve->fBoardVector )
		{
			// Now loop over test groups, enable them, loop over Vcth, read & analyze data, fill histograms & disable test groups again
			for ( uint8_t cGroupId = 0; cGroupId < 8; cGroupId++ )
			{
				// set Offsets to 0 initially
				ToggleTestGroup( board, cGroupId, cHoleMode, false );

				std::cout << BOLDYELLOW << "Looping over OffsetBits ... " << RESET << std::endl;

				// loop over offsets bitwise
				// for(int cTargetBit = 0x07; cTargetBit >= 0x00; cTargetBit-- ){
				uint8_t cTargetBit = 0x07;
				do
				{
					//Set Offset Target bit
					uint32_t cTotalChannels = SetOffsetTargetBitTestGroup( board, cGroupId, cHoleMode, cTargetBit, cTargetVcth );

					// Now set Vplus to the correct value for all Cbc's connected to the current board
					initializeSCurves( board, cGroupId, cTargetBit, "Offset_TargetBit" );

					measureSCurves( board, cGroupId, cEventsperVcth, cTotalChannels, cHoleMode );

					// the bool at the end toggles the printing of the SCurves
					processSCurvesOffset( board, cGroupId, cEventsperVcth, cTargetVcth, cTargetBit, "Offset_TargetBit", cHoleMode, true );

				}
				while ( cTargetBit-- );

				// Disbale the current TestGroup
				ToggleTestGroup( board, cGroupId, cHoleMode, false );

				// Here I am deliberately creating a discrepancy between memory object and HW in order to keep the final offset values
				UpdateCbcObject( board, cGroupId );

			}
		}
	}
}

uint32_t OldCalibration::SetOffsetTargetBitTestGroup( BeBoard* pBoard, uint8_t pGroupId, bool pHoleMode, uint8_t pTargetBit, uint8_t pTargetVcth )
{

	uint32_t cTotalNChannels = 0;
	for ( auto& cGroupIt : fTestGroupMap )
	{
		// check if the Group belongs to the right Shelve, BE and if it is the right group
		if ( cGroupIt.first.fShelveId == pBoard->getShelveId() && cGroupIt.first.fBeId == pBoard->getBeId() && cGroupIt.first.fGroupId == pGroupId )
		{
			// Iterate through the channels and set Offset to 0x50
			// Vector of pairs for the write MultiReg
			std::vector< std::pair< std::string, uint8_t > > cRegVec;

			for ( Channel& cChannel : cGroupIt.second )
			{

				TString cRegName = Form( "Channel%03d", cChannel.fChannelId );
				// uint8_t cOffset = cChannel.getOffset();
				uint8_t cOffset = pBoard->getModule( cGroupIt.first.fFeId )->getCbc( cGroupIt.first.fCbcId )->getReg( cRegName.Data() );

				if ( pHoleMode )
				{
					// cOffset = 0x00;
					// independently of the CBC logic, just toggle the bit
					cOffset |= ( 1 << pTargetBit );
				}
				else
				{
					//      cOffset = 0xFF;
					cOffset &= ~( 1 << pTargetBit );
				}

				std::pair<std::string, uint8_t> cRegPair = { cRegName.Data(), cOffset };
				cRegVec.push_back( cRegPair );

				// Update the Channel Object
				cChannel.setOffset( cOffset );

				cTotalNChannels++;
			}

			if ( !cRegVec.empty() ) fCbcInterface->WriteCbcMultReg( pBoard->getModule( cGroupIt.first.fFeId )->getCbc( cGroupIt.first.fCbcId ), cRegVec, READBACK );
		}
	}
	std::cout << GREEN << "Flipping Bit " << YELLOW << +pTargetBit << GREEN << " on Test Group " << YELLOW << +pGroupId << RESET << std::endl;
	return cTotalNChannels;
}



void OldCalibration::processSCurvesOffset( BeBoard* pBoard, uint8_t pGroupId, uint32_t pEventsperVcth, uint8_t pTargetVcth, uint8_t pTargetBit, TString pParameter, bool pHoleMode, bool pDoDraw )
{
	// Here Loop over all FE's Cbc's, Test Groups & Channels to fill Histograms
	for ( auto& cGroupIt : fTestGroupMap )
	{

		// check if the Group belongs to the right Shelve, BE and if it is the right group
		if ( cGroupIt.first.fShelveId == pBoard->getShelveId() && cGroupIt.first.fBeId == pBoard->getBeId() && cGroupIt.first.fGroupId == pGroupId )
		{

			TCanvas* currentCanvas;

			if ( pDoDraw )
			{

				std::map<Cbc*, TCanvas*>::iterator cCanvasMapIt = fCbcCanvasMap.find( pBoard->getModule( cGroupIt.first.fFeId )->getCbc( cGroupIt.first.fCbcId ) );

				if ( cCanvasMapIt != fCbcCanvasMap.end() )
				{
					currentCanvas = cCanvasMapIt->second;
					currentCanvas->cd( pGroupId + 1 );
				}
				else pDoDraw = false;
			}

			std::vector< std::pair< std::string, uint8_t > > cRegVec;

			bool cFirst = true;
			TString cOption;
			uint32_t cChannelCounter = 1;

			for ( Channel& cChannel : cGroupIt.second )
			{
				cChannel.fitHist( pEventsperVcth, pHoleMode, pTargetBit, pParameter, fResultFile );

				if ( pDoDraw )
				{
					gStyle->SetOptStat( 00000000000 );
					// Drawing SCurves of current test Group
					if ( cFirst )
					{
						cOption = "P0";
						cFirst = false;
					}
					else cOption = "P0 same";

					cChannel.fScurve->SetMarkerColor( cChannelCounter );
					cChannel.fScurve->Draw( cOption );
					cChannel.fFit->Draw( "same" );
					cChannelCounter++;
				}

				// here we need to check if the SCurve midpoint(cChannel.getPedestal()) with this offset target bit is > or < than pTargetVcth and then leave the bit up or flip it back down

				// std::cout << +pTargetVcth << " " << cChannel.getPedestal() << std::endl;

				if ( pHoleMode && int( cChannel.getPedestal() ) > pTargetVcth )
				{
					uint8_t cOffset = cChannel.getOffset();
					// uint8_t cOffset = pBoard->getModule(cGroupIt.first.fFeId)->getCbc(cGroupIt.first.fCbcId)->getReg(cRegName.Data());

					cOffset &= ~( 1 << pTargetBit );

					TString cRegName = Form( "Channel%03d", cChannel.fChannelId );
					std::pair<std::string, uint8_t> cRegPair = { cRegName.Data(), cOffset };
					cRegVec.push_back( cRegPair );
					cChannel.setOffset( cOffset );
				}
				else if ( !pHoleMode && int( cChannel.getPedestal() ) < pTargetVcth )
				{
					uint8_t cOffset = cChannel.getOffset();
					// uint8_t cOffset = pBoard->getModule(cGroupIt.first.fFeId)->getCbc(cGroupIt.first.fCbcId)->getReg(cRegName.Data());

					cOffset |= ( 1 << pTargetBit );
					// toggle bit
					// cOffset ^= (1<<pTargetBit);

					TString cRegName = Form( "Channel%03d", cChannel.fChannelId );
					std::pair<std::string, uint8_t> cRegPair = { cRegName.Data(), cOffset };
					cRegVec.push_back( cRegPair );
					cChannel.setOffset( cOffset );
				}
			}

			if ( !cRegVec.empty() ) fCbcInterface->WriteCbcMultReg( pBoard->getModule( cGroupIt.first.fFeId )->getCbc( cGroupIt.first.fCbcId ), cRegVec, READBACK );
			currentCanvas->Update();
#ifdef __HTTP__
			fHttpServer->ProcessRequests();
#endif
		}
	}
	// std::cout << "Processed SCurves for Target Bit " << GREEN <<  +pTargetBit << RESET << std::endl;
}

void OldCalibration::UpdateCbcObject( BeBoard* pBoard, uint8_t pGroupId )
{

	for ( auto& cGroupIt : fTestGroupMap )
	{
		// check if the Group belongs to the right Shelve, BE and if it is the right group
		if ( cGroupIt.first.fShelveId == pBoard->getShelveId() && cGroupIt.first.fBeId == pBoard->getBeId() && cGroupIt.first.fGroupId == pGroupId )
		{

			for ( Channel& cChannel : cGroupIt.second )
			{

				TString cRegName = Form( "Channel%03d", cChannel.fChannelId );
				uint8_t cOffset = cChannel.getOffset();

				pBoard->getModule( cGroupIt.first.fFeId )->getCbc( cGroupIt.first.fCbcId )->setReg( cRegName.Data(), cOffset );

			}
		}
	}
}



void OldCalibration::VplusScan()
{

	fVplusValues.push_back( 0x40 );
	fVplusValues.push_back( 0x80 );
	fVplusValues.push_back( 0xB0 );

	// Read values from Settings
	uint32_t cEventsperVcth = fSettingsMap.find( "Nevents" )->second;
	bool cHoleMode = fSettingsMap.find( "HoleMode" )->second;
	uint8_t cTargetVcth = fSettingsMap.find( "TargetVcth" )->second;

	for ( Shelve* cShelve : fShelveVector )
	{
		// Iterating over the Boards
		for ( BeBoard* board : cShelve->fBoardVector )
		{

			// Now loop over test groups, enable them, loop over Vcth, read & analyze data, fill histograms & disable test groups again
			for ( uint8_t cGroupId = 0; cGroupId < 8; cGroupId++ )
			{
				// Enable the current TestGroup
				uint32_t cTotalChannels = ToggleTestGroup( board, cGroupId, cHoleMode, true );

				std::cout << BOLDYELLOW << "Scanning Vplus ... " << RESET << std::endl;

				for ( uint8_t& cVplus : fVplusValues )
				{

					// Now set Vplus to the correct value for all Cbc's connected to the current board
					setGlobalReg( board, "Vplus", cVplus );

					initializeSCurves( board, cGroupId, cVplus, "Vplus" );

					measureSCurves( board, cGroupId, cEventsperVcth, cTotalChannels, cHoleMode );
					// Now Fit the hists & fill the VplusVcth Graph & set the TestGroup Offset back to 0x00 or 0xFF
					processSCurves( board, cGroupId, cEventsperVcth, cVplus, "Vplus", cHoleMode, true );

				}

				// Disbale the current TestGroup
				ToggleTestGroup( board, cGroupId, cHoleMode, false );

			}

			// Fit the Thing
			FitVplusVcth( board, cTargetVcth, true );
		}
	}
}



void OldCalibration::FitVplusVcth( BeBoard* pBoard, uint8_t pTargetVcth,  bool pDoDraw )
{

	for ( Module* cFe : pBoard->fModuleVector )
	{
		for ( Cbc* cCbc : cFe->fCbcVector )
		{
			TString multigraphname = Form( "VplusVcth_Be%d_Fe%d_Cbc%d", pBoard->getBeId(), cFe->getFeId(), cCbc->getCbcId() );
			TMultiGraph* cVplusVcthMultiGraph = new TMultiGraph( multigraphname, Form( "Vplus vs. Vcth; Vcth; Vplus" ) );

			TCanvas* currentCanvas;

			if ( pDoDraw )
			{
				TString canvasname = Form( "BE%d_FE%d_CBC%d", pBoard->getBeId(), cFe->getFeId(), cCbc->getCbcId() );
				currentCanvas = dynamic_cast<TCanvas*>( gROOT->FindObject( canvasname ) );
				if ( currentCanvas == nullptr )
				{
					pDoDraw = false;
					std::cout << "No Canvas Found to display Result!" << std::endl;
				}
				else currentCanvas->cd( 9 );
			}

			for ( uint8_t cGroupId = 0; cGroupId < 8; cGroupId++ )
			{

				// Create a test Group to find the corresponding in the map
				TestGroup cTempTestGroup( pBoard->getShelveId(), pBoard->getBeId(), cFe->getFeId(), cCbc->getCbcId(), cGroupId );

				// Find it!
				TestGroupGraphMap::iterator cGroup = fTestGroupGraphMap.find( cTempTestGroup );
				if ( cGroup != fTestGroupGraphMap.end() )
				{
					cGroup->second.fVplusVcthGraph->SetMarkerStyle( 8 );
					cGroup->second.fVplusVcthGraph->SetMarkerColor( cGroupId + 1 );
					cVplusVcthMultiGraph->Add( cGroup->second.fVplusVcthGraph );
				}
				else std::cout << BOLDRED << "This test group (BE" << int( pBoard->getBeId() ) << " FE" << int( cFe->getFeId() ) << " CBC" << int( cCbc->getCbcId() ) << " Group" << +cGroupId << ") was not found!" << RESET << std::endl;
			}

			// All Points in the Multigraph; Fit it per Cbc, draw it Per Cbc
			cVplusVcthMultiGraph->Fit( "pol1", "Q+" );
			TF1* cFit = cVplusVcthMultiGraph->GetFunction( "pol1" );

			if ( pDoDraw )
			{
				cVplusVcthMultiGraph->Draw( "AP" );
				cFit->Draw( "same" );
				gPad->Modified();
				cVplusVcthMultiGraph->GetXaxis()->SetLimits( 0, 255 );
				cVplusVcthMultiGraph->SetMinimum( 0 );
				cVplusVcthMultiGraph->SetMaximum( 255 );
				currentCanvas->Update();
#ifdef __HTTP__
				fHttpServer->ProcessRequests();
#endif
				cVplusVcthMultiGraph->Write( cVplusVcthMultiGraph->GetName(), TObject::kOverwrite );
				cFit->Write( Form( "Fit_Fe%d_Cbc%d", cFe->getFeId(), cCbc->getCbcId() ), TObject::kOverwrite );
				fResultFile->Flush();
			}
			// Get the right Vplus setting & write to the Cbc
			uint8_t cVplusResult = static_cast<uint8_t>( cFit->GetParameter( 1 ) * pTargetVcth + cFit->GetParameter( 0 ) );

			fCbcInterface->WriteCbcReg( cCbc, "Vplus", cVplusResult );
			// manually set the register on the CBC because some people complained that it would not work correctly
			cCbc->setReg( "Vplus", cVplusResult );

			std::cout << "Vplus Setting for Be " << int( pBoard->getBeId() ) << " Fe " << int( cFe->getFeId() ) << " Cbc " << int( cCbc->getCbcId() ) << " : " << +cVplusResult << std::endl;
			CbcRegReader myReader( fCbcInterface, "Vplus" );
			accept( myReader );
			cVplusVcthMultiGraph->Write( cVplusVcthMultiGraph->GetName(), TObject::kOverwrite );
			cFit->Write( cFit->GetName(), TObject::kOverwrite );

		} // End of Cbc Loop
	} // End of Fe Loop
}

void OldCalibration::setGlobalReg( BeBoard* pBoard, const std::string& pRegName, uint8_t pRegValue )
{
	for ( Module* cFe : pBoard->fModuleVector )
	{
		for ( Cbc* cCbc : cFe->fCbcVector )
			fCbcInterface->WriteCbcReg( cCbc, pRegName, pRegValue );
	}
	if ( pRegName != "VCth" ) std::cout << "Setting " << RED << pRegName << RESET << " to Value " << GREEN << +pRegValue << RESET << " on all CBCs connected to Be " << YELLOW << int( pBoard->getBeId() ) << RESET << std::endl;
}

void OldCalibration::initializeSCurves( BeBoard* pBoard, uint8_t pGroupId, uint8_t pValue, TString pParameter )
{

	for ( auto& cGroupIt : fTestGroupMap )
	{
		// check if the Group belongs to the right Shelve, BE and if it is the right group
		if ( cGroupIt.first.fShelveId == pBoard->getShelveId() && cGroupIt.first.fBeId == pBoard->getBeId() && cGroupIt.first.fGroupId == pGroupId )
		{
			for ( Channel& cChannel : cGroupIt.second ) cChannel.initializeHist( pValue, pParameter );
		}
	}
	// std::cout << "Initialized SCurves for " << pParameter << " = " << +pValue << " and Test group " << GREEN <<  +pGroupId << RESET << " on all Cbc's connected to Be " << RED <<  int(pBoard->getBeId()) << RESET << std::endl;
}

void OldCalibration::measureSCurves( BeBoard* pBoard, uint8_t pGroupId, uint32_t pEventsperVcth, uint32_t pTotalChannels, bool pHoleMode )
{

	// This is a smart Loop that scans the necessary Vcth range

	int cVcthMin( 0x00 ), cVcthMax( 0xFF );
	int cDoubleVcth = 0;
	int cVcth = pHoleMode ? cVcthMax : cVcthMin;
	int cStep = pHoleMode ? -10 : 10;
	bool cNonZero = false;
	uint32_t cAllOne = 0;

	// std::cout << BOLDYELLOW << "Scanning VCth ... " << RESET << std::endl;
	while ( 0x00 <= cVcth && cVcth <= 0xFF )
	{
		if ( cVcth == cDoubleVcth )
		{
			cVcth +=  cStep;
			continue;
		}

		// Set current Vcth value on all Cbc's of the current board
		setGlobalReg( pBoard, "VCth", cVcth );

		uint32_t cN = 1;
		uint32_t cNthAcq = 0;
		uint32_t cTotalHits = 0;

		fBeBoardInterface->Start( pBoard );

		while ( cN <= pEventsperVcth )
		{
			// Run( pBoard, cNthAcq );
			fBeBoardInterface->ReadData( pBoard, cNthAcq, false );
			const Event* cEvent = fBeBoardInterface->GetNextEvent( pBoard );
#if 0
			std::cout << ">>> pGroupId = " << +pGroupId << " cVcth = " << cVcth << std::endl;
			std::cout << ">>> Event #" << cN << " Limit = " << pEventsperVcth << std::endl;
			std::cout << *cEvent << std::endl;
#endif

			// Loop over Events from this Acquisition
			while ( cEvent )
			{

				if ( cN > pEventsperVcth )
					break;

				uint32_t cNHits = fillScurveHists( pBoard, pGroupId, cVcth, cEvent );
				cTotalHits += cNHits;
				cN++;

				if ( cN < pEventsperVcth )
				{
					cEvent = fBeBoardInterface->GetNextEvent( pBoard );
#if 0
					std::cout << ">>> pGroupId = " << +pGroupId << " cVcth = " << cVcth << std::endl;
					std::cout << ">>> Event #" << cN << std::endl;
					std::cout << *cEvent << std::endl;
#endif
				}
				else break;
			}
			cNthAcq++;
		} // End of Analyze Events of last Acquistion loop

		fBeBoardInterface->Stop( pBoard, cNthAcq );

		// This is the condition for some channels being different from 0
		if ( cNonZero == false && cTotalHits != 0 )
		{
			//if( cNonZero == false && cTotalHits > 0.3 * pEventsperVcth * pTotalChannels){
			cDoubleVcth = cVcth;
			cNonZero = true;
			cVcth -= 2 * cStep;
			cStep /= 10;
			continue;
		}
		// This is the condition for all of the S-curves having reached 1
		if ( cTotalHits == pEventsperVcth * pTotalChannels ) cAllOne++;
		//      if(cTotalHits >= 0.85 * pEventsperVcth * pTotalChannels) cAllOne++;
		if ( cAllOne == 8 ) break;
		cVcth += cStep;
	}  // done looping over Vcth, Scurves created
}




void OldCalibration::processSCurves( BeBoard* pBoard, uint8_t pGroupId, uint32_t pEventsperVcth, uint8_t pValue, TString pParameter, bool pHoleMode, bool pDoDraw )
{
	// Here Loop over all FE's Cbc's, Test Groups & Channels to fill Histograms
	for ( auto& cGroupIt : fTestGroupMap )
	{

		// check if the Group belongs to the right Shelve, BE and if it is the right group
		if ( cGroupIt.first.fShelveId == pBoard->getShelveId() && cGroupIt.first.fBeId == pBoard->getBeId() && cGroupIt.first.fGroupId == pGroupId )
		{

			TCanvas* currentCanvas;

			if ( pDoDraw )
			{

				std::map<Cbc*, TCanvas*>::iterator cCanvasMapIt = fCbcCanvasMap.find( pBoard->getModule( cGroupIt.first.fFeId )->getCbc( cGroupIt.first.fCbcId ) );

				if ( cCanvasMapIt != fCbcCanvasMap.end() )
				{
					currentCanvas = cCanvasMapIt->second;
					currentCanvas->cd( pGroupId + 1 );
				}
				else pDoDraw = false;
			}

			//              if(pDoDraw){
			currentCanvas = fCbcCanvasMap[pBoard->getModule( cGroupIt.first.fFeId )->getCbc( cGroupIt.first.fCbcId )];
			//                      currentCanvas->cd(pGroupId+1);
			//   }

			bool cFirst = true;
			TString cOption;
			uint32_t cChannelCounter = 1;

			for ( Channel& cChannel : cGroupIt.second )
			{
				cChannel.fitHist( pEventsperVcth, pHoleMode, pValue, pParameter, fResultFile );

				if ( pDoDraw )
				{
					gStyle->SetOptStat( 00000000000 );
					// Drawing SCurves of current test Group
					if ( cFirst )
					{
						cOption = "P0";
						cFirst = false;
					}
					else cOption = "P0 same";

					cChannel.fScurve->SetMarkerColor( cChannelCounter );
					cChannel.fScurve->Draw( cOption );
					cChannel.fFit->Draw( "same" );
					cChannelCounter++;
				}

				// fill test Group TGraphErrors here
				TestGroupGraphMap::iterator cGraphIt = fTestGroupGraphMap.find( cGroupIt.first );
				if ( cGraphIt != fTestGroupGraphMap.end() )
				{

					cGraphIt->second.FillVplusVcthGraph( pValue, cChannel.getPedestal(), cChannel.getNoise() );
					if ( cChannel.getPedestal() == 0 ) std::cout << RED << "ERROR " << RESET << "The fit for Channel " << int( cChannel.fChannelId ) << " CBC " << int( cChannel.fCbcId ) << " FE " << int( cChannel.fFeId ) << " did not work correctly!" << std::endl;
				}
				else std::cout << "The Graph for this test Group could not be found! There is a problem!" << std::endl;
			}

			fResultFile->Flush();
			currentCanvas->Update();
#ifdef __HTTP__
			fHttpServer->ProcessRequests();
#endif
		}
	}
	// std::cout << "Processed SCurves for Test group " << GREEN <<  +pGroupId << RESET << " on all Cbc's connected to Be " << RED << int(pBoard->getBeId()) << RESET << std::endl;
}

uint32_t OldCalibration::fillScurveHists( BeBoard* pBoard, uint8_t pGroupId, uint8_t pVcth, const Event* pEvent )
{
	// Here Loop over all FE's Cbc's, Test Groups & Channels to fill Histograms
	uint32_t cNHits = 0;

	for ( auto& cGroupIt : fTestGroupMap )
	{
		// check if the Group belongs to the right Shelve, BE and if it is the right group
		if ( cGroupIt.first.fShelveId == pBoard->getShelveId() && cGroupIt.first.fBeId == pBoard->getBeId() && cGroupIt.first.fGroupId == pGroupId )
		{
			std::vector<bool> cDataBitVector = pEvent->DataBitVector( cGroupIt.first.fFeId, cGroupIt.first.fCbcId );

			// Now loop over all channels in the TestGroup
			for ( Channel& cChannel : cGroupIt.second )
			{
				// In the CBC, Channels start with 1, so decrement by 1 to have the right element from the data bit vector
				if ( cDataBitVector[cChannel.fChannelId - 1] )
				{
					cChannel.fillHist( pVcth );
					cNHits++;
				}
			}
		}
	}

	return cNHits;
}

uint32_t OldCalibration::ToggleTestGroup( BeBoard* pBoard, uint8_t pGroupId, bool pHoleMode, bool pEnable )
{

	uint32_t cTotalNChannels = 0;
	for ( auto& cGroupIt : fTestGroupMap )
	{
		// check if the Group belongs to the right Shelve, BE and if it is the right group
		if ( cGroupIt.first.fShelveId == pBoard->getShelveId() && cGroupIt.first.fBeId == pBoard->getBeId() && cGroupIt.first.fGroupId == pGroupId )
		{
			// Iterate through the channels and set Offset to 0x50
			// Vector of pairs for the write MultiReg
			std::vector< std::pair< std::string, uint8_t > > cRegVec;

			for ( Channel& cChannel : cGroupIt.second )
			{
				// Write the offset to the Cbcs
				TString cRegName = Form( "Channel%03d", cChannel.fChannelId );
				uint8_t cRegValue;
				if ( pEnable ) cRegValue = 0x50;
				else cRegValue = ( pHoleMode ) ?  0x00 : 0xFF;
				std::pair<std::string, uint8_t> cRegPair = { cRegName.Data(), cRegValue };
				cRegVec.push_back( cRegPair );

				// Update the Channel Object
				// if(pEnable) cChannel.setOffset(0x50);
				// else if (!pEnable && pHoleMode) cChannel.setOffset(0x00);
				// else cChannel.setOffset(0xFF);
				cTotalNChannels++;
			}

			fCbcInterface->WriteCbcMultReg( pBoard->getModule( cGroupIt.first.fFeId )->getCbc( cGroupIt.first.fCbcId ), cRegVec, READBACK );
		}
	}
	if ( pEnable ) std::cout << GREEN << "Enabled Test group " << YELLOW << +pGroupId << GREEN << " on all Cbc's connected to Be " << YELLOW <<  int( pBoard->getBeId() ) << RESET << std::endl;
	else std::cout << RED << "Disabled Test group " << YELLOW << +pGroupId << RED << " on all Cbc's connected to Be " << YELLOW << int( pBoard->getBeId() ) << RESET << std::endl;
	return cTotalNChannels;
}


void OldCalibration::SaveResults()
{
	uint8_t cTargetVCth = fSettingsMap.find( "TargetVcth" )->second;

	if ( !fDirectoryName.empty() )
	{
		for ( Shelve* cShelve : fShelveVector )
		{
			for ( BeBoard* cBoard : cShelve->fBoardVector )
			{
				for ( Module* cFe : cBoard->fModuleVector )
				{
					for ( Cbc* cCbc : cFe->fCbcVector )
					{
						// set Target VCth, just to be sure
						fCbcInterface->WriteCbcReg( cCbc, "VCth", cTargetVCth );

						TString cFilename = Form( "/FE%dCBC%d.txt", cFe->getFeId(), cCbc->getCbcId() );

						std::string cPathname = fDirectoryName + cFilename.Data();

						std::cout << "Dumping OldCalibration Results to file: " << cPathname << std::endl;

						cCbc->saveRegMap( cPathname );
					}
				}
			}
		}
		fResultFile->Write();
		fResultFile->Close();
	}
	else std::cout << RED << "ERROR: " << RESET << "No Result Directory initialized - not saving results!" << std::endl;
}
