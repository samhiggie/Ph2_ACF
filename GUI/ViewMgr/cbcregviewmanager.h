#pragma once
#include <QObject>

namespace GUI{

    class CbcRegistersTab;
    class CbcRegisters;

    class CbcRegViewManager : public QObject
    {
        Q_OBJECT
    public:
        explicit CbcRegViewManager(QObject *parent,
                                   CbcRegistersTab& cbcRegTab,
                                   CbcRegisters& cbcReg);
    ~CbcRegViewManager();

    signals:

        void sendGlobalEnable(bool enable);
        void receiveGlobalEnable(bool enable);

        void sendSh(int idSh);
        void sendBe(const int idSh, const int idBe);
        void sendFe(const int idSh, const int idBe, const int idFe);
        void sendCbc(const int idSh, const int idBe, const int idFe, const int idCbc);

        void notifyConfigFinished();
        void sendInitialiseRegistersView();

    private:
        void WireConnections();
        void WireExternalConnections();

        CbcRegistersTab& m_cbcRegistersTab;
        CbcRegisters& m_cbcRegisters;
        explicit CbcRegViewManager(const CbcRegViewManager& rhs) = delete;
        CbcRegViewManager& operator= (const CbcRegViewManager& rhs) = delete;
    };
}



