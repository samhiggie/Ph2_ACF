#pragma once
#include <QObject>
#include <QThread>

namespace GUI{

    class HybridTestTab;
    class HybridTest;

    class HybridTestViewManager : public QObject
    {
        Q_OBJECT
    public:
        explicit HybridTestViewManager(QObject *parent,
                                     HybridTestTab& hybridTab,
                                     HybridTest &hybridTest);
    ~HybridTestViewManager();

    signals:
        void sendGlobalEnable(bool enable);
        void receiveGlobalEnable(bool enable);
        void startedHybridTest();
        void finishedHybridTest();

    private:

        HybridTestTab& m_hybridTestTab;
        HybridTest& m_hybridTest;

        void WireButtons();
        void WireExternalCalls();

        explicit HybridTestViewManager(const HybridTestViewManager& rhs) = delete;
        HybridTestViewManager& operator= (const HybridTestViewManager& rhs) = delete;
    };
}



