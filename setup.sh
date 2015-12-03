#!/bin/bash

#CACTUS
export CACTUSBIN=/opt/cactus/bin
export CACTUSLIB=/opt/cactus/lib
export CACTUSINCLUDE=/opt/cactus/include

# BOOST
export BOOST_LIB=/opt/cactus/lib
export BOOST_INCLUDE=/opt/cactus/include

# QT if installed
export QTDIR=/usr/local/Trolltech/Qt-4.8.5
export QMAKESPEC=
export QTROOTSYSDIR=/usr/local/qtRoot/root

#ROOT
source /usr/local/bin/thisroot.sh

#Ph2_ACF
export BASE_DIR=$(pwd)

#ANTENNA
export ANTENNALIB=../CMSPh2_AntennaDriver/lib
export ANTENNAINCLUDE=../CMSPh2_AntennaDriver
export PATH=$QTDIR/bin:$BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$BASE_DIR/RootWeb/lib:/opt/cactus/lib:$QTROOTSYSDIR/lib:$QTDIR/lib:$BASE_DIR/lib:$ANTENNALIB:${LD_LIBRARY_PATH}
