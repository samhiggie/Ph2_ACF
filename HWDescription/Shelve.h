/*!

        \file                           Shelve.h
        \brief                          Shelve Description class, handles a vector of Board
        \author                         Lorenzo BIDEGAIN
        \version            1.0
        \date                           11/08/14
        Support :                       mail to : lorenzo.bidegain@gmail.com

 */

#ifndef Shelve_h__
#define Shelve_h__

#include "BeBoard.h"

#include "../Utils/Visitor.h"

/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */


namespace Ph2_HwDescription
{

	/*!
	 * \class Shelve
	 * \brief handles a vector of Board which are connected to the Shelve
	 */
	class Shelve
	{

	  public:

		//C'tor which takes the ShelveId
		Shelve( uint8_t pShelveId );

		// Default C'tor
		Shelve();

		// D'tor
		~Shelve() {
		        for ( auto& pBoard: fBoardVector )
				delete pBoard;
			fBoardVector.clear();
		}

		/*!
		 * \brief acceptor method for HwDescriptionVisitor
		 * \param pVisitor
		 */
		void accept( HwDescriptionVisitor& pVisitor ) {
			pVisitor.visit( *this );
			for ( auto& cBoard : fBoardVector )
				cBoard->accept( pVisitor );
		}
		// void accept( HwDescriptionVisitor& pVisitor )  const {
		//  pVisitor.visit( *this );
		//  for ( auto& cBoard : fBoardVector )
		//      cBoard.accept( pVisitor );
		// }
		/*!
		* \brief Get the number of Board connected to the Shelve
		* \return The size of the vector
		*/
		uint8_t getNBoard() const {
			return fBoardVector.size();
		}
		/*!
		 * \brief Adding a Board to the vector
		 * \param pBoard
		 */
		void addBoard( BeBoard& pBoard ) {
			fBoardVector.push_back( &pBoard );
		}
		void addBoard( BeBoard* pBoard ) {
			fBoardVector.push_back( pBoard );
		}
		/*!
		 * \brief Remove a Board from the vector
		 * \param pBeId
		 * \return a bool which indicate if the removing was successful
		 */
		bool   removeBoard( uint8_t pBeId );
		/*!
		 * \brief Get a Board from the vector
		 * \param pBeId
		 * \return a pointer of Board, so we can manipulate directly the Board contained in the vector
		 */
		BeBoard*   getBoard( uint8_t pBeId ) const;

		/*!
		* \brief Get the Shelve Id
		* \return The Shelve ID
		*/
		uint8_t getShelveId() const {
			return fShelveId;
		}
		/*!
		 * \brief Set the Shelve Id
		 * \param pShelveId
		 */
		void setShelveId( uint8_t pShelveId ) {
			fShelveId = pShelveId;
		};

		// Vector of Boards
		std::vector< BeBoard* > fBoardVector;

	  protected:

		uint8_t fShelveId;


	};
}

#endif
