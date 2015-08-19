#include "hybridtest.h"
#include <QDebug>
#include <QVector>
#include <QThread>

#include "Model/hybridtestworker.h"



namespace GUI
{
    HybridTest::HybridTest(QObject *parent) :
        QObject(parent),
        m_thread(new QThread()),
        m_worker(new HybridTestWorker(nullptr))
        //m_Vcth(0),
        //m_Events(0)
    {
        m_worker->moveToThread(m_thread);
        WireThreadConnections();
    }

    HybridTest::~HybridTest()
    {
        //m_worker->abort();
        m_thread->wait();
        delete m_thread;
        qDebug() << "Deleting HybridTest worker thread " <<this->QObject::thread()->currentThreadId();
        qDebug() << "Destructing " << this;
    }

    void HybridTest::WireThreadConnections()
    {
        connect(m_worker, SIGNAL(workRequested()),
                m_thread, SLOT(start()));

        connect(m_thread, SIGNAL(started()),
              m_worker, SLOT(doWork()));

         connect(m_worker, SIGNAL(finished()),
               this, SIGNAL(finishedHybridTest()));

        connect(m_worker, SIGNAL(finished()),
                m_thread, SLOT(quit()), Qt::DirectConnection);

        connect(m_worker, SIGNAL(globalEnable(bool)),
                this, SIGNAL(globalEnable(bool)));
    }

    void HybridTest::initialiseSettings()
    {
        emit getVcthValue();
        emit getEventsValue();
        emit getIsRegTestChecked();
        emit getIsScanChecked();
        emit getIsHoleModeChecked();

        emit startedHybridTest();

        //m_worker->abort();
        m_thread->wait();

        m_worker->requestWork(m_Vcth, m_Events, m_TestReg, m_ScanThreshold, m_HoleMode);
    }

    void HybridTest::setVcthValue(int cVcth)
    {
        m_Vcth = cVcth;
    }

    void HybridTest::setEventsValue(int cEvents)
    {
        m_Events = cEvents;
    }

    void HybridTest::setTestReg(const bool testReg)
    {
        m_TestReg = testReg;
    }

    void HybridTest::setScanThreshold(const bool scanThreshhold)
    {
        m_ScanThreshold = scanThreshhold;
    }

    void HybridTest::setHoleMode(const bool holeMode)
    {
        m_HoleMode = holeMode;
    }

}
