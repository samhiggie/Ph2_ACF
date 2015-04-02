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
        WireSetupVmMessages();
        WireLaunchButtons();
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

    void MainViewManager::WireSetupVmMessages()
    {
        connect(&m_setupVm, SIGNAL(enableAlltabs(bool)),
                &m_mainView, SLOT(enableAllTabsSlot(bool)));
    }

    void MainViewManager::WireLaunchButtons()
    {
        connect(&m_hybridTestVm, SIGNAL(startedHybridTest()),
                &m_calibrateVm, SIGNAL(disableLaunch()));
        connect(&m_hybridTestVm, SIGNAL(finishedHybridTest()),
                &m_calibrateVm, SIGNAL(enableLaunch()));
        connect(&m_hybridTestVm, SIGNAL(startedHybridTest()),
                &m_cmVm, SIGNAL(disableLaunch()));
        connect(&m_hybridTestVm, SIGNAL(finishedHybridTest()),
                &m_cmVm, SIGNAL(enableLaunch()));

        connect(&m_calibrateVm, SIGNAL(startedCalibration()),
                &m_hybridTestVm, SIGNAL(disableLaunch()));
        connect(&m_calibrateVm, SIGNAL(finishedCalibration()),
                &m_hybridTestVm, SIGNAL(enableLaunch()));
        connect(&m_calibrateVm, SIGNAL(startedCalibration()),
                &m_cmVm, SIGNAL(disableLaunch()));
        connect(&m_calibrateVm, SIGNAL(finishedCalibration()),
                &m_cmVm, SIGNAL(enableLaunch()));

        connect(&m_cmVm, SIGNAL(onCmTestStart()),
                &m_hybridTestVm, SIGNAL(disableLaunch()));
        connect(&m_cmVm, SIGNAL(onCmTestFinished()),
                &m_hybridTestVm, SIGNAL(enableLaunch()));
        connect(&m_cmVm, SIGNAL(onCmTestStart()),
                &m_calibrateVm, SIGNAL(disableLaunch()));
        connect(&m_cmVm, SIGNAL(onCmTestFinished()),
                &m_calibrateVm, SIGNAL(enableLaunch()));

    }
}
