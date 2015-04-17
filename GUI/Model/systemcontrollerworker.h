#pragma once
#include <QObject>
#include <QMutex>
#include "Model/settings.h"
#include <QVariantMap>

#include "../tools/Tool.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

namespace GUI
{
    class SystemControllerWorker : public QObject, public Tool
    {
        Q_OBJECT

    public:
        explicit SystemControllerWorker(QObject *parent,
                                        Settings &config);
        ~SystemControllerWorker();

        void requestWork(std::string cHwFile);
        void requestConfigureHw();
        void abort();

    signals:
        void workRequested();
        void workConfigureHwRequested();
        void finishedInitialiseHw();
        void finishedConfigureHw();

    public slots:
        void onInitialiseHw();
        void onConfigureHw();

    private:

        std::string m_HwFile;

        bool _abort;
        bool _working;
        QMutex mutex;

        explicit SystemControllerWorker(const SystemControllerWorker& rhs) = delete;
        SystemControllerWorker& operator= (const SystemControllerWorker& rhs) = delete;
    };
}
