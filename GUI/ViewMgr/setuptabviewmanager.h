#pragma once
#include <QObject>
#include "Model/systemcontroller.h"

//#include <QStandardItemModel>

namespace GUI{
    class SetupTab;
    class SystemController;
    class Settings;


    class SetupTabViewManager : public QObject
    {
        Q_OBJECT
    public:
        explicit SetupTabViewManager(QObject *parent,
                                     SetupTab& tab,
                                     SystemController& sysCtrl,
                                     Settings& config);
    ~SetupTabViewManager();

    signals:
        void notifyStatusUpdated(const QString& value);
        void onBtnLoadClicked();
        void sendSh(const int idSh);
        void sendBe(const int idSh, const int idBe);
        void sendFe(const int idSh, const int idBe, const int idFe);
        void sendCbc(const int idSh, const int idBe, const int idFe, const int idCbc);

        void enableAlltabs(const bool enable);
        void on2CbcToggle(const bool);
        void notifyConfigFinished();
        void sendInitialiseBeRegistersView();
        void sendInitialiseRegistersView();
        void onConfigFinished();

    private:

        SetupTab& m_setupTab;
        SystemController& m_systemController;

        void WireMessages(Settings &config);
        void WireSetupTabButtons(Settings &config);
        void WireCallToOtherTabs(Settings &config);
        void WireCallFromOtherTabs();

        explicit SetupTabViewManager(const SetupTabViewManager& rhs) = delete;
        SetupTabViewManager& operator= (const SetupTabViewManager& rhs) = delete;
    };
}



