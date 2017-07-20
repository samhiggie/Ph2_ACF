# - Try to find Ph2_UsbInstLib
# Once done this will define
#
#  PH2_USBINSTLIB_FOUND - system has Ph2 USbInstLib
#  PH2_USBINSTLIB_INCLUDE_DIRS
#  PH2_USBINSTLIB_LIBRARIES 
#
#  Copyright (c) 2017 Georg Auzinger <georg.auzinger@SPAMNOTcern.ch>
#

if (PH2_USBINSTLIB_LIBRARIES AND PH2_USBINSTLIB_INCLUDE_DIRS)
  # in cache already
  set(PH2_USBINSTILB_FOUND TRUE)
else (PH2_USBINSTLIB_LIBRARIES AND PH2_USBINSTLIB_INCLUDE_DIRS)

  set(PH2_USBINSTLIB_DIR $ENV{USBINSTDIR})
    #find_path(PH2_USBINSTLIB_INCLUDE_DIR
    #NAMES
        #Utils/Utilities.h
        #Ke2110/
        #HMP4040/
        #Drivers/
        #ArdNano/
    #PATHS
        #$ENV{USBINSTDIR}
  #)
  #message("Ph2 USBInstLib Include Dirs: ${PH2_USBINSTLIB_INCLUDE_DIR}")

  find_library(PH2_USBINSTLIB_LIBRARY
    NAMES
      lib/Ph2_Drivers
      lib/Ph2_HMP4040
      lib/Ph2_Ke2110
      lib/Ph2_ArdNano
      lib/Ph2_USBUtils
    PATHS
      ${PH2_USBINSTLIB_DIR}
  )
  #message("Ph2 USBInstLib Library: ${PH2_USBINSTLIB_LIBRARY}")

  set(PH2_USBINSTLIB_INCLUDE_DIRS
      ${PH2_USBINSTLIB_DIR}/Utils
      ${PH2_USBINSTLIB_DIR}/Ke2110
      ${PH2_USBINSTLIB_DIR}/HMP4040
      ${PH2_USBINSTLIB_DIR}/Drivers
      ${PH2_USBINSTLIB_DIR}/ArdNano
  )

  set(PH2_USBINSTLIB_LIBRARIES
      ${PH2_USBINSTLIB_DIR}/lib/libPh2_Drivers.so
      ${PH2_USBINSTLIB_DIR}/lib/libPh2_Ke2110.so
      ${PH2_USBINSTLIB_DIR}/lib/libPh2_HMP4040.so
      ${PH2_USBINSTLIB_DIR}/lib/libPh2_ArdNano.so
      ${PH2_USBINSTLIB_DIR}/lib/libPh2_USBUtils.so
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(PH2_USBINSTLIB PH2_USBINSTLIB_LIBRARIES PH2_USBINSTLIB_INCLUDE_DIRS)

  # show the PH2_USBINSTLIB_INCLUDE_DIRS and PH2_USBINSTLIB_LIBRARIES variables only in the advanced view
  #mark_as_advanced(ZMQ_INCLUDE_DIRS ZMQ_LIBRARIES)

endif (PH2_USBINSTLIB_LIBRARIES AND PH2_USBINSTLIB_INCLUDE_DIRS)

