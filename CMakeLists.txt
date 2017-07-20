cmake_minimum_required(VERSION 2.8)
project(Ph2_ACF)

#set the output directory
#set(CMAKE_BINARY_DIR $(PROJECT_SOURCE_DIR}/bin))
#set(EXECUTABLE_OUTPUT_PATH $(CMAKE_BINARY_DIR))
#set(LIBRARY_OUTPUT_PATH $(PROJECT_SOURCE_DIR/lib))

#set the cmakeL module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set (CMAKE_CXX_FLAGS "-std=c++11 -Wcpp -pedantic -Wall -w -g -fPIC ${CMAKE_CXX_FLAGS}")

#check for external dependences

#ZeroMQ optional
find_package(ZMQ)
#libPh2_UsbInstLib
find_package(PH2_USBINSTLIB)

#all the subdirs
add_subdirectory(Utils)
#add_subdirectory(HWDescription)
#add_subdirectory(HWInterface)
#add_subdirectory(System)
#add_subdirectory(tools)
#add_subdirectory(RootWeb)
#add_subdirectory(src)
#add_subdirectory(miniDAQ)

