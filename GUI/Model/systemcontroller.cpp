#include "Model/systemcontroller.h"
#include "settings.h"
#include "Model/systemcontrollerworker.h"

#include <QThread>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>

#include"provider.h"

#include<QDebug>

#include <QVariantMap>
#include <QVector>

namespace GUI
{
    SystemController::SystemController(QObject *parent,
                                       Settings &config) :
        QObject(parent),
        m_Settings(config),
        m_thread(new QThread()),
        m_worker(new SystemControllerWorker(nullptr,
                                            config))
    {
        m_worker->moveToThread(m_thread);
        WireThreadConnections();
    }

    SystemController::~SystemController()
    {
        m_worker->abort();
        m_thread->quit(); //as thread is persistant
        m_thread->wait();
        delete m_thread;
        qDebug() << "Deleting SystemController worker thread " <<this->QObject::thread()->currentThreadId();
        qDebug() << "Destructing " << this;
    }

    void SystemController::WireThreadConnections()
    {
        connect(m_worker, SIGNAL(workRequested()),
                m_thread, SLOT(start()));
        connect(m_thread, SIGNAL(started()),
                m_worker, SLOT(onInitialiseHw()));
        connect(m_worker, SIGNAL(workConfigureHwRequested()),
                m_worker, SLOT(onConfigureHw()));
        connect(m_worker, SIGNAL(finishedInitialiseHw()),
                this, SLOT(finishInitialiseHw()));
        connect(m_worker, SIGNAL(finishedConfigureHw()),
                this, SLOT(finishConfigureHw()));
    }

    void SystemController::startInitialiseHw()
    {
        m_worker->abort();
        m_thread->wait();

        std::string cHwFile = (m_Settings.getHwFileName()).toStdString();

        m_worker->requestWork(cHwFile);
    }

    void SystemController::startConfigureHw()
    {
        m_worker->requestConfigureHw();
    }

    void SystemController::finishInitialiseHw()
    {
        SendStatusMessage(tr("Initialised Hw"));
        emit notifyInitFinished();
    }

    void SystemController::finishConfigureHw()
    {
        SendStatusMessage(tr("Hardware configured"));
        SendStatusMessage(tr("Initialising register values"));
        emit sendInitialiseBeRegistersView();
        //SendStatusMessage(tr("BE board values initialised"));
        //emit sendInitialiseRegistersView();
        //SendStatusMessage(tr("Initialising CBC register values"));
        //emit notifyConfigFinished();
        //SendStatusMessage(tr("Register CBC values initialised"));
    }

    void SystemController::SendStatusMessage(QString msg)
    {
        emit notifyStatusMessage(msg);
    }
}

