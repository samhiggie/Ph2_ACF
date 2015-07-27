#!/bin/bash
export QTDIR=/usr/local/Trolltech/Qt-4.8.5
export QMAKESPEC=
# export ROOTSYS=/usr/local/qtRoot/root
export QTROOTSYSDIR=/usr/local/qtRoot/root

export BOOST_INCLUDE=/opt/cactus/include
export BOOST_LIB=/opt/cactus/lib
export PATH=$QTDIR/bin:~/Ph2_ACF/bin:$PATH
export LD_LIBRARY_PATH=/opt/cactus/lib:$QTROOTSYSDIR/lib:$QTDIR/lib:~/Ph2_ACF/lib:${LD_LIBRARY_PATH}
