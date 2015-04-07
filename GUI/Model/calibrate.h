#pragma once

#include <QObject>
#include "cmtestworker.h"

namespace GUI{

    class CalibrateWorker;

    class Calibrate : public QObject
    {
        Q_OBJECT
    public:
        explicit Calibrate(QObject *parent);
        ~Calibrate();

    signals:
        void getIsFastCalib();
        void getIsScan();
        void getIsBitwise();
        void getIsAllCh();
        void startedCalibration();
        void finishedCalibration();
        void globalEnable(bool enable);

    public slots:
        void initialiseSettings();
        void setFastCalibValue(bool fast);
        void setBitwise(bool bit);
        void setScanValue(bool scan);
        void setAllChValue(bool ch);

    private:

        bool m_calibFast;
        bool m_scan;
        bool m_bitwise;
        bool m_allCh;

        QThread *m_thread;
        CalibrateWorker *m_worker;

        void WireThreadConnections();

        explicit Calibrate(const Calibrate& rhs) = delete;
        Calibrate& operator= (const Calibrate& rhs) = delete;

    };
}
