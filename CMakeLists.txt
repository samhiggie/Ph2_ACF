cmake_minimum_required(VERSION 2.8)
project(Ph2_ACF)

#set the output directory
#set(CMAKE_BINARY_DIR $(PROJECT_SOURCE_DIR}/bin))
#set(CMAKE_BINARY_DIR $(PROJECT_SOURCE_DIR}/build))
#set(EXECUTABLE_OUTPUT_PATH $(CMAKE_BINARY_DIR))
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    #message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
#endif()

#set the cmakeL module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/" "/usr/share/root/cmake/")

set (CMAKE_CXX_FLAGS "-std=c++11 -Wcpp -pthread -pedantic -Wall -w -g -fPIC ${CMAKE_CXX_FLAGS}")

#check for external dependences
message("#### Checking for external Dependences ####")
#ROOT
find_package(ROOT COMPONENTS RHTTP)
if(${ROOT_FOUND})
    #check for THttpServer
    if(EXISTS ${ROOT_RHTTP_LIBRARY})
        message("-- Found THttp Server support - enabling compiler flags")
        set(ROOT_HAS_HTTP TRUE)
    elseif()
        message(" -- ROOT built without THttp Server - disabling compiler flags")
    endif()
endif()

if(EXISTS /opt/cactus/include/amc13)
    message("-- Found AMC13 Software installed on this machine - building the necessary components")
    set(AMC13INSTALLED TRUE)
else()
    message("-- AMC13 Software not installed on this machine -- continuing without")
endif()

if(EXISTS $ENV{ANTENNADIR})
    message("-- Found Antenna Driver installed on this machine - building the necessary components")
    set(ANTENNAINSTALLED TRUE)
else()
    message("-- Antenna Driver not installed on this machine -- continuing without")
endif()

#ZeroMQ optional
find_package(ZMQ)
if(ZMQ_FOUND)
    message("-- Found ZMQ Library - checking for Ph2_UsbInstLib")
    #libPh2_UsbInstLib
    find_package(PH2_USBINSTLIB)
    if(PH2_USBINSTLIB_FOUND)
        message("-- Found Ph2_UsbInstLib - enabling support")
    endif()
endif()

#Boost
if(EXISTS /opt/cactus/include/boost)
    set(UHAL_BOOST_FOUND TRUE)
    set(UHAL_BOOST_INCLUDE_DIRS /opt/cactus/include/boost)
    set(UHAL_BOOST_LIBRARY_DIRS /opt/cactus/lib)
    message("-- Found uHAL BOOST installation - using this version")
endif()

message("#### Done ####")

#all the subdirs
if(${AMC13INSTALLED})
    add_subdirectory(AMC13)
endif()
add_subdirectory(Utils)
add_subdirectory(HWDescription)
add_subdirectory(HWInterface)
add_subdirectory(System)
add_subdirectory(tools)
add_subdirectory(RootWeb)
#add_subdirectory(src)
#add_subdirectory(miniDAQ)

