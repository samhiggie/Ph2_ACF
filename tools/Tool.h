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

using CbcHistogramMap = std::map<Cbc*, std::map<std::string, TObject*> >;
using ModuleHistogramMap = std::map<Module*, std::map<std::string, TObject*> >;
using CanvasMap = std::map<Ph2_HwDescription::FrontEndDescription*, TCanvas*>;


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

    Tool()
    {
        fResultFile = nullptr;
#ifdef __HTTP__
        fHttpServer = nullptr;
#endif
    }

    ~Tool()
    {
    }

    void Destroy()
    {
        LOG (INFO) << BOLDRED << "Destroying memory objects" << RESET;
        SystemController::Destroy();
#ifdef __HTTP__

        if (fHttpServer) delete fHttpServer;

#endif

        if (fResultFile != nullptr)
        {
            if (fResultFile->IsOpen() ) fResultFile->Close();

            if (fResultFile) delete fResultFile;
        }
    }


  public:
    CanvasMap fCanvasMap;
    CbcHistogramMap fCbcHistMap;
    ModuleHistogramMap fModuleHistMap;
    ChipType fType;


    /*!
     * \brief Create a result directory at the specified path + ChargeMode + Timestamp
     * \param pDirectoryname : the name of the directory to create
     * \param pDate : apend the current date and time to the directoryname
     */
  public:
    Tool (const Tool& pTool);

    void Inherit (Tool* pTool)
    {
        fBeBoardInterface = pTool->fBeBoardInterface;
        fCbcInterface = pTool->fCbcInterface;
        fBoardVector = pTool->fBoardVector;
        fBeBoardFWMap = pTool->fBeBoardFWMap;
        fSettingsMap = pTool->fSettingsMap;
        fFileHandler = pTool->fFileHandler;
        fDirectoryName = pTool->fDirectoryName;
        fResultFile = pTool->fResultFile;
        fType = pTool->fType;
#ifdef __HTTP__
        fHttpServer = pTool->fHttpServer;
#endif
        fCanvasMap = pTool->fCanvasMap;
        fCbcHistMap = pTool->fCbcHistMap;
        fModuleHistMap = pTool->fModuleHistMap;
    }

    void CreateReport()
    {
        std::ofstream report;
        report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
        report.close();
    };
    void AmmendReport (std::string pString )
    {
        std::ofstream report;
        report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
        report << pString << std::endl;
        report.close();
    };

    void bookHistogram ( Cbc* pCbc, std::string pName, TObject* pObject );

    void bookHistogram ( Module* pModule, std::string pName, TObject* pObject );

    TObject* getHist ( Cbc* pCbc, std::string pName );

    TObject* getHist ( Module* pModule, std::string pName );

    void SaveResults();

    void CreateResultDirectory ( const std::string& pDirname, bool pMode = true, bool pDate = true );

    /*!
     * \brief Initialize the result Root file
     * \param pFilename : Root filename
     */
    void InitResultFile ( const std::string& pFilename );
    void CloseResultFile()
    {
        LOG (INFO) << BOLDRED << "closing result file!" << RESET;

        if (fResultFile)
            fResultFile->Close();
    }

    void StartHttpServer ( const int pPort = 8080, bool pReadonly = true );
    void ProcessRequests()
    {
#ifdef __HTTP__

        if (fHttpServer) fHttpServer->ProcessRequests();

#endif
    }
    void dumpConfigFiles();
    // general stuff that can be useful
    void setSystemTestPulse ( uint8_t pTPAmplitude, uint8_t pTestGroup, bool pTPState = false, bool pHoleMode = false );
    //enable commissioning loops and Test Pulse
    void setFWTestPulse();

    uint8_t reverse ( uint8_t n )
    {
        // Reverse the top and bottom nibble then swap them.
        return ( fLookup[n & 0b1111] << 4 ) | fLookup[n >> 4];
    }

    // helper methods
    void setRegBit ( uint16_t& pRegValue, uint8_t pPos, bool pValue )
    {
        pRegValue ^= ( -pValue ^ pRegValue ) & ( 1 << pPos );
    }

    void toggleRegBit ( uint16_t& pRegValue, uint8_t pPos )
    {
        pRegValue ^= 1 << pPos;
    }

    bool getBit ( uint16_t& pRegValue, uint8_t pPos )
    {
        return ( pRegValue >> pPos ) & 1;
    }

    /*!
    * \brief reverse the endianess before writing in to the register
    * \param pDelay: the actual delay
    * \param pGroup: the actual group number
    * \return the reversed endianness
    */
    uint8_t to_reg ( uint8_t pDelay, uint8_t pGroup )
    {

        uint8_t cValue = ( ( reverse ( pDelay ) ) & 0xF8 ) |
                         ( ( reverse ( pGroup ) ) >> 5 );

        //LOG(DBUG) << std::bitset<8>( cValue ) << " cGroup " << +pGroup << " " << std::bitset<8>( pGroup ) << " pDelay " << +pDelay << " " << std::bitset<8>( pDelay ) ;
        return cValue;
    }

    unsigned char fLookup[16] =
    {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
    }; /*!< Lookup table for reverce the endianness */
};
#endif
