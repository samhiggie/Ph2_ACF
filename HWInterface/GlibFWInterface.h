/*!

        \file                           GlibFWInterface.h
        \brief                          GlibFWInterface init/config of the Glib and its Cbc's
        \author                         Lorenzo BIDEGAIN, Nicolas PIERRE
        \version            1.0
        \date                           28/07/14
        Support :                       mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#ifndef __GLIBFWINTERFACE_H__
#define __GLIBFWINTERFACE_H__

#include <string>
#include <map>
#include <vector>
#include <limits.h>
#include <stdint.h>
#include "BeBoardFWInterface.h"
#include "../HWDescription/Module.h"
#include "../Utils/Visitor.h"



using namespace Ph2_HwDescription;

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface
{
	class FpgaConfig;

	/*!
	 * \class GlibFWInterface
	 * \brief init/config of the Glib and its Cbc's
	 */
	class GlibFWInterface : public BeBoardFWInterface
	{

	  private:
		Data* fData; /*!< Data read storage*/

		struct timeval fStartVeto;
		std::string fStrSram, fStrSramUserLogic, fStrFull, fStrReadout, fStrOtherSram, fStrOtherSramUserLogic;
		std::string fCbcStubLat, fCbcI2CCmdAck, fCbcI2CCmdRq, fCbcHardReset, fCbcFastReset;
		FpgaConfig* fpgaConfig;
		FileHandler* fFileHandler ;


	  private:
		/*!
		 * \brief SRAM selection for DAQ
		 * \param pNthAcq : actual number of acquisitions
		 */
		void SelectDaqSRAM( uint32_t pNthAcq );

	  public:
		/*!
		 * \brief Constructor of the GlibFWInterface class
		 * \param puHalConfigFileName : path of the uHal Config File
		 * \param pBoardId
		 */
		GlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId );
		/*!
		 * \brief Constructor of the GlibFWInterface class
		 * \param puHalConfigFileName : path of the uHal Config File
		 * \param pBoardId
		 * \param pFileHandler : pointer to file handler for saving Raw Data
		 */
		GlibFWInterface( const char* puHalConfigFileName, uint32_t pBoardId, FileHandler* pFileHandler );

		/*!
		 * \brief Destructor of the GlibFWInterface class
		 */
		~GlibFWInterface() {
			delete fData;
		}
		/*!
		 * \brief Configure the board with its Config File
		 * \param pBoard
		 */
		void ConfigureBoard( const BeBoard* pBoard ) override;
		/*!
		 * \brief Detect the right FE Id to write the right registers (not working with the latest Firmware)
		 */
		void SelectFEId();
		/*!
		 * \brief Start a DAQ
		 */
		void Start() override;
		/*!
		 * \brief Stop a DAQ
		 * \param pNthAcq : actual number of acquisitions
		 */
		void Stop( uint32_t pNthAcq ) override;
		/*!
		 * \brief Pause a DAQ
		 */
		void Pause() override;
		/*!
		 * \brief Unpause a DAQ
		 */
		void Resume() override;
		/*!
		 * \brief Read data from DAQ
		 * \param pNthAcq : actual number of acquisitions
		 * \param pBreakTrigger : if true, enable the break trigger
		 * \return cNPackets: the number of packets read
		 */
		uint32_t ReadData( BeBoard* pBoard, uint32_t pNthAcq, bool pBreakTrigger ) override;
		/*!
		 * \brief Get next event from data buffer
		 * \return Next event
		 */
		const Event* GetNextEvent( const BeBoard* pBoard ) const override {
			return fData->GetNextEvent( pBoard );
		}
		const Event* GetEvent( const BeBoard* pBoard, int i ) const override {
			return fData->GetEvent( pBoard, i );
		}
		const std::vector<Event*>& GetEvents( const BeBoard* pBoard ) const override {
			return fData->GetEvents( pBoard );
		}
		/*! \brief Read a block of a given size
		 * \param pRegNode Param Node name
		 * \param pBlocksize Number of 32-bit words to read
		 * \return Vector of validated 32-bit values
		 */
		std::vector<uint32_t> ReadBlockRegValue( const std::string& pRegNode, const uint32_t& pBlocksize ) override;

		bool WriteBlockReg( const std::string& pRegNode, const std::vector< uint32_t >& pValues ) override;

		void StartThread( BeBoard* pBoard, uint32_t uNbAcq, HwInterfaceVisitor* visitor ) override;
		//Methods for the Cbc's:

	  private:

		//I2C Methods

		/*!
		 * \brief Wait for the I2C command acknowledgement
		 * \param pAckVal : Expected status of acknowledgement, 1/0 -> true/false
		 * \param pNcount : Number of registers at stake
		 * \return boolean confirming the acknowledgement
		 */
		bool I2cCmdAckWait( uint32_t pAckVal, uint8_t pNcount = 1 );
		/*!
		 * \brief Send request to r/w blocks via I2C
		 * \param pVecReq : Block of words to send
		 * \param pWrite : 1/0 -> Write/Read
		 */
		void SendBlockCbcI2cRequest( std::vector<uint32_t>& pVecReq, bool pWrite );
		/*!
		 * \brief Read blocks from SRAM via I2C
		 * \param pVecReq : Vector to stack the read words
		 */
		void ReadI2cBlockValuesInSRAM( std::vector<uint32_t>& pVecReq );
		/*!
		 * \brief Enable I2C communications
		 * \param pEnable : 1/0 -> Enable/Disable
		 */
		void EnableI2c( bool pEnable );

		void SelectFeSRAM( uint32_t pFe );

		/*! Compute the size of an acquisition data block
		 * \return Number of 32-bit words to be read at each iteration */
		uint32_t computeBlockSize( BeBoard* pBoard );


	  public:

		//r/w the Cbc registers
		/*!
		 * \brief Read register blocks of a Cbc
		 * \param pFeId : FrontEnd to work with
		 * \param pVecReq : Vector to stack the read words
		 */
		void WriteCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq );
		/*! \brief Read register blocks of a Cbc
		 * \param pFeId : FrontEnd to work with
		 * \param pVecReq : Vector to stack the read words
		 */
		void ReadCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq );
		/*! \brief Upload a firmware (FPGA configuration) from a file in MCS format into a given configuration
		 * \param numConfig FPGA configuration number (1 or 2)
		 * \param pstrFile path to MCS file
		 */
		void FlashProm( uint16_t numConfig, const char* pstrFile );
		/*! \brief Is the FPGA being configured ?
		 * \return FPGA configuring process or NULL if configuration occurs */
		const FpgaConfig* getConfiguringFpga() {
			return fpgaConfig;
		}
		void threadAcquisitionLoop( BeBoard* pBoard, HwInterfaceVisitor* visitor );

	};
}

#endif
