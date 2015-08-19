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
        qRegisterMetaType<QMap<QString, int>>("QMap<QString, int>");
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
        connect(m_worker, SIGNAL(finished()),
                m_thread, SLOT(quit()), Qt::DirectConnection);

        connect(m_worker, SIGNAL(globalEnable(bool)),
                this, SIGNAL(globalEnable(bool)));

        connect(this, SIGNAL(getInitialBeRegValues()),
                m_worker, SLOT(getInitialBeBoardRegMap()));
        connect(this, SIGNAL(getBeRegValues()),
                m_worker, SLOT(getBeBoardRegMap()));

        connect(m_worker, SIGNAL(finishedInitialiseBeBoardRegValues()),
                this, SIGNAL(finishedInitialiseBeBoardRegValues()));

        connect(m_worker, SIGNAL(sendInitialBeBoardRegValues(int,int,std::map<std::string,uint32_t>)),
                this, SIGNAL(sendInitialBeBoardRegValues(int,int,std::map<std::string,uint32_t>)));
        connect(m_worker, SIGNAL(sendBeBoardRegValues(int,int,std::map<std::string,uint32_t>)),
                this, SIGNAL(sendBeBoardRegValues(int,int,std::map<std::string,uint32_t>)));

        connect(this, SIGNAL(writeBeRegisters(int,int,QMap<QString,int>)),
                m_worker, SLOT(writeBeRegisters(int,int,QMap<QString,int>)));

    }

}
