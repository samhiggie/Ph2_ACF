#include "cbcregisters.h"
#include "Model/systemcontroller.h"
#include "Model/cbcregisterworker.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/CbcRegItem.h"
#include <QDebug>
#include <QThread>

#include "Model/cbcregisterworker.h"

namespace GUI
{
    CbcRegisters::CbcRegisters(QObject *parent,
                               SystemController &sysCtrl) :
        QObject(parent),
        m_thread(new QThread()),
        m_worker(new CbcRegisterWorker(nullptr,
                                       sysCtrl))
    {
        qRegisterMetaType<std::map<std::string,CbcRegItem>>("std::map<std::string,CbcRegItem>");
        qRegisterMetaType<std::vector<std::pair<std::string,std::uint8_t> >>("std::vector<std::pair<std::string,std::uint8_t> >");
        m_worker->moveToThread(m_thread);
        WireThreadConnections();
        Initialise();
    }

    CbcRegisters::~CbcRegisters()
    {
        m_worker->abort();
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        qDebug() << "Deleting CbcRegister worker thread " <<this->QObject::thread()->currentThreadId();
        qDebug() << "Destructing " << this;
    }

    void CbcRegisters::WireThreadConnections()
    {
        connect(m_worker, SIGNAL(workRequested()),
                m_thread, SLOT(start()));
        connect(m_thread, SIGNAL(started()),
                m_worker, SLOT(doWork()));
        connect(m_worker, SIGNAL(finished()),
                m_thread, SLOT(quit()), Qt::DirectConnection);


        connect(this, SIGNAL(createInitialCbcRegistersMap()),
                m_worker, SLOT(getInitialCbcRegistersMap()));
        connect(m_worker, SIGNAL(sendInitialCbcRegisterValues(int,int,int,int,std::map<std::string,CbcRegItem>)),
                this, SIGNAL(sendInitialCbcRegisterValue(int,int,int,int,std::map<std::string,CbcRegItem>)));
        connect(m_worker, SIGNAL(finishedInitCbcReg()),
                this, SIGNAL(finishedInitCbcReg()));

        connect(this, SIGNAL(getCbcRegistersMap()),
                m_worker, SLOT(getCbcRegistersMap()));
        connect(this, SIGNAL(writeCbcRegisterValue(int,std::vector<std::pair<std::string,std::uint8_t> >)),
                m_worker, SLOT(writeCbcRegisters(int,std::vector<std::pair<std::string,std::uint8_t> >)));
        /*connect(m_worker, SIGNAL(sendCbcRegisterValue(int,std::map<std::string,CbcRegItem>)),
                this, SIGNAL(sendCbcRegisterValue(int,std::map<std::string,CbcRegItem>)));*/


    }

    void CbcRegisters::Initialise()
    {
        m_worker->abort();
        m_thread->wait();
        m_worker->requestWork();
    }
}
