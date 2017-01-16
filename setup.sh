#!/bin/bash

export CBCDAQ=$HOME/CBCDAQ

#CACTUS
export CACTUSBIN=/opt/cactus/bin
export CACTUSLIB=/opt/cactus/lib
export CACTUSINCLUDE=/opt/cactus/include

# BOOST
export BOOST_LIB=/opt/cactus/lib
export BOOST_INCLUDE=/opt/cactus/include

#ROOT
#source /usr/local/bin/thisroot.sh

#Ph2_ACF
export BASE_DIR=$(pwd)

#External Plugins
export ANTENNADIR=$BASE_DIR/CMSPh2_AntennaDriver
export AMC13DIR=/opt/cactus/include/amc13
export USBINSTDIR=$BASE_DIR/../Ph2_USBInstDriver
export ZMQ_HEADER_PATH=/usr/include/zmq.hpp
#ANTENNA
export ANTENNALIB=$ANTENNADIR/lib
#HMP4040
export USBINSTLIB=$USBINSTDIR/lib


export PATH=$BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$USBINSTLIB:$ANTENNALIB:$BASE_DIR/RootWeb/lib:/opt/cactus/lib:$BASE_DIR/lib:${LD_LIBRARY_PATH}
