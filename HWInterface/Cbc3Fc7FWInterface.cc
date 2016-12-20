/*!

        \file                           ICICCbc3Fc7FWInterface.h
        \brief                          ICICCbc3Fc7FWInterface init/config of the Glib and its Cbc's
        \author                         G. Auzinger, K. Uchida
        \version            1.0
        \date                           25.02.2016
        Support :                       mail to : georg.auzinger@SPAMNOT.cern.ch

 */

#include <time.h>
#include <chrono>
#include <uhal/uhal.hpp>
#include "Cbc3Fc7FWInterface.h"
#include "CtaFpgaConfig.h"
//#include "CbcInterface.h"


namespace Ph2_HwInterface {

    Cbc3Fc7FWInterface::Cbc3Fc7FWInterface ( const char* puHalConfigFileName,
            uint32_t pBoardId ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig (nullptr),
        fBroadcastCbcId (0),
        fNCbc (0),
        fFMCId (1)
    {}


    Cbc3Fc7FWInterface::Cbc3Fc7FWInterface ( const char* puHalConfigFileName,
            uint32_t pBoardId,
            FileHandler* pFileHandler ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig (nullptr),
        fBroadcastCbcId (0),
        fNCbc (0),
        fFileHandler ( pFileHandler ),
        fFMCId (1)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }

    Cbc3Fc7FWInterface::Cbc3Fc7FWInterface ( const char* pId,
            const char* pUri,
            const char* pAddressTable ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fBroadcastCbcId (0),
        fNCbc (0),
        fFMCId (1)
    {}


    Cbc3Fc7FWInterface::Cbc3Fc7FWInterface ( const char* pId,
            const char* pUri,
            const char* pAddressTable,
            FileHandler* pFileHandler ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fBroadcastCbcId (0),
        fNCbc (0),
        fFileHandler ( pFileHandler ),
        fFMCId (1)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }

    void Cbc3Fc7FWInterface::setFileHandler (FileHandler* pHandler)
    {
        if (pHandler != nullptr )
        {
            fFileHandler = pHandler;
            fSaveToFile = true;
        }
        else LOG (INFO) << "Error, can not set NULL FileHandler" ;
    }

    uint32_t Cbc3Fc7FWInterface::getBoardInfo()
    {
        //LOG(INFO) << "FMC1 present : " << ReadReg ( "user_stat.current_fec_fmc2_cbc0" ) ;
        //LOG(INFO) << "FMC2 present : " << ReadReg ( "user_stat.current_fec_fmc2_cbc1" ) ;
        uint32_t cVersionMajor, cVersionMinor;
        cVersionMajor = ReadReg ( "cbc_system_stat.global.version.ver_major" );
        cVersionMinor = ReadReg ( "cbc_system_stat.global.version.ver_minor" );
        LOG (INFO) << "FW version : " << cVersionMajor << "." << cVersionMinor << "." << std::to_string (ReadReg ( "cbc_system_stat.global.version.ver_build" ) ) ;

        uhal::ValWord<uint32_t> cBoardType = ReadReg ( "cbc_system_stat.global.id" );

        LOG (INFO) << "BoardType : ";

        char cChar = ( ( cBoardType & cMask4 ) >> 24 );
        LOG (INFO) << cChar;

        cChar = ( ( cBoardType & cMask3 ) >> 16 );
        LOG (INFO) << cChar;

        cChar = ( ( cBoardType & cMask2 ) >> 8 );
        LOG (INFO) << cChar;

        cChar = ( cBoardType & cMask1 );
        LOG (INFO) << cChar ;

        uint32_t cVersionWord = ( (cVersionMajor & 0x0000FFFF) << 16 || (cVersionMinor & 0x0000FFFF) );
        return cVersionWord;
    }

    void Cbc3Fc7FWInterface::ConfigureBoard ( const BeBoard* pBoard )
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        //OK, first we need to apply the configuration to the config part of the FW residing at address 0x40000100
        //all IDs start with 1
        cVecReg.push_back ({"cbc_system_cnfg.global.be.id", pBoard->getBeId() + 1  });
        //enable fast signal ipbus

        //then loop the HWDescription and find out about our Connected CBCs
        for (Module* cFe : pBoard->fModuleVector)
        {
            //configure the CBCs - preliminary FW only supports 1 CBC but put the rest of the code there and comment
            for ( Cbc* cCbc : cFe->fCbcVector)
            {
                //need to increment the NCbc counter for I2C controller
                fNCbc++;

                uint8_t cCbcId = cCbc->getCbcId();
                uint8_t cFeId = cCbc->getFeId();
                uint32_t cAddress = 0x41 + cCbcId;
                char cTmpChar[30];
                sprintf (cTmpChar, "cbc_system_cnfg.global.cbc%d.", cCbcId );
                std::string cRegString (cTmpChar);
                cVecReg.push_back ({cRegString + "active", 0x1});
                // TODO
                //all IDs start with 1
                cVecReg.push_back ({cRegString + "id", cCbcId + 1});
                cVecReg.push_back ({cRegString + "fe_id", cFeId + 1});
                cVecReg.push_back ({cRegString + "i2c_address", cAddress});
            }
        }

        //this might need some toggling
        //cVecReg.push_back ({"cbc_system_cnfg.global.misc.cbc_clk_phase_shift", 0x1});

        //last, loop over the variable registers from the HWDescription.xml file
        //this is where I should get all the clocking and FastCommandInterface settings
        BeBoardRegMap cGlibRegMap = pBoard->getBeBoardRegMap();

        for ( auto const& it : cGlibRegMap )
        {
            cVecReg.push_back ( {it.first, it.second} );
        }

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        //hard reset CBC and the DAQ
        this->CbcHardReset();

        //this will do everthing that comes below eventually
        //WriteReg ("cbc_system_ctrl.global.init", 0x1);
        //temporary, should be in global init later
        WriteReg ("cbc_system_ctrl.cbc_i2c_bus_managers.fe0.reset_fifos", 0x1);
        WriteReg ("cbc_system_ctrl.cbc_i2c_bus_managers.fe0.reset", 0x1);
        WriteReg ("cbc_system_ctrl.cbc_i2c_bus_managers.fe0.init", 0x1);

        std::this_thread::sleep_for (std::chrono::microseconds (50) * fNCbc );

        //read the replies for the pings!
        std::vector<uint32_t> pReplies;
        ReadI2C (  fNCbc, pReplies);

        bool cSuccess = false;

        for (auto& cWord : pReplies)
            cSuccess = ( ( (cWord >> 20) & 0x1) == 0 && ( (cWord) & 0x000000FF) != 0 ) ? true : false;

        if (cSuccess) LOG (INFO) << "Successfully received *Pings* from " << fNCbc << " Cbcs";
        else LOG (INFO) << "Error, did not receive the correct number of *Pings*; expected: " << fNCbc << ", received: " << pReplies.size() ;

        //perform a global reset, just to be sure
        WriteReg ("cbc_system_ctrl.global.reset", 0x1);
    }

    void Cbc3Fc7FWInterface::FindPhase()
    {
        //this is to run the idelay tuning, similar to what we had to do for the pixels
        uint32_t cCount = 0;

        while (ReadReg ("cbc_system_stat.global.misc.idelayctrl_rdy") == 0)
        {
            WriteReg ("cbc_system_ctrl.global.idelayctrl_reset", 1);

            if (++cCount > 10)
            {
                LOG (ERROR) << "Error, idelayctrl does not go to ready! Aborting!";
                exit (1);
            }

            std::this_thread::sleep_for (std::chrono::microseconds (1000) );
        }

        WriteReg ("cbc_system_ctrl.cbc_data_processor.cbc0.ser_data_delay_reset", 1);
        WriteReg ("cbc_system_ctrl.cbc_data_processor.cbc0.ser_data_delay_start_tuning", 1);
        std::this_thread::sleep_for (std::chrono::microseconds (20) );

        uint32_t cFsm = ReadReg ("cbc_system_stat.cbc_data_processor.cbc0.ser_data_delay_idelay_tuning_fsm");
        cCount = 1;

        while (cFsm != 4)
        {
            LOG (DEBUG) << "FSM: " << cFsm;
            //here read "SerialIface&Error" register of CBC
            //this whole block is needed for the manual low-level I2C transaction
            CbcRegItem cRegItem (0, 0x1D, 0, 0);
            std::vector<uint32_t> cVecReq;
            this->EncodeReg (cRegItem, 0, cVecReq, true, false);
            this->ReadCbcBlockReg (cVecReq);
            bool cFailed = false;
            bool cRead;
            uint8_t cId;
            this->DecodeReg (cRegItem, cId, cVecReq.at (0), cRead, cFailed);
            LOG (DEBUG) << "Read CbcI2C Register \"SerialIface&Error\" on page 0, address 0x1D: ";
            LOG (DEBUG) << "RAM Buffer Overflow: " << ( (cRegItem.fValue >> 4) & 0x1);
            LOG (DEBUG) << "Latency Error:       " << ( (cRegItem.fValue >> 3) & 0x1);
            LOG (DEBUG) << "Sync Lost:           " << ( (cRegItem.fValue >> 2) & 0x1);
            LOG (DEBUG) << "Sync Stat:           " << ( (cRegItem.fValue >> 1) & 0x1);
            LOG (DEBUG) << "Bad Code:            " << ( (cRegItem.fValue ) & 0x1   );
            LOG (INFO) << "sending Cbc fast reset!";
            this->CbcFastReset();
            //here read "SerialIface&Error" register of CBC again
            //this whole block is needed for the manual low-level I2C transaction
            cRegItem.fValue = 0;
            cVecReq.clear();
            this->EncodeReg (cRegItem, 0, cVecReq, true, false);
            this->ReadCbcBlockReg (cVecReq);
            cFailed = false;
            this->DecodeReg (cRegItem, cId, cVecReq.at (0), cRead, cFailed);
            LOG (DEBUG) << "Read CbcI2C Register \"SerialIface&Error\" on page 0, address 0x1D: ";
            LOG (DEBUG) << "RAM Buffer Overflow: " << ( (cRegItem.fValue >> 4) & 0x1);
            LOG (DEBUG) << "Latency Error:       " << ( (cRegItem.fValue >> 3) & 0x1);
            LOG (DEBUG) << "Sync Lost:           " << ( (cRegItem.fValue >> 2) & 0x1);
            LOG (DEBUG) << "Sync Stat:           " << ( (cRegItem.fValue >> 1) & 0x1);
            LOG (DEBUG) << "Bad Code:            " << ( (cRegItem.fValue ) & 0x1   );

            WriteReg ("cbc_system_ctrl.cbc_data_processor.cbc0.ser_data_delay_reset", 1);
            WriteReg ("cbc_system_ctrl.cbc_data_processor.cbc0.ser_data_delay_start_tuning", 1);
            std::this_thread::sleep_for (std::chrono::microseconds (20) );
            cFsm = ReadReg ("cbc_system_stat.cbc_data_processor.cbc0.ser_data_delay_idelay_tuning_fsm");

            if (++cCount > 5)
            {
                LOG (ERROR) << "Error, idelay tuning failed! Aborting!";
                exit (1);
            }
        }

        uint32_t cDelay = ReadReg ("cbc_system_stat.cbc_data_processor.cbc0.ser_data_delay_idelay_delay");
        LOG (INFO) << "Idelay tuned to delay tap = " << cDelay;
    }

    void Cbc3Fc7FWInterface::Start()
    {
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_generator_stop", 0x1);
        WriteReg ("cbc_system_ctrl.fast_command_manager.stop_trigger", 0x1);
        //first reset the DAQ
        //global daq reset is not implemented yet
        //WriteReg ("cbc_system_ctrl.global.daq_reset", 0x1);
        //in the meantime, do this
        //std::vector< std::pair<std::string, uint32_t> > cVecReg;
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_reset", 0x1);
        WriteReg ("cbc_system_ctrl.cbc_data_processor.cbc0.reset", 0x1);
        WriteReg ("cbc_system_ctrl.event_builder.reset", 0x1);
        WriteReg ("cbc_system_ctrl.data_buffer.reset", 0x1);
        //WriteStackReg ( cVecReg );
        //cVecReg.clear();

        //this could go into Configure() once it is more stable
        this->FindPhase();

        //then start the triggers
        WriteReg ("cbc_system_ctrl.fast_command_manager.start_trigger", 0x1);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );
        //reload the config of the fast_command_manager
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_generator_load_config", 0x1);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );
        //start the periodic fast signals if enabled
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_generator_start", 0x1);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );
        LOG (DEBUG) << "Reading " << "cbc_system_stat.serial_command_generator.fast_signal_generator_fsm " << static_cast<uint32_t> (ReadReg ("cbc_system_stat.serial_command_generator.fast_signal_generator_fsm") );
    }

    void Cbc3Fc7FWInterface::Stop()
    {
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_generator_stop", 0x1);
        WriteReg ("cbc_system_ctrl.fast_command_manager.stop_trigger", 0x1);
    }


    void Cbc3Fc7FWInterface::Pause()
    {
        //this should just brake triggers
        //WriteReg ("cbc_system_ctrl.serial_command_generator.fast_signal_generator_stop", 0x1);
        WriteReg ("cbc_system_ctrl.fast_command_manager.stop_trigger", 0x1);
    }


    void Cbc3Fc7FWInterface::Resume()
    {
        //WriteReg ("cbc_system_ctrl.serial_command_generator.fast_signal_generator_start", 0x1);
        WriteReg ("cbc_system_ctrl.fast_command_manager.start_trigger", 0x1);
    }

    uint32_t Cbc3Fc7FWInterface::ReadData ( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData )
    {
        //ok, first query the number of words to read from FW and if it is 0, wait for half a second
        //in other words, poll for the ring buffer to be NOT empty
        uint32_t cNWords = ReadReg ("cbc_system_stat.data_buffer.nword_events");
        //this->CbcTrigger();

        while (cNWords == 0)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (500) );
            //cNWords = ReadReg ("cbc_system_stat.data_buffer.nword_all");
            cNWords = ReadReg ("cbc_system_stat.data_buffer.nword_events");
            LOG (DEBUG) << cNWords;
        }

        pData = ReadBlockRegValue ("data", cNWords);

        if (fSaveToFile)
        {
            fFileHandler->set (pData);
            //fFileHandler->writeFile();
        }

        //need to return the number of events read
        uint32_t cEventSize = computeEventSize (pBoard);
        uint32_t cNEvents = 0;

        if (cNWords % cEventSize == 0 ) cNEvents = cNWords / cEventSize;
        else
            LOG (ERROR) << "Packet Size is not a multiple of the event size!";

        //return nEvents;
        return cNEvents;
    }


    void Cbc3Fc7FWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData )
    {
        //as per Kirika's recommendation, I will use the internal fast signal generator for this - if a method shows up to do this also with external triggers I can always modify in the future!
        //Procedure as follows:
        //1)configure a cycle that only sends 100 triggers
        //2)send a serials_command_generator.fast_signal_generator_start
        //3)wait sufficiently long or make sure that the number of words to be read is pNEvents*EventSize
        //4)read data

        //Reset the DAQ and clear all the buffers
        //not implemented yet, in the meantime do below
        //WriteReg ("cbc_system_ctrl.global.daq_reset", 0x1);
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_generator_stop", 0x1);
        WriteReg ("cbc_system_ctrl.fast_command_manager.stop_trigger", 0x1);
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_reset", 0x1);
        WriteReg ("cbc_system_ctrl.cbc_data_processor.cbc0.reset", 0x1);
        WriteReg ("cbc_system_ctrl.event_builder.reset", 0x1);
        WriteReg ("cbc_system_ctrl.data_buffer.reset", 0x1);
        //WriteStackReg ( cVecReg );
        //cVecReg.clear();

        std::vector< std::pair<std::string, uint32_t> > cVecReg;
        // configure the fast command cycle to send triggers
        cVecReg.push_back ({"cbc_system_cnfg.fast_signal_generator.enable.trigger", 0x1});
        cVecReg.push_back ({"cbc_system_cnfg.fast_signal_generator.Ncycle", pNEvents});
        WriteStackReg ( cVecReg );
        cVecReg.clear();


        //then start the triggers
        WriteReg ("cbc_system_ctrl.fast_command_manager.start_trigger", 0x1);
        //reload the config of the fast_command_manager
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_generator_load_config", 0x1);
        //start the periodic fast signals if enabled
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_generator_start", 0x1);

        //now wait until nword_event is equal to pNEvents * eventSize
        uint32_t cNWords = 0;
        uint32_t cEventSize = 3 + fNCbc * 11; // 3 words event header + nCbc*11 words per CBC in unsparsified mode

        while (cNWords < pNEvents * cEventSize )
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (100) );
            cNWords = ReadReg ("cbc_system_stat.data_buffer.nword_events");
        }

        if (cNWords != pNEvents * cEventSize) LOG (ERROR) << "Error, did not read correct number of words for " << pNEvents << " Events! (read value= " << cNWords << "; expected= " << pNEvents* cEventSize << ")";

        //disable triggers
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_generator_stop", 0x1);
        WriteReg ("cbc_system_ctrl.fast_command_manager.stop_trigger", 0x1);

        //and read data
        pData = ReadBlockRegValue ("data", cNWords);

        if ( fSaveToFile )
        {
            fFileHandler->set ( pData );
            //fFileHandler->writeFile();
        }
    }

    /** compute the block size according to the number of CBC's on this board
     * this will have to change with a more generic FW */
    uint32_t Cbc3Fc7FWInterface::computeEventSize ( BeBoard* pBoard )
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

        //return 3 words header + fNCbc * CBC Event Size  (11 words)
        return cCounter.getNCbc() * CBC_EVENT_SIZE_32_CBC3 + EVENT_HEADER_TDC_SIZE_32_CBC3;
    }

    std::vector<uint32_t> Cbc3Fc7FWInterface::ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize )
    {
        uhal::ValVector<uint32_t> valBlock = ReadBlockReg ( pRegNode, pBlocksize );
        std::vector<uint32_t> vBlock = valBlock.value();
        return vBlock;
    }

    bool Cbc3Fc7FWInterface::WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
    {
        bool cWriteCorr = RegManager::WriteBlockReg ( pRegNode, pValues );
        return cWriteCorr;
    }

    ///////////////////////////////////////////////////////
    //      CBC Methods                                 //
    /////////////////////////////////////////////////////
    //TODO: check what to do with fFMCid and if I need it!
    // this is clearly for addressing individual CBCs, have to see how to deal with broadcast commands
    // for this FW, page values are 1 and 2 so need to increment the 0 and 1 from config file
    void Cbc3Fc7FWInterface::EncodeReg ( const CbcRegItem& pRegItem,
                                         uint8_t pCbcId,
                                         std::vector<uint32_t>& pVecReq,
                                         bool pRead,
                                         bool pWrite )
    {
        //use fBroadcastCBCId for broadcast commands
        pVecReq.push_back ( ( (fFMCId ) << 29 ) | ( (pCbcId + 1) << 24 ) | (  pRead << 21 ) | (  pWrite << 20 ) | ( (pRegItem.fPage) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
    }
    void Cbc3Fc7FWInterface::EncodeReg ( const CbcRegItem& pRegItem,
                                         uint8_t pFeId,
                                         uint8_t pCbcId,
                                         std::vector<uint32_t>& pVecReq,
                                         bool pRead,
                                         bool pWrite )
    {
        //use fBroadcastCBCId for broadcast commands
        pVecReq.push_back ( ( (pFeId + 1) << 29 ) | ( (pCbcId + 1) << 24 ) | (  pRead << 21 ) | (  pWrite << 20 ) | ( (pRegItem.fPage ) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
    }

    void Cbc3Fc7FWInterface::BCEncodeReg ( const CbcRegItem& pRegItem,
                                           uint8_t pNCbc,
                                           std::vector<uint32_t>& pVecReq,
                                           bool pRead,
                                           bool pWrite )
    {
        //use fBroadcastCBCId for broadcast commands
        pVecReq.push_back ( ( (fFMCId ) << 29 ) | ( fBroadcastCbcId << 24 ) | (  pRead << 21 ) | (  pWrite << 20 )  | ( (pRegItem.fPage ) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
    }

    void Cbc3Fc7FWInterface::DecodeReg ( CbcRegItem& pRegItem,
                                         uint8_t& pCbcId,
                                         uint32_t pWord,
                                         bool& pRead,
                                         bool& pFailed )
    {
        pCbcId   =  ( ( pWord & 0x07000000 ) >> 24) - 1;
        pFailed  =  ( ( pWord & 0x00100000 ) >> 20) ;
        //pRead is 1 for read transaction, 0 for a write transaction
        pRead    =  ( pWord & 0x00020000 ) >> 17;
        pRegItem.fPage    =  ( (pWord & 0x00010000  ) >> 16) ;
        pRegItem.fAddress =  ( pWord & 0x0000FF00 ) >> 8;
        pRegItem.fValue   =  ( pWord & 0x000000FF );
    }

    bool Cbc3Fc7FWInterface::ReadI2C (  uint32_t pNReplies, std::vector<uint32_t>& pReplies)
    {
        usleep (SINGLE_I2C_WAIT * pNReplies );

        bool cFailed (false);

        //read the number of received replies from ndata and use this number to compare with the number of expected replies and to read this number 32-bit words from the reply FIFO
        uint32_t cNReplies = ReadReg ("cbc_system_stat.cbc_i2c_bus_managers.fe0.reply_fifo_ndata");


        if (cNReplies != pNReplies)
        {
            LOG (INFO) << "Error: Read " << cNReplies << " I2C replies whereas " << pNReplies << " are expected!" ;
            cFailed = true;
        }

        try
        {
            pReplies = ReadBlockRegValue ( "cbc_i2c_regs.reply_fifos.fe0", cNReplies );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        //reset the i2c controller here?
        return cFailed;
    }

    bool Cbc3Fc7FWInterface::WriteI2C ( std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies, bool pReadback, bool pBroadcast )
    {
        bool cFailed ( false );
        //reset the I2C controller
        WriteReg ("cbc_system_ctrl.cbc_i2c_bus_managers.fe0.reset_fifos", 0x1);

        try
        {
            WriteBlockReg ( "cbc_i2c_regs.command_fifos", pVecSend );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        uint32_t cNReplies = pVecSend.size() * ( pReadback ? 2 : 1 ) * ( pBroadcast ? fNCbc : 1 );

        cFailed = ReadI2C (  cNReplies, pReplies) ;

        return cFailed;
    }



    bool Cbc3Fc7FWInterface::WriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, uint8_t& pWriteAttempts , bool pReadback)
    {

        uint8_t cMaxWriteAttempts = 5;
        // the actual write & readback command is in the vector
        std::vector<uint32_t> cReplies;
        bool cSuccess = !WriteI2C ( pVecReg, cReplies, pReadback, false );

        for (int i = 0; i < pVecReg.size(); i++)
        {
            LOG (DEBUG) << std::bitset<16> ( pVecReg.at (i)  >> 16)  << " " << std::bitset<16> ( pVecReg.at (i) );
            LOG (DEBUG) << std::bitset<16> ( cReplies.at (2 * i)  >> 16)  << " " << std::bitset<16> ( cReplies.at (2 * i) );
            LOG (DEBUG) << std::bitset<16> ( cReplies.at (2 * i + 1 )  >> 16)  << " " << std::bitset<16> ( cReplies.at (2 * i + 1 ) );
            LOG (DEBUG) << std::endl;
        }

        LOG (DEBUG) << "Command Size: " << pVecReg.size() << " Reply size " << cReplies.size();

        // the reply format is different from the sent format, therefore a binary predicate is necessary to compare
        // fValue is in the 8 lsb, then address is in 15 downto 8, page is in 16, CBCId is in 24

        //here make a distinction: if pReadback is true, compare only the read replies using the binary predicate
        //else, just check that info is 0 and thus the CBC acqnowledged the command if the writeread is 0
        std::vector<uint32_t> cWriteAgain;

        if (pReadback)
        {
            //split the reply vector in even and odd replies
            //even is the write reply, odd is the read reply
            //since I am already reading back, might as well forget about the CMD acknowledge from the CBC and directly look at the read back value
            std::vector<uint32_t> cOdd;
            getOddElements (cReplies, cOdd);

            //now use the Template from BeBoardFWInterface to return a vector with all written words that have been read back incorrectly
            cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cOdd.begin(), Cbc3Fc7FWInterface::cmd_reply_comp);

            // now clear the initial cmd Vec and set the read-back
            pVecReg.clear();
            pVecReg = cOdd;
        }
        else
        {
            //since I do not read back, I can safely just check that the info bit of the reply is 0 and that it was an actual write reply
            //then i put the replies in pVecReg so I can decode later in CBCInterface
            cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cReplies.begin(), Cbc3Fc7FWInterface::cmd_reply_ack);
            pVecReg.clear();
            pVecReg = cReplies;
        }

        // now check the size of the WriteAgain vector and assert Success or not
        // also check that the number of write attempts does not exceed cMaxWriteAttempts
        if (cWriteAgain.empty() ) cSuccess = true;
        else
        {
            cSuccess = false;

            // if the number of errors is greater than 100, give up
            if (cWriteAgain.size() < 100 && pWriteAttempts < cMaxWriteAttempts )
            {
                if (pReadback)  LOG (INFO) << BOLDRED <<  "(WRITE#"  << std::to_string (pWriteAttempts) << ") There were " << cWriteAgain.size() << " Readback Errors -trying again!" << RESET ;
                else LOG (INFO) << BOLDRED <<  "(WRITE#"  << std::to_string (pWriteAttempts) << ") There were " << cWriteAgain.size() << " CBC CMD acknowledge bits missing -trying again!" << RESET ;

                pWriteAttempts++;
                this->WriteCbcBlockReg ( cWriteAgain, pWriteAttempts, true);
            }
            else if ( pWriteAttempts >= cMaxWriteAttempts )
            {
                cSuccess = false;
                pWriteAttempts = 0 ;
            }
            else throw Exception ( "Too many CBC readback errors - no functional I2C communication. Check the Setup" );
        }


        return cSuccess;
    }

    bool Cbc3Fc7FWInterface::BCWriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback)
    {
        std::vector<uint32_t> cReplies;
        bool cSuccess = !WriteI2C ( pVecReg, cReplies, false, true );

        //just as above, I can check the replies - there will be NCbc * pVecReg.size() write replies and also read replies if I chose to enable readback
        //this needs to be adapted
        if (pReadback)
        {
            //TODO: actually, i just need to check the read write and the info bit in each reply - if all info bits are 0, this is as good as it gets, else collect the replies that faild for decoding - potentially no iterative retrying
            //TODO: maybe I can do something with readback here - think about it
            for (auto& cWord : cReplies)
            {
                //it was a write transaction!
                if ( ( (cWord >> 17) & 0x1) == 0)
                {
                    // infor bit is 0 which means that the transaction was acknowledged by the CBC
                    if ( ( (cWord >> 20) & 0x1) == 0)
                        cSuccess = true;
                    else cSuccess == false;
                }
                else
                    cSuccess = false;

                //LOG(INFO) << std::bitset<32>(cWord) ;
            }

            //cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cReplies.begin(), Cbc3Fc7FWInterface::cmd_reply_ack);
            pVecReg.clear();
            pVecReg = cReplies;

        }

        return cSuccess;
    }

    void Cbc3Fc7FWInterface::ReadCbcBlockReg (  std::vector<uint32_t>& pVecReg )
    {
        std::vector<uint32_t> cReplies;
        //it sounds weird, but ReadI2C is called inside writeI2c, therefore here I have to write and disable the readback. The actual read command is in the words of the vector, no broadcast, maybe I can get rid of it
        WriteI2C ( pVecReg, cReplies, false, false);
        pVecReg.clear();
        pVecReg = cReplies;
    }

    void Cbc3Fc7FWInterface::CbcFastReset()
    {
        WriteReg ( "cbc_system_ctrl.fast_command_manager.fast_signal_reset", 0x1 );
    }

    void Cbc3Fc7FWInterface::CbcHardReset()
    {
        WriteReg ("cbc_system_ctrl.global.cbc_hard_reset", 0x1);
    }

    void Cbc3Fc7FWInterface::CbcTestPulse()
    {
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_test_pulse_req", 0x1);
    }

    void Cbc3Fc7FWInterface::CbcTrigger()
    {
        WriteReg ("cbc_system_ctrl.fast_command_manager.fast_signal_trigger", 0x1);
    }

    void Cbc3Fc7FWInterface::FlashProm ( const std::string& strConfig, const char* pstrFile )
    {
        checkIfUploading();

        fpgaConfig->runUpload ( strConfig, pstrFile );
    }

    void Cbc3Fc7FWInterface::JumpToFpgaConfig ( const std::string& strConfig)
    {
        checkIfUploading();

        fpgaConfig->jumpToImage ( strConfig);
    }

    void Cbc3Fc7FWInterface::DownloadFpgaConfig ( const std::string& strConfig, const std::string& strDest)
    {
        checkIfUploading();
        fpgaConfig->runDownload ( strConfig, strDest.c_str() );
    }

    std::vector<std::string> Cbc3Fc7FWInterface::getFpgaConfigList()
    {
        checkIfUploading();
        return fpgaConfig->getFirmwareImageNames( );
    }

    void Cbc3Fc7FWInterface::DeleteFpgaConfig ( const std::string& strId)
    {
        checkIfUploading();
        fpgaConfig->deleteFirmwareImage ( strId);
    }

    void Cbc3Fc7FWInterface::checkIfUploading()
    {
        if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
            throw Exception ( "This board is uploading an FPGA configuration" );

        if ( !fpgaConfig )
            fpgaConfig = new CtaFpgaConfig ( this );
    }

    bool Cbc3Fc7FWInterface::cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2)
    {
        //TODO: cleanup
        //if ( (cWord1 & 0x0F00FFFF) != (cWord2 & 0x0F00FFFF) )
        //{
        //LOG (INFO)  << " ## " << std::bitset<32> (cWord1) << " ### Written: FMCId " <<  + ( (cWord1 >> 29) & 0xF) << " CbcId " << + ( (cWord1 >> 24) & 0xF) << " Read " << + ( (cWord1 >> 21) & 0x1) << " Write " << + ( (cWord1 >> 20) & 0x1) << " Page  " << + ( (cWord1 >> 16) & 0x1) << " Address " << + ( (cWord1 >> 8) & 0xFF) << " Value " << + ( (cWord1) & 0xFF);

        //LOG (INFO) << " ## " << std::bitset<32> (cWord2) << " ### Read:           CbcId " << + ( (cWord2 >> 24) & 0xF) << " Info " << + ( (cWord2 >> 20) & 0x1) << " Read? " << + ( (cWord2 >> 17) & 0x1) << " Page  " << + ( (cWord2 >> 16) & 0x1) << " Address " << + ( (cWord2 >> 8) & 0xFF) << " Value " << + ( (cWord2) & 0xFF)  ;
        //}

        //if the Register is FrontEndControl at p0 addr0, page is not defined and therefore I ignore it!
        if ( ( (cWord1 >> 16) & 0x1) == 0 && ( (cWord1 >> 8 ) & 0xFF) == 0) return ( (cWord1 & 0x0F00FFFF) == (cWord2 & 0x0F00FFFF) );
        else return ( (cWord1 & 0x0F01FFFF) == (cWord2 & 0x0F01FFFF) );
    }

    bool Cbc3Fc7FWInterface::cmd_reply_ack (const uint32_t& cWord1, const
                                            uint32_t& cWord2)
    {
        // if Info (>>20) is 0 and  it was a write transaction (>>17 == 0) and
        // the CBC id matches it is false
        if ( ( (cWord2 >> 20) & 0x1) == 0 && ( (cWord2 >> 17) & 0x1 ) == 0 &&
                (cWord1 & 0x0F000000) == (cWord2 & 0x0F000000) ) return true;
        else
            return false;
    }


}
