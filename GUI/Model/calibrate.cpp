#pragma once
#include "calibrate.h"
#include <QDebug>
#include <QThread>

#include "Model/calibrateworker.h"

namespace GUI {

    Calibrate::Calibrate(QObject *parent) :
        QObject(parent),
        m_thread(new QThread()),
        m_worker(new CalibrateWorker(nullptr))
    {
        m_worker->moveToThread(m_thread);
        WireThreadConnections();

    }

    Calibrate::~Calibrate()
    {
        m_thread->wait();
        delete m_thread;
        qDebug() << "Deleting Calibration worker thread " <<this->QObject::thread()->currentThreadId();
        qDebug() << "Destructing " << this;
    }

    void Calibrate::WireThreadConnections()
    {
        connect(m_worker, SIGNAL(workRequested()),
                m_thread, SLOT(start()));
        connect(m_thread, SIGNAL(started()),
                m_worker, SLOT(doWork()));
        connect(m_worker, SIGNAL(finished()),
                this, SIGNAL(finishedCalibration()));
        connect(m_worker, SIGNAL(finished()),
                m_thread, SLOT(quit()), Qt::DirectConnection);

        connect(m_worker, SIGNAL(globalEnable(bool)),
                this, SIGNAL(globalEnable(bool)));
    }

    void Calibrate::initialiseSettings()
    {
        emit getIsFastCalib();
        emit getIsScan();
        emit getIsBitwise();
        emit getIsAllCh();

        emit startedCalibration();

        m_thread->wait();
        m_worker->requestWork(m_calibFast, m_scan, m_bitwise, m_allCh);
    }

    void Calibrate::setFastCalibValue(bool fast)
    {
        m_calibFast = fast;
    }

    void Calibrate::setBitwise(bool bit)
    {
        m_bitwise = bit;
    }

    void Calibrate::setScanValue(bool scan)
    {
        m_scan = scan;
    }

    void Calibrate::setAllChValue(bool ch)
    {
        m_allCh = ch;
    }
}

