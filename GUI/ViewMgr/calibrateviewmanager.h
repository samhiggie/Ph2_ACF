#pragma once

#include <QObject>

namespace GUI{
    class CalibrateTab;
    class Calibrate;

    class CalibrateViewManager : public QObject
    {
        Q_OBJECT
    public:
        explicit CalibrateViewManager(QObject *parent,
                                      CalibrateTab& calibTab,
                                      Calibrate& calib);
        ~CalibrateViewManager();

    signals:
        void startedCalibration();
        void finishedCalibration();
        void disableLaunch();
        void enableLaunch();
        void sendGlobalEnable(bool enable);
        void receiveGlobalEnable(bool enable);

    private:

        CalibrateTab& m_calibrateTab;
        Calibrate& m_calibrate;

        void WireConnections();
        void WireExternalConnections();

        explicit CalibrateViewManager(const CalibrateViewManager& rhs) = delete;
        CalibrateViewManager& operator= (const CalibrateViewManager& rhs) = delete;
    };
}
