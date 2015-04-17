#pragma once
#include <QDebug>
#include <QThread>
#include "Model/systemcontrollerworker.h"
#include "../System/SystemController.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

namespace GUI
{

    SystemControllerWorker::SystemControllerWorker(QObject *parent,
                                                   Settings &config) :
        QObject(parent)
    {
        _working = false;
        _abort = false;
    }

    SystemControllerWorker::~SystemControllerWorker()
    {
        qDebug() << "Destructing SystemController Worker" << this;
    }

    void SystemControllerWorker::requestWork(std::string cHwFile)
    {
        m_HwFile = cHwFile;

        mutex.lock();
        _working = true;
        _abort = false;
        qDebug() << "Starting SystemController Worker on thread " << this;
        mutex.unlock();

        emit workRequested();
    }

    void SystemControllerWorker::requestConfigureHw()
    {
        mutex.lock();
        _working = true;
        _abort = false;
        mutex.unlock();

        emit workConfigureHwRequested();
    }

    void SystemControllerWorker::abort()
    {
        mutex.lock();
        if (_working) {
            _abort = true;
            qDebug()<<"SystemController worker aborting in Thread "<<thread()->currentThreadId();
        }
        mutex.unlock();
    }

    void SystemControllerWorker::onInitialiseHw()
    {
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if (abort) {
            return;
        }

        InitializeHw(m_HwFile);

        mutex.lock();
        _working = false;
        mutex.unlock();
        emit finishedInitialiseHw();
    }

    void SystemControllerWorker::onConfigureHw()
    {

        ConfigureHw();

        mutex.lock();
        _working = false;
        mutex.unlock();
        emit finishedConfigureHw();
    }

}
