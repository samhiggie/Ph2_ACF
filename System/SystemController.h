/*!

        \file                    SystemController.h
        \brief                   Controller of the System, overall wrapper of the framework
        \author                  Nicolas PIERRE
        \version                 1.0
        \date                    10/08/14
        Support :                mail to : lorenzo.bidegain@cern.ch, nico.pierre@icloud.com

*/


#ifndef __SYSTEMCONTROLLER_H__
#define __SYSTEMCONTROLLER_H__

#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWInterface/BeBoardFWInterface.h"
#include "../HWInterface/GlibFWInterface.h"
#include "../HWInterface/ICGlibFWInterface.h"
#include "../HWInterface/CtaFWInterface.h"
#include "../HWInterface/ICFc7FWInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/Visitor.h"
#include "../Utils/Utilities.h"
#include "../Utils/picojson.h"
#include "../Utils/FileHandler.h"
#include "../Utils/pugixml.hpp"
#include "../Utils/ConsoleColor.h"
#include <iostream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <string.h>


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

/*!
 * \namespace Ph2_System
 * \brief Namespace regrouping the framework wrapper
 */
namespace Ph2_System {

    using BeBoardVec = std::vector<BeBoard*>;               /*!< Vector of Board pointers */
    using SettingsMap = std::map<std::string, uint32_t>;    /*!< Maps the settings */

    /*!
     * \class SystemController
     * \brief Create, initialise, configure a predefined HW structure
     */
    class SystemController
    {
      public:
        BeBoardInterface*       fBeBoardInterface;                     /*!< Interface to the BeBoard */
        CbcInterface*           fCbcInterface;                         /*!< Interface to the Cbc */
        BeBoardVec              fBoardVector;                          /*!< Vector of Board pointers */
        BeBoardFWMap            fBeBoardFWMap;
        SettingsMap             fSettingsMap;                          /*!< Maps the settings */
        FileHandler*            fFileHandler;

      public:
        /*!
         * \brief Constructor of the SystemController class
         */
        SystemController();
        /*!
         * \brief Destructor of the SystemController class
         */
        ~SystemController();
        /*!
         * \brief Method to construct a system controller object from another one while re-using the same members
         */
        //here all my members are set to the objects contained already in pController, I can then safely delete pController (because the destructor does not delete any of the objects)
        void Inherit(SystemController* pController)
        {
             fBeBoardInterface = pController->fBeBoardInterface;
             fCbcInterface = pController->fCbcInterface;
             fBoardVector = pController->fBoardVector;
             fBeBoardFWMap = pController->fBeBoardFWMap;
             fSettingsMap = pController->fSettingsMap;
             fFileHandler = pController->fFileHandler;
        }
        /*!
         * \brief Destroy the SystemController object: clear the HWDescription Objects, FWInterface etc.
         */
        void Destroy();
        /*!
        * \brief create a FileHandler object with
         * \param pFilename : the filename of the binary file
        */
        void addFileHandler ( const std::string& pFilename, char pOption );

        /*!
        * \brief read file in the a FileHandler object
         * \param pVec : the data vector
        */
        void readFile ( std::vector<uint32_t>& pVec, uint32_t pNWords32 = 0 );
        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        void accept ( HwDescriptionVisitor& pVisitor )
        {
            pVisitor.visit ( *this );

            for ( BeBoard* cBoard : fBoardVector )
                cBoard->accept ( pVisitor );
        }

        /*!
         * \brief Initialize the Hardware via a config file
         * \param pFilename : HW Description file
         *\param os : ostream to dump output
         */
        void InitializeHw ( const std::string& pFilename, std::ostream& os = std::cout );
        /*!
         * \brief Initialize the settings
         * \param pFilename :   settings file
         *\param os : ostream to dump output
        */
        void InitializeSettings ( const std::string& pFilename, std::ostream& os = std::cout );
        /*!
         * \brief Configure the Hardware with XML file indicated values
         */
        void ConfigureHw ( std::ostream& os = std::cout, bool bIgnoreI2c = false );
        /*!
         * \brief Run a DAQ
         * \param pBeBoard
         */
        void Run ( BeBoard* pBeBoard );

        /*!
         * \brief converts any char array to int by automatically detecting if it is hex or dec
         * \param pRegValue: parsed xml parmaeter char*
         * \return converted integer
         */
        uint32_t convertAnyInt ( const char* pRegValue )
        {
            if ( std::string ( pRegValue ).find ( "0x" ) != std::string::npos ) return static_cast<uint32_t> ( strtoul ( pRegValue , 0, 16 ) );
            else return static_cast<uint32_t> ( strtoul ( pRegValue , 0, 10 ) );

        }

        const BeBoard* getBoard (int index) const
        {
            return (index < (int)fBoardVector.size() ) ? fBoardVector.at (index) : nullptr;
        }
        /*!
         * \brief Get next event from data buffer
         * \param pBoard
         * \return Next event
         */
        const Event* GetNextEvent ( const BeBoard* pBoard )
        {
            return fBeBoardInterface->GetNextEvent ( pBoard );
        }
        const Event* GetEvent ( const BeBoard* pBoard, int i ) const
        {
            return fBeBoardInterface->GetEvent ( pBoard, i );
        }
        const std::vector<Event*>& GetEvents ( const BeBoard* pBoard ) const
        {
            return fBeBoardInterface->GetEvents ( pBoard );
        }

        /*!
         * \brief Initialize the hardware via  XML config file
         * \param pFilename : HW Description file
         *\param os : ostream to dump output
         */
        void parseHWxml ( const std::string& pFilename, std::ostream& os = std::cout );

      protected:
        /*!
         * \convert a voltage level to it's 8bit DAC value
         * \param pVoltage: the Voltage level
         * \return corresponding 8-bit DAC value
         */
        uint32_t Vto8Bit ( float pVoltage )
        {
            return static_cast<uint32_t> ( pVoltage / 3.3 * 256 + 0.5 );
        }

      private:
        /*!
         * \brief Initialize the hardware via JSON config file
         * \param pFilename : HW Description file
         *\param os : ostream to dump output
         */
        void parseSettingsxml ( const std::string& pFilename, std::ostream& os = std::cout);

        BeBoard* parseBeBoard (pugi::xml_node pNode, std::ostream& os = std::cout);
        void parseRegister (pugi::xml_node pNode, BeBoard* pBoard, std::ostream& os = std::cout);
        void parseCbc (pugi::xml_node pModuleNode, Module* pModule, std::ostream& os = std::cout);
        /*! \brief Expand environment variables in string
         * \param s input string
         * \return Result with variables expanded */
        std::string expandEnvironmentVariables ( std::string s ) ;
    };
}

#endif
