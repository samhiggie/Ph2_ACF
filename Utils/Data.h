/*

    \file                          Data.h
    \brief                         Data handling from DAQ
    \author                        Nicolas PIERRE
    \version                       1.0
    \date                          10/07/14
    Support :                      mail to : nicolas.pierre@icloud.com

 */

#ifndef __DATA_H__
#define __DATA_H__

#include <uhal/uhal.hpp>
#include <ios>
#include <istream>
#include "../Utils/Event.h"
#include "../HWDescription/BeBoard.h"
#include "../HWDescription/Definition.h"


using namespace Ph2_HwDescription;
namespace Ph2_HwInterface
{

	/*!
	 * \class Data
	 * \brief Data buffer class for CBC data
	 */
	class Data
	{
	  private:
		char* fBuf;              /*! Data buffer <*/
		uint32_t fBufSize;           /*! Size of Data buffer <*/
		uint32_t fNevents;                 /*! Number of Events<*/
		Event* fEvent;                /*! Ptr. to Events container < */
		uint32_t fCurrentEvent;         /*! Current EventNumber in use <*/
		uint32_t fNCbc  ;       /*! Number of CBCs in the setup <*/
		uint32_t fEventSize  ;       /*! Size of 1 Event <*/


	  private:
		/*!
		 * \brief Byte swapping (not used)
		 * \param *org : input byte string
		 * \param *swapped : output byte string
		 * \param nbytes : lenght of byte string
		 */
		void swapByteOrder( const char* org, char* swapped, unsigned int nbyte );

	  public:
		/*!
		 * \brief Constructor of the Data class
		 * \param pNbCbc
		 */
		Data( ) : fBuf( nullptr ), fEvent( nullptr ), fCurrentEvent( 0 ), fEventSize( 0 ) {
		}
		/*!
		 * \brief Copy Constructor of the Data class
		 */
		Data( const Data& pData );
		/*!
		 * \brief Destructor of the Data class
		 */
		~Data() {
			if ( fBuf ) delete fBuf;
		}
		/*!
		 * \brief Set the data in the data map
		 * \param *pData : Data from the Cbc
		 * \param pNevents : The number of events in this acquisiton
		 */
		void Set( const std::vector<uint32_t>* pData, uint32_t pNevents );
		/*!
		 * \brief Reset the data structure
		 */
		void Reset();
		/*!
		 * \brief Copy the data buffer
		 * \param pData : Data to copy in
		 */
		void CopyBuffer( const Data& pData );
		/*!
		 * \brief Copy the data buffer
		 * \param pBufSize : size of the buffer
		 * \return Data buffer
		 */
		const char* GetBuffer( uint32_t& pBufSize ) const;
		/*!
		 * \brief Get the next Event
		 * \param pBoard: pointer to BeBoard
		 * \return Next Event
		 */
		const Event* GetNextEvent( const BeBoard* pBoard );

	};

}

#endif
