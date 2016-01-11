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

namespace Ph2_HwInterface
{

	GlibFWInterface::GlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId ) :
		BeBoardFWInterface( puHalConfigFileName, pBoardId ),
		fpgaConfig( nullptr ),
		fData( nullptr )
	{}


	GlibFWInterface::GlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId, FileHandler* pFileHandler ) :
		BeBoardFWInterface( puHalConfigFileName, pBoardId ),
		fpgaConfig( nullptr ),
		fData( nullptr ),

		fFileHandler( pFileHandler )
	{
		if ( fFileHandler == nullptr ) fSaveToFile = false;
		else fSaveToFile = true;
	}

	GlibFWInterface::GlibFWInterface( const char* pId, const char* pUri, const char* pAddressTable ) :
		BeBoardFWInterface( pId, pUri, pAddressTable ),
		fpgaConfig( nullptr ),
		fData( nullptr )
	{}


	GlibFWInterface::GlibFWInterface( const char* pId, const char* pUri, const char* pAddressTable, FileHandler* pFileHandler ) :
		BeBoardFWInterface( pId, pUri, pAddressTable ),
		fpgaConfig( nullptr ),
		fData( nullptr ),

		fFileHandler( pFileHandler )
	{
		if ( fFileHandler == nullptr ) fSaveToFile = false;
		else fSaveToFile = true;
	}


	void GlibFWInterface::ConfigureBoard( const BeBoard* pBoard )
	{
		std::vector< std::pair<std::string, uint32_t> > cVecReg;

		std::chrono::milliseconds cPause( 200 );

		//Primary Configuration
		cVecReg.push_back( {"pc_commands.PC_config_ok", 1} );
		cVecReg.push_back( {"pc_commands.SRAM1_end_readout", 0} );
		cVecReg.push_back( {"pc_commands.SRAM2_end_readout", 0} );
		cVecReg.push_back( {"ctrl_sram.sram1_user_logic", 1} );
		cVecReg.push_back( {"ctrl_sram.sram2_user_logic", 1} );

		// iterate the BeBoardRegMap to get the user configuration
		BeBoardRegMap cGlibRegMap = pBoard->getBeBoardRegMap();
		for ( auto const& it : cGlibRegMap )
			cVecReg.push_back( {it.first, it.second} );

		cVecReg.push_back( {"pc_commands.SPURIOUS_FRAME", 0} );
		cVecReg.push_back( {"pc_commands2.force_BG0_start", 0} );
		cVecReg.push_back( {"cbc_acquisition.CBC_TRIGGER_ONE_SHOT", 0} );
		cVecReg.push_back( {"pc_commands.PC_config_ok", 0} );

		WriteStackReg( cVecReg );
		cVecReg.clear();
	}


	void GlibFWInterface::Start()
	{
		std::vector< std::pair<std::string, uint32_t> > cVecReg;

		//Starting the DAQ
		cVecReg.push_back( {"break_trigger", 0} );
		cVecReg.push_back( {"pc_commands.PC_config_ok", 1} );
		cVecReg.push_back( {"pc_commands2.force_BG0_start", 1} );

		WriteStackReg( cVecReg );
		cVecReg.clear();

		// Since the Number of  Packets is a FW register, it should be read from the Settings Table which is one less than is actually read
		cNPackets = ReadReg( "pc_commands.CBC_DATA_PACKET_NUMBER" ) + 1 ;

		//Wait for start acknowledge
		uhal::ValWord<uint32_t> cVal;
		std::chrono::milliseconds cWait( 100 );
		do
		{
			cVal = ReadReg( "status_flags.CMD_START_VALID" );
			if ( cVal == 0 )
				std::this_thread::sleep_for( cWait );
		}
		while ( cVal == 0 );
	}

	void GlibFWInterface::Stop( uint32_t pNthAcq )
	{
		std::vector< std::pair<std::string, uint32_t> > cVecReg;

		uhal::ValWord<uint32_t> cVal;

		//Select SRAM
		SelectDaqSRAM( pNthAcq );
		//Stop the DAQ
		cVecReg.push_back( {"break_trigger", 1} );
		cVecReg.push_back( {"pc_commands.PC_config_ok", 0} );
		cVecReg.push_back( {"pc_commands2.force_BG0_start", 0} );

		WriteStackReg( cVecReg );
		cVecReg.clear();

		std::chrono::milliseconds cWait( 100 );

		//Wait for the selected SRAM to be full then empty it
		do
		{
			cVal = ReadReg( fStrFull );
			if ( cVal == 1 )
				std::this_thread::sleep_for( cWait );
		}
		while ( cVal == 1 );

		WriteReg( fStrReadout, 0 );
		fNTotalAcq++;
	}


	void GlibFWInterface::Pause()
	{
		WriteReg( "break_trigger", 1 );
	}


	void GlibFWInterface::Resume()
	{
		WriteReg( "break_trigger", 0 );
	}

	uint32_t GlibFWInterface::ReadData( BeBoard* pBoard, unsigned int pNthAcq, bool pBreakTrigger )
	{
		//Readout settings
		std::chrono::milliseconds cWait( 1 );

		uhal::ValWord<uint32_t> cVal;

		if ( pBoard )
			cBlockSize = computeBlockSize( pBoard );
		//FIFO goes to write_data state
		//Select SRAM
		SelectDaqSRAM( pNthAcq );

		//Wait for the SRAM full condition.
		cVal = ReadReg( fStrFull );
		do
		{
			cVal = ReadReg( fStrFull );
			if ( cVal == 0 )
				std::this_thread::sleep_for( cWait );
		}
		while ( cVal == 0 );

		//break trigger
		if ( pBreakTrigger ) WriteReg( "break_trigger", 1 );

		//Set read mode to SRAM
		WriteReg( fStrSramUserLogic, 0 );

		//Read SRAM
		std::vector<uint32_t> cData =  ReadBlockRegValue( fStrSram, cBlockSize );

		WriteReg( fStrSramUserLogic, 1 );
		WriteReg( fStrReadout, 1 );

		//Wait for the non SRAM full condition starts,
		do
		{
			cVal = ReadReg( fStrFull );
			if ( cVal == 1 )
				std::this_thread::sleep_for( cWait );
		}
		while ( cVal == 1 );

		//Wait for the non SRAM full condition ends.

		WriteReg( fStrReadout, 0 );
		if ( pBreakTrigger ) WriteReg( "break_trigger", 0 );

		// just creates a new Data object, setting the pointers and getting the correct sizes happens in Set()
		if ( fData ) delete fData;

		fData = new Data();

		// set the vector<uint32_t> as event buffer and let him know how many packets it contains
		fData->Set( pBoard, cData , cNPackets, true );
		if ( fSaveToFile ){
			fFileHandler->set( cData );
			fFileHandler->writeFile();
		}
		return cNPackets;
	}

	/** compute the block size according to the number of CBC's on this board
	 * this will have to change with a more generic FW */
	uint32_t GlibFWInterface::computeBlockSize( BeBoard* pBoard )
	{
		//use a counting visitor to find out the number of CBCs
		struct CbcCounter : public HwDescriptionVisitor
		{
			uint32_t fNCbc = 0;

			void visit( Cbc& pCbc ) {
				fNCbc++;
			}
			uint32_t getNCbc() {
				if ( fNCbc == 2 )
					// since the 2 CBC FW outputs data for 4 CBCs (beamtest heritage, might have to change in the future)
					return 2 * fNCbc;
				else return fNCbc;
			}
		};

		CbcCounter cCounter;
		pBoard->accept( cCounter );
		if ( pBoard->getNCbcDataSize() != 0 ) return cNPackets * ( pBoard->getNCbcDataSize() * CBC_EVENT_SIZE_32 + EVENT_HEADER_TDC_SIZE_32 );
		else return cNPackets * ( cCounter.getNCbc() * CBC_EVENT_SIZE_32 + EVENT_HEADER_TDC_SIZE_32 ); // in 32 bit words
	}

	std::vector<uint32_t> GlibFWInterface::ReadBlockRegValue( const std::string& pRegNode, const uint32_t& pBlocksize )
	{
		uhal::ValVector<uint32_t> valBlock = ReadBlockReg( pRegNode, pBlocksize );
		std::vector<uint32_t> vBlock = valBlock.value();

		// To avoid the IPBUS bug
		// need to convert uHal::ValVector to vector<uint32_t> so we can replace the 256th word
		if ( pBlocksize > 255 )
		{
			std::string fSram_256 = pRegNode + "_256";
			uhal::ValWord<uint32_t> cWord = ReadReg( fSram_256 );
			vBlock[255] = cWord.value();
		}
		return vBlock;
	}

	bool GlibFWInterface::WriteBlockReg( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
	{
		bool cWriteCorr = RegManager::WriteBlockReg( pRegNode, pValues );

		if ( pValues.size() > 255 )
			WriteReg( pRegNode + "_256", pValues[255] );
		return cWriteCorr;
	}

	void GlibFWInterface::SelectDaqSRAM( uint32_t pNthAcq )
	{
		fStrSram  = ( ( pNthAcq % 2 + 1 ) == 1 ? "sram1" : "sram2" );
		fStrSramUserLogic = ( ( pNthAcq % 2 + 1 ) == 1 ? "ctrl_sram.sram1_user_logic" : "ctrl_sram.sram2_user_logic" );
		fStrFull = ( ( pNthAcq % 2 + 1 ) == 1 ? "flags.SRAM1_full" : "flags.SRAM2_full" );
		fStrReadout = ( ( pNthAcq % 2 + 1 ) == 1 ? "pc_commands.SRAM1_end_readout" : "pc_commands.SRAM2_end_readout" );
	}



	//Methods for Cbc's:

	void GlibFWInterface::StartThread( BeBoard* pBoard, uint32_t uNbAcq, HwInterfaceVisitor* visitor )
	{
		if ( runningAcquisition ) return;

		runningAcquisition = true;
		numAcq = 0;
		nbMaxAcq = uNbAcq;

		thrAcq = boost::thread( &Ph2_HwInterface::GlibFWInterface::threadAcquisitionLoop, this, pBoard, visitor );
	}

	void GlibFWInterface::threadAcquisitionLoop( BeBoard* pBoard, HwInterfaceVisitor* visitor )
	{
		Start( );
		cBlockSize = computeBlockSize( pBoard );
		while ( runningAcquisition && ( nbMaxAcq == 0 || numAcq < nbMaxAcq ) )
		{
			ReadData( nullptr, numAcq, true );
			for ( const Ph2_HwInterface::Event* cEvent = GetNextEvent( pBoard ); cEvent; cEvent = GetNextEvent( pBoard ) )
				visitor->visit( *cEvent );

			if ( runningAcquisition )
				numAcq++;

		}
		Stop( numAcq );
		runningAcquisition = false;
	};

	bool GlibFWInterface::I2cCmdAckWait( uint32_t pAckVal, uint8_t pNcount )
	{
		unsigned int cWait( 100 );
		if ( pAckVal )
			cWait = pNcount * 500;
		usleep( cWait );

		uhal::ValWord<uint32_t> cVal;
		uint32_t cLoop = 0;
		do
		{
			cVal = ReadReg( "cbc_i2c_cmd_ack" );
			if ( cVal != pAckVal )
				usleep( cWait );
			else return true;
		}
		while ( cVal != pAckVal && ++cLoop < 70 );
		return false;
	}

	void GlibFWInterface::WriteI2C( std::vector<uint32_t>& pVecReq, bool pWrite )
	{
		pVecReq.push_back( 0xFFFFFFFF );

		std::vector< std::pair<std::string, uint32_t> > cVecReg;

		WriteReg( "ctrl_sram.sram1_user_logic", 0 );
		WriteBlockReg( "sram1", pVecReq );

		cVecReg.push_back( {"ctrl_sram.sram1_user_logic", 1} );
		cVecReg.push_back( {"cbc_i2c_cmd_rq", pWrite ? 3 : 1} );
		WriteStackReg( cVecReg );

		if ( I2cCmdAckWait( ( uint32_t )1, pVecReq.size() ) == 0 )
			throw Exception( "CbcInterface: I2cCmdAckWait 1 failed." );

		WriteReg( "cbc_i2c_cmd_rq", 0 );

		if ( I2cCmdAckWait( ( uint32_t )0, pVecReq.size() ) == 0 )
			throw Exception( "CbcInterface: I2cCmdAckWait 0 failed." );
	}

	void GlibFWInterface::ReadI2C( std::vector<uint32_t>& pVecReq )
	{
		WriteReg( "ctrl_sram.sram1_user_logic", 0 );
		pVecReq = ReadBlockRegValue( "sram1", pVecReq.size() );
		std::vector< std::pair<std::string, uint32_t> > cVecReg;
		cVecReg.push_back( {"ctrl_sram.sram1_user_logic", 1} );
		cVecReg.push_back( {"cbc_i2c_cmd_rq", 0} );
		WriteStackReg( cVecReg );
	}


	void GlibFWInterface::WriteCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq )
	{
		try
		{
			WriteI2C( pVecReq, true );
		}
		catch ( Exception& except )
		{
			throw except;
		}
	}

	void GlibFWInterface::ReadCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq )
	{
		try
		{
			WriteI2C( pVecReq, false );
		}
		catch ( Exception& e )
		{
			throw e;
		}
		ReadI2C( pVecReq );
	}

	void GlibFWInterface::FlashProm( const std::string& strConfig, const char* pstrFile )
	{
		if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
			throw Exception( "This board is already uploading an FPGA configuration" );

		if ( !fpgaConfig )
			fpgaConfig = new GlibFpgaConfig( this );

		fpgaConfig->runUpload( strConfig, pstrFile );
	}

	void GlibFWInterface::JumpToFpgaConfig( const std::string& strConfig )
	{
		if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
			throw Exception( "This board is uploading an FPGA configuration" );

		if ( !fpgaConfig )
			fpgaConfig = new GlibFpgaConfig( this );

		fpgaConfig->jumpToImage( strConfig );
	}

}
