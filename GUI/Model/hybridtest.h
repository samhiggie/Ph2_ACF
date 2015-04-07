#pragma once
#include <QObject>

namespace GUI{

    class HybridTestWorker;

    class HybridTest : public QObject
    {
        Q_OBJECT
    public:
        explicit HybridTest(QObject *parent);

        void abort();

        ~HybridTest();

    signals:
        void globalEnable(bool enable);
        void getVcthValue();
        void getEventsValue();
        void getIsRegTestChecked();
        void getIsScanChecked();
        void getIsHoleModeChecked();

        void startedHybridTest();
        void finishedHybridTest();

    public slots:
        void initialiseSettings();
        void setVcthValue(int cVcth);
        void setEventsValue(int cEvents);
        void setTestReg(const bool testReg);
        void setScanThreshold(const bool scanThreshhold);
        void setHoleMode(const bool holeMode);

    private:

        int m_Vcth;
        int m_Events;
        bool m_TestReg;
        bool m_ScanThreshold;
        bool m_HoleMode;

        QThread *m_thread;
        HybridTestWorker *m_worker;

        void WireThreadConnections();

        explicit HybridTest(const HybridTest& rhs) = delete;
        HybridTest& operator= (const HybridTest& rhs) = delete;
    };
}



