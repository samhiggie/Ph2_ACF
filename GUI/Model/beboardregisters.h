#pragma once

#include <QObject>
#include <map>
#include <QMap>

namespace GUI {

    class BeBoardRegistersWorker;
    class SystemController;

    class BeBoardRegisters : public QObject
    {
        Q_OBJECT
    public:
        explicit BeBoardRegisters(QObject *parent,
                                  SystemController &sysCtrl);
        ~BeBoardRegisters();

    signals:
        void createInitialBeBoardMap();
        void getInitialBeRegValues();
        void getBeRegValues();
        void sendInitialBeBoardRegValues(const int idSh, const int idBe, const std::map< std::string, uint32_t >  cMap);
        void sendBeBoardRegValues(const int idSh, const int idBe, const std::map< std::string, uint32_t >  cMap);
        void finishedInitialiseBeBoardRegValues();
        void writeBeRegisters(const int idSh, const int idBe, QMap<QString, int>);
        void globalEnable(bool enable);

    private :

        QThread *m_thread;
        BeBoardRegistersWorker *m_worker;

        void Initialise();
        void WireThreadConnections();

        explicit BeBoardRegisters(const BeBoardRegisters& rhs) = delete;
        BeBoardRegisters& operator= (const BeBoardRegisters& rhs) = delete;
    };
}
