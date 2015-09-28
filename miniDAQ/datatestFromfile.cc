#include <cstring>
#include <stdint.h>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <inttypes.h>

#include "../Utils/Utilities.h"
#include "../Utils/Data.h"
#include "../Utils/Event.h"
#include "../Utils/Timer.h"
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"

#include "publisher.h"

#include "TROOT.h"
#include "TH1.h"
#include "TFile.h"
#include "TString.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;

using namespace CommandLineProcessing;

void tokenize( const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters )
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of( delimiters, 0 );

	// Find first "non-delimiter".
	std::string::size_type pos = str.find_first_of( delimiters, lastPos );

	while ( std::string::npos != pos || std::string::npos != lastPos )
	{
		// Found a token, add it to the vector.
		tokens.push_back( str.substr( lastPos, pos - lastPos ) );

		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of( delimiters, pos );

		// Find next "non-delimiter"
		pos = str.find_first_of( delimiters, lastPos );
	}
}


void fillDQMhisto( const std::vector<Event*>& elist, const std::string& outFileName )
{
	std::map<std::string, TH1I*> herrbitmap;
	std::map<std::string, TH1I*> hnstubsmap;
	std::map<std::string, std::vector<TH1I*>> hchdatamap;
	std::map<std::string, std::vector<int>> sensorNhits ;
        sensorNhits["even"] = std::vector<int>();
        sensorNhits["odd"] = std::vector<int>();

	TH1I* hdut0 = new TH1I( "evenSensor_hitprofile", "Even Sensor Hitmap", 1016, 0.5, 1016.5 );
	TH1I* hdut1 = new TH1I( "oddSensor_hitprofile", "Odd Sensor Hitmap", 1016, 0.5, 1016.5 );
	TH1I* hsensCorr = new TH1I( "sensorHitcorr", "Sensor Hit Correlation", 4, 0.5, 4.5 );

	TH1I* hl1A = new TH1I( "l1A", "L1A Counter", 10000, 0, 10000 );
	TH1I* htdc = new TH1I( "tdc", "TDC counter", 256, -0.5, 255.5 );
	for ( const auto& ev : elist )
	{
		htdc->Fill( ev->GetTDC() );
		hl1A->Fill( ev->GetEventCount() );
		const EventMap& evmap = ev->GetEventMap();
		sensorNhits["even"].clear();
		sensorNhits["odd"].clear();
		for ( auto const& it : evmap )
		{
			uint32_t feId = it.first;
			for ( auto const& jt : it.second )
			{
				uint32_t cbcId = jt.first;

				// create histogram name tags
				std::stringstream ss;
				ss << "fed" << feId << "cbc" << cbcId;
				std::string key = ss.str();

				// Error bit histogram
				if ( herrbitmap.find( key ) == herrbitmap.end() )
					herrbitmap[key] = new TH1I( TString( "errbit" + key ), "Error bit", 3, -0.5, 2.5 );
				herrbitmap[key]->Fill( static_cast<int>( ev->Error( feId, cbcId ) ) );

				// #Stubs info
				int nstubs = std::stoi( ev->StubBitString( feId, cbcId ), nullptr, 10 );
				if ( hnstubsmap.find( key ) == hnstubsmap.end() )
					hnstubsmap[key] = new TH1I( TString( "nstubs" + key ), "Number of stubs", 21, -0.5, 20.5 );
				hnstubsmap[key]->Fill( nstubs );

				// channel data
				const std::vector<bool>& dataVec = ev->DataBitVector( feId, cbcId );
				if ( hchdatamap.find( key ) == hchdatamap.end() )
				{
					hchdatamap[key].push_back( new TH1I( TString( "chOccupancy_even" + key ), "Even channel occupancy", 127, 0.5, 127.5 ) );
					hchdatamap[key].push_back( new TH1I( TString( "chOccupancy_odd" + key ), "Odd channel occupancy", 127, 0.5, 127.5 ) );
				}
				for ( unsigned int ch = 0; ch < dataVec.size(); ch++ )
				{
					if ( dataVec[ch] )
					{
						if ( ch % 2 == 0 ) {
							hchdatamap[key][0]->Fill( ch / 2 + 1 );
                                                        hdut0->Fill( 127*cbcId + ch / 2 + 1);
                                                        sensorNhits["even"].push_back(1);
                                                }
						else {
							hchdatamap[key][1]->Fill( ch / 2 + 1 );
                                                        hdut1->Fill( 127*cbcId + ch / 2 + 1);
                                                        sensorNhits["odd"].push_back(1);
                                                }
					}
				}
			}
		}
                if(sensorNhits["even"].empty() && sensorNhits["odd"].empty()) hsensCorr->Fill(1);
                else if(!sensorNhits["even"].empty() && sensorNhits["odd"].empty()) hsensCorr->Fill(2);
                else if(sensorNhits["even"].empty() && !sensorNhits["odd"].empty()) hsensCorr->Fill(3);
                else if(!sensorNhits["even"].empty() && !sensorNhits["odd"].empty()) hsensCorr->Fill(4);
	}
	TFile* fout = TFile::Open( outFileName.c_str(), "RECREATE" );
	for ( auto& he : herrbitmap )
	{
		fout->mkdir( he.first.c_str() );
		fout->cd( he.first.c_str() );
		he.second->Write();
	}
	for ( auto& he : hnstubsmap )
	{
		fout->cd( he.first.c_str() );
		he.second->Write();
	}


	for ( auto& hVec : hchdatamap )
	{
		fout->cd( hVec.first.c_str() );
		hVec.second.at( 0 )->Write();
		hVec.second.at( 1 )->Write();
	}

        hsensCorr->GetXaxis()->SetBinLabel(1,"No hits");
        hsensCorr->GetXaxis()->SetBinLabel(2,"Even & !Odd");
        hsensCorr->GetXaxis()->SetBinLabel(3,"Odd & !Even");
        hsensCorr->GetXaxis()->SetBinLabel(4,"Even & Odd");
	fout->cd();
	hl1A->Write();
	htdc->Write();
	fout->mkdir("Sensor");
        fout->cd("Sensor");
        hdut0->Write();
        hdut1->Write();
        hsensCorr->Write();
	fout->Close();
}

void dumpEvents( const std::vector<Event*>& elist )
{
	for ( int i = 0; i < elist.size(); i++ )
	{
		std::cout << "Event index: " << i + 1 << std::endl;
		std::cout << *elist[i] << std::endl;
	}
}

int main( int argc, char* argv[] )
{
	ArgvParser cmd;

	// init
	cmd.setIntroductoryDescription( "CMS Ph2_ACF  miniDQM application" );
	// error codes
	cmd.addErrorCode( 0, "Success" );
	cmd.addErrorCode( 1, "Error" );
	// options
	cmd.setHelpOption( "h", "help", "Print this help page" );

	cmd.defineOption( "file", "Binary Data File", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "output", "Output Directory for DQM plots & page. Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "output", "o" );

	cmd.defineOption( "dqm", "Build DQM webpage. Default = false", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "dqm", "d" );

	cmd.defineOption( "swap", "Swap endianness in Data::set. Default = true (Ph2_ACF); should be false for GlibStreamer Data", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "swap", "s" );

	cmd.defineOption( "8cbc", "Use 8CBC system. Default = false", ArgvParser::NoOptionAttribute /*| ArgvParser::OptionRequired*/ );

	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	// now query the parsing results
	std::string rawFilename = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "";
	if ( rawFilename.empty() )
	{
		std::cerr << "Error, no binary file provided. Quitting" << std::endl;
		exit( 1 );
	}

	bool cSwap = ( cmd.foundOption( "swap" ) ) ? false : true;

	bool cDQMPage = ( cmd.foundOption( "dqm" ) ) ? true : false;

	bool c8CBC = ( cmd.foundOption( "8cbc" ) ) ? true : false;

	std::string cDirBasePath;

	if ( cDQMPage )
	{
		if ( cmd.foundOption( "output" ) )
		{
			cDirBasePath = cmd.optionValue( "output" );
			cDirBasePath += "/";

		}
		else cDirBasePath = "Results/";
	}
	else std::cout << "Not creating DQM files!" << std::endl;

	std::vector<uint32_t> dataVec;
	SystemController cSystemController;

	cSystemController.addFileHandler( rawFilename, 'r' ); //add it for read test
	//	dataVec = cSystemController.fFileHandler->readFile( );  //add it for read test
	cSystemController.readFile(dataVec);  //add it for read test

	std::string cHWFile = getenv( "BASE_DIR" );
        cHWFile += "/";
	cHWFile += (c8CBC) ? XML_DESCRIPTION_FILE_8CBC : XML_DESCRIPTION_FILE_2CBC;
	cSystemController.parseHWxml( cHWFile );
	BeBoard* pBoard = cSystemController.fShelveVector.at( 0 )->fBoardVector.at( 0 );

	Data d;
        int eventSize =  EVENT_HEADER_TDC_SIZE_32 + CBC_EVENT_SIZE_32*(c8CBC ? 8 : 4);
	int nEvents = dataVec.size() / eventSize;
	d.Set( pBoard, dataVec, nEvents, cSwap );
	const std::vector<Event*>& elist = d.GetEvents( pBoard );

	// Create the DQM plots and generate the root file
	// first of all strip the folder name
	std::vector<std::string> tokens;
	tokenize( rawFilename, tokens, "/" );
	std::string fname = tokens.back();

	// now form the output Root filename
	tokens.clear();
	tokenize( fname, tokens, "." );
	std::string runLabel = tokens[0];
	std::string dqmFilename =  runLabel + "_dqm.root";

	if ( cDQMPage )
	{
		fillDQMhisto( elist, dqmFilename );
		// cDirBasePath += runLabel;
		RootWeb::makeDQMmonitor( dqmFilename, cDirBasePath, runLabel );
		std::cout << "Saving root file to " << dqmFilename << " and webpage to " << cDirBasePath << std::endl;
	}
	else dumpEvents( elist );
	return 0;
}

