#pragma once
#include "hybridtestworker.h"
#include <QThread>
#include <QDebug>
#include <QProcess>

namespace GUI {

    HybridTestWorker::HybridTestWorker(QObject *parent) :
        QObject(parent)
    {

    }

    HybridTestWorker::~HybridTestWorker()
    {
        qDebug() << "Destructing HybridTest Worker" << this;
    }

    void HybridTestWorker::doWork()
    {

        QProcess *process = new QProcess(this);
        QString cmd = QString("./bin/hybridtest --gui");

        cmd = cmd + " -v " + QString::number(m_Vcth);
        cmd = cmd + " -n " + QString::number(m_Events);

        cmd = m_HoleMode ? cmd + " --hm" : cmd;

        cmd = m_ScanThreshold ? cmd + " -s" : cmd;

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

    void HybridTestWorker::requestWork(int vcth,int events, bool testreg,bool scanthreshold, bool holemode)
    {
        m_Vcth = vcth;
        m_Events = events;
        m_TestReg = testreg;
        m_ScanThreshold = scanthreshold;
        m_HoleMode = holemode;
        emit workRequested();
    }
}

