#include "beboardregistersviewmanager.h"
#include "View/beboardregisterstab.h"
#include "Model/beboardregisters.h"

#include <QDebug>

namespace GUI {

    BeBoardRegistersViewManager::BeBoardRegistersViewManager(QObject *parent,
                                                             BeBoardRegistersTab &beTab,
                                                             BeBoardRegisters &beBoard):
        QObject(parent),
        m_beBoardTab(beTab),
        m_beBoard(beBoard)
    {
        WireConnections();
        WireExternalCalls();
    }

    BeBoardRegistersViewManager::~BeBoardRegistersViewManager()
    {
        qDebug() << "Destructing " << this;
    }

    void BeBoardRegistersViewManager::WireConnections()
    {
        connect(&m_beBoard, SIGNAL(sendInitialBeBoardRegValues(int,int,std::map<std::string,uint32_t>)),
                &m_beBoardTab, SLOT(createBeBoardRegisterValues(int,int,std::map<std::string,uint32_t>)));
        connect(&m_beBoard, SIGNAL(sendBeBoardRegValues(int,int,std::map<std::string,uint32_t>)),
                &m_beBoardTab, SLOT(refreshBeBoardRegisterValues(int,int,std::map<std::string,uint32_t>)));

        connect(&m_beBoardTab, SIGNAL(refreshBeValues()),
                &m_beBoard, SIGNAL(getBeRegValues()));
        connect(&m_beBoardTab, SIGNAL(writeBeRegisters(int,int,QMap<QString,int>)),
                &m_beBoard, SIGNAL(writeBeRegisters(int,int,QMap<QString,int>)));
    }

    void BeBoardRegistersViewManager::WireExternalCalls()
    {
        connect(this, SIGNAL(initialiseBeRegView()),
                &m_beBoard, SIGNAL(getInitialBeRegValues()));

        connect(this, SIGNAL(onBtnLoadClicked()),
                &m_beBoardTab, SLOT(reset()));

        connect(this, SIGNAL(sendSh(int)),
                &m_beBoardTab, SLOT(setupSh(int)));
        connect(this, SIGNAL(sendBe(int,int)),
                &m_beBoardTab, SLOT(setupBe(int,int)));

        connect(&m_beBoard, SIGNAL(globalEnable(bool)),
                this, SIGNAL(sendGlobalEnable(bool)));
        connect(&m_beBoardTab, SIGNAL(globalEnable(bool)),
                this, SIGNAL(sendGlobalEnable(bool)));

        connect(this, SIGNAL(receiveGlobalEnable(bool)),
                &m_beBoardTab, SLOT(enable(bool)));

        connect(&m_beBoard, SIGNAL(finishedInitialiseBeBoardRegValues()),
                this, SIGNAL(finishedInitialiseBeBoardRegValues()));

    }
}

