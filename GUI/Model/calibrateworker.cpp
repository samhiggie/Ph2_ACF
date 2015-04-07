#pragma once

#include "calibrateworker.h"

#include <QThread>
#include <QDebug>
#include <QProcess>

namespace GUI{

    CalibrateWorker::CalibrateWorker(QObject *parent) :
        QObject(parent)
    {

    }

    CalibrateWorker::~CalibrateWorker()
    {
        qDebug() << "Destructing Calibrate Worker " << this;
    }

    void CalibrateWorker::doWork()
    {
        QProcess *process = new QProcess(this);
        QString cmd = QString("./bin/calibrate --gui");

        cmd = !m_calibFast ? cmd + " --old" : cmd;
        cmd = !m_scan ? cmd + " --skip" : cmd;
        cmd = m_bitwise ? cmd + " --bm" : cmd;
        cmd = m_allCh ? cmd + " -a" : cmd;

        qDebug() << cmd;

        process->start(cmd);

        QByteArray ba;

        if (process->waitForStarted(-1)) {
            while(process->waitForReadyRead(-1)) {
                ba += process->readAllStandardOutput();
            }
        }

        char *someData = ba.data();

        qDebug() << someData;
        emit globalEnable(true);
        emit finished();

    }

    void CalibrateWorker::requestWork(bool calibFast, bool scan, bool bitwise, bool allCh)
    {
        m_calibFast = calibFast;
        m_scan = scan;
        m_bitwise = bitwise;
        m_allCh = allCh;
        emit workRequested();
    }
}
