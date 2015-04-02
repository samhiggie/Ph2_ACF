#include "startup.h"
#include "View/setuptab.h"
#include "View/beboardregisterstab.h"
#include "View/cbcregisterstab.h"
#include "View/hybridtesttab.h"
#include "View/calibratetab.h"
#include "View/cmtesttab.h"

#include "View/mainview.h"

#include "Model/settings.h"
#include "Model/systemcontroller.h"
#include "Model/systemcontrollerworker.h"

#include "Model/beboardregisters.h"
#include "Model/cbcregisters.h"
#include "Model/hybridtest.h"
#include "Model/calibrate.h"
#include "Model/cmtest.h"

#include "ViewMgr/setuptabviewmanager.h"
#include "provider.h"
#include "ViewMgr/beboardregistersviewmanager.h"
#include "ViewMgr/cbcregviewmanager.h"
#include "ViewMgr/hybridtestviewmanager.h"
#include "ViewMgr/calibrateviewmanager.h"
#include "ViewMgr/mainviewmanager.h"
#include "ViewMgr/cmtestviewmanager.h"

#include "utils.h"

#include <QDebug>

namespace GUI
{
    Startup::Startup() :
        QObject(nullptr),

        m_setupTab(*new SetupTab(nullptr)), //nullptr - transferring ownership in mainview
        m_beTab(*new BeBoardRegistersTab(nullptr)),
        m_regTab(*new CbcRegistersTab(nullptr)),
        m_hybridTab(*new HybridTestTab(nullptr)),
        m_calibrateTab(*new CalibrateTab(nullptr)),
        m_cmTestTab(*new CmTestTab(nullptr)),

        m_mainView(*new MainView(nullptr,
                                 m_setupTab,
                                 m_beTab,
                                 m_regTab,
                                 m_hybridTab,
                                 m_calibrateTab,
                                 m_cmTestTab)),

        m_systemController(new SystemController(this,
                                                Provider::getSettingsAsSingleton())),


        m_beReg(new BeBoardRegisters(this,
                                          *m_systemController)),
        m_cbcReg(new CbcRegisters(this,
                                  *m_systemController)),
        m_hybridTest(new HybridTest(this)),
        m_calibrate(new Calibrate(this)),
        m_cm(new CmTest(this)),

        m_setupTabVm(*new SetupTabViewManager(this,
                                             m_setupTab,
                                             *m_systemController,
                                             Provider::getSettingsAsSingleton() )),
        m_beRegTabVm(*new BeBoardRegistersViewManager(this,
                                                      m_beTab,
                                                      *m_beReg)),
        m_cbcRegTabVm(*new CbcRegViewManager(this,
                                             m_regTab,
                                             *m_cbcReg)),
        m_hybridTabVm(*new HybridTestViewManager(this,
                                            m_hybridTab,
                                            *m_hybridTest)),
        m_calibrateTabVm(*new CalibrateViewManager(this,
                                                m_calibrateTab,
                                                *m_calibrate)),
        m_cmTabVm(*new CmTestViewManager(this,
                                         m_cmTestTab,
                                         *m_cm)),


        m_mainViewVm(new MainViewManager(this,
                                         m_mainView,
                                         m_setupTabVm,
                                         m_beRegTabVm,
                                         m_cbcRegTabVm,
                                         m_hybridTabVm,
                                         m_calibrateTabVm,
                                         m_cmTabVm))
    {
    }

    Startup::~Startup(){
        qDebug() << "Destructing " << this;
        delete &m_mainView;
    }

    void Startup::show() const{
        m_mainView.show();
    }
}
