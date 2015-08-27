/*!

        \file                   Tool.h
        \brief                                   Controller of the System, overall wrapper of the framework
        \author                                  Georg AUZINGER
        \version                 1.0
        \date                                    06/02/15
        Support :                                mail to : georg.auzinger@cern.ch

 */


#ifndef __TOOL_H__
#define __TOOL_H__

#include "../System/SystemController.h"
#include "TROOT.h"
#include "TFile.h"
#include "TObject.h"
#include "TCanvas.h"

#ifdef __HTTP__
#include "THttpServer.h"
#endif

using namespace Ph2_System;

typedef std::map<Cbc*, std::map<std::string, TObject*> >  CbcHistogramMap;
typedef std::map<Module*, std::map<std::string, TObject*> > ModuleHistogramMap;
typedef std::map<Ph2_HwDescription::FrontEndDescription*, TCanvas*> CanvasMap;


/*!
 * \class Tool
 * \brief A base class for all kinds of applications that have to use ROOT that inherits from SystemController which does not have any dependence on ROOT
 */
class Tool : public SystemController
{
  public:
	std::string fDirectoryName;             /*< the Directoryname for the Root file with results */
	TFile* fResultFile;                /*< the Name for the Root file with results */
#ifdef __HTTP__
	THttpServer* fHttpServer;
#endif
	Tool() {}

	~Tool() {}



  protected:
	CanvasMap fCanvasMap;
	CbcHistogramMap fCbcHistMap;
	ModuleHistogramMap fModuleHistMap;


	/*!
	 * \brief Create a result directory at the specified path + ChargeMode + Timestamp
	 * \param pDirectoryname : the name of the directory to create
	 * \param pDate : apend the current date and time to the directoryname
	 */
  public:

	void bookHistogram( Cbc* pCbc, std::string pName, TObject* pObject ) {
		// find or create map<string,TOBject> for specific CBC
		auto cCbcHistMap = fCbcHistMap.find( pCbc );
		if ( cCbcHistMap == std::end( fCbcHistMap ) ) {
			std::cerr << "Histo Map for CBC " << int( pCbc->getCbcId() ) <<  " (FE " << int( pCbc->getFeId() ) << ") does not exist - creating " << std::endl;
			std::map<std::string, TObject*> cTempCbcMap;

			fCbcHistMap[pCbc] = cTempCbcMap;
			cCbcHistMap = fCbcHistMap.find( pCbc );
		}
		// find histogram with given name: if it exists, delete the object, if not create
		auto cHisto = cCbcHistMap->second.find( pName );
		if ( cHisto != std::end( cCbcHistMap->second ) ) cCbcHistMap->second.erase( cHisto );
		cCbcHistMap->second[pName] = pObject;
	}

	void bookHistogram( Module* pModule, std::string pName, TObject* pObject ) {
		// find or create map<string,TOBject> for specific CBC
		auto cModuleHistMap = fModuleHistMap.find( pModule );
		if ( cModuleHistMap == std::end( fModuleHistMap ) ) {
			std::cerr << "Histo Map for Module " << int( pModule->getFeId() ) << " does not exist - creating " << std::endl;
			std::map<std::string, TObject*> cTempModuleMap;

			fModuleHistMap[pModule] = cTempModuleMap;
			cModuleHistMap = fModuleHistMap.find( pModule );
		}
		// find histogram with given name: if it exists, delete the object, if not create
		auto cHisto = cModuleHistMap->second.find( pName );
		if ( cHisto != std::end( cModuleHistMap->second ) ) cModuleHistMap->second.erase( cHisto );
		cModuleHistMap->second[pName] = pObject;
	}

	TObject* getHist( Cbc* pCbc, std::string pName ) {
		auto cCbcHistMap = fCbcHistMap.find( pCbc );
		if ( cCbcHistMap == std::end( fCbcHistMap ) ) std::cerr << RED << "Error: could not find the Histograms for CBC " << int( pCbc->getCbcId() ) <<  " (FE " << int( pCbc->getFeId() ) << ")" << RESET << std::endl;
		else {
			auto cHisto = cCbcHistMap->second.find( pName );
			if ( cHisto == std::end( cCbcHistMap->second ) ) std::cerr << RED << "Error: could not find the Histogram with the name " << pName << RESET << std::endl;
			else
				return cHisto->second;
		}
	}

	TObject* getHist( Module* pModule, std::string pName ) {
		auto cModuleHistMap = fModuleHistMap.find( pModule );
		if ( cModuleHistMap == std::end( fModuleHistMap ) ) std::cerr << RED << "Error: could not find the Histograms for Module " << int( pModule->getFeId() ) << RESET << std::endl;
		else {
			auto cHisto = cModuleHistMap->second.find( pName );
			if ( cHisto == std::end( cModuleHistMap->second ) ) std::cerr << RED << "Error: could not find the Histogram with the name " << pName << RESET << std::endl;
			else return cHisto->second;
		}
	}

	void SaveResults() {
		// Now per FE
		for ( const auto& cFe : fModuleHistMap ) {
			TString cDirName = Form( "FE%d", cFe.first->getFeId() );
			TObject* cObj = gROOT->FindObject( cDirName );
			if ( cObj ) delete cObj;
			fResultFile->mkdir( cDirName );
			fResultFile->cd( cDirName );

			for ( const auto& cHist : cFe.second )
				cHist.second->Write( cHist.second->GetName(), TObject::kOverwrite );
			fResultFile->cd();
		}
		for ( const auto& cCbc : fCbcHistMap ) {
			TString cDirName = Form( "FE%dCBC%d", cCbc.first->getFeId(), cCbc.first->getCbcId() );
			TObject* cObj = gROOT->FindObject( cDirName );
			if ( cObj ) delete cObj;
			fResultFile->mkdir( cDirName );
			fResultFile->cd( cDirName );

			for ( const auto& cHist : cCbc.second )
				cHist.second->Write( cHist.second->GetName(), TObject::kOverwrite );
			fResultFile->cd();
		}

		// Save Canvasses too
		for ( const auto& cCanvas : fCanvasMap ) {
			cCanvas.second->Write( cCanvas.second->GetName(), TObject::kOverwrite );
			std::string cPdfName = fDirectoryName + "/" + cCanvas.second->GetName() + ".pdf";
			cCanvas.second->SaveAs( cPdfName.c_str() );
		}

		fResultFile->Write();
		fResultFile->Close();

		// dumpConfigFiles();

		std::cout << "Results saved!" << std::endl;
	}

	void CreateResultDirectory( const std::string& pDirname, bool pDate = true ) {
		bool cCheck;
		bool cHoleMode;
		auto cSetting = fSettingsMap.find( "HoleMode" );
		if ( cSetting != std::end( fSettingsMap ) ) {
			cCheck = true;
			cHoleMode = ( cSetting->second == 1 ) ? true : false;
		}
		std::string cMode;
		if ( cCheck ) {
			if ( cHoleMode ) cMode = "_Hole";
			else cMode = "_Electron";
		}

		std::string nDirname = pDirname;
		if ( cCheck ) nDirname +=  cMode;
		if ( pDate ) nDirname +=  currentDateTime();
		std::cout << std::endl << "Creating directory: " << nDirname << std::endl << std::endl;
		std::string cCommand = "mkdir -p " + nDirname;

		system( cCommand.c_str() );

		fDirectoryName = nDirname;
	}
	/*!
	 * \brief Initialize the result Root file
	 * \param pFilename : Root filename
	 */
	void InitResultFile( const std::string& pFilename ) {

		if ( !fDirectoryName.empty() ) {
			std::string cFilename = fDirectoryName + "/" + pFilename + ".root";
			fResultFile = TFile::Open( cFilename.c_str(), "RECREATE" );
		}
		else std::cout << RED << "ERROR: " << RESET << "No Result Directory initialized - not saving results!" << std::endl;
	}

	void StartHttpServer( int pPort = 8082, bool pReadonly = true ) {
#ifdef __HTTP__
		fHttpServer = new THttpServer( Form( "http:%d", pPort ) );
		fHttpServer->SetReadOnly( pReadonly );
		fHttpServer->SetJSROOT( "https://root.cern.ch/js/3.3" );
#else
		std::cout << "Error, ROOT version < 5.34 detected or not compiled with Http Server support!" << std::endl << " No THttpServer available! - The webgui will fail to show plots!" << std::endl;
		std::cout << "ROOT must be built with '--enable-http' flag to use this feature." << std::endl;
#endif
	}
};
#endif

