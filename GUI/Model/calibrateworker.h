#pragma once

#include <QObject>

namespace GUI{
    class CalibrateWorker : public QObject
    {
        Q_OBJECT
    public:
        explicit CalibrateWorker(QObject *parent);
        ~CalibrateWorker();

        void requestWork(bool calibFast, bool scan, bool bitwise, bool allCh);
    signals:
        void globalEnable(bool enable);
        void workRequested();
        void finished();

    public slots:
        void doWork();

    private:

        bool m_calibFast;
        bool m_scan;
        bool m_bitwise;
        bool m_allCh;

        explicit CalibrateWorker(const CalibrateWorker& rhs) = delete;
        CalibrateWorker& operator= (const CalibrateWorker& rhs) = delete;
    };

}
