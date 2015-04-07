#pragma once
#include <QObject>
#include <map>

#include "Model/systemcontroller.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

namespace GUI{

    class CbcRegisterWorker;

    class CbcRegisters : public QObject
    {
        Q_OBJECT
    public:
        explicit CbcRegisters(QObject *parent,
                              SystemController &sysCtrl);

        ~CbcRegisters();

    signals:
        void createInitialCbcRegistersMap();
        void sendInitialCbcRegisterValue(const int idBe,
                                  const int idFe,
                                  const int idFmc,
                                  const int idCbc,
                                  const std::map<std::string, CbcRegItem> mapReg);
        void sendCbcRegisterValues(const int idBe,
                                  const int idFe,
                                  const int idFmc,
                                  const int idCbc,
                                  const std::map<std::string, CbcRegItem> mapReg);
        void finishedInitCbcReg();
        void writeCbcRegisters(const int idSh, const int idBe, const int idFe, const int idCbc,
                               std::vector<std::pair<std::string, std::uint8_t>>);
        void getCbcRegistersMap();

    private:

        QThread *m_thread;
        CbcRegisterWorker *m_worker;

        void Initialise();

        void WireThreadConnections();

        explicit CbcRegisters(const CbcRegisters& rhs) = delete;
        CbcRegisters& operator= (const CbcRegisters& rhs) = delete;

    };
}



