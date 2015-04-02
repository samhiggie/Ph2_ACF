#include "setuptabviewmanager.h"
#include "utils.h"
#include "View/setuptab.h"
#include "Model/systemcontroller.h"
#include "Model/settings.h"
#include <QDebug>


namespace GUI
{
    SetupTabViewManager::SetupTabViewManager(QObject *parent,
                                             SetupTab& tab,
                                             SystemController& sysCtrl,
                                             Settings& config) :
        QObject(parent),
        m_setupTab(tab),
        m_systemController(sysCtrl)
    {
        WireMessages(config);
        WireSetupTabButtons(config);
        WireCallToOtherTabs(config);
        WireCallFromOtherTabs();
    }

    SetupTabViewManager::~SetupTabViewManager()
    {
        qDebug() << "Destructing " << this;
    }

    void SetupTabViewManager::WireMessages(Settings& config)
    {
        connect(&config, SIGNAL(notifyStatusMessage(QString)),
                &m_setupTab, SLOT(onStatusUpdate(QString)) );

        connect(&m_systemController, SIGNAL(notifyStatusMessage(QString)),
                &m_setupTab, SLOT(onStatusUpdate(QString)));
    }

    void SetupTabViewManager::WireSetupTabButtons(Settings& config)
    {
        connect(&m_setupTab, SIGNAL(onBtnLoad()),
                this, SIGNAL(onBtnLoadClicked()));

        connect(&m_setupTab, SIGNAL(onBtnLoadSettingsClicked(bool)),
                &config, SLOT(onLoadButtonClicked(bool)) );

        connect(&m_setupTab, SIGNAL(onBtnCustomLoadSettingsClicked(QString)),
                &config, SLOT(onCustomLoadButtonClicked(QString)));

        connect(&config, SIGNAL(setHwTree(QStandardItemModel*)),
                &m_setupTab, SLOT(setHwTreeView(QStandardItemModel*)));

        connect(&m_setupTab, SIGNAL(onBtnInitClicked()),
                &m_systemController, SLOT(startInitialiseHw()));
        connect(&m_setupTab, SIGNAL(onBtnCfgClicked()),
                &m_systemController, SLOT(startConfigureHw()));

        connect(&m_systemController, SIGNAL(notifyInitFinished()),
                &m_setupTab, SLOT(onInitFinished()));
    }

    void SetupTabViewManager::WireCallToOtherTabs(Settings &config)
    {
        connect(&m_setupTab, SIGNAL(enableAllTabs(bool)),
                this, SIGNAL(enableAlltabs(bool)));
        connect(&m_setupTab, SIGNAL(on2CbcToggle(bool)),
                this, SIGNAL(on2CbcToggle(bool)));

        connect(&m_systemController, SIGNAL(sendInitialiseBeRegistersView()),
                this, SIGNAL(sendInitialiseBeRegistersView()));

        connect(&config, SIGNAL(sendSh(int)),
                this, SIGNAL(sendSh(int)));
        connect(&config, SIGNAL(sendBe(int,int)),
                this, SIGNAL(sendBe(int,int)));
        connect(&config, SIGNAL(sendFe(int,int,int)),
                this, SIGNAL(sendFe(int,int,int)));
        connect(&config, SIGNAL(sendCbc(int,int,int,int)),
                this, SIGNAL(sendCbc(int,int,int,int)));


    }

    void SetupTabViewManager::WireCallFromOtherTabs()
    {
        connect(this, SIGNAL(onConfigFinished()),
                &m_setupTab, SLOT(onConfigFinished()));
    }
}
