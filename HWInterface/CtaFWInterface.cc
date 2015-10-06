/*

        FileName :                    CtaFWInterface.h
        Content :                     CtaFWInterface init/config of the CTA and its Cbc's
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            28/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#include <time.h>
#include <chrono>
#include <uhal/uhal.hpp>
#include "CtaFWInterface.h"
#include "CtaFpgaConfig.h"

namespace Ph2_HwInterface
{

	CtaFWInterface::CtaFWInterface( const char* puHalConfigFileName, uint32_t pBoardId ) :
		BeBoardFWInterface( puHalConfigFileName, pBoardId ), 
		fpgaConfig(nullptr),
		fData( nullptr )
	{}


	CtaFWInterface::CtaFWInterface( const char* puHalConfigFileName, uint32_t pBoardId, FileHandler* pFileHandler ) :
		BeBoardFWInterface( puHalConfigFileName, pBoardId ),
		fpgaConfig(nullptr),
		fData( nullptr ),

		fFileHandler( pFileHandler )
	{
		if ( fFileHandler == nullptr ) fSaveToFile = false;
		else fSaveToFile = true;
	}


	void CtaFWInterface::ConfigureBoard( const BeBoard* pBoard )
	{

		//We may here switch in the future with the StackReg method of the RegManager
		//when the timeout thing will be implemented in a transparent and pretty way

		std::vector< std::pair<std::string, uint32_t> > cVecReg;
		std::pair<std::string, uint32_t> cPairReg;

		std::chrono::milliseconds cPause( 200 );

		//Primary Configuration
		cPairReg.first = PC_CONFIG_OK;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM1_END_READOUT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM2_END_READOUT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM1_USR_LOGIC;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM2_USR_LOGIC;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );

		cVecReg.clear();

		std::this_thread::sleep_for( cPause );


		BeBoardRegMap cRegMap = pBoard->getBeBoardRegMap();
		for ( auto const& it : cRegMap )
		{
			cPairReg.first = it.first;
			cPairReg.second = it.second;
			cVecReg.push_back( cPairReg );
		}

		WriteStackReg( cVecReg );

		cVecReg.clear();

		cPairReg.first = SPURIOUS_FRAME;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = FORCE_BG0_START;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = CBC_TRIGGER_1SHOT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		// cPairReg.first = BREAK_TRIGGER;
		// cPairReg.second = 1;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );

		cVecReg.clear();


		cPairReg.first = PC_CONFIG_OK;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM1_END_READOUT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM2_END_READOUT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM1_USR_LOGIC;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );
		cPairReg.first = SRAM2_USR_LOGIC;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );

		cVecReg.clear();

		std::this_thread::sleep_for( cPause );

		cPairReg.first = SPURIOUS_FRAME;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = FORCE_BG0_START;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = CBC_TRIGGER_1SHOT;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		// cPairReg.first = BREAK_TRIGGER;
		// cPairReg.second = 1;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );

		cVecReg.clear();

		std::this_thread::sleep_for( cPause * 3 );

	}


	void CtaFWInterface::SelectFEId()
	{
		if ( static_cast<uint32_t>( ReadReg( HYBRID_TYPE ) ) == 8 )
		{
			fCbcStubLat  = ( static_cast<uint32_t>( ReadReg( FMC1_PRESENT ) ) ? CBC_STUB_LATENCY_FE1 : CBC_STUB_LATENCY_FE2 );
			fCbcI2CCmdAck = ( static_cast<uint32_t>( ReadReg( FMC1_PRESENT ) ) ? CBC_I2C_CMD_ACK_FE1 : CBC_I2C_CMD_ACK_FE2 );
			fCbcI2CCmdRq = ( static_cast<uint32_t>( ReadReg( FMC1_PRESENT ) ) ? CBC_I2C_CMD_RQ_FE1 : CBC_I2C_CMD_RQ_FE2 );
			fCbcHardReset = ( static_cast<uint32_t>( ReadReg( FMC1_PRESENT ) ) ? CBC_HARD_RESET_FE1 : CBC_HARD_RESET_FE2 );
			fCbcFastReset = ( static_cast<uint32_t>( ReadReg( FMC1_PRESENT ) ) ? CBC_FAST_RESET_FE1 : CBC_FAST_RESET_FE2 );
		}
		else
		{
			fCbcStubLat  = CBC_STUB_LATENCY;
			fCbcI2CCmdAck =  CBC_I2C_CMD_ACK;
			fCbcI2CCmdRq = CBC_I2C_CMD_RQ;
			fCbcHardReset = CBC_HARD_RESET;
			fCbcFastReset = CBC_FAST_RESET;
		}
	}

	void CtaFWInterface::Start()
	{
		std::vector< std::pair<std::string, uint32_t> > cVecReg;
		std::pair<std::string, uint32_t> cPairReg;

		//Starting the DAQ

		cPairReg.first = BREAK_TRIGGER;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = PC_CONFIG_OK;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );
		cPairReg.first = FORCE_BG0_START;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );

		WriteStackReg( cVecReg );

		cVecReg.clear();

		// Since the Number of  Packets is a FW register, it should be read from the Settings Table which is one less than is actually read
		cNPackets = ReadReg( CBC_PACKET_NB ) + 1 ;

		//Wait for start acknowledge
		uhal::ValWord<uint32_t> cVal;
		std::chrono::milliseconds cWait( 100 );
		do
		{
			cVal = ReadReg( CMD_START_VALID );

			if ( cVal == 0 )
				std::this_thread::sleep_for( cWait );

		}
		while ( cVal == 0 );

	}

	void CtaFWInterface::Stop( uint32_t pNthAcq )
	{

		std::vector< std::pair<std::string, uint32_t> > cVecReg;
		std::pair<std::string, uint32_t> cPairReg;

		uhal::ValWord<uint32_t> cVal;

		//Select SRAM
		SelectDaqSRAM( pNthAcq );

		//Stop the DAQ
		cPairReg.first = BREAK_TRIGGER;
		cPairReg.second = 1;
		cVecReg.push_back( cPairReg );
		cPairReg.first = PC_CONFIG_OK;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );
		cPairReg.first = FORCE_BG0_START;
		cPairReg.second = 0;
		cVecReg.push_back( cPairReg );

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


	void CtaFWInterface::Pause()
	{

		WriteReg( BREAK_TRIGGER, 1 );

	}


	void CtaFWInterface::Resume()
	{

		WriteReg( BREAK_TRIGGER, 0 );
	}

	uint32_t CtaFWInterface::ReadData( BeBoard* pBoard, unsigned int pNthAcq, bool pBreakTrigger )
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
		if ( pBreakTrigger ) WriteReg( BREAK_TRIGGER, 1 );

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
		if ( pBreakTrigger ) WriteReg( BREAK_TRIGGER, 0 );

		// just creates a new Data object, setting the pointers and getting the correct sizes happens in Set()
		if ( fData ) delete fData;

		fData = new Data();

		// set the vector<uint32_t> as event buffer and let him know how many packets it contains
		fData->Set( pBoard, cData , cNPackets, true );
		if ( fSaveToFile )
			fFileHandler->set( cData );
		return cNPackets;
	}
	/** compute the block size according to the number of CBC's on this board
	 * this will have to change with a more generic FW */
	uint32_t CtaFWInterface::computeBlockSize( BeBoard* pBoard )
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
		return cNPackets * ( cCounter.getNCbc() * CBC_EVENT_SIZE_32 + EVENT_HEADER_TDC_SIZE_32 ); // in 32 bit words
	}

	std::vector<uint32_t> CtaFWInterface::ReadBlockRegValue( const std::string& pRegNode, const uint32_t& pBlocksize )
	{
		uhal::ValVector<uint32_t> valBlock = ReadBlockReg( pRegNode, pBlocksize );
		return valBlock.value();
	}

	void CtaFWInterface::SelectDaqSRAM( uint32_t pNthAcq )
	{
		fStrSram  = ( ( pNthAcq % 2 + 1 ) == 1 ? SRAM1 : SRAM2 );
		fStrSramUserLogic = ( ( pNthAcq % 2 + 1 ) == 1 ? SRAM1_USR_LOGIC : SRAM2_USR_LOGIC );
		fStrFull = ( ( pNthAcq % 2 + 1 ) == 1 ? SRAM1_FULL : SRAM2_FULL );
		fStrReadout = ( ( pNthAcq % 2 + 1 ) == 1 ? SRAM1_END_READOUT : SRAM2_END_READOUT );
	}



	//Methods for Cbc's:

	void CtaFWInterface::SelectFeSRAM( uint32_t pFe )
	{
		pFe = 0;
		fStrSram = ( pFe ? SRAM2 : SRAM1 );
		fStrOtherSram = ( pFe ? SRAM1 : SRAM2 );
		fStrSramUserLogic = ( pFe ? SRAM2_USR_LOGIC : SRAM1_USR_LOGIC );
		fStrOtherSramUserLogic = ( pFe ? SRAM2_USR_LOGIC : SRAM1_USR_LOGIC );
	}


	void CtaFWInterface::StartThread( BeBoard* pBoard, uint32_t uNbAcq, HwInterfaceVisitor* visitor )
	{
		if ( runningAcquisition ) return;

		runningAcquisition = true;
		numAcq = 0;
		nbMaxAcq = uNbAcq;

		thrAcq = boost::thread( &Ph2_HwInterface::CtaFWInterface::threadAcquisitionLoop, this, pBoard, visitor );
	}

	void CtaFWInterface::threadAcquisitionLoop( BeBoard* pBoard, HwInterfaceVisitor* visitor )
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

	bool CtaFWInterface::I2cCmdAckWait( uint32_t pAckVal, uint8_t pNcount )
	{
		unsigned int cWait( 100 );

		if ( pAckVal )
			cWait = pNcount * 500;


		usleep( cWait );

		uhal::ValWord<uint32_t> cVal;
		uint32_t cLoop = 0;

		do
		{
			cVal = ReadReg( CBC_I2C_CMD_ACK );

			if ( cVal != pAckVal )
			{
				// std::cout << "Waiting for the I2c command acknowledge to be " << pAckVal << " for " << pNcount << " registers." << std::endl;
				usleep( cWait );
			}

		}
		while ( cVal != pAckVal && ++cLoop < MAX_NB_LOOP );

		if ( cLoop >= MAX_NB_LOOP )
		{
			std::cout << "Warning: time out in I2C acknowledge loop (" << pAckVal << ")" << std::endl;
			return false;
		}

		return true;
	}

	void CtaFWInterface::SendBlockCbcI2cRequest( std::vector<uint32_t>& pVecReq, bool pWrite )
	{
		WriteReg( fStrSramUserLogic, 1 );

		pVecReq.push_back( 0xFFFFFFFF );

		WriteReg( fStrSramUserLogic, 0 );

		WriteBlockReg( fStrSram, pVecReq );
		WriteReg( fStrOtherSram, 0xFFFFFFFF );

		WriteReg( fStrSramUserLogic, 1 );

		WriteReg( CBC_HARD_RESET, 0 );

		//r/w request
		WriteReg( CBC_I2C_CMD_RQ, pWrite ? 3 : 1 );
		// WriteReg( CBC_I2C_CMD_RQ, 1 );

		pVecReq.pop_back();

		if ( I2cCmdAckWait( ( uint32_t )1, pVecReq.size() ) == 0 )
			throw Exception( "CbcInterface: I2cCmdAckWait 1 failed." );

		WriteReg( CBC_I2C_CMD_RQ, 0 );

		if ( I2cCmdAckWait( ( uint32_t )0, pVecReq.size() ) == 0 )
			throw Exception( "CbcInterface: I2cCmdAckWait 0 failed." );

	}

	void CtaFWInterface::ReadI2cBlockValuesInSRAM( std::vector<uint32_t>& pVecReq )
	{

		WriteReg( fStrSramUserLogic, 0 );

		pVecReq = ReadBlockRegValue( fStrSram, pVecReq.size() );
		WriteReg( fStrSramUserLogic, 1 );
		WriteReg( CBC_I2C_CMD_RQ, 0 );


	}


	void CtaFWInterface::EnableI2c( bool pEnable )
	{
		uint32_t cValue = I2C_CTRL_ENABLE;

		if ( !pEnable )
			cValue = I2C_CTRL_DISABLE;

		WriteReg( I2C_SETTINGS, cValue );

		if ( pEnable )
			usleep( 100000 );
	}

	void CtaFWInterface::WriteCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq )
	{
		SelectFeSRAM( pFeId );
		EnableI2c( 1 );

		try
		{
			SendBlockCbcI2cRequest( pVecReq, true );
		}

		catch ( Exception& except )
		{
			throw except;
		}

		EnableI2c( 0 );
	}

	void CtaFWInterface::ReadCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq )
	{
		SelectFeSRAM( pFeId );
		EnableI2c( 1 );

		try
		{
			SendBlockCbcI2cRequest( pVecReq, false );
		}

		catch ( Exception& e )
		{
			throw e;
		}

		ReadI2cBlockValuesInSRAM( pVecReq );

		EnableI2c( 0 );
	}

	void CtaFWInterface::FlashProm( const std::string& strConfig, const char* pstrFile )
	{
		checkIfUploading();

		fpgaConfig->runUpload( strConfig, pstrFile );
	}

	void CtaFWInterface::JumpToFpgaConfig( const std::string& strConfig)
	{
		checkIfUploading();

		fpgaConfig->jumpToImage( strConfig);
	}

	std::vector<std::string> CtaFWInterface::getFpgaConfigList()
	{
		checkIfUploading();
		return fpgaConfig->getFirmwareImageNames( );
	}

	void CtaFWInterface::DeleteFpgaConfig( const std::string& strId)
	{
		checkIfUploading();
		fpgaConfig->deleteFirmwareImage( strId);
	}

	void CtaFWInterface::checkIfUploading(){
		if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
			throw Exception( "This board is uploading an FPGA configuration" );

		if ( !fpgaConfig )
			fpgaConfig = new CtaFpgaConfig( this );
	}
}
