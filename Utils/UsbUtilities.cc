/*

\file                          Utilities.h
\brief                         Some objects that might come in handy when using Ph2_USBInstDriver
\author                        Sarah Seif El Nasr-Storey
\version                       1.0
\date                          18/01/16
Support :                      mail to : Sarah.Storey@cern.ch

*/

#include "../Utils/UsbUtilities.h"

void HMP4040server_tmuxSession (std::string pInitScript, std::string pConfigFile, std::string pHostname, PortsInfo pPortsInfo, int pMeasureInterval_s  )
{
    char buffer[256];
    std::string currentDir = getcwd (buffer, sizeof (buffer) );
    std::string baseDirectory  = return_InstDriverHomeDirectory() + "/Ph2_USBInstDriver";

    // // create bash script to launch HMP4040 sessions
    sprintf (buffer, "%s/%s", baseDirectory.c_str(), pInitScript.c_str() );
    std::cout << BOLDBLUE << "Creating launch script : " << buffer << " ." << RESET << std::endl;
    ofstream starterScript ( buffer );
    starterScript << "#!/bin/bash" << std::endl;

    starterScript << "SESSION_NAME=HMP4040_Server" << std::endl <<  std::endl ;
    starterScript << "tmux list-session 2>&1 | grep -q \"^$SESSION_NAME\" || tmux new-session -s $SESSION_NAME -d" << std::endl;
    // send chdir via tmux session
    sprintf (buffer, "tmux send-keys -t $SESSION_NAME \"cd %s\" Enter", baseDirectory.c_str() );
    starterScript << buffer << std::endl << std::endl;
    // set-up environment for Ph2_USB_InstDriver
    starterScript << "tmux send-keys -t $SESSION_NAME \". ./setup.sh\" Enter" << std::endl;
    // launch HMP4040 server
    sprintf (buffer, "tmux send-keys -t $SESSION_NAME \"lvSupervisor -f %s -r %d -p %d -i %d -S\" Enter", pConfigFile.c_str(), pPortsInfo.first, pPortsInfo.second, pMeasureInterval_s ) ;
    starterScript << buffer << std::endl;
    starterScript.close();
}
void Ke2110server_tmuxSession (std::string pInitScript, std::string pConfigFile, std::string pHostname, PortsInfo pPortsInfo, int pMeasureInterval_s  )
{
    char buffer[256];
    std::string currentDir = getcwd (buffer, sizeof (buffer) );
    std::string baseDirectory  = return_InstDriverHomeDirectory() + "/Ph2_USBInstDriver";

    // // create bash script to launch HMP4040 sessions
    sprintf (buffer, "%s/%s", baseDirectory.c_str(), pInitScript.c_str() );
    std::cout << BOLDBLUE << "Creating launch script : " << buffer << " ." << RESET << std::endl;
    ofstream starterScript ( buffer );
    starterScript << "#!/bin/bash" << std::endl;

    starterScript << "SESSION_NAME=Ke2110_Server" << std::endl <<  std::endl ;
    starterScript << "tmux list-session 2>&1 | grep -q \"^$SESSION_NAME\" || tmux new-session -s $SESSION_NAME -d" << std::endl;
    // send chdir via tmux session
    sprintf (buffer, "tmux send-keys -t $SESSION_NAME \"cd %s\" Enter", baseDirectory.c_str() );
    starterScript << buffer << std::endl << std::endl;
    // set-up environment for Ph2_USB_InstDriver
    starterScript << "tmux send-keys -t $SESSION_NAME \". ./setup.sh\" Enter" << std::endl;
    // TODO - change to configure DMM supervisor
    //sprintf (buffer, "tmux send-keys -t $SESSION_NAME \"lvSupervisor -f %s -r %d -p %d -i %d -S\" Enter" , pConfigFile.c_str() , pPortsInfo.first, pPortsInfo.second , pMeasureInterval_s ) ;
    starterScript << buffer << std::endl;
    starterScript.close();
}

std::string return_InstDriverHomeDirectory()
{
    char buffer[256];
    std::string currentDir = getcwd (buffer, sizeof (buffer) );
    std::vector<std::string> directories ;
    tokenize ( currentDir, directories, "/");

    std::string homeDir = "";

    for ( unsigned int i = 0 ; i < directories.size() - 1 ; i++)
        homeDir += "/" + directories[i] ;

    return homeDir;
}

PortsInfo parse_ServerInfo ( std::string pInfo )
{
    int cZmqPortNumber, cHttpPortNumber ;

    //LOG (INFO) << "Retreived the following parameters from the info file: " << pInfo;
    //tokenize the cInfo string to recover the port numbers so the client can be smart enough to connect to the correct port!
    std::vector<std::string> cTokens;
    cTokens.clear();
    tokenize ( pInfo, cTokens,  " ");
    std::vector<int> cPorts;
    cPorts.clear();

    for ( auto token : cTokens )
    {
        //if ( std::stoi(token) )  cPorts.push_back ( boost::lexical_cast<int> (token) );
        // this instead of lexical_cast from boost
        try
        {
            cPorts.push_back ( std::stoi (token) );
        }
        catch ( const std::invalid_argument& ia )
        {
        }
    }

    // THttp port comes first, then ZMQ
    cZmqPortNumber = cPorts[0];
    cHttpPortNumber = cPorts[1];
    return std::pair<int, int> (cZmqPortNumber, cHttpPortNumber);
}

void query_Server ( std::string pConfigFile, std::string pInstrumentName, std::string pHostname, PortsInfo& pPortsInfo, int pMeasureInterval_s)
{
#if __ZMQ__
    std::string baseDirectory  = return_InstDriverHomeDirectory() + "/Ph2_USBInstDriver";
    InstrMaps cServerList = { { "HMP4040", "lvSupervisor"},  {"Ke2110", "dmmSupervisor"} };
    InstrMaps cLaunchScriptsList = { { "HMP4040", "start_HMP4040.sh"},  {"Ke2110", "start_Ke2110.sh"} };

    int cZmqPortNumber, cHttpPortNumber ;
    auto search = cServerList.find (pInstrumentName);

    if (search != cServerList.end() )
    {
        std::string cLockFileName = "/tmp/" + search->second + ".lock";
        AppLock* cLock = new AppLock (cLockFileName);

        // server already running - so lock file exists
        if (cLock->getLockDescriptor() < 0)
        {
            //retreive the info before quitting!
            std::string cInfo = cLock->get_info();
            std::cout << BOLDBLUE <<  "Port information from the " << cLockFileName << " lock file:\n\t" << cInfo << RESET << std::endl ;

            pPortsInfo = parse_ServerInfo ( cInfo );
        }
        else
        {
            auto cLaunchScript = cLaunchScriptsList.find (pInstrumentName);

            if (cLaunchScript != cLaunchScriptsList.end() )
            {
                std::cout << BOLDBLUE <<  pInstrumentName << " server not running .... nothing to query." << RESET << std::endl ;

                if ( cLaunchScript->first == "HMP4040")
                    HMP4040server_tmuxSession (cLaunchScript->second, pConfigFile, pHostname, pPortsInfo,  pMeasureInterval_s  );
                else if ( cLaunchScript->first == "Ke2110")
                    Ke2110server_tmuxSession (cLaunchScript->second, pConfigFile, pHostname, pPortsInfo,  pMeasureInterval_s  );

                char cmd[120];
                sprintf (cmd, ". %s/%s",  baseDirectory.c_str(), (cLaunchScript->second).c_str() );
                std::cout << BOLDBLUE << "Launching server using set-up script : " << cLaunchScript->second << " ." << RESET << std::endl ;
                system (cmd);
            }
        }

        delete cLock;
    }

#endif
}

int  launch_Server ( std::string pConfigFile, std::string pHostname, PortsInfo& pPortsInfo, int pMeasureInterval_s )
{
#if __ZMQ__
    char buffer[256];
    std::string currentDir = getcwd (buffer, sizeof (buffer) );
    std::string baseDirectory  = return_InstDriverHomeDirectory() + "/Ph2_USBInstDriver";

    std::string cHWfile = baseDirectory + "/settings/" + pConfigFile;

    // get device name from configuration file
    if (cHWfile.find (".xml") != std::string::npos)
    {
        //Get instrument name from XML file
        //std::cout << "Reading " << cHWfile << std::endl;
        pugi::xml_document cDoc;
        pugi::xml_parse_result cResult = cDoc.load_file (cHWfile.c_str() );
        std::string cInstrtName = std::string (cDoc.child ("InstrumentDescription").first_child() .name() );

        //Now server stuff  - is it open? on which ports?
        query_Server ( cHWfile, cInstrtName, "localhost", pPortsInfo, 2 );
        return 1;
    }
    else
        std::cout << BOLDRED << "Configuration file not found." << RESET << std::endl;

#endif
    return 0 ;
}
