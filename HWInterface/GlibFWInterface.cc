/*

        FileName :                    GlibFWInterface.h
        Content :                     GlibFWInterface init/config of the Glib and its Cbc's
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            28/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#include <time.h>
#include <chrono>
#include <uhal/uhal.hpp>
#include "GlibFWInterface.h"
#include "GlibFpgaConfig.h"

namespace Ph2_HwInterface {

    GlibFWInterface::GlibFWInterface ( const char* puHalConfigFileName,
                                       uint32_t pBoardId ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig ( nullptr ),
        fData ( nullptr ),
        fNthAcq (0)
    {
    }


    GlibFWInterface::GlibFWInterface ( const char* puHalConfigFileName,
                                       uint32_t pBoardId,
                                       FileHandler* pFileHandler ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig ( nullptr ),
        fData ( nullptr ),
        fFileHandler ( pFileHandler ),
        fNthAcq (0)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }

    GlibFWInterface::GlibFWInterface ( const char* pId,
                                       const char* pUri,
                                       const char* pAddressTable ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fData ( nullptr ),
        fNthAcq (0)
    {
    }


    GlibFWInterface::GlibFWInterface ( const char* pId,
                                       const char* pUri,
                                       const char* pAddressTable,
                                       FileHandler* pFileHandler ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fData ( nullptr ),
        fFileHandler ( pFileHandler ),
        fNthAcq (0)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }


    void GlibFWInterface::getBoardInfo()
    {
        std::cout << "FMC1 present : " << ReadReg ( "status.fmc1_present" ) << std::endl;
        std::cout << "FMC2 present : " << ReadReg ( "status.fmc2_present" ) << std::endl;
        std::cout << "FW version : " << ReadReg ( "firm_id.firmware_major" ) << "." << ReadReg ( "firm_id.firmware_minor" ) << "." << ReadReg ( "firm_id.firmware_build" ) << std::endl;

        uhal::ValWord<uint32_t> cBoardType = ReadReg ( "board_id" );

        std::cout << "BoardType : ";

        char cChar = ( ( cBoardType & cMask4 ) >> 24 );
        std::cout << cChar;

        cChar = ( ( cBoardType & cMask3 ) >> 16 );
        std::cout << cChar;

        cChar = ( ( cBoardType & cMask2 ) >> 8 );
        std::cout << cChar;

        cChar = ( cBoardType & cMask1 );
        std::cout << cChar << std::endl;

        std::cout << "FMC User Board ID : " << ReadReg ( "user_wb_ttc_fmc_regs.user_board_id" ) << std::endl;
        std::cout << "FMC User System ID : " << ReadReg ( "user_wb_ttc_fmc_regs.user_sys_id" ) << std::endl;
        std::cout << "FMC User Version : " << ReadReg ( "user_wb_ttc_fmc_regs.user_version" ) << std::endl;

    }


    void GlibFWInterface::ConfigureBoard ( const BeBoard* pBoard )
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        std::chrono::milliseconds cPause ( 200 );

        //Primary Configuration
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 1} );
        cVecReg.push_back ( {"pc_commands.SRAM1_end_readout", 0} );
        cVecReg.push_back ( {"pc_commands.SRAM2_end_readout", 0} );
        cVecReg.push_back ( {"ctrl_sram.sram1_user_logic", 1} );
        cVecReg.push_back ( {"ctrl_sram.sram2_user_logic", 1} );

        // iterate the BeBoardRegMap to get the user configuration
        BeBoardRegMap cGlibRegMap = pBoard->getBeBoardRegMap();

        for ( auto const& it : cGlibRegMap )
            cVecReg.push_back ( {it.first, it.second} );

        cVecReg.push_back ( {"pc_commands.SPURIOUS_FRAME", 0} );
        cVecReg.push_back ( {"pc_commands2.force_BG0_start", 0} );
        cVecReg.push_back ( {"cbc_acquisition.CBC_TRIGGER_ONE_SHOT", 0} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        cVecReg.push_back ( {"pc_commands.PC_config_ok", 0} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();
    }


    void GlibFWInterface::Start()
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        //Starting the DAQ
        cVecReg.push_back ( {"break_trigger", 0} );
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 1} );
        cVecReg.push_back ( {"pc_commands2.force_BG0_start", 1} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        // Since the Number of  Packets is a FW register, it should be read from the Settings Table which is one less than is actually read
        fNpackets = ReadReg ( "pc_commands.CBC_DATA_PACKET_NUMBER" ) + 1 ;

        //Wait for start acknowledge
        uhal::ValWord<uint32_t> cVal;
        std::chrono::milliseconds cWait ( 100 );

        do
        {
            cVal = ReadReg ( "status_flags.CMD_START_VALID" );

            if ( cVal == 0 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 0 );
    }

    void GlibFWInterface::Stop()
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        uhal::ValWord<uint32_t> cVal;

        //Select SRAM
        SelectDaqSRAM();
        //Stop the DAQ
        cVecReg.push_back ( {"break_trigger", 1} );
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 0} );
        cVecReg.push_back ( {"pc_commands2.force_BG0_start", 0} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        std::chrono::milliseconds cWait ( 100 );

        //Wait for the selected SRAM to be full then empty it
        do
        {
            cVal = ReadReg ( fStrFull );

            if ( cVal == 1 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 1 );

        WriteReg ( fStrReadout, 0 );
        fNTotalAcq++;
    }


    void GlibFWInterface::Pause()
    {
        WriteReg ( "break_trigger", 1 );
    }


    void GlibFWInterface::Resume()
    {
        WriteReg ( "break_trigger", 0 );
    }

    uint32_t GlibFWInterface::ReadData ( BeBoard* pBoard,  bool pBreakTrigger )
    {
        //Readout settings
        std::chrono::milliseconds cWait ( 1 );

        uhal::ValWord<uint32_t> cVal;

        if ( pBoard )
            fBlockSize = computeBlockSize ( pBoard );

        //FIFO goes to write_data state
        //Select SRAM
        SelectDaqSRAM();

        //Wait for the SRAM full condition.
        cVal = ReadReg ( fStrFull );

        do
        {
            cVal = ReadReg ( fStrFull );

            if ( cVal == 0 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 0 );

        //break trigger
        if ( pBreakTrigger ) WriteReg ( "break_trigger", 1 );

        //Set read mode to SRAM
        WriteReg ( fStrSramUserLogic, 0 );

        //Read SRAM
        std::vector<uint32_t> cData =  ReadBlockRegValue ( fStrSram, fBlockSize );

        WriteReg ( fStrSramUserLogic, 1 );
        WriteReg ( fStrReadout, 1 );

        //now I did an acquistion, so I need to increment the counter
        fNthAcq++;

        //Wait for the non SRAM full condition starts,
        do
        {
            cVal = ReadReg ( fStrFull );

            if ( cVal == 1 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 1 );

        //Wait for the non SRAM full condition ends.

        WriteReg ( fStrReadout, 0 );

        if ( pBreakTrigger ) WriteReg ( "break_trigger", 0 );

        // just creates a new Data object, setting the pointers and getting the correct sizes happens in Set()
        if ( fData ) delete fData;

        fData = new Data();

        // set the vector<uint32_t> as event buffer and let him know how many packets it contains
        fData->Set ( pBoard, cData , fNpackets, true );

        if ( fSaveToFile )
        {
            fFileHandler->set ( cData );
            fFileHandler->writeFile();
        }

        return fNpackets;
    }

    void GlibFWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents )
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        fNpackets = pNEvents;
        //Starting the DAQ
        cVecReg.push_back ( {"pc_commands.CBC_DATA_PACKET_NUMBER", pNEvents - 1} );
        cVecReg.push_back ( {"break_trigger", 0} );
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 1} );
        cVecReg.push_back ( {"pc_commands2.force_BG0_start", 1} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();


        //Wait for start acknowledge
        uhal::ValWord<uint32_t> cVal;
        std::chrono::milliseconds cWait ( 100 );

        do
        {
            cVal = ReadReg ( "status_flags.CMD_START_VALID" );

            if ( cVal == 0 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 0 );

        if ( pBoard )
            fBlockSize = computeBlockSize ( pBoard );

        //Select SRAM
        SelectDaqSRAM();

        //Wait for the SRAM full condition.
        cVal = ReadReg ( fStrFull );

        do
        {
            cVal = ReadReg ( fStrFull );

            if ( cVal == 0 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 0 );

        //break trigger
        cVecReg.push_back ({ "break_trigger", 1 } );
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 0} );
        cVecReg.push_back ( {"pc_commands2.force_BG0_start", 0} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        //Set read mode to SRAM
        WriteReg ( fStrSramUserLogic, 0 );

        //Read SRAM
        std::vector<uint32_t> cData =  ReadBlockRegValue ( fStrSram, fBlockSize );

        WriteReg ( fStrSramUserLogic, 1 );

        //need to increment the internal Acquisition counter
        fNthAcq++;

        // just creates a new Data object, setting the pointers and getting the correct sizes happens in Set()
        if ( fData ) delete fData;

        fData = new Data();

        // set the vector<uint32_t> as event buffer and let him know how many packets it contains
        fData->Set ( pBoard, cData , fNpackets, true );

        if ( fSaveToFile )
        {
            fFileHandler->set ( cData );
            fFileHandler->writeFile();
        }
    }


    /** compute the block size according to the number of CBC's on this board
     * this will have to change with a more generic FW */
    uint32_t GlibFWInterface::computeBlockSize ( BeBoard* pBoard )
    {
        //use a counting visitor to find out the number of CBCs
        struct CbcCounter : public HwDescriptionVisitor
        {
            uint32_t fNCbc = 0;

            void visit ( Cbc& pCbc )
            {
                fNCbc++;
            }
            uint32_t getNCbc()
            {
                if ( fNCbc == 2 )
                    // since the 2 CBC FW outputs data for 4 CBCs (beamtest heritage, might have to change in the future)
                    return 2 * fNCbc;
                else return fNCbc;
            }
        };

        CbcCounter cCounter;
        pBoard->accept ( cCounter );

        if ( pBoard->getNCbcDataSize() != 0 ) return fNpackets * ( pBoard->getNCbcDataSize() * CBC_EVENT_SIZE_32 + EVENT_HEADER_TDC_SIZE_32 );
        else return fNpackets * ( cCounter.getNCbc() * CBC_EVENT_SIZE_32 + EVENT_HEADER_TDC_SIZE_32 ); // in 32 bit words
    }

    std::vector<uint32_t> GlibFWInterface::ReadBlockRegValue ( const std::string& pRegNode, const uint32_t& pBlocksize )
    {
        uhal::ValVector<uint32_t> valBlock = ReadBlockReg ( pRegNode, pBlocksize );
        std::vector<uint32_t> vBlock = valBlock.value();

        // To avoid the IPBUS bug
        // need to convert uHal::ValVector to vector<uint32_t> so we can replace the 256th word
        if ( pBlocksize > 255 )
        {
            std::string fSram_256 = pRegNode + "_256";
            uhal::ValWord<uint32_t> cWord = ReadReg ( fSram_256 );
            vBlock[255] = cWord.value();
        }

        return vBlock;
    }

    bool GlibFWInterface::WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
    {
        bool cWriteCorr = RegManager::WriteBlockReg ( pRegNode, pValues );

        if ( pValues.size() > 255 )
            WriteReg ( pRegNode + "_256", pValues[255] );

        return cWriteCorr;
    }

    void GlibFWInterface::SelectDaqSRAM()
    {
        fStrSram  = ( ( fNthAcq % 2 + 1 ) == 1 ? "sram1" : "sram2" );
        fStrSramUserLogic = ( ( fNthAcq % 2 + 1 ) == 1 ? "ctrl_sram.sram1_user_logic" : "ctrl_sram.sram2_user_logic" );
        fStrFull = ( ( fNthAcq % 2 + 1 ) == 1 ? "flags.SRAM1_full" : "flags.SRAM2_full" );
        fStrReadout = ( ( fNthAcq % 2 + 1 ) == 1 ? "pc_commands.SRAM1_end_readout" : "pc_commands.SRAM2_end_readout" );
    }



    //Methods for Cbc's:

    void GlibFWInterface::StartThread ( BeBoard* pBoard, uint32_t uNbAcq, HwInterfaceVisitor* visitor )
    {
        if ( runningAcquisition ) return;

        runningAcquisition = true;
        numAcq = 0;
        nbMaxAcq = uNbAcq;

        thrAcq = boost::thread ( &Ph2_HwInterface::GlibFWInterface::threadAcquisitionLoop, this, pBoard, visitor );
    }

    void GlibFWInterface::threadAcquisitionLoop ( BeBoard* pBoard, HwInterfaceVisitor* visitor )
    {
        Start( );
        fBlockSize = computeBlockSize ( pBoard );

        while ( runningAcquisition && ( nbMaxAcq == 0 || numAcq < nbMaxAcq ) )
        {
            ReadData ( pBoard, true );

            for ( const Ph2_HwInterface::Event* cEvent = GetNextEvent ( pBoard ); cEvent; cEvent = GetNextEvent ( pBoard ) )
                visitor->visit ( *cEvent );

            if ( runningAcquisition )
                numAcq++;

        }

        Stop ( );
        runningAcquisition = false;
    };

    ///////////////////////////////////////////////////////
    //      CBC Methods                                 //
    /////////////////////////////////////////////////////

    void GlibFWInterface::EncodeReg ( const CbcRegItem& pRegItem,
                                      uint8_t pCbcId,
                                      std::vector<uint32_t>& pVecReq,
                                      bool pRead,
                                      bool pWrite )
    {
        // temporary for 16CBC readout FW  (Beamtest NOV 15)
        // will have to be corrected if we want to read two modules from the same GLIB
        // (pCbcId >> 3) becomes FE ID and is encoded starting from bit21 (not used so far)
        // (pCbcId & 7) restarts CbcIDs from 0 for FE 1 (if CbcID > 7)
        pVecReq.push_back ( ( pCbcId >> 3 ) << 21 | ( pCbcId & 7 ) << 17 | pRegItem.fPage << 16 | pRegItem.fAddress << 8 | pRegItem.fValue );
    }

    void GlibFWInterface::EncodeReg ( const CbcRegItem& pRegItem,
                                      uint8_t pFeId,
                                      uint8_t pCbcId,
                                      std::vector<uint32_t>& pVecReq,
                                      bool pRead,
                                      bool pWrite )
    {
        // (pCbcId & 7) restarts CbcIDs from 0 for FE 1 (if CbcID > 7)
        pVecReq.push_back ( pFeId  << 21 | pCbcId << 17 | pRegItem.fPage << 16 | pRegItem.fAddress << 8 | pRegItem.fValue );
    }

    void GlibFWInterface::BCEncodeReg ( const CbcRegItem& pRegItem,
                                        uint8_t pNCbc,
                                        std::vector<uint32_t>& pVecReq,
                                        bool pRead,
                                        bool pWrite )
    {
        // here I need to loop over all CBCs somehow...
        for (uint8_t cCbcId = 0; cCbcId < pNCbc; cCbcId++)
            pVecReq.push_back ( ( cCbcId >> 3 ) << 21 | ( cCbcId & 7 ) << 17 | pRegItem.fPage << 16 | pRegItem.fAddress << 8 | pRegItem.fValue );
    }

    void GlibFWInterface::DecodeReg ( CbcRegItem& pRegItem,
                                      uint8_t& pCbcId,
                                      uint32_t pWord,
                                      bool& pRead,
                                      bool& pFailed )
    {
        // temporary for 16CBC readout FW  (Beamtest NOV 15)
        // will have to be corrected if we want to read two modules from the same GLIB
        uint8_t cFeId = ( pWord & cMask7 ) >> 21;
        pCbcId = ( ( pWord & cMask5 ) | ( cFeId << 3 ) ) >> 17;
        pRegItem.fPage = ( pWord & cMask6 ) >> 16;
        pRegItem.fAddress = ( pWord & cMask2 ) >> 8;
        pRegItem.fValue = pWord & cMask1;
        //std::cout << "FEID " << +(cFeId) << " pCbcID " << +(pCbcId) << std::endl;
    }

    bool GlibFWInterface::I2cCmdAckWait ( uint32_t pAckVal, uint8_t pNcount )
    {
        unsigned int cWait ( 100 );

        if ( pAckVal )
            cWait = pNcount * 500;

        usleep ( cWait );

        uhal::ValWord<uint32_t> cVal;
        uint32_t cLoop = 0;

        do
        {
            cVal = ReadReg ( "cbc_i2c_cmd_ack" );

            if ( cVal != pAckVal )
                usleep ( cWait );
            else return true;
        }
        while ( cVal != pAckVal && ++cLoop < 70 );

        return false;
    }

    void GlibFWInterface::WriteI2C ( std::vector<uint32_t>& pVecReq, bool pWrite )
    {
        pVecReq.push_back ( 0xFFFFFFFF );

        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        WriteReg ( "ctrl_sram.sram1_user_logic", 0 );
        WriteBlockReg ( "sram1", pVecReq );

        cVecReg.push_back ( {"ctrl_sram.sram1_user_logic", 1} );
        cVecReg.push_back ( {"cbc_i2c_cmd_rq", pWrite ? 3 : 1} );
        WriteStackReg ( cVecReg );

        if ( I2cCmdAckWait ( ( uint32_t ) 1, pVecReq.size() ) == 0 )
            throw Exception ( "CbcInterface: I2cCmdAckWait 1 failed." );

        WriteReg ( "cbc_i2c_cmd_rq", 0 );

        if ( I2cCmdAckWait ( ( uint32_t ) 0, pVecReq.size() ) == 0 )
            throw Exception ( "CbcInterface: I2cCmdAckWait 0 failed." );
    }

    void GlibFWInterface::ReadI2C ( std::vector<uint32_t>& pVecReq )
    {
        WriteReg ( "ctrl_sram.sram1_user_logic", 0 );
        pVecReq = ReadBlockRegValue ( "sram1", pVecReq.size() );
        std::vector< std::pair<std::string, uint32_t> > cVecReg;
        cVecReg.push_back ( {"ctrl_sram.sram1_user_logic", 1} );
        cVecReg.push_back ( {"cbc_i2c_cmd_rq", 0} );
        WriteStackReg ( cVecReg );
    }


    bool GlibFWInterface::WriteCbcBlockReg ( uint8_t pFeId, std::vector<uint32_t>& pVecReq, bool pReadback)
    {
        bool cSuccess = false;
        std::vector<uint32_t> cWriteVec = pVecReq;

        try
        {
            WriteI2C ( pVecReq, true );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        // now read back
        // not sure if I should clear the last 8 bits of the vector, let's assume it is safe to not do that
        if (pReadback)
        {
            try
            {
                WriteI2C ( pVecReq, false );
            }
            catch ( Exception& e )
            {
                throw e;
            }

            try
            {
                ReadI2C ( pVecReq );
            }
            catch (Exception& e )
            {
                throw e;
            }

            // now pVecReq contains the read data, I could add the comparison in here by making a copy of the original data in the beginning
            // now I need to make sure that the written and the read-back vector are the same
            auto cMismatchWord = std::mismatch ( cWriteVec.begin(), cWriteVec.end(), pVecReq.begin() );

            if ( cMismatchWord.first == cWriteVec.end() ) cSuccess = true;
            else
            {
                std::vector<uint32_t> cWriteAgain;

                while ( cMismatchWord.first != cWriteVec.end() )
                {
                    //here decode the items for printout if necessary
                    //CbcRegItem cWriteItem;
                    //uint8_t cCbcId;
                    //DecodeReg (cWriteItem, cCbcId, *cMismatchWord.first );
                    //CbcRegItem cReadItem;
                    //DecodeReg (cVecReq, cCbcId, *cMismatchWord.second);

                    cWriteAgain.push_back (*cMismatchWord.first);
                    //move the iterator oneward
                    cMismatchWord = std::mismatch (++cMismatchWord.first, cWriteVec.end(), ++cMismatchWord.second );
                    cSuccess = false;
                }

                // this is recursive - da chit!
                if (cWriteAgain.size() < 100)
                {
                    std::cout << "There were readback errors, retrying!" << std::endl;
                    this->WriteCbcBlockReg (pFeId, cWriteAgain, true);

                }
                else std::cout << "There were too many errors (>100 Registers). Something is wrong - aborting!" << std::endl;
            }
        }
        else cSuccess = true;

        return cSuccess;
    }


    bool GlibFWInterface::BCWriteCbcBlockReg (uint8_t pFeId, std::vector<uint32_t>& pVecReq, bool pReadback)
    {
        //use the method above for that!
        bool cSuccess = false;
        std::vector<uint32_t> cWriteVec = pVecReq;

        try
        {
            WriteI2C ( pVecReq, true );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        return cSuccess;
    }

    void GlibFWInterface::ReadCbcBlockReg ( uint8_t pFeId, std::vector<uint32_t>& pVecReq )
    {
        try
        {
            WriteI2C ( pVecReq, false );
        }
        catch ( Exception& e )
        {
            throw e;
        }

        try
        {
            ReadI2C ( pVecReq );
        }
        catch (Exception& e )
        {
            throw e;
        }
    }

    void GlibFWInterface::CbcFastReset()
    {
        WriteReg ( "cbc_fast_reset", 1 );

        usleep ( 2000 );

        WriteReg ( "cbc_fast_reset", 0 );
    }

    void GlibFWInterface::CbcHardReset()
    {
        WriteReg ( "cbc_hard_reset", 1 );

        usleep ( 2000 );

        WriteReg ( "cbc_hard_reset", 0 );

        usleep ( 20 );
    }

    void GlibFWInterface::FlashProm ( const std::string& strConfig, const char* pstrFile )
    {
        if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
            throw Exception ( "This board is already uploading an FPGA configuration" );

        if ( !fpgaConfig )
            fpgaConfig = new GlibFpgaConfig ( this );

        fpgaConfig->runUpload ( strConfig, pstrFile );
    }

    void GlibFWInterface::JumpToFpgaConfig ( const std::string& strConfig )
    {
        if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
            throw Exception ( "This board is uploading an FPGA configuration" );

        if ( !fpgaConfig )
            fpgaConfig = new GlibFpgaConfig ( this );

        fpgaConfig->jumpToImage ( strConfig );
    }

}
