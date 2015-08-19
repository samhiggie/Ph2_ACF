/*!  
   \file 		FpgaConfig.h
   \brief 		FPGA configuration by uploading the firware in MCS file format
   \version             1.0
   \author	 	Christian Bonnin
   \date		02/03/2015
   Support : 		mail to : christian.bonnin@iphc.cnrs.fr
*/
#ifndef _FPGACONFIG_H_
#define _FPGACONFIG_H_

#include "HWDescription/BeBoard.h"
using namespace Ph2_HwDescription;
namespace Ph2_HwInterface
{
	class BeBoardFWInterface;
/*!
 * \brief Upload MCS files into Flash EPROM as FPGA configuration
 * @author cbonnin
 */
	class FpgaConfig{
		public:
/*! \brief Constructor from a BeBoardFWInterface 
 * \param pbbi Reference to the BeBoardFWInterface
 */
			FpgaConfig(BeBoardFWInterface* pbbi);
/*! \brief Launch the firmware upload in a separate thread
 * \param numConfig FPGA configuration number
 * \param pstrFile absolute path to the MCS file
 */
			void runUpload(uint16_t numConfig, const char* pstrFile) throw (std::string);
/*! \brief Tells if a configuration is currently been uploaded
 * \return Configuration number or 0 if no upload is been processed
 */
			uint32_t getUploadingFpga() const {return numUploadingFpga;}
/*! \brief Percentage of the upload already done
 * \return 0 to 100 percentage value
 */
			uint32_t getProgressValue() const  {return progressValue;}
/*! \brief Description of the upload current status
 * \return String describing the upload status containing the block number and the estimated remaining time
 */
			const std::string& getProgressString() const {return progressString;}

/*! \brief Jump to an FPGA configuration
 * \param numConfig FPGA configuration number
 */
			void jumpToImage( uint16_t numImage);

		private:

			timeval timStart, timEnd;
			uint32_t progressValue, numUploadingFpga;
			std::string progressString;
			BeBoardFWInterface* fwManager;
    ///Sets the read mode as asynchronous.
			void confAsyncRead() throw (std::string);
	///Locks or unlocks a block of the flash (Xilinx DS617(v3.0.1) page 75, figure 43).
			void blockLockOrUnlock(uint32_t block_number, char operation) throw (std::string);
	///Erases a block of the flash (Xilinx DS617(v3.0.1) page 73, figure 41).
			void blockErase(uint32_t block_number) throw (std::string);
 	///Writes up to 32 words to the flash (Xilinx DS617(v3.0.1) page 71, figure 39).
			void bufferProgram(uint32_t block_number, uint32_t data_address, std::vector<uint32_t>& write_buffer, uint32_t words) throw (std::string);
/*! \brief Main uploading loop
 * \param bGolden true for the golden (first) configuration. The golden configuration will be used when the board is rebooted.
 * \param pstrFile Absolute path the MCS configuration file
 */
			void dumpFromFileIntoFlash(bool bGolden, const char* pstrFile) throw (std::string);
	};
}
#endif
