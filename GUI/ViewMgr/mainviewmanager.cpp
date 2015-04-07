#include "mainviewmanager.h"
#include "View/mainview.h"
#include "ViewMgr/beboardregistersviewmanager.h"
#include "ViewMgr/cbcregviewmanager.h"
#include "ViewMgr/hybridtestviewmanager.h"
#include "ViewMgr/setuptabviewmanager.h"
#include "ViewMgr/calibrateviewmanager.h"
#include "ViewMgr/cmtestviewmanager.h"

#include <QDebug>

namespace GUI
{
    MainViewManager::MainViewManager(QObject *parent,
                                     MainView &mainView,
                                     SetupTabViewManager &setupVm,
                                     BeBoardRegistersViewManager &beVm,
                                     CbcRegViewManager &cbcVm,
                                     HybridTestViewManager &hybridVm,
                                     CalibrateViewManager &calibVm,
                                     CmTestViewManager &cmVm) :
        QObject(parent),
        m_mainView(mainView),
        m_setupVm(setupVm),
        m_beVm(beVm),
        m_cbcRegVm(cbcVm),
        m_hybridTestVm(hybridVm),
        m_calibrateVm(calibVm),
        m_cmVm(cmVm)
    {
        WireStartup();
        WireGlobalEnable();
        WireSetupVmMessages();
    }

    MainViewManager::~MainViewManager()
    {
        qDebug() << "Destructing " << this;
    }

    void MainViewManager::WireStartup()
    {

        connect(&m_setupVm, SIGNAL(onBtnLoadClicked()),
                &m_beVm, SIGNAL(onBtnLoadClicked()));
        
        connect(&m_setupVm, SIGNAL(sendSh(int)),
                &m_cbcRegVm, SIGNAL(sendSh(int)));
        connect(&m_setupVm, SIGNAL(sendSh(int)),
                &m_beVm, SIGNAL(sendSh(int)));

        connect(&m_setupVm, SIGNAL(sendBe(int,int)),
                &m_cbcRegVm, SIGNAL(sendBe(int,int)));
        connect(&m_setupVm, SIGNAL(sendBe(int,int)),
                &m_beVm, SIGNAL(sendBe(int,int)));

        connect(&m_setupVm, SIGNAL(sendFe(int,int,int)),
                &m_cbcRegVm, SIGNAL(sendFe(int,int,int)));

        connect(&m_setupVm, SIGNAL(sendCbc(int,int,int,int)),
                &m_cbcRegVm, SIGNAL(sendCbc(int,int,int,int)));
        
        connect(&m_setupVm, SIGNAL(sendInitialiseBeRegistersView()),
                &m_beVm, SIGNAL(initialiseBeRegView()));

        connect(&m_beVm, SIGNAL(finishedInitialiseBeBoardRegValues()),
                &m_cbcRegVm, SIGNAL(sendInitialiseRegistersView()));

        connect(&m_cbcRegVm, SIGNAL(notifyConfigFinished()),
                &m_setupVm, SIGNAL(onConfigFinished()));

    }

    void MainViewManager::WireGlobalEnable()
    {
        connect(&m_beVm, SIGNAL(sendGlobalEnable(bool)),
                this, SIGNAL(globalEnable(bool)));
        connect(&m_calibrateVm, SIGNAL(sendGlobalEnable(bool)),
                this, SIGNAL(globalEnable(bool)));
        connect(&m_cbcRegVm, SIGNAL(sendGlobalEnable(bool)),
                this, SIGNAL(globalEnable(bool)));
        connect(&m_cmVm, SIGNAL(sendGlobalEnable(bool)),
                this, SIGNAL(globalEnable(bool)));
        connect(&m_hybridTestVm, SIGNAL(sendGlobalEnable(bool)),
                this, SIGNAL(globalEnable(bool)));

        connect(this, SIGNAL(globalEnable(bool)),
                &m_beVm, SIGNAL(receiveGlobalEnable(bool)));
        connect(this, SIGNAL(globalEnable(bool)),
                &m_calibrateVm, SIGNAL(receiveGlobalEnable(bool)));
        connect(this, SIGNAL(globalEnable(bool)),
                &m_cbcRegVm, SIGNAL(receiveGlobalEnable(bool)));
        connect(this, SIGNAL(globalEnable(bool)),
                &m_cmVm, SIGNAL(receiveGlobalEnable(bool)));
        connect(this, SIGNAL(globalEnable(bool)),
                &m_hybridTestVm, SIGNAL(receiveGlobalEnable(bool)));
        connect(this, SIGNAL(globalEnable(bool)),
                &m_setupVm, SIGNAL(receiveGlobalEnable(bool)));
    }

    void MainViewManager::WireSetupVmMessages()
    {
        connect(&m_setupVm, SIGNAL(enableAlltabs(bool)),
                &m_mainView, SLOT(enableAllTabsSlot(bool)));
    }
}
