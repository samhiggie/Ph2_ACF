#include "FileParser.h"


namespace Ph2_System {

    void FileParser::parseHW ( const std::string& pFilename, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, std::ostream& os )
    {
        if ( pFilename.find ( ".xml" ) != std::string::npos )
            parseHWxml ( pFilename, pBeBoardFWMap, pBoardVector, os );
        else
            LOG (ERROR) << "Could not parse settings file " << pFilename << " - it is not .xml!" ;
    }

    void FileParser::parseSettings ( const std::string& pFilename, SettingsMap& pSettingsMap,  std::ostream& os)
    {
        if ( pFilename.find ( ".xml" ) != std::string::npos )
            parseSettingsxml ( pFilename, pSettingsMap, os );
        else
            LOG (ERROR) << "Could not parse settings file " << pFilename << " - it is not .xm!" ;
    }


    void FileParser::parseHWxml ( const std::string& pFilename, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, std::ostream& os )
    {
        pugi::xml_document doc;
        uint32_t cBeId, cModuleId, cCbcId;
        uint32_t cNBeBoard = 0;
        int i, j;

        pugi::xml_parse_result result = doc.load_file ( pFilename.c_str() );

        if ( !result )
        {
            os << "ERROR :\n Unable to open the file : " << pFilename << std::endl;
            os << "Error description : " << result.description() << std::endl;
            return;
        }

        os << "\n\n\n";

        for ( i = 0; i < 80; i++ )
            os << "*";

        os << "\n";

        for ( j = 0; j < 40; j++ )
            os << " ";

        os << BOLDRED << "HW SUMMARY: " << RESET << std::endl;

        for ( i = 0; i < 80; i++ )
            os << "*";

        os << "\n";
        os << "\n";
        const std::string strUhalConfig = expandEnvironmentVariables (doc.child ( "HwDescription" ).child ( "Connections" ).attribute ( "name" ).value() );

        // Iterate over the BeBoard Nodes
        for ( pugi::xml_node cBeBoardNode = doc.child ( "HwDescription" ).child ( "BeBoard" ); cBeBoardNode; cBeBoardNode = cBeBoardNode.next_sibling() )
        {
            BeBoard* cBeBoard = this->parseBeBoard (cBeBoardNode, pBoardVector, os);
            std::string cBoardType = cBeBoardNode.attribute ( "boardType" ).value();

            if (cBoardType == "GLIB") cBeBoard->setBoardType (BoardType::GLIB);
            else if (cBoardType == "CTA") cBeBoard->setBoardType (BoardType::CTA);
            else if (cBoardType == "ICGLIB") cBeBoard->setBoardType (BoardType::ICGLIB);
            else if (cBoardType == "ICFC7") cBeBoard->setBoardType (BoardType::ICFC7);
            else if (cBoardType == "CBC3FC7") cBeBoard->setBoardType (BoardType::CBC3FC7);
            else
            {
                LOG (ERROR) << "Error: Unknown Board Type: " << cBoardType << " - aborting!";
                exit (1);
            }

            pugi::xml_node cBeBoardConnectionNode = cBeBoardNode.child ("connection");

            std::string cId = cBeBoardConnectionNode.attribute ( "id" ).value();
            std::string cUri = cBeBoardConnectionNode.attribute ( "uri" ).value();
            std::string cAddressTable = expandEnvironmentVariables (cBeBoardConnectionNode.attribute ( "address_table" ).value() );

            if (!strUhalConfig.empty() )
                RegManager::setDummyXml (strUhalConfig);

            os << BOLDBLUE << "	" <<  "|"  << "----" << "Board Id: " << BOLDYELLOW << cId << BOLDBLUE << " URI: " << BOLDYELLOW << cUri << BOLDBLUE << " Address Table: " << BOLDYELLOW << cAddressTable << std::endl;
            os << BOLDBLUE << "|\t|----Type: " << BOLDYELLOW << cBoardType << RESET << std::endl << BLUE << "|\t|" << RESET << std::endl;

            //else LOG(INFO) << BOLDBLUE << "   " <<  "|"  << "----" << "Board Id: " << BOLDYELLOW << cId << BOLDBLUE << " Type: " << BOLDYELLOW << cBoardType << RESET ;

            // Iterate over the BeBoardRegister Nodes
            for ( pugi::xml_node cBeBoardRegNode = cBeBoardNode.child ( "Register" ); cBeBoardRegNode; cBeBoardRegNode = cBeBoardRegNode.next_sibling() )
            {
                if (std::string (cBeBoardRegNode.name() ) == "Register")
                {
                    std::string cNameString;
                    uint32_t cValue;
                    this->parseRegister (cBeBoardRegNode, cNameString, cValue, cBeBoard, os);
                }
            }

            os << BLUE <<  "|\t|" << RESET << std::endl;

            if (cBeBoard->getBoardType() == BoardType::GLIB)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new GlibFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
            else if (cBeBoard->getBoardType() == BoardType::ICGLIB)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new ICGlibFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
            else if (cBeBoard->getBoardType() == BoardType::CTA)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new CtaFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
            else if (cBeBoard->getBoardType() == BoardType::ICFC7)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new ICFc7FWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
            else if (cBeBoard->getBoardType() == BoardType::CBC3FC7)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new Cbc3Fc7FWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );

            //else
            //cBeBoardFWInterface = new OtherFWInterface();

            // Iterate the module node
            for ( pugi::xml_node cModuleNode = cBeBoardNode.child ( "Module" ); cModuleNode; cModuleNode = cModuleNode.next_sibling() )
            {
                if ( static_cast<std::string> ( cModuleNode.name() ) == "Module" )
                {
                    bool cStatus = cModuleNode.attribute ( "Status" ).as_bool();

                    //LOG(INFO) << cStatus ;
                    if ( cStatus )
                    {
                        os << BOLDCYAN << "|" << "	" << "|" << "----" << cModuleNode.name() << "  "
                           << cModuleNode.first_attribute().name() << " :" << cModuleNode.attribute ( "ModuleId" ).value() << RESET << std:: endl;

                        cModuleId = cModuleNode.attribute ( "ModuleId" ).as_int();

                        Module* cModule = new Module ( cBeId, cModuleNode.attribute ( "FMCId" ).as_int(), cModuleNode.attribute ( "FeId" ).as_int(), cModuleId );
                        cBeBoard->addModule ( cModule );

                        this->parseCbc (cModuleNode, cModule, os);
                    }
                }
            }

        }

        cNBeBoard++;

        os << "\n";
        os << "\n";

        for ( i = 0; i < 80; i++ )
            os << "*";

        os << "\n";

        for ( j = 0; j < 40; j++ )
            os << " ";

        os << BOLDRED << "END OF HW SUMMARY: " << RESET << std::endl;

        for ( i = 0; i < 80; i++ )
            os << "*";

        os << "\n";
        os << "\n";
    }

    BeBoard* FileParser::parseBeBoard (pugi::xml_node pNode, BeBoardVec& pBoardVector,  std::ostream& os)
    {
        uint32_t cBeId = pNode.attribute ( "Id" ).as_int();
        BeBoard* cBeBoard = new BeBoard ( cBeId );

        os << BOLDCYAN << "|" << "----" << pNode.name() << "  " << pNode.first_attribute().name() << " :" << pNode.attribute ( "Id" ).value() << RESET << std:: endl;

        pugi::xml_node cBeBoardFWVersionNode = pNode.child ( "FW_Version" );
        uint16_t cNCbcDataSize = 0;
        cNCbcDataSize = static_cast<uint16_t> ( cBeBoardFWVersionNode.attribute ( "NCbcDataSize" ).as_int() );

        if ( cNCbcDataSize != 0 ) os << BOLDCYAN << "|" << "	" << "|" << "----" << cBeBoardFWVersionNode.name() << " NCbcDataSize: " << cNCbcDataSize  <<  RESET << std:: endl;

        cBeBoard->setNCbcDataSize ( cNCbcDataSize );
        pBoardVector.push_back ( cBeBoard );
        return cBeBoard;
    }

    void FileParser::parseRegister (pugi::xml_node pNode, std::string& pAttributeString, uint32_t& pValue, BeBoard* pBoard, std::ostream& os)
    {
        if (std::string (pNode.name() ) == "Register")
        {
            if (std::string (pNode.first_child().value() ).empty() ) // the node has no value associated and thus is just a container
            {
                if (!pAttributeString.empty() ) pAttributeString += ".";

                pAttributeString += pNode.attribute ("name").value();

                //ok, I have appended .name to the attribute string, now loop over all the children
                for ( pugi::xml_node cNode = pNode.child ( "Register" ); cNode; cNode = cNode.next_sibling() )
                {
                    std::string cAttributeString = pAttributeString;
                    this->parseRegister (cNode, cAttributeString, pValue, pBoard, os);
                }
            }
            else // this node has a value and thus is the last down the hierarchy - so I am terminate the attribute string, get the value and thats it
            {
                if (!pAttributeString.empty() ) // this is the first node in the hierarchy and thus no children
                    pAttributeString += ".";

                pAttributeString += pNode.attribute ("name").value();
                pValue = convertAnyInt (pNode.first_child().value() );
                os << BLUE << "|" << "\t" << "|" << "----" << pNode.name() << "  " << pAttributeString << ": " << pValue << RESET << std:: endl;
                pBoard->setReg ( pAttributeString, pValue );
            }
        }

    }

    void FileParser::parseCbc (pugi::xml_node pModuleNode, Module* pModule, std::ostream& os )
    {
        pugi::xml_node cCbcPathPrefixNode = pModuleNode.child ( "CBC_Files" );
        std::string cFilePrefix = expandEnvironmentVariables (static_cast<std::string> ( cCbcPathPrefixNode.attribute ( "path" ).value() ) );

        if ( !cFilePrefix.empty() ) os << GREEN << "|" << "	" << "|" << "	" << "|" << "----" << "CBC Files Path : " << cFilePrefix << RESET << std::endl;

        // Iterate the CBC node
        for ( pugi::xml_node cCbcNode = pModuleNode.child ( "CBC" ); cCbcNode; cCbcNode = cCbcNode.next_sibling() )
        {
            os << BOLDCYAN << "|" << "	" << "|" << "	" << "|" << "----" << cCbcNode.name() << "  "
               << cCbcNode.first_attribute().name() << " :" << cCbcNode.attribute ( "Id" ).value()
               << ", File: " << expandEnvironmentVariables (cCbcNode.attribute ( "configfile" ).value() ) << RESET << std:: endl;


            std::string cFileName;

            if ( !cFilePrefix.empty() )
                cFileName = cFilePrefix + expandEnvironmentVariables (cCbcNode.attribute ( "configfile" ).value() );
            else cFileName = expandEnvironmentVariables (cCbcNode.attribute ( "configfile" ).value() );

            Cbc* cCbc = new Cbc ( pModule->getBeId(), pModuleNode.attribute ( "FMCId" ).as_int(), pModuleNode.attribute ( "FeId" ).as_int(), cCbcNode.attribute ( "Id" ).as_int(), cFileName );

            //here parse the specific CBC settings so that Registers take precedence

            for ( pugi::xml_node cCbcRegisterNode = cCbcNode.child ( "Register" ); cCbcRegisterNode; cCbcRegisterNode = cCbcRegisterNode.next_sibling() )
                cCbc->setReg ( std::string ( cCbcRegisterNode.attribute ( "name" ).value() ), atoi ( cCbcRegisterNode.first_child().value() ) );

            // here parese the GlobalCbcSettings so that Global CBC regisers take precedence over Global CBC settings which take precedence over CBC specific settings
            for ( pugi::xml_node cCbcGlobalNode = pModuleNode.child ( "Global_CBC_Register" ); cCbcGlobalNode != pModuleNode.child ( "CBC" ) && cCbcGlobalNode != pModuleNode.child ( "CBC_Files" ) && cCbcGlobalNode != nullptr; cCbcGlobalNode = cCbcGlobalNode.next_sibling() )
            {

                if ( cCbcGlobalNode != nullptr )
                {
                    std::string regname = std::string ( cCbcGlobalNode.attribute ( "name" ).value() );
                    uint32_t regvalue = convertAnyInt ( cCbcGlobalNode.first_child().value() ) ;
                    cCbc->setReg ( regname, uint8_t ( regvalue ) ) ;

                    os << GREEN << "|" << "	" << "|" << "	" << "|" << "----" << cCbcGlobalNode.name()
                       << "  " << cCbcGlobalNode.first_attribute().name() << " :"
                       << regname << " =  0x" << std::hex << std::setw ( 2 ) << std::setfill ( '0' ) << regvalue << std::dec << RESET << std:: endl;
                }
            }

            pModule->addCbc (cCbc);
        }
    }

    void FileParser::parseGlobalCbcSettings (pugi::xml_node pModuleNode, Module* pModule, std::ostream& os)
    {
        //use this to parse GlobalCBCRegisters and the Global CBC settings
        //i deliberately pass the Module object so I can loop the CBCs of the Module inside this method
        //this has to be called at the end of the parseCBC() method
        //Global_CBC_Register takes precedence over Global

    }

    void FileParser::parseCbcSettings (pugi::xml_node pCbcNode, Cbc* pCbc, std::ostream& os)
    {
        //parse the cbc settings here and put them in the corresponding registers of the Cbc object
        //call this for every CBC, Register nodes should take precedence over specific settings??
        ChipType cType = pCbc->getChipType();
        os << RED << "ChipType: ";

        if (cType == ChipType::CBC2)
            os << "CBC2";
        else
            os << "CBC3";

        os << RESET << std::endl;

        //THRESHOLD
        pugi::xml_node cThresholdNode = pCbcNode.child ( "Threshold" );
        uint16_t cThreshold = convertAnyInt (cThresholdNode.value() );

        //the moment the cbc object is constructed, it knows which chip type it is
        if (cType == ChipType::CBC2)
            pCbc->SetReg ("VCth", uint8_t (cThreshold) );
        else
        {
            pCbc->SetReg ("Vth1", (cThreshold & 0x00FF) );
            pCbc->SetReg ("Vth2", (cThreshold & 0x0300) >> 8);
        }

        os << "VCth: " << std::hex << "0x" << cThreshold << std::dec << " (" << cThreshold << ")" << std::endl;

        //TEST PULSE
        pugi::xml_node cTPNode = pCbcNode.child ("TestPulse");
        uint8_t cEnable, cPolarity, cGroundOthers;
        uint8_t cAmplitude, cChanGroup, cDelay;

        cEnable = static_cast<uint8_t> (cTPNode.attribute ("enable").as_int() );
        cPolarity = static_cast<uint8_t> (cTPNode.attribute ("polarity").as_int() );
        cAmplitude = static_cast<uint8_t> (cTPNode.attribute ("amplitude").as_int() );
        cChanGroup = static_cast<uint8_t> (cTPNode.attribute ("channel_group").as_int() );
        cDelay = static_cast<uint8_t> (cTPNode.attribute ("delay").as_int() ) ;
        cGroundOthers = static_cast<uint8_t> (cTPNode.attribute ("ground_others").as_int() );

        if (cType == ChipType::CBC2)
        {
            pCbc->SetReg ("TestPulsePot", cAmplitude );
            pCbc->SetReg ("SelTestPulseDel&ChanGroup", reverse_bits ( (cChanGroup & 0x07) << 5 | (cDelay & 0x1F) ) );
            uint8_t cAmuxValue = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux");
            pCbc->SetReg ("MiscTestPulseCtrl&AnalogMux", ( ( (cPolarity & 0x01) << 7) | ( (cEnable & 0x01) << 6) | ( (cGroundOhters & 0x01) << 5) | (cAmuxValue & 0x1F) ) );

        }
        else
        {
            pCbc->SetReg ("TestPulsePotNodeSel", cAmplitude );
            pCbc->SetReg ("TestPulseDel&ChanGroup", reverse_bits ( (cChanGroup & 0x07) << 5 | (cDelay & 0x1F) ) );
            uint8_t cAmuxValue = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux");
            pCbc->SetReg ("MiscTestPulseCtrl&AnalogMux", ( ( (cPolarity & 0x01) << 7) | ( (cEnable & 0x01) << 6) | ( (cGroundOhters & 0x01) << 5) | (cAmuxValue & 0x1F) ) );
        }

        os << "TestPulse: " << "enabled: " << cEnable << ", polarity: " << cPolarity << ", amplitude: 0x" << std::hex << cAmplitude << std::dec << ", channel group: " << cChanGroup << ", delay: " << cDelay << ", ground ohters: " << cGroundOthers std::endl;


        //CLUSTERS & STUBS
        pugi::xml_node cStubNode = pCbcNode.child ("Cluster&Stub");
        uint8_t cCluWidth, cPtWidth, cLayerswap, cOffset1, cOffset2, cOffset3, cOffset4;

        cCluWidth = static_cast<uint8_t> (cStubNode.attribute ("clusterwidth").as_int() );
        cPtWidth = static_cast<uint8_t> (cStubNode.attribute ("ptwidth").as_int() );
        cLayerswap = static_cast<uint8_t> (cStubNode.attribute ("layerswap").as_int() );
        cOffset1 = static_cast<uint8_t> (cStubNode.attribute ("off1").as_int() );
        cOffset2 = static_cast<uint8_t> (cStubNode.attribute ("off2").as_int() );
        cOffset3 = static_cast<uint8_t> (cStubNode.attribute ("off3").as_int() );
        cOffset4 = static_cast<uint8_t> (cStubNode.attribute ("off4").as_int() );

        if (cType == ChipType::CBC2)
        {
            pCbc->SetReg ("CwdWindow&Coincid", ( ( (cCluWidth & 0x03) << 6) | (cOffset1 & 0x3F) ) );
            uint8_t cMiscStubLogic = pCbc->getReg ("MiscStubLogic");
            pCbc->SetReg ("MiscStubLogic", ( (cPtWidth & 0x0F) << 4 | cMiscStubLogic & 0x0F) );

            os << "Cluster & Stub Logic: " << " ClusterWidthDiscrimination: " << cCluWidth << ", PtWidth: " << cPtWidth << ", Offset: " << cOffset << std::endl;
        }
        else
        {
            uint8_t cLogicSel = pCbc->getReg ("Pipe&StubInpSel&Ptwidth");
            pCbc->SetReg ("Pipe&StubInpSel&Ptwidth", ( (cLogicSel & 0xF0) | (cPtWidth & 0x0F) ) );
            pCbc->SetReg ("LayerSwap&CluWidth", ( ( (cLayerswap & 0x01) << 3) | cCluWidth & 0x07) );
            pCbc->SetReg ("CoincWind&Offset34", ( ( (cOffset4 & 0x0F) << 4) | cOffset3 & 0x0F) );
            pCbc->SetReg ("CoincWind&Offset12", ( ( (cOffset2 & 0x0F) << 4) | cOffset1 & 0x0F) );

            os << "Cluster & Stub Logic: " << "ClusterWidthDiscrimination: " << cCluWidth << ", PtWidth: " << cPtWidth << ", Layerswap: " << cLayerswap << ", Offset1: " << cOffset1 << ", Offset2: " << cOffset2 << ", Offset3: " << cOffset3 << ", Offset4: " << cOffset4 << std::endl;
        }

        //MISC
        pugi::xml_node cMiscNode = pCbcNode.child ("Misc");
        uint8_t cPipeLogic, cStubLogic, cOr254, cTpgClock, cDll;

        cPipeLogic = static_cast<uint8_t> (cMiscNode.attribute ("pipelogic").as_int() );
        cStubLogic = static_cast<uint8_t> (cMiscNode.attribute ("stublogic").as_int() );
        cOr254 = static_cast<uint8_t> (cMiscNode.attribute ("or254").as_int() );
        cTpgClock = static_cast<uint8_t> (cMiscNode.attribute ("tpgclock").as_int() );
        cDll = static_cast<uint8_t> (cMiscNode.attribute ("dll").as_int() );

        if (cType == ChipType::CBC2)
        {
            //uint8_t cLogic = pCbc->getReg("MiscStubLogic");
            os << "Curerntly not supported for CBC2, please set manually!" << std::endl;

        }
        else
        {
            pCbc->SetReg ("40MhzClk&Or254", ( ( (cTpgClock & 0x01) << 7) | ( (cOr254 & 0x01) << 6) | cDll & 0x1F) );
            cPtWidthRead = pCbc->getReg ("Pipe&StubInpSel&Ptwidt");
            pCbc->SetReg ("Pipe&StubInpSel&Ptwidt", ( ( (cPipeLogic & 0x03) << 6) | ( (cStubLogic & 0x03) >> 4) | cPtWidthRead & 0x0F) );

            os << "Misc Settings: " << " PipelineLogicSource (): " << cPipeLogic << ", StubLogicSource (): " << cStubLogic << ", OR254: " << cOr254 << ", TPG Clock: " << cTpgClock << ", DLL: " << cDll << std::endl;
        }


        // CHANNEL MASK
        pugi::xml_node cDisableNode = pCbcNode.child ("ChannelMask");
        std::string cList = std::string (cDisableNode.attribute ("disable").value() );
        std::string ctoken;
        std::stringstream cStr (cList);
        os << "List of disabled Channels: ";

        std::vector<uint8_t> cDisableVec;

        while (std::getline (cStr, ctoken, ',') )
        {
            cDisableVec.push_back (convertAnyInt ( ctoken.c_str() ) );
            os << ctoken << ", ";
        }

        os << std::endl;
        //TODO: mask channels in ChannelMaskRegisters


    }

    void FileParser::parseSettingsxml ( const std::string& pFilename, SettingsMap& pSettingsMap,  std::ostream& os)
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file ( pFilename.c_str() );

        if ( !result )
        {
            os << "ERROR :\n Unable to open the file : " << pFilename << std::endl;
            os << "Error description : " << result.description() << std::endl;
            return;
        }

        for ( pugi::xml_node nSettings = doc.child ( "Settings" ); nSettings; nSettings = nSettings.next_sibling() )
        {
            os << std::endl;

            for ( pugi::xml_node nSetting = nSettings.child ( "Setting" ); nSetting; nSetting = nSetting.next_sibling() )
            {
                pSettingsMap[nSetting.attribute ( "name" ).value()] = convertAnyInt ( nSetting.first_child().value() );
                os <<  RED << "Setting" << RESET << " --" << BOLDCYAN << nSetting.attribute ( "name" ).value() << RESET << ":" << BOLDYELLOW << convertAnyInt ( nSetting.first_child().value() ) << RESET << std:: endl;
            }
        }
    }

    std::string FileParser::expandEnvironmentVariables ( std::string s )
    {
        if ( s.find ( "${" ) == std::string::npos ) return s;

        std::string pre  = s.substr ( 0, s.find ( "${" ) );
        std::string post = s.substr ( s.find ( "${" ) + 2 );

        if ( post.find ( '}' ) == std::string::npos ) return s;

        std::string variable = post.substr ( 0, post.find ( '}' ) );
        std::string value    = "";

        post = post.substr ( post.find ( '}' ) + 1 );

        if ( getenv ( variable.c_str() ) != NULL ) value = std::string ( getenv ( variable.c_str() ) );

        return expandEnvironmentVariables ( pre + value + post );
    }
}
