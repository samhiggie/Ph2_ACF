#pragma once

#include <QObject>

namespace GUI {
    class BeBoardRegistersTab;
    class BeBoardRegisters;

    class BeBoardRegistersViewManager : public QObject
    {
        Q_OBJECT
    public:
        explicit BeBoardRegistersViewManager(QObject *parent,
                                    BeBoardRegistersTab& beTab,
                                    BeBoardRegisters& beBoard);
        ~BeBoardRegistersViewManager();

    signals:
        void onBtnLoadClicked();
        void initialiseBeRegView();
        void finishedInitialiseBeBoardRegValues();
        void sendSh(const int idSh);
        void sendBe(const int idSh, const int idBe);
        void globalEnable(bool enable);

    private:

        BeBoardRegistersTab& m_beBoardTab;
        BeBoardRegisters& m_beBoard;

        void WireConnections();
        void WireExternalCalls();


        explicit BeBoardRegistersViewManager(const BeBoardRegistersViewManager& rhs) = delete;
        BeBoardRegistersViewManager& operator= (const BeBoardRegistersViewManager& rhs) = delete;
    };
}

