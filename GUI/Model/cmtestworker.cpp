#pragma once
#include "cmtestworker.h"
#include <QThread>
#include <QDebug>
#include <QProcess>

namespace GUI{
    CmTestWorker::CmTestWorker(QObject *parent) :
        QObject(parent)
    {

    }

    CmTestWorker::~CmTestWorker()
    {
        qDebug() << "Destructing CmTest Worker " << this;
    }

    void CmTestWorker::doWork()
    {
        QProcess *process = new QProcess(this);
        QString cmd = QString("./bin/cmtest --gui");

        cmd = m_noiseScan ? cmd + " --scan" : cmd;

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

    void CmTestWorker::requestWork(bool noiseScan)
    {
        m_noiseScan = noiseScan;
        emit workRequested();
    }
}

