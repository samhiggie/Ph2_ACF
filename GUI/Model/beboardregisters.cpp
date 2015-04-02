#pragma once
#include "beboardregisters.h"

#include <QDebug>
#include <QThread>

#include "Model/beboardregistersworker.h"

namespace GUI {

    BeBoardRegisters::BeBoardRegisters(QObject *parent,
                                       SystemController &sysCtrl) :
        QObject(parent),
        m_thread(new QThread()),
        m_worker(new BeBoardRegistersWorker(nullptr,
                                            sysCtrl))
    {
        qRegisterMetaType<std::map< std::string, uint32_t >>("std::map< std::string, uint32_t >");
        m_worker->moveToThread(m_thread);
        WireThreadConnections();
        Initialise();
    }

    BeBoardRegisters::~BeBoardRegisters()
    {
        //m_worker->abort();
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        qDebug() << "Deleting BeBoardRegister worker thread " <<this->QObject::thread()->currentThreadId();
        qDebug() << "Destructing " << this;
    }

    void BeBoardRegisters::Initialise()
    {
        //m_worker->abort();
        m_thread->wait();
        m_worker->requestWork();
    }

    void BeBoardRegisters::WireThreadConnections()
    {

        connect(m_worker, SIGNAL(workRequested()),
                m_thread, SLOT(start()));
        connect(m_thread, SIGNAL(started()),
                m_worker, SLOT(doWork()));
        connect(m_worker, SIGNAL(finished()),
                m_thread, SLOT(quit()), Qt::DirectConnection);

        connect(this, SIGNAL(getInitialBeRegValues()),
                m_worker, SLOT(getInitialBeBoardRegMap()));

        connect(m_worker, SIGNAL(finishedInitialiseBeBoardRegValues()),
                this, SIGNAL(finishedInitialiseBeBoardRegValues()));

        connect(m_worker, SIGNAL(sendInitialBeBoardRegValues(int,int,std::map<std::string,uint32_t>)),
                this, SIGNAL(sendInitialBeBoardRegValues(int,int,std::map<std::string,uint32_t>)));

    }

}
