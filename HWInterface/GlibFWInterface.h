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
		struct timeval fStartVeto;
		std::string fStrSram, fStrSramUserLogic, fStrFull, fStrReadout, fStrOtherSram, fStrOtherSramUserLogic;
		std::string fCbcStubLat, fCbcI2CCmdAck, fCbcI2CCmdRq, fCbcHardReset, fCbcFastReset;
		uint32_t cBlockSize, cNPackets;
		FpgaConfig* fpgaConfig;

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
		 * \brief Destructor of the GlibFWInterface class
		 */
		~GlibFWInterface() {
		}

		/*!
		 * \brief Configure the board with its Config File
		 * \param pBoard
		 */
		void ConfigureBoard( const BeBoard* pBoard );
		/*!
		 * \brief Detect the right FE Id to write the right registers (not working with the latest Firmware)
		 */
		void SelectFEId();
		/*!
		 * \brief Start a DAQ
		 */
		void Start();
		/*!
		 * \brief Stop a DAQ
		 * \param pNthAcq : actual number of acquisitions
		 */
		void Stop( uint32_t pNthAcq );
		/*!
		 * \brief Pause a DAQ
		 */
		void Pause();
		/*!
		 * \brief Unpause a DAQ
		 */
		void Resume();
		/*!
		 * \brief Read data from DAQ
		 * \param pNthAcq : actual number of acquisitions
		 * \param pBreakTrigger : if true, enable the break trigger
		 */
		void ReadData( BeBoard* pBoard, uint32_t pNthAcq, bool pBreakTrigger );
		/*!
		 * \brief Get next event from data buffer
		 * \return Next event
		 */
		const Event* GetNextEvent( const BeBoard* pBoard );
		/*!
		* \brief Get the data buffer
		* \param pBufSize : recovers the data buffer size
		* \return Data buffer
		*/
		const char* GetBuffer( uint32_t& pBufSize ) const;


		std::vector<uint32_t> ReadBlockRegValue( const std::string& pRegNode, const uint32_t& pBlocksize );

		bool WriteBlockReg( const std::string& pRegNode, const std::vector< uint32_t >& pValues );

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

	  public:

		//r/w the Cbc registers
		/*!
		 * \brief Read register blocks of a Cbc
		 * \param pFeId : FrontEnd to work with
		 * \param pVecReq : Vector to stack the read words
		 */
		void WriteCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq );
		/* \brief Read register blocks of a Cbc
		 * \param pFeId : FrontEnd to work with
		 * \param pVecReq : Vector to stack the read words
		 */
		void ReadCbcBlockReg( uint8_t pFeId, std::vector<uint32_t>& pVecReq );

		void FlashProm(uint16_t numConfig, const char* pstrFile);
		const FpgaConfig* getConfiguringFpga(){ return fpgaConfig; }

	};
}

#endif
