#include "hybridtestviewmanager.h"
#include "View/hybridtesttab.h"
#include "Model/hybridtest.h"
#include <QDebug>
#include <QThread>

namespace GUI
{
    HybridTestViewManager::HybridTestViewManager(QObject *parent,
                                             HybridTestTab& hybridTab,
                                             HybridTest& hybridTest) :
        QObject(parent),
        m_hybridTestTab(hybridTab),
        m_hybridTest(hybridTest)
    {
        WireButtons();
        WireExternalCalls();
    }

    HybridTestViewManager::~HybridTestViewManager()
    {
        qDebug() << "Destructing " << this;
    }

    void HybridTestViewManager::WireButtons()
    {
        connect(&m_hybridTestTab, SIGNAL(startHybridTest()),
                &m_hybridTest, SLOT(initialiseSettings()));

        connect(&m_hybridTest, SIGNAL(getVcthValue()),
                &m_hybridTestTab, SLOT(getVcthDialValue()));
        connect(&m_hybridTestTab, SIGNAL(sendVcthValue(int)),
                &m_hybridTest, SLOT(setVcthValue(int)));
        connect(&m_hybridTest, SIGNAL(getEventsValue()),
                &m_hybridTestTab, SLOT(getEventsDial()));
        connect(&m_hybridTestTab, SIGNAL(sendEventsNumber(int)),
                &m_hybridTest, SLOT(setEventsValue(int)));
        connect(&m_hybridTest, SIGNAL(getIsRegTestChecked()),
                &m_hybridTestTab, SLOT(getIsRegTestChecked()));
        connect(&m_hybridTest, SIGNAL(getIsScanChecked()),
                &m_hybridTestTab, SLOT(getIsScanChecked()));
        connect(&m_hybridTestTab, SIGNAL(sendIsRegTestChecked(bool)),
                &m_hybridTest, SLOT(setTestReg(bool)));
        connect(&m_hybridTestTab, SIGNAL(sendIsScanChecked(bool)),
                &m_hybridTest, SLOT(setScanThreshold(bool)));
        connect(&m_hybridTest, SIGNAL(getIsHoleModeChecked()),
                &m_hybridTestTab, SLOT(getIsHoleModeChecked()));
        connect(&m_hybridTestTab, SIGNAL(sendIsHoleModeChecked(bool)),
                &m_hybridTest, SLOT(setHoleMode(bool)));
    }

    void HybridTestViewManager::WireExternalCalls()
    {
        connect(&m_hybridTest, SIGNAL(startedHybridTest()),
                this, SIGNAL(startedHybridTest()));

        connect(&m_hybridTest, SIGNAL(globalEnable(bool)),
                this, SIGNAL(sendGlobalEnable(bool)));
        connect(&m_hybridTestTab, SIGNAL(globalEnable(bool)),
                this, SIGNAL(sendGlobalEnable(bool)));
        connect(this, SIGNAL(receiveGlobalEnable(bool)),
                &m_hybridTestTab, SLOT(enable(bool)));
    }
}
