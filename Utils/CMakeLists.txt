#includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#find source files
file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cc)
#message("Sources " ${SOURCES})
#but withhold UsbUtilities for the moment until we know we have 
#Ph2_UsbInstLib installed
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/UsbUtilities.cc)
list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/UsbUtilities.h)
#message("Sources " ${SOURCES})

#add the library
add_library(Ph2_Utils SHARED ${SOURCES} ${HEADERS})

#check for ZMQ installed
if(ZMQ_FOUND)
    #here, now check for UsbInstLib
    if(PH2_USBINSTLIB_FOUND)
        #append the UsbUtilities files
        list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/UsbUtilities.cc)
        list(APPEND HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/UsbUtilities.h)

        #add include directoreis for ZMQ and USBINSTLIB
        include_directories(${PH2_USBINSTLIB_INCLUDE_DIRS})
        include_directories(${ZMQ_INCLUDE_DIRS})

        #and link against the libs
        set(LIBS ${LIBS} ${ZMQ_LIBRARIES} ${PH2_USBINSTLIB_LIBRARIES})

        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lPh2_Drivers -lPh2_HMP4040 -lPh2_Ke2110 -lPh2_USBUtils -lzmq -D__ZMQ__ -D__USBINST__")
        TARGET_LINK_LIBRARIES(Ph2_Utils ${LIBS})
    endif()
endif()
