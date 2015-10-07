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

void bookHistogram( Cbc* pCbc, std::string pName, TObject* pObject );

	void bookHistogram( Module* pModule, std::string pName, TObject* pObject );

	TObject* getHist( Cbc* pCbc, std::string pName );

	TObject* getHist( Module* pModule, std::string pName );

	void SaveResults();

	void CreateResultDirectory( const std::string& pDirname, bool pDate = true );

	/*!
	 * \brief Initialize the result Root file
	 * \param pFilename : Root filename
	 */
	void InitResultFile( const std::string& pFilename );

	void StartHttpServer( const int pPort = 8082, const int pRefreshTime = 100, bool pReadonly = true );
};
#endif

