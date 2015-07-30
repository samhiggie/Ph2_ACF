#include <cstdlib>
#include "TH1D.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TKey.h"
#include "TClass.h"
#include "TDirectory.h"
#include "publisher.h"

namespace RootWeb
{
	void makePageOne( RootWSite& site )
	{
		// Adding one page
		auto& home = site.addPage( "Home" );
		home.setAddress( "index.html" );
#if 0
		// With one collapsing content (default is open)
		auto& content1 = home.addContent( "Section 1" );
		// With a table of strings
		auto& aTableS = content1.addTable();
		for ( int i = 1; i <= 10; ++i )
		{
			for ( int j = 1; j <= 10; ++j )
				aTableS.setContent( i, j, Form( "G%d", i * j ) );
		}
		// With a table of doubles (3 decimal places)
		auto& aTableD = content1.addTable();
		for ( int i = 1; i <= 10; ++i )
		{
			for ( int j = 1; j <= 10; ++j )
				aTableD.setContent( i, j, sqrt( i * j ), 3 );
		}
		// The second content is collapsed by defauls
		auto& content2 = home.addContent( "Section 2", false );
		content2.addText( "Hello, there, I was hiding here" );
#endif
	}

	TH1D* createPlot()
	{
		TH1D* myHisto = new TH1D( "test", "Here I am", 10, 0, 10 );
		myHisto->Fill( 3 );
		myHisto->Fill( 3 );
		myHisto->Fill( 4 );
		myHisto->Fill( 5 );
		return myHisto;
	}

	void makePageTwo( RootWSite& site, const std::string& inFilename )
	{
		auto& myPage = site.addPage( "DQM Plots" );

		// With a content containing some plots
		auto& content1 = myPage.addContent( "Common Plots" );

		// open DQM file
		TFile* fin = TFile::Open( TString( inFilename ) );
		std::vector<TKey*> directoryKey;
		TIter next( fin->GetListOfKeys() );
		TObject* obj;
		// Header etc.
		while ( ( obj = next() ) )
		{
			TKey* key = dynamic_cast<TKey*>( obj );
			TClass* cl = gROOT->GetClass( key->GetClassName() );
			if ( cl->InheritsFrom( "TDirectory" ) )
				directoryKey.push_back( key );
			else if ( cl->InheritsFrom( "TH1" ) )
			{
				TH1* h = dynamic_cast<TH1*>( key->ReadObj() );
				h->SetDirectory( 0 );
				TString canvasName( "c" + std::string( key->GetName() ) );
				TCanvas* myCanvas = new TCanvas( canvasName, "c1", 500, 500 );
				myCanvas->cd();
				h->Draw();
				auto& myImage = content1.addImage( myCanvas, 600, 400 );
				myImage.setComment( "A little explanation here always helps" );
				myImage.setName( h->GetTitle() );
			}
		}
		std::cout << "DirKeyVec size=" << directoryKey.size() << std::endl;

		// CBC Plots
		for ( auto& key : directoryKey )
		{
			auto& content2 = myPage.addContent( "Plots for " + std::string( key->GetName() ) );
			TDirectory* dir = dynamic_cast<TDirectory*>( key->ReadObj() );
			TIter next( dir->GetListOfKeys() );
			TKey* hkey;
			while ( ( hkey = ( TKey* )next() ) )
			{
				TClass* cl = gROOT->GetClass( hkey->GetClassName() );
				if ( !cl->InheritsFrom( "TH1" ) ) continue;
				TH1* h = dynamic_cast<TH1*>( hkey->ReadObj() );
				h->SetDirectory( 0 );
				TString canvasName( "c" + std::string( hkey->GetName() ) );
				TCanvas* myCanvas = new TCanvas( canvasName, "c1", 500, 500 );
				myCanvas->cd();
				h->Draw();
				auto& myImage = content2.addImage( myCanvas, 600, 400 );
				myImage.setComment( "A little explanation here always helps" );
				myImage.setName( h->GetTitle() );
			}
		}
		fin->Close();

		auto& content3 = myPage.addContent( "All plots here" );
		content3.addBinaryFile( inFilename, "content of all your plots is here", inFilename );
	}

	void prepareSiteStuff( RootWSite& site, const std::string& run )
	{
		std::string myDirectory = run;
		site.setTargetDirectory( myDirectory );
		site.setTitle( run );
		site.setComment( "Complete run list" );
		site.setCommentLink( "../" );
		site.addAuthor( "Mickey Mouse" );
		site.addAuthor( "Dylan Dog" );
		site.addAuthor( "Nathan Never" );
		site.setRevision( "0.1" );
		site.setProgram( "Ph2_DAQ", "https://github.com/gauzinge/Ph2_ACF" );
	}

	void makeDQMmonitor( const std::string& inFilename, const std::string& run )
	{
		RootWSite site;
		prepareSiteStuff( site, run );
		makePageOne( site );
		makePageTwo( site, inFilename );
		site.makeSite( false );
	}
}
