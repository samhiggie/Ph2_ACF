
#includes
include_directories($ENV{CACTUSINCLUDE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/HWDescription)
include_directories(${PROJECT_SOURCE_DIR}/HWInterface)
include_directories(${PROJECT_SOURCE_DIR}/Utils)
include_directories(${PROJECT_SOURCE_DIR}/System)
include_directories(${PROJECT_SOURCE_DIR})

#library dirs
link_directories(/opt/cactus/lib)

#find source files
file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cc)

#add the library
add_library(Ph2_Tools SHARED ${SOURCES} ${HEADERS})

#check for ZMQ installed
if(ZMQ_FOUND)
    #here, now check for UsbInstLib
    if(PH2_USBINSTLIB_FOUND)

        #add include directoreis for ZMQ and USBINSTLIB
        include_directories(${PH2_USBINSTLIB_INCLUDE_DIRS})
        include_directories(${ZMQ_INCLUDE_DIRS})

        #and link against the libs
        set(LIBS ${LIBS} ${ZMQ_LIBRARIES} ${PH2_USBINSTLIB_LIBRARIES})

        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ZMQ__ -D__USBINST__")
        #TARGET_LINK_LIBRARIES(Ph2_Tools ${LIBS})
    endif()
endif()

#check for AMC13 libraries
if(${AMC13INSTALLED})
    include_directories(${PROJECT_SOURCE_DIR}/AMC13)
    include_directories(/opt/cactus/include/amc13)
    link_directories(/opt/cactus/lib)
    set(LIBS ${LIBS} cactus_amc13_amc13 Ph2_Amc13)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__AMC13__")
endif()

#check for AntennaDriver
if(${ANTENNAINSTALLED})
    include_directories($ENV(ANTENNADIR))
    link_directories($ENV{ANTENNADIR}/lib)
    set(LIBS ${LIBS} usb Ph2_Antenna)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ANTENNA__")
endif()

#last but not least, find root and link against it
if(${ROOT_FOUND})
    include_directories(${ROOT_INCLUDE_DIRS})
    #link_directories(${ROOT_LIBRARIES})
    set(LIBS ${LIBS} ${ROOT_LIBRARIES})
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_DEFINITIONS}")

    #check for THttpServer
    if(${ROOT_HAS_HTTP})
        #link_directories(${ROOT_RHTTP_LIBRARY})
        set(LIBS ${LIBS} ${ROOT_RHTTP_LIBRARY})
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__HTTP__")
    endif()
endif()

set(LIBS ${LIBS} Ph2_Description Ph2_Interface Ph2_Utils Ph2_System)
TARGET_LINK_LIBRARIES(Ph2_Tools ${LIBS})
