#pragma once

#include <QObject>

namespace GUI{
    class CmTestWorker : public QObject
    {
        Q_OBJECT
    public:
        explicit CmTestWorker(QObject *parent);
        void requestWork(bool scanNoise);
        ~CmTestWorker();

    signals:
        void globalEnable(bool enable);
        void workRequested();
        void finished();

    public slots:
        void doWork();

    private:

        bool m_noiseScan;
        explicit CmTestWorker(const CmTestWorker& rhs) = delete;
        CmTestWorker& operator= (const CmTestWorker& rhs) = delete;

    };
}

