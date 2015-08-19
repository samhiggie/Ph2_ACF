#include "calibrateviewmanager.h"
#include "View/calibratetab.h"
#include "Model/calibrate.h"

#include <QDebug>

namespace GUI
{
    CalibrateViewManager::CalibrateViewManager(QObject *parent,
                                               CalibrateTab& calibTab,
                                               Calibrate& calib):
        QObject(parent),
        m_calibrateTab(calibTab),
        m_calibrate(calib)
    {
        WireConnections();
        WireExternalConnections();
    }

    CalibrateViewManager::~CalibrateViewManager()
    {
        qDebug() << "Destructing " << this;
    }

    void CalibrateViewManager::WireConnections()
    {
        connect(&m_calibrateTab, SIGNAL(startCalibration()),
                &m_calibrate, SLOT(initialiseSettings()));
        connect(&m_calibrate, SIGNAL(getIsFastCalib()),
                &m_calibrateTab, SLOT(getIsFastCalib()));
        connect(&m_calibrateTab, SIGNAL(sendIsFastCalib(bool)),
                &m_calibrate, SLOT(setFastCalibValue(bool)));
        connect(&m_calibrate, SIGNAL(getIsScan()),
                &m_calibrateTab, SLOT(getIsScan()));
        connect(&m_calibrateTab, SIGNAL(sendIsScan(bool)),
                &m_calibrate, SLOT(setScanValue(bool)));
        connect(&m_calibrate, SIGNAL(getIsBitwise()),
                &m_calibrateTab, SLOT(getIsBitwise()));
        connect(&m_calibrateTab, SIGNAL(sendIsBitwise(bool)),
                &m_calibrate, SLOT(setBitwise(bool)));
        connect(&m_calibrate, SIGNAL(getIsAllCh()),
                &m_calibrateTab, SLOT(getIsAllCh()));
        connect(&m_calibrateTab, SIGNAL(sendIsAllCh(bool)),
                &m_calibrate, SLOT(setAllChValue(bool)));
    }

    void CalibrateViewManager::WireExternalConnections()
    {
        connect(&m_calibrate, SIGNAL(globalEnable(bool)),
                this, SIGNAL(sendGlobalEnable(bool)));
        connect(&m_calibrateTab, SIGNAL(globalEnable(bool)),
                this, SIGNAL(sendGlobalEnable(bool)));
        connect(this, SIGNAL(receiveGlobalEnable(bool)),
                &m_calibrateTab, SLOT(enable(bool)));

        connect(&m_calibrate, SIGNAL(startedCalibration()),
                this, SIGNAL(startedCalibration()));
        connect(&m_calibrate, SIGNAL(finishedCalibration()),
                this, SIGNAL(finishedCalibration()));
    }
}
