#pragma once

#include <QObject>


namespace GUI{

    class CmTestWorker;

    class CmTest : public QObject
    {
        Q_OBJECT
    public:
        explicit CmTest(QObject *parent);
        ~CmTest();

    signals:
        void globalEnable(bool enable);
        void getIsScan();
        void startedCmTest();
        void finishedCmTest();

    public slots:
        void initialiseSettings();
        void setIsScan(bool scanNoise);


    private:

        bool m_scanNoise;

        QThread *m_thread;
        CmTestWorker *m_worker;

        void WireThreadConnections();


        explicit CmTest(const CmTest& rhs) = delete;
        CmTest& operator= (const CmTest& rhs) = delete;
    };

}
