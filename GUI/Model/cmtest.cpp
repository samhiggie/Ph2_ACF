#include "cmtest.h"
#include <QDebug>
#include <QThread>

#include "Model/cmtestworker.h"

namespace GUI{
    CmTest::CmTest(QObject *parent) :
        QObject(parent),
        m_thread(new QThread()),
        m_worker(new CmTestWorker(nullptr))
    {
        m_worker->moveToThread(m_thread);
        WireThreadConnections();
    }

    CmTest::~CmTest()
    {
        m_thread->wait();
        delete m_thread;
        qDebug() << "Deleting CmTest worker thread " <<this->QObject::thread()->currentThreadId();
        qDebug() << "Destructing " << this;
    }

    void CmTest::WireThreadConnections()
    {
        connect(m_worker, SIGNAL(workRequested()),
                m_thread, SLOT(start()));
        connect(m_thread, SIGNAL(started()),
                m_worker, SLOT(doWork()));
        connect(m_worker, SIGNAL(finished()),
                this, SIGNAL(finishedCmTest()));
        connect(m_worker, SIGNAL(finished()),
                m_thread, SLOT(quit()), Qt::DirectConnection);

        connect(m_worker, SIGNAL(globalEnable(bool)),
                this, SIGNAL(globalEnable(bool)));
    }
    void CmTest::initialiseSettings()
    {
        emit getIsScan();

        emit startedCmTest();

        m_thread->wait();
        m_worker->requestWork(m_scanNoise);

    }

    void CmTest::setIsScan(bool scanNoise)
    {
        m_scanNoise = scanNoise;
    }
}
