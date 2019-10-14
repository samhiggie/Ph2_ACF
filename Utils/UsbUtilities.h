/*

    \file                          Utilities.h
    \brief                         Some objects that might come in handy when using Ph2_USBInstDriver
    \author                        Sarah Seif El Nasr-Storey
    \version                       1.0
    \date                          18/01/16
    Support :                      mail to : Sarah.Storey@cern.ch

 */

#ifndef __USBUTILITES_H__
#define __USBUTILITES_H__

#include <cstring>
#include <unistd.h>
#include <math.h>
#include <thread>
#include <chrono>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdint.h>
#include <ios>
#include <istream>
#include <limits>
//#include <boost/lexical_cast.hpp>
#include "/usr/include/pugixml.hpp"
#include "Utilities.h"
#include "ConsoleColor.h"
#include "easylogging++.h"

#include "Timer.h"
#include "../HWDescription/Definition.h"
#include <iostream>
#include <fstream>
#include <bitset>
#include <sys/stat.h>
#include <vector>
#include <sys/wait.h>

#ifdef __USBINST__
#include "AppLock.cc"
#endif



typedef std::map<std::string, std::string> InstrMaps ;
typedef std::map<std::string, int> InstrPorts ;
typedef std::pair<int, int> PortsInfo;

// some constants to make the code more legible
const PortsInfo defaultPorts{8081, 8082};

// Generic functions to get base directory
std::string return_InstDriverHomeDirectory();


// Generic functions to launch/query servers
bool InitializeMonitoring (std::string pHostname, std::string pDevName, PortsInfo& pPortsInfo, int pMonitorInterval = 5, std::string pLogFile = "");

// function returns port information <zmqPort,httpPort> from server that is running [ pInfo is the return value of AppLock::get_info() ]
PortsInfo parse_ServerInfo ( std::string pInfo );

// function checks if the server is alreadt running using the lock file, if not it launches the server in a tmux session
void query_Server ( std::string pConfigFile, std::string pInstrumentName, std::string pHostname, int pMeasureInterval_s, std::string pLogFile = "");

// launches server
int launch_HMPServer ( std::string pConfigFile, std::string pHostname, PortsInfo& pPortsInfo, int pMeasureInterval_s, std::string pLogFile = "./Current_log.txt" );
int  launch_DMMServer ( std::string pHostname, PortsInfo& pPortsInfo, int pMeasureInterval_s, std::string pLogFile = "./Temperature_log.txt");


// Device specific functions
void HMP4040server_tmuxSession (std::string pInitScript, std::string pConfigFile, std::string pHostname, PortsInfo pPortsInfo, int pMeasureInterval_s, std::string pLogFile = "./Current_log.txt" );
void Ke2110server_tmuxSession (std::string pInitScript, std::string pConfigFile, std::string pHostname, PortsInfo pPortsInfo, int pMeasureInterval_s, std::string pLogFile = "./Temperature_log.txt");

#endif
