#pragma once
#include <QObject>

namespace GUI{
    class HybridTestWorker : public QObject
    {
        Q_OBJECT
    public:
        explicit HybridTestWorker(QObject *parent);
        void requestWork(int vcth,
                         int events,
                         bool testreg,
                         bool scanthreshold,
                         bool holemode);
        ~HybridTestWorker();

    signals:
        void globalEnable(bool enable);
        void workRequested();
        void finished();

    public slots:
        void doWork();

    private:

        int m_Vcth;
        int m_Events;
        bool m_TestReg;
        bool m_ScanThreshold;
        bool m_HoleMode;

        explicit HybridTestWorker(const HybridTestWorker& rhs) = delete;
        HybridTestWorker& operator= (const HybridTestWorker& rhs) = delete;
    };
}
