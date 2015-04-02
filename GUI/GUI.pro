#-------------------------------------------------
#
# Project created by QtCreator 2014-09-10T13:51:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Ph2_ACF
TEMPLATE = app

CONFIG += c++11 debug

QMAKE_CXXFLAGS += -g -O1 -w -pedantic -fPIC -std=c++11 -lrt `root-config --cflags --evelibs` -Wcpp

LIBS += -L../lib -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Tools -lPh2_Utils
LIBS += -L/opt/cactus/lib -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal
LibraryDirs = /opt/cactus/lib /opt/xdaq/lib ../
LibraryPaths = $(LibraryDirs:%=-L%)
LIBS += $(LibraryPaths:%=-L%) -uhal `root-config --glibs`

INCLUDEPATH += $(ROOTSYS)/include

INCLUDEPATH += /opt/cactus/include /../../Ph2DAQ_dev/

INCLUDEPATH += /../../Ph2DAQ_dev/

LIBS += -L/usr/local/boost/libs -lrt -lboost_system


SOURCES += main.cpp\
        View/mainview.cpp \
    startup.cpp \
    View/setuptab.cpp \
    ViewMgr/setuptabviewmanager.cpp \
    Model/settings.cpp \
    Model/systemcontroller.cpp \
    View/cbcregisterstab.cpp \
    ViewMgr/cbcregviewmanager.cpp \
    provider.cpp \
    View/aboutbox.cpp \
    Model/systemcontrollerworker.cpp \
    Model/cbcregisters.cpp \
    ViewMgr/mainviewmanager.cpp \
    Model/cbcregisterworker.cpp \
    Model/hybridtest.cpp \
    Model/hybridtestworker.cpp \
    ViewMgr/hybridtestviewmanager.cpp \
    View/hybridtesttab.cpp \
    View/calibratetab.cpp \
    ViewMgr/calibrateviewmanager.cpp \
    Model/calibrate.cpp \
    Model/calibrateworker.cpp \
    View/cmtesttab.cpp \
    ViewMgr/cmtestviewmanager.cpp \
    Model/cmtest.cpp \
    Model/cmtestworker.cpp \
    Model/beboardregisters.cpp \
    Model/beboardregistersworker.cpp \
    ViewMgr/beboardregistersviewmanager.cpp \
    View/beboardregisterstab.cpp


HEADERS  += View/mainview.h \
    startup.h \
    View/setuptab.h \
    ViewMgr/setuptabviewmanager.h \
    Model/settings.h \
    utils.h \
    Model/systemcontroller.h \
    View/cbcregisterstab.h \
    ViewMgr/cbcregviewmanager.h \
    provider.h \
    View/aboutbox.h \
    Model/systemcontrollerworker.h \
    Model/cbcregisters.h \
    ViewMgr/mainviewmanager.h \
    Model/cbcregisterworker.h \
    Model/hybridtest.h \
    Model/hybridtestworker.h \
    ViewMgr/hybridtestviewmanager.h \
    View/hybridtesttab.h \
    View/calibratetab.h \
    ViewMgr/calibrateviewmanager.h \
    Model/calibrate.h \
    Model/calibrateworker.h \
    View/cmtesttab.h \
    ViewMgr/cmtestviewmanager.h \
    Model/cmtest.h \
    Model/cmtestworker.h \
    Model/beboardregisters.h \
    Model/beboardregistersworker.h \
    ViewMgr/beboardregistersviewmanager.h \
    View/beboardregisterstab.h


FORMS    += View/mainview.ui \
    View/setuptab.ui \
    View/cbcregisterstab.ui \
    View/aboutbox.ui \
    View/hybridtesttab.ui \
    View/calibratetab.ui \
    View/cmtesttab.ui \
    View/beboardregisterstab.ui

OTHER_FILES +=

RESOURCES +=
